#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Compare ADC response in events without an accepted track, clean tracks, and
tracks with one isolated extra strip cluster.

Track acceptance comes exclusively from a corresponding Tracking.run .tl file.
Only the selected events are decoded in detail; EBE and track IDs are joined in
streaming order. A strict X/Y geometrical match defines the track subsets.

The per-chamber ADC is not a charge measurement for any individual strip.
The single-strip category is a selection on spatial occupancy, not a particle ID.
"""

import argparse
import csv
import glob
import json
import math
import statistics
import sys
import time
from collections import Counter, defaultdict
from datetime import datetime
from pathlib import Path

import HitTopologyAnalysis as ht

CLASSES = ("no_accepted_track", "clean_track", "clean_plus_single_strip", "other_accepted_track")


def adc_configuration(db_path, nch):
    n_adc = None
    mapping = None
    pedestals = []
    scales = []
    for line in Path(db_path).read_text(encoding="utf-8", errors="replace").splitlines():
        t = line.split()
        if not t or t[0].startswith("#"):
            continue
        if t[0] == "NAdcs":
            n_adc = int(t[1])
            if "Assign" not in t:
                raise ValueError("NAdcs is missing Assign")
            i = t.index("Assign") + 1
            mapping = list(map(int, t[i:i+n_adc]))
        if t[0] == "ChamberBasics":
            pedestals.append(float(t[t.index("AdcPedestal")+1]) if "AdcPedestal" in t else 0.)
            scales.append(float(t[t.index("AdcScale")+1]) if "AdcScale" in t else 1.)
    if n_adc is None or mapping is None or len(mapping) != n_adc or len(pedestals) != nch:
        raise ValueError("Incomplete ADC mapping or chamber pedestal information")
    if any(ch >= nch or ch < -1 for ch in mapping):
        raise ValueError("ADC channel mapping outside detector")
    return {"n_adc": n_adc, "mapping": mapping, "ped": pedestals, "scale": scales}


def get_adc(tokens, cfg, nch):
    i = tokens.index(b"Adc") + 1
    values = tokens[i:i + cfg["n_adc"]]
    if len(values) != cfg["n_adc"]:
        raise ValueError("Missing ADC channel value")
    raw = [None] * nch
    corr = [None] * nch
    for j, value in enumerate(values):
        ch = cfg["mapping"][j]
        if ch < 0:
            continue
        if raw[ch] is not None:
            raise ValueError("Duplicate ADC assignment")
        q = int(value)
        raw[ch] = q
        corr[ch] = 0. if q == 0 else (q - cfg["ped"][ch]) * cfg["scale"][ch]
    return raw, corr


def track_records(paths):
    last = -1
    for path in paths:
        with open(path, encoding="utf-8", errors="replace") as fh:
            for lineno, line in enumerate(fh, 1):
                if not line.strip() or line.lstrip().startswith("#"):
                    continue
                t = line.split()
                if len(t) < 10:
                    raise ValueError(f"Malformed track list {path}:{lineno} (expected >=10 columns)")
                try:
                    eid = int(t[0])
                    sx, sy, x0, y0 = map(float, t[4:8])
                    qx, qy = map(float, t[8:10])
                except ValueError as exc:
                    raise ValueError(f"Malformed track values {path}:{lineno}") from exc
                if eid <= last:
                    raise ValueError(f"Track IDs must increase strictly: {path}:{lineno}, {eid} after {last}")
                last = eid
                yield (eid, sx, sy, x0, y0, qx, qy)


def expand_inputs(patterns):
    out = ht.expand(patterns)
    if not out:
        raise ValueError(f"No files matched: {patterns}")
    return out


def classify_track(raw_ch, det, tr, max_chi2, min_pairs, match_mm, max_cluster_size):
    """Return a conservative geometry category and the optional extra-strip location.

    A paired plane must contain one X and one Y cluster near the external fitted
    trajectory. All clusters in other axes/planes must be absent for the clean
    selection. The extra strip must form its own cluster and be farther than the
    matching window from the fitted trajectory in that coordinate.
    """
    _, sx, sy, x0, y0, qx, qy = tr
    if not all(math.isfinite(x) for x in (sx, sy, x0, y0, qx, qy)):
        return "other_accepted_track", None, 0
    if qx > max_chi2 or qy > max_chi2 or qx < 0 or qy < 0:
        return "other_accepted_track", None, 0

    paired = 0
    extras = []
    for idx, ((raw_x, raw_y), cfg) in enumerate(zip(raw_ch, det["ch"])):
        x = ht.correct(raw_x, cfg["sx"], cfg["misx"], cfg["brokenx"])
        y = ht.correct(raw_y, cfg["sy"], cfg["misy"], cfg["brokeny"])
        cx = ht.clusterize(x)
        cy = ht.clusterize(y)
        if any(size > max_cluster_size for _, size in (cx + cy)):
            return "other_accepted_track", None, paired

        z = cfg["z"]
        pred_x = sx * z + x0
        pred_y = sy * z + y0
        x_good = [j for j, (center, size) in enumerate(cx)
                  if abs((center + .5) * cfg["padx"] + cfg["px"] - pred_x) <= match_mm]
        y_good = [j for j, (center, size) in enumerate(cy)
                  if abs((center + .5) * cfg["pady"] + cfg["py"] - pred_y) <= match_mm]

        # Multiple near-trajectory clusters make the assignment ambiguous.
        if len(x_good) > 1 or len(y_good) > 1:
            return "other_accepted_track", None, paired
        has_pair = len(x_good) == 1 and len(y_good) == 1
        if has_pair:
            paired += 1
        for axis, clusters, good, step, offset, prediction in (
            ("X", cx, x_good if has_pair else (), cfg["padx"], cfg["px"], pred_x),
            ("Y", cy, y_good if has_pair else (), cfg["pady"], cfg["py"], pred_y),
        ):
            for j, (center, size) in enumerate(clusters):
                if j in good:
                    continue
                position = (center + .5) * step + offset
                extras.append((idx, axis, size, abs(position-prediction)))
                if len(extras) > 1:
                    return "other_accepted_track", None, paired

    if paired < min_pairs:
        return "other_accepted_track", None, paired
    if not extras:
        return "clean_track", None, paired
    idx, axis, size, distance = extras[0]
    if size == 1 and distance > match_mm:
        return "clean_plus_single_strip", (idx, axis), paired
    return "other_accepted_track", None, paired


def bin_value(q, width):
    if q is None or not math.isfinite(q):
        return None
    return math.floor(q / width) * width


def median_from_hist(h):
    if not h:
        return None
    count = sum(h.values())
    target = (count + 1)//2
    n = 0
    for v, c in sorted(h.items()):
        n += c
        if n >= target:
            return v
    return None


def quantile(values, frac):
    if not values:
        return None
    vals = sorted(values)
    p = frac * (len(vals)-1)
    i = int(p)
    return vals[i] + (vals[min(i+1, len(vals)-1)]-vals[i]) * (p-i)


def pearson(pairs):
    n, sx, sy, xx, yy, xy = pairs
    if n < 30:
        return None
    vx = xx - sx*sx/n
    vy = yy - sy*sy/n
    return (xy-sx*sy/n)/math.sqrt(vx*vy) if vx > 0 and vy > 0 else None


def run_data(label, ebes, tracks, det, adc_cfg, args, hist, groups, trigger_counts,
             chamber_stats, quiet_vectors, examples):
    t0 = time.monotonic()
    idx_track = 0
    idx_no_track = 0
    n_scan = 0
    n_matched = 0
    last_id = -1
    track_iter = track_records(tracks)
    next_track = next(track_iter, None)
    truncated = False
    per_class = Counter()

    for source, line_number, line in iter_ebe(ebes):
        if args.max_events and n_scan >= args.max_events:
            truncated = True
            break
        n_scan += 1
        try:
            eid = int(line.split(None, 1)[0])
        except (ValueError, IndexError) as exc:
            raise ValueError(f"Bad EBE event ID at {source}:{line_number}") from exc
        if eid <= last_id:
            raise ValueError(f"EBE event IDs are not increasing at {source}:{line_number}: {eid}")
        last_id = eid
        if next_track is not None and next_track[0] < eid:
            raise ValueError(f"Track ID {next_track[0]} not found before EBE ID {eid}; "
                             "track list and EBE sequence may not correspond")

        accepted = next_track is not None and next_track[0] == eid
        if accepted:
            tr = next_track
            next_track = next(track_iter, None)
            n_matched += 1
            idx_track += 1
            selected = (idx_track - 1) % args.track_sample == 0
        else:
            tr = None
            idx_no_track += 1
            selected = (idx_no_track - 1) % args.no_track_sample == 0
        if args.progress_every and n_scan % args.progress_every == 0:
            print(f"{label}: scanned {n_scan:,} events, joined {n_matched:,} accepted tracks", flush=True)
        if not selected:
            continue

        tokens, pattern = ht.parse_p_and_tokens(line, det["nbits"])
        if pattern is None:
            raise ValueError(f"No trigger mask at {source}:{line_number}")
        raw_ch = ht.parse_ch_payload(tokens, det)
        if raw_ch is None:
            raise ValueError(f"Malformed chamber payload at {source}:{line_number}")
        raw_adc, corr_adc = get_adc(tokens, adc_cfg, det["nch"])

        if accepted:
            category, extra, paired = classify_track(raw_ch, det, tr, args.max_chi2,
                args.min_paired_chambers, args.match_mm, args.max_cluster_size)
        else:
            category, extra, paired = "no_accepted_track", None, 0
        key = (label, category)
        per_class[category] += 1
        n_triggers = pattern.bit_count()
        trigger_counts[(label, category, n_triggers)] += 1
        g = groups[key]
        g["events"] += 1
        trig_ch = set(ht.trig_ch(pattern, det))
        active_ch = {i for i, (xs, ys) in enumerate(raw_ch) if xs or ys}
        quiet_values = [None] * det["nch"]
        active_vals = []
        triggered_vals = []

        for ch in range(det["nch"]):
            val = corr_adc[ch]
            if val is None:
                continue
            raw_x, raw_y = raw_ch[ch]
            active = ch in active_ch
            triggered = ch in trig_ch
            quiet = not active and not triggered
            scopes = ["all"]
            if active: scopes.append("hit")
            if triggered: scopes.append("triggered")
            if quiet and raw_adc[ch] != 0:
                scopes.append("quiet")
                quiet_values[ch] = val
            if extra is not None and extra[0] == ch:
                scopes.append("extra_strip_chamber")
            if accepted and paired >= args.min_paired_chambers and active:
                scopes.append("accepted_hit")
            for scope in scopes:
                k = (label, category, ch, scope)
                d = chamber_stats[k]
                d["n"] += 1
                d["sum"] += val
                d["sum2"] += val*val
                hist[(label, category, ch, scope, bin_value(val, args.adc_bin))] += 1
            if active: active_vals.append(val)
            if triggered: triggered_vals.append(val)

        if active_vals:
            g["sum_hit_adc_mean"] += statistics.mean(active_vals)
            g["events_with_hits"] += 1
        if triggered_vals:
            g["sum_triggered_adc_mean"] += statistics.mean(triggered_vals)
            g["events_with_triggers"] += 1
        g["sum_raw_hit_strips"] += sum(len(x)+len(y) for x,y in raw_ch)
        g["sum_hit_chambers"] += len(active_ch)
        g["sum_paired_chambers"] += paired

        timestamp = ""
        if b"T" in tokens:
            pos = tokens.index(b"T")
            if pos + 1 < len(tokens):
                timestamp = tokens[pos + 1].decode("ascii", errors="replace")
        if sum(x is not None for x in quiet_values) >= args.min_quiet_chambers:
            quiet_vectors[label].append((category, eid, timestamp, quiet_values))

        if len(examples[key]) < args.examples_per_class:
            examples[key].append(dict(dataset=label, event_id=eid, source=source,
                line_number=line_number, category=category, ntrigger=n_triggers,
                chi2x="" if tr is None else tr[5], chi2y="" if tr is None else tr[6],
                paired_chambers=paired, extra_chamber="" if extra is None else extra[0],
                extra_axis="" if extra is None else extra[1],
                raw_strips=sum(len(x)+len(y) for x,y in raw_ch),
                hit_chambers=len(active_ch),
                adc_corr_hit_sum=round(sum(active_vals), 3)))

    if not truncated and next_track is not None:
        raise ValueError(f"Track list contains ID {next_track[0]} beyond end of EBE inputs")
    elapsed = time.monotonic() - t0
    info = {"events_scanned": n_scan, "truncated": truncated, "accepted_tracks_joined": n_matched,
            "no_track_events": idx_no_track, "track_sample_every": args.track_sample,
            "no_track_sample_every": args.no_track_sample, "sampled_class_counts": dict(per_class),
            "seconds": round(elapsed, 1)}
    print(label, json.dumps(info, sort_keys=True), flush=True)
    return info


def iter_ebe(ebes):
    for filename in ebes:
        print("Reading", filename, flush=True)
        yield from ht.lines_from(filename)


def write_csv(path, headers, rows):
    with path.open("w", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(headers)
        writer.writerows(rows)


def analyze_common_mode(quiet_vectors, run_names, nch, minimum):
    run_rows = []
    pair_rows = []
    time_rows = []
    for label in run_names:
        samples = quiet_vectors[label]
        # Estimate each chamber's quiet baseline independently of the configured pedestal.
        reference = []
        for ch in range(nch):
            vals = [q[ch] for category, eid, dt, q in samples
                    if category == "no_accepted_track" and q[ch] is not None]
            if len(vals) < 30:
                vals = [q[ch] for _, _, _, q in samples if q[ch] is not None]
            reference.append(statistics.median(vals) if vals else None)
        cms = []
        pair_acc = defaultdict(lambda: [0,0.,0.,0.,0.,0.])
        temporal = defaultdict(list)
        for category, eid, timestamp, q in samples:
            vals = {ch: q[ch]-reference[ch] for ch in range(nch)
                    if q[ch] is not None and reference[ch] is not None}
            if len(vals) < minimum:
                continue
            cm = statistics.median(vals.values())
            cms.append(cm)
            try:
                date = datetime.strptime(timestamp, "%Y-%m-%d_%H:%M:%S")
                minute = date.hour*60+date.minute
                start = (minute // 10)*10
                period = f"{date.date()}_{start//60:02d}:{start%60:02d}"
                temporal[(period, category)].append(cm)
            except ValueError:
                pass
            for i in vals:
                for j in vals:
                    if j <= i:
                        continue
                    x,y = vals[i], vals[j]
                    p = pair_acc[(i,j)]
                    p[0] += 1; p[1] += x; p[2] += y
                    p[3] += x*x; p[4] += y*y; p[5] += x*y
        run_rows.append((label, len(samples), len(cms),
            "" if not cms else round(statistics.median(cms), 3),
            "" if not cms else round(quantile(cms,.16),3),
            "" if not cms else round(quantile(cms,.84),3),
            json.dumps(reference)))
        for (i,j), acc in sorted(pair_acc.items()):
            r = pearson(acc)
            pair_rows.append((label, i, j, acc[0], "" if r is None else round(r,6)))
        for (period, category), values in sorted(temporal.items()):
            time_rows.append((label, category, period, len(values),
                round(statistics.median(values),3), round(quantile(values,.16),3),
                round(quantile(values,.84),3)))
    return run_rows, pair_rows, time_rows


def main():
    a = argparse.ArgumentParser(description=__doc__)
    a.add_argument("-b", "--db", required=True)
    a.add_argument("--reference", nargs="+", required=True, help="Run103 EBE globs")
    a.add_argument("--test", nargs="+", required=True, help="Run113 EBE globs")
    a.add_argument("--reference-tracks", nargs="+", required=True, help="Accepted-track lists matched to reference EBE")
    a.add_argument("--test-tracks", nargs="+", required=True, help="Accepted-track lists matched to test EBE")
    a.add_argument("--reference-label", default="Run103")
    a.add_argument("--test-label", default="Run113")
    a.add_argument("--track-sample", type=int, default=20, help="Decode every Nth accepted-track event")
    a.add_argument("--no-track-sample", type=int, default=200, help="Decode every Nth event without an accepted track")
    a.add_argument("--max-chi2", type=float, default=2.5, help="Maximum chi2/ndf in each fitted axis")
    a.add_argument("--min-paired-chambers", type=int, default=6)
    a.add_argument("--max-cluster-size", type=int, default=8)
    a.add_argument("--match-mm", type=float, default=15., help="Maximum distance of cluster centroid from track")
    a.add_argument("--min-quiet-chambers", type=int, default=3)
    a.add_argument("--adc-bin", type=float, default=16., help="Corrected ADC histogram bin width")
    a.add_argument("--examples-per-class", type=int, default=30)
    a.add_argument("--progress-every", type=int, default=1000000)
    a.add_argument("--max-events", type=int, default=0, help="Optional per-run event limit for a quick validation run; 0 scans all")
    a.add_argument("-o", "--output", default="Results")
    args = a.parse_args()
    for attr in ("track_sample", "no_track_sample", "min_paired_chambers", "min_quiet_chambers", "examples_per_class"):
        if getattr(args, attr) < 1:
            a.error(f"--{attr.replace('_','-')} must be >=1")
    if args.max_events < 0:
        a.error("--max-events must be nonnegative")
    if args.match_mm <= 0 or args.adc_bin <= 0 or args.max_cluster_size <= 0:
        a.error("Matching window, ADC bin width and cluster size limit must be positive")

    runs = [(args.reference_label, expand_inputs(args.reference), expand_inputs(args.reference_tracks)),
            (args.test_label, expand_inputs(args.test), expand_inputs(args.test_tracks))]
    if runs[0][0] == runs[1][0]:
        a.error("Dataset labels must differ")
    det = ht.parse_db(args.db)
    adc_cfg = adc_configuration(args.db, det["nch"])
    out = Path(args.output)
    out.mkdir(parents=True, exist_ok=True)

    hist = Counter()
    groups = defaultdict(lambda: defaultdict(float))
    trigger_counts = Counter()
    chamber_stats = defaultdict(lambda: defaultdict(float))
    quiet_vectors = defaultdict(list)
    examples = defaultdict(list)
    manifest = {"configuration": vars(args), "runs": {},
                "ebe_files": {label: ebes for label, ebes, _ in runs},
                "track_files": {label: tracks for label, _, tracks in runs}}

    for label, ebes, tracks in runs:
        ht.warn_parts(label, ebes)
        manifest["runs"][label] = run_data(label, ebes, tracks, det, adc_cfg, args,
            hist, groups, trigger_counts, chamber_stats, quiet_vectors, examples)

    write_csv(out / "adc_class_summary.csv",
        ("dataset", "class", "sampled_events", "mean_raw_strips", "mean_hit_chambers",
         "mean_paired_chambers", "mean_adc_active_chambers", "mean_adc_triggered_chambers"),
        ((label, cls, int(d["events"]), d["sum_raw_hit_strips"]/d["events"],
          d["sum_hit_chambers"]/d["events"], d["sum_paired_chambers"]/d["events"],
          d["sum_hit_adc_mean"]/d["events_with_hits"] if d["events_with_hits"] else "",
          d["sum_triggered_adc_mean"]/d["events_with_triggers"] if d["events_with_triggers"] else "")
         for (label, cls), d in sorted(groups.items())))
    write_csv(out / "adc_chamber_summary.csv",
        ("dataset", "class", "chamber", "chamber_name", "scope", "entries",
         "mean_adc_corrected", "std_adc_corrected"),
        ((label, cls, ch, det["ch"][ch]["name"], scope, int(d["n"]),
          d["sum"]/d["n"],
          math.sqrt(max(0., d["sum2"]/d["n"]-(d["sum"]/d["n"])**2)))
         for (label, cls, ch, scope), d in sorted(chamber_stats.items())))
    write_csv(out / "adc_histograms.csv",
        ("dataset", "class", "chamber", "scope", "adc_bin_low", "count"),
        ((label, cls, ch, scope, b, n) for (label,cls,ch,scope,b),n in sorted(hist.items())))
    write_csv(out / "adc_trigger_summary.csv",
        ("dataset", "class", "ntrigger", "sampled_events"),
        ((label, cls, ntrig, num) for (label, cls, ntrig), num in sorted(trigger_counts.items())))
    header = ("dataset", "event_id", "source", "line_number", "category", "ntrigger",
              "chi2x", "chi2y", "paired_chambers", "extra_chamber", "extra_axis",
              "raw_strips", "hit_chambers", "adc_corr_hit_sum")
    write_csv(out / "adc_examples.csv", header,
        ([row.get(key, "") for key in header] for _, rows in sorted(examples.items()) for row in rows))

    labels = [row[0] for row in runs]
    common, pairs, temporal = analyze_common_mode(quiet_vectors, labels, det["nch"], args.min_quiet_chambers)
    write_csv(out / "adc_common_mode.csv",
        ("dataset", "events_with_quiet_channels", "events_used", "median_adc_shift",
         "p16_adc_shift", "p84_adc_shift", "chamber_quiet_reference_adc_json"), common)
    write_csv(out / "adc_quiet_correlations.csv",
        ("dataset", "chamber_i", "chamber_j", "both_quiet_events", "pearson_r"), pairs)
    write_csv(out / "adc_common_mode_time.csv",
        ("dataset", "class", "time_bin_10min", "events", "median_shift", "p16_shift", "p84_shift"), temporal)
    (out / "adc_manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote ADC summaries, histograms, quiet-channel diagnostics and manifest to {out}")
    print("Interpretation: clean-plus-single-strip is a conservative geometric category, not confirmed photon background.")


if __name__ == "__main__":
    main()

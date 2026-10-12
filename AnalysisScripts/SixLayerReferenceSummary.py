#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Build a detector-response control sample from accepted >=6-point tracking events.

Compare its distributional summaries with the Run103/Run113 populations.
This compares *separate* samples: it does not estimate 3-trigger muon purity.

Requires HitTopologyAnalysis.py and BackgroundCharacterization.py in same folder.
EBE files and track list must share globally strictly increasing event IDs.
"""
import argparse
import csv
import json
import sys
from pathlib import Path

import HitTopologyAnalysis as ht
import BackgroundCharacterization as bc


def track_list(path):
    last = -1
    with open(path, encoding="utf-8", errors="replace") as f:
        for lineno, line in enumerate(f, 1):
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            t = line.split()
            if len(t) < 10:
                raise ValueError(f"Track-list line {lineno}: expected >=10 columns")
            try:
                event_id = int(t[0])
                chi_x = float(t[8])
                chi_y = float(t[9])
            except ValueError as exc:
                raise ValueError(f"Malformed track-list line {lineno}: {line[:100]}") from exc
            if event_id <= last:
                raise ValueError(f"Non-increasing/duplicate .tl IDs: {event_id} after {last}")
            last = event_id
            yield event_id, chi_x, chi_y


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-b", "--detector-db", required=True)
    parser.add_argument("-ap", "--analysis-parameters", required=True)
    parser.add_argument("--ebe", nargs="+", required=True)
    parser.add_argument("--tracks", required=True)
    parser.add_argument("--label", default="Run103_6Layer")
    parser.add_argument("--sample-every", type=int, default=100,
                        help="Sample every Nth QUALITY-selected accepted 6-layer track; 1 = full")
    parser.add_argument("--max-axis-chi2", type=float, default=2.5,
                        help="Require BOTH track-list chi2/ndf values <= threshold; 0 disables")
    parser.add_argument("--progress-every", type=int, default=2000000)
    parser.add_argument("-o", "--outdir", default="Results/Validation/SixLayerReference")
    args = parser.parse_args()
    if args.sample_every < 1:
        parser.error("--sample-every must be >= 1")
    files = ht.expand(args.ebe)
    if not files:
        parser.error("No EBE files matched")
    det = ht.parse_db(args.detector_db)
    ap = ht.parse_ap(args.analysis_parameters)
    adc = bc.parse_adc_and_hist_config(args.detector_db, args.analysis_parameters)
    ht.warn_parts(args.label, files)
    out = Path(args.outdir)
    out.mkdir(parents=True, exist_ok=True)

    # TrackMain already enforces the reconstruction acceptance.  The additional
    # two-axis fit cut makes a particularly conservative reference subset.
    g = bc.GroupAccumulator(det["nch"], adc["AdcMin"], adc["AdcMax"], adc["AdcBinSize"])
    iterator = track_list(args.tracks)
    next_track = next(iterator, None)
    previous_id = -1
    n_ebe = 0
    n_trackmatched = 0
    n_quality = 0
    n_sampled = 0
    n_bad_ch = 0
    n_bad_adc = 0
    n_sampled_trig3 = 0
    n_sampled_active_xy6 = 0

    with (out / "sixlayer_reference_event_examples.csv").open("w", newline="") as ef:
        writer = csv.writer(ef)
        writer.writerow(["event_id", "pattern", "ntrigger", "chi2x", "chi2y",
                         "hits_x", "hits_y", "active_any", "active_xy",
                         "clusters_x", "clusters_y", "adc_triggered_sum"])
        for ifile, filename in enumerate(files, 1):
            print(f"[{ifile}/{len(files)}] {filename}", flush=True)
            for source, lineno, line in ht.lines_from(filename):
                tokens, pattern = ht.parse_p_and_tokens(line, det["nbits"])
                if pattern is None:
                    raise ValueError(f"No trigger pattern at {source}:{lineno}")
                try:
                    event_id = int(tokens[0])
                except (ValueError, IndexError) as exc:
                    raise ValueError(f"Malformed EBE event ID at {source}:{lineno}") from exc
                if event_id <= previous_id:
                    raise ValueError(f"EBE IDs not increasing at {source}:{lineno}")
                previous_id = event_id
                n_ebe += 1
                if next_track is not None and next_track[0] < event_id:
                    raise ValueError(f"Track event {next_track[0]} absent from EBE sequence")
                if next_track is None or next_track[0] != event_id:
                    if args.progress_every and n_ebe % args.progress_every == 0:
                        print(f"  scanned={n_ebe:,}, matched={n_trackmatched:,}, selected={n_sampled:,}", flush=True)
                    continue
                _, chi_x, chi_y = next_track
                next_track = next(iterator, None)
                n_trackmatched += 1
                if args.max_axis_chi2 > 0 and (chi_x > args.max_axis_chi2 or chi_y > args.max_axis_chi2):
                    continue
                n_quality += 1
                if (n_quality - 1) % args.sample_every:
                    continue
                n_sampled += 1
                raw = ht.parse_ch_payload(tokens, det)
                if raw is None:
                    n_bad_ch += 1
                    continue
                prep = ht.prepare(raw, det, ap)
                trigger_ch = ht.trig_ch(pattern, det)
                adc_raw, adc_corr = bc.parse_adc_values(tokens, adc, det["nch"])
                if adc_raw is None:
                    n_bad_adc += 1
                g.add_event(pattern, prep, trigger_ch, adc_raw, adc_corr, ap)
                n_sampled_trig3 += int(pattern.bit_count() == 3)
                n_sampled_active_xy6 += int(sum(bool(x["hx"] and x["hy"]) for x in prep) >= 6)
                if n_sampled <= 1000:
                    writer.writerow([
                        event_id, pattern, pattern.bit_count(), chi_x, chi_y,
                        sum(len(x["hx"]) for x in prep), sum(len(x["hy"]) for x in prep),
                        sum(bool(x["hx"] or x["hy"]) for x in prep),
                        sum(bool(x["hx"] and x["hy"]) for x in prep),
                        sum(len(x["cx"]) for x in prep), sum(len(x["cy"]) for x in prep),
                        sum(adc_corr[j] for j in trigger_ch) if adc_corr is not None else "",
                    ])
    if next_track is not None:
        raise ValueError(f"Track-list ID {next_track[0]} extends beyond EBE files")
    groups = {(args.label, "sixlayer", "quality_track"): g}
    prefix = "sixlayer_reference"
    bc.write_class_summary(out / f"{prefix}_class_summary.csv", groups)
    bc.write_chamber_summary(out / f"{prefix}_chamber_summary.csv", groups, det)
    bc.write_integer_hists(out / f"{prefix}_event_multiplicity_hist.csv", groups)
    bc.write_cluster_size_hists(out / f"{prefix}_cluster_size_hist.csv", groups, det)
    bc.write_raw_cluster_size_hists(out / f"{prefix}_raw_cluster_size_hist.csv", groups, det)
    bc.write_adc_hists(out / f"{prefix}_adc_hist.csv", groups, det)
    manifest = dict(ebe_events=n_ebe, track_entries_matched=n_trackmatched,
                    quality_track_entries=n_quality, sampled_quality_track_entries=n_sampled,
                    malformed_ch=n_bad_ch, missing_adc=n_bad_adc,
                    selected_trigger3=n_sampled_trig3, selected_active_xy_ge6=n_sampled_active_xy6,
                    sample_every=args.sample_every, max_axis_chi2=args.max_axis_chi2,
                    analysis_db=args.detector_db, analysis_ap=args.analysis_parameters,
                    caveat="Separate >=6-point track reference, NOT a muon truth sample; per-event "
                           "morphology uses approximate Python corrections/clusterization.")
    (out / f"{prefix}_manifest.json").write_text(json.dumps(manifest, indent=2))
    print(json.dumps(manifest, indent=2))
    print("Wrote", out)


if __name__ == "__main__":
    main()

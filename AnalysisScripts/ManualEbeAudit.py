#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Decode and verify selected raw .ebe events with an auditable report.

Uses two reproducible event examples from prior geometrical analysis:
  Run103 event 783  (contiguous three-trigger, line-compatible)
  Run113 event 2223 (gapped three-trigger, line-incompatible)

Data must come from --examples (selected_examples.csv). With --verify-ebes,
the exact recorded line is checked against the original local EBE data.
This cannot validate StdDump automatically; that comparison needs the actual
StdDump output and the user's specific StdDump version/command.

Put this script alongside HitTopologyAnalysis.py in AnalysisScripts/.
"""
import argparse
import csv
import hashlib
import itertools
import sys
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
try:
    import HitTopologyAnalysis as ht
except ImportError as e:
    raise SystemExit("Place HitTopologyAnalysis.py beside ManualEbeAudit.py") from e

EXAMPLES = [("Run103", "783"), ("Run113", "2223")]
FALLBACK_NAMES = ["Mt4040_sn13", "Mt4040_sn14", "Mt4040_sn09", "Mt4040_sn10",
                  "Mt4040_sn11", "Mt4040_sn12", "Mt4040_sn15", "Mt4040_sn16"]
FALLBACK_Z = [0, 40, 80, 120, 160, 200, 240, 280]


def metadata(detpath):
    if not detpath:
        return {
            "names": FALLBACK_NAMES,
            "z": FALLBACK_Z,
            "assign": list(range(8)),
            "n_adc": 8,
            "adc_assign": list(range(8)),
            "ped": [None] * 8,
            "scale": [None] * 8,
            "det": None,
        }

    det = ht.parse_db(detpath)
    ped, scale = [], []
    adc_assign = None
    for raw in Path(detpath).read_text(errors="replace").splitlines():
        s = raw.strip()
        t = s.split()
        if not t or t[0].startswith("#"):
            continue
        if t[0] == "NAdcs":
            if len(t) < 2:
                raise ValueError("NAdcs configuration missing channel count")
            n_adc = int(t[1])
            if "Assign" not in t:
                raise ValueError("NAdcs config missing Assign mapping")
            j = t.index("Assign") + 1
            adc_assign = list(map(int, t[j:j+n_adc]))
            if len(adc_assign) != n_adc:
                raise ValueError("NAdcs Assign mapping too short")
        if t[0] != "ChamberBasics":
            continue
        ped.append(float(t[t.index("AdcPedestal") + 1]) if "AdcPedestal" in t else 0.0)
        scale.append(float(t[t.index("AdcScale") + 1]) if "AdcScale" in t else 1.0)
    if adc_assign is None:
        raise ValueError("No NAdcs configuration found")
    if len(ped) != det["nch"]:
        raise ValueError("Chamber configuration / pedestal count mismatch")
    names = [c["name"] for c in det["ch"]]
    return {
        "names": names,
        "z": [c["z"] for c in det["ch"]],
        "assign": det["assign"],
        "n_adc": len(adc_assign),
        "adc_assign": adc_assign,
        "ped": ped,
        "scale": scale,
        "det": det,
    }


def value_after(t, label, n):
    if label not in t:
        return None
    i = t.index(label) + 1
    if len(t[i:i + n]) != n:
        raise ValueError(f"Incomplete {label} block")
    return t[i:i + n]


def parse_event(raw, meta):
    t = raw.strip().split()
    event_id = int(t[0]); date = value_after(t, "T", 2)
    p = int(value_after(t, "P", 1)[0])
    if p >> len(meta["assign"]):
        raise ValueError(f"Trigger bits above configured mask for P={p}")
    adc_channels = list(map(int, value_after(t, "Adc", meta["n_adc"])))
    adc = [None] * len(meta["names"])
    for i, v in enumerate(adc_channels):
        ch = meta["adc_assign"][i]
        if ch >= 0:
            if ch >= len(adc): raise ValueError("ADC assignment out of chamber range")
            adc[ch] = v
    count = value_after(t, "Count", 2)
    hv = value_after(t, "Hv", 3)
    thp = value_after(t, "Thp", 4)
    check = value_after(t, "Check", 1)
    try:
        i = t.index("Ch") + 1
    except ValueError:
        raise ValueError("No 'Ch' marker") from None
    ch = []
    for k in range(len(meta["names"])):
        nx = int(t[i]); i += 1
        if nx < 0: raise ValueError("Negative X count")
        xs = list(map(int, t[i:i + nx])); i += nx
        if len(xs) != nx: raise ValueError("Truncated X pads")
        ny = int(t[i]); i += 1
        if ny < 0: raise ValueError("Negative Y count")
        ys = list(map(int, t[i:i + ny])); i += ny
        if len(ys) != ny: raise ValueError("Truncated Y pads")
        ch.append({"index": k, "x": xs, "y": ys})
    if i != len(t):
        raise ValueError(f"Unexpected {len(t) - i} trailing tokens after Ch")
    trigger = [meta["assign"][bit] for bit in range(len(meta["assign"]))
               if (p >> bit) & 1 and meta["assign"][bit] >= 0]
    z = [meta["z"][i] for i in trigger]
    if len(z) != 3:
        raise ValueError("Expected exactly 3 trigger bits")
    sorted_ids = sorted(range(len(meta["names"])), key=lambda i: meta["z"][i])
    ranks = sorted(sorted_ids.index(i) for i in trigger)
    return {
        "event_id": event_id, "datetime": date[0], "time_diff_us": int(date[1]),
        "trigger_mask": p, "triggers": trigger, "contiguous": ranks[-1] - ranks[0] == 2,
        "adc": adc, "count": count, "hv": hv, "thp": thp, "check": check,
        "ch": ch, "raw": raw.rstrip("\r\n"),
    }


def read_line_from_source(source, line_number):
    """Read a single one-based EBE source line, including a .zip:member source."""
    if ".zip:" in source:
        zippath, member = source.split(".zip:", 1)
        zippath += ".zip"
        with zipfile.ZipFile(zippath) as z, z.open(member, "r") as fh:
            try:
                raw = next(itertools.islice(fh, line_number - 1, line_number))
            except StopIteration:
                raise ValueError(f"Line {line_number} is absent in {source}") from None
        return raw.decode(errors="replace").rstrip("\r\n")
    with open(source, "r", errors="replace") as fh:
        try:
            return next(itertools.islice(fh, line_number - 1, line_number)).rstrip("\r\n")
        except StopIteration:
            raise ValueError(f"Line {line_number} is absent in {source}") from None


def read_examples(path):
    rows = list(csv.DictReader(open(path, newline="")))
    selected = []
    for dataset, ev in EXAMPLES:
        found = [r for r in rows if r["dataset"] == dataset and r["event_id"] == ev]
        if len(found) != 1:
            raise ValueError(f"Expected exactly one archived example for {dataset} event {ev}; got {len(found)}")
        selected.append(found[0])
    return selected


def render_document(events, meta, args):
    text = [
        "# Raw `.ebe` event audit",
        "",
        "**Status:** Raw fields decoded from the selected event records. ",
        "Original-file comparison is shown for each event below. `StdDump` visual/output reconciliation remains a separate manual check.",
        "",
        "**Reader contract:** A raw X/Y hit list contains individual pad/strip addresses, NOT preformed clusters and NOT particle IDs. "
        "The trigger-mask bit indicates a chamber's trigger decision; it is NOT identical to that chamber having readout hits. "
        "ADC readings are separate per-chamber electronics values and require pedestal correction for comparisons.",
        "",
        "**Caution:** Python clusterization is approximate and does not reproduce data-derived position smoothing. "
        "The values of `Check`, `Count`, and `Thp` are reproduced as recorded without interpreting undocumented subfields.",
    ]
    for row, ev, verified in events:
        prefix = f"{row['dataset']} event {ev['event_id']}"
        key = "line-compatible" if row["category"] == "compatible" else row["category"]
        text.extend([
            "", f"## {prefix}: {key}", "",
            f"- Source: `{row['source']}`, one-based line **{row['line']}**.",
            f"- Raw EBE verification: **{'PASSED' if verified else 'NOT RUN (selected example only)'}**.",
            f"- Raw line SHA-256: `{hashlib.sha256(ev['raw'].encode()).hexdigest()}`.",
            f"- Time field: `{ev['datetime']}`; time difference: `{ev['time_diff_us']}` microseconds.",
            f"- Trigger mask: decimal **{ev['trigger_mask']}** = "
            f"binary **{ev['trigger_mask']:0{len(meta['assign'])}b}** "
            f"(bit 0 is rightmost); trigger-chamber indices **{ev['triggers']}**; "
            f"**{'contiguous' if ev['contiguous'] else 'gapped'}** in z ordering.",
            f"- Recorded `Count`: `{ev['count']}`; `Check`: `{ev['check']}`; "
            f"`Hv`: `{ev['hv']}`; `Thp`: `{ev['thp']}`.",
            f"- Three-layer geometry: X chi²/ndf `{row['chi2x']}`; Y chi²/ndf `{row['chi2y']}`. "
            "These are **three-layer** fits, not muon identification.",
            "", "| Chamber index | Name | Trigger? | Raw X hit list | Raw Y hit list | X/Y counts | ADC raw | ADC corrected |",
            "|---:|---|:---:|---|---|---:|---:|---:|"
        ])
        for c in ev["ch"]:
            i = c["index"]
            corrected = (ev["adc"][i] - meta["ped"][i]) * meta["scale"][i] if (meta["ped"][i] is not None and ev["adc"][i] is not None) else None
            if ev["adc"][i] == 0 and corrected is not None: corrected = 0.0
            text.append(
                f"| {i} | `{meta['names'][i]}` | {'Yes' if i in ev['triggers'] else 'No'} | "
                f"`{','.join(map(str,c['x'])) or '—'}` | `{','.join(map(str,c['y'])) or '—'}` | "
                f"{len(c['x'])}/{len(c['y'])} | {ev['adc'][i] if ev['adc'][i] is not None else 'unmapped'} | "
                f"{f'{corrected:g}' if corrected is not None else 'N/A (supply -b Mts41.db)'} |"
            )
        hit_ch=[x["index"] for x in ev["ch"] if x["x"] or x["y"]]
        text.extend([
            "", f"Raw hit-bearing chambers: `{hit_ch}`. Triggered chambers: `{ev['triggers']}`. "
            "Compare these explicitly; they need not agree.",
            "", "**Raw `.ebe` record (exact archived text):**", "", "```text", ev["raw"], "```", "",
            "**Pending `StdDump` comparison:** locate this **event ID** in the corresponding StdDump output; "
            "compare trigger chamber indices, X/Y pad lists, per-chamber ADC and raw timestamp. "
            "Do not compare its displayed cluster/track positions directly with raw pad numbers: reconstruction involves corrections.",
        ])
    text.extend([
        "", "## Verification steps", "",
        "1. Execute this audit again with `--verify-ebes` so the archived raw rows are checked token-by-token (ignoring spacing) against your original local `.ebe` files.",
        "2. Obtain a small `StdDump` excerpt/screenshot for event **783** in Run103 and event **2223** in Run113. "
        "The `StdDump` command depends on your local version; this script does not invent its CLI.",
        "3. Match original event IDs, trigger masks, ADC readings and X/Y pad lists. "
        "After this comparison, record the outcome in the event audit.",
    ])
    return "\n".join(text) + "\n"


def main():
    a = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    a.add_argument("--examples", required=True, help="selected_examples.csv with original 'raw' records")
    a.add_argument("-b", "--db", help="The actual Settings/Mts41.db (required for ADC correction and verified mappings)")
    a.add_argument("--verify-ebes", action="store_true", help="Check the original EBE source/line against the CSV, then fail on mismatch")
    a.add_argument("-o", "--output", default="Results", help="Directory for audit outputs")
    args = a.parse_args()
    meta = metadata(args.db)
    rows = read_examples(args.examples)
    decoded = []
    checks = []
    for row in rows:
        ev = parse_event(row["raw"], meta)
        if str(ev["event_id"]) != row["event_id"] or ev["trigger_mask"] != int(row["pattern"]):
            raise ValueError(f"Stored example ID/mask mismatch for {row['dataset']}")
        if ev["contiguous"] != bool(int(row["contiguous"])):
            raise ValueError(f"Stored example topology mismatch for {row['dataset']}")
        n_x = sum(len(c["x"]) for c in ev["ch"])
        n_y = sum(len(c["y"]) for c in ev["ch"])
        if n_x != int(row["hits_x"]) or n_y != int(row["hits_y"]):
            raise ValueError(f"Stored sample raw hit count mismatch for {row['dataset']}")
        verified = False
        if args.verify_ebes:
            actual = read_line_from_source(row["source"], int(row["line"]))
            if actual.split() != ev["raw"].split():
                raise ValueError(f"Original EBE differs from archived example: {row['dataset']} event {ev['event_id']}")
            verified = True
        decoded.append((row, ev, verified))
        checks.append({
            "run": row["dataset"], "event_id": ev["event_id"],
            "source": row["source"], "line": row["line"],
            "trigger_dec": ev["trigger_mask"],
            "trigger_bin": format(ev["trigger_mask"], f"0{len(meta['assign'])}b"),
            "trigger_chambers": " ".join(map(str,ev["triggers"])),
            "raw_x_hits": n_x, "raw_y_hits": n_y,
            "raw_ebe_verified": verified,
            "stddump_checked": False,
        })
    outdir = Path(args.output); outdir.mkdir(parents=True, exist_ok=True)
    report = outdir / "event_audit.md"
    report.write_text(render_document(decoded, meta, args))
    with open(outdir / "event_checks.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=checks[0].keys()); w.writeheader(); w.writerows(checks)
    print(f"Wrote {report}")
    print(f"Wrote {outdir / 'event_checks.csv'}")
    for c in checks:
        print(f"{c['run']} event {c['event_id']}: "
              f"raw X/Y {c['raw_x_hits']}/{c['raw_y_hits']}, "
              f"EBE source verified={c['raw_ebe_verified']}")
    print("StdDump comparison is still required to complete the manual audit.")


if __name__ == "__main__":
    main()

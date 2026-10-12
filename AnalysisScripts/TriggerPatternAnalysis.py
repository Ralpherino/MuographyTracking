#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""
TriggerPatternAnalysis.py

Repository-compatible trigger-pattern diagnostic for MuographyTracking.

Designed to live at:
    AnalysisScripts/TriggerPatternAnalysis.py

It follows the MuographyTracking package conventions where practical:
  - detector geometry/mapping is read from the .db file (-b/--basic-info)
  - raw EBE inputs may be .ebe or .ebe.zip
  - output is written below Results by default (-rd/--dir-results)
  - trigger-bit -> chamber mapping is taken from:
        Trigger <N> Assign ...
  - chamber names and PosZ are taken from ChamberBasicsInfo

Example:
    python3 AnalysisScripts/TriggerPatternAnalysis.py \
        -b Settings/Mts41.db \
        --reference "../../ebe/Run103/Mts41_Run103*.ebe*" \
        --test "../../ebe/Run113/Mts41_Run113*.ebe*" \
        --reference-label Run103 \
        --test-label Run113 \
        -rd Results/Directory \
        -r result_filename
"""

import argparse
import csv
import glob
import math
import re
import sys
import time
import zipfile
from collections import defaultdict
from pathlib import Path


# ----------------------------------------------------------------------
# Detector database
# ----------------------------------------------------------------------

def parse_detector_db(filename):
    """
    Parse the MuographyTracking detector .db fields needed here:
      NChambers
      Trigger <N> Assign ...
      ChamberBasics Name ... PosZ ...
    """
    path = Path(filename)
    if not path.exists():
        raise FileNotFoundError(f"Detector DB not found: {filename}")

    text = path.read_text(errors="replace")
    lines = text.splitlines()

    n_chambers = None
    n_trigger_bits = None
    assign_trigger = None
    chamber_names = []
    chamber_z = []

    for raw in lines:
        line = raw.strip()
        if not line or line.startswith("#"):
            continue

        toks = line.split()

        if toks[0] == "NChambers" and len(toks) >= 2:
            n_chambers = int(toks[1])

        elif toks[0] == "Trigger" and len(toks) >= 4:
            n_trigger_bits = int(toks[1])
            try:
                i_assign = toks.index("Assign")
            except ValueError:
                raise ValueError(
                    f"Trigger line in {filename} has no Assign field:\n{line}"
                )
            assign_trigger = [
                int(x) for x in toks[i_assign + 1:i_assign + 1 + n_trigger_bits]
            ]

        elif toks[0] == "ChamberBasics":
            try:
                i_name = toks.index("Name")
                name = toks[i_name + 1]
            except (ValueError, IndexError):
                raise ValueError(f"Cannot parse chamber Name from:\n{line}")

            try:
                i_z = toks.index("PosZ")
                z = float(toks[i_z + 1])
            except (ValueError, IndexError):
                raise ValueError(f"Cannot parse chamber PosZ from:\n{line}")

            chamber_names.append(name)
            chamber_z.append(z)

    if n_chambers is None:
        raise ValueError(f"NChambers not found in {filename}")

    if len(chamber_names) != n_chambers:
        raise ValueError(
            f"{filename}: NChambers={n_chambers}, but "
            f"{len(chamber_names)} ChamberBasics lines were parsed."
        )

    if n_trigger_bits is None or assign_trigger is None:
        raise ValueError(f"Trigger ... Assign ... not found in {filename}")

    if len(assign_trigger) != n_trigger_bits:
        raise ValueError(
            f"{filename}: expected {n_trigger_bits} trigger assignments, "
            f"got {len(assign_trigger)}"
        )

    # Tracking.cpp convention:
    # AssignTriggerBit[NTriggerBit] gives the chamber index receiving that bit.
    trigger_names = []
    trigger_z = []
    for bit, chamber_index in enumerate(assign_trigger):
        if chamber_index < 0:
            trigger_names.append(f"unused_bit{bit}")
            trigger_z.append(float("nan"))
        elif chamber_index >= n_chambers:
            raise ValueError(
                f"Trigger bit {bit} maps to invalid chamber index {chamber_index}"
            )
        else:
            trigger_names.append(chamber_names[chamber_index])
            trigger_z.append(chamber_z[chamber_index])

    return {
        "path": str(path),
        "n_chambers": n_chambers,
        "n_trigger_bits": n_trigger_bits,
        "assign_trigger": assign_trigger,
        "chamber_names": chamber_names,
        "chamber_z": chamber_z,
        "trigger_names": trigger_names,
        "trigger_z": trigger_z,
    }


# ----------------------------------------------------------------------
# Input handling
# ----------------------------------------------------------------------

_PART_RE = re.compile(r"\.part(\d+)\.")


def expand_patterns(patterns):
    """Expand quoted globs and preserve sorted unique file names."""
    out = []
    seen = set()

    for pat in patterns:
        matches = sorted(glob.glob(pat))

        if not matches and Path(pat).exists():
            matches = [pat]

        for filename in matches:
            filename = str(Path(filename))
            if filename not in seen:
                seen.add(filename)
                out.append(filename)

    return out


def warn_part_sequence(label, filenames):
    """Warn if numbered part files appear to have gaps or do not start at 00."""
    parts = []

    for fn in filenames:
        m = _PART_RE.search(Path(fn).name)
        if m:
            parts.append(int(m.group(1)))

    if not parts:
        return

    parts = sorted(set(parts))
    expected = list(range(parts[0], parts[-1] + 1))
    missing = sorted(set(expected) - set(parts))

    if parts[0] != 0:
        print(
            f"WARNING [{label}]: first detected part is part{parts[0]:02d}; "
            f"part00 is not in the input list.",
            file=sys.stderr,
        )

    if missing:
        print(
            f"WARNING [{label}]: missing part numbers: "
            + ", ".join(f"{x:02d}" for x in missing),
            file=sys.stderr,
        )


def iter_lines(filename):
    """Yield (source, line_no, line_bytes) from .ebe or zip archive."""
    path = Path(filename)

    if path.suffix.lower() == ".zip":
        with zipfile.ZipFile(path, "r") as zf:
            members = [x for x in zf.namelist() if not x.endswith("/")]
            for member in members:
                with zf.open(member, "r") as fh:
                    for lineno, line in enumerate(fh, 1):
                        yield f"{path}:{member}", lineno, line
    else:
        with open(path, "rb", buffering=1024 * 1024) as fh:
            for lineno, line in enumerate(fh, 1):
                yield str(path), lineno, line


def extract_trigger_pattern(line):
    """
    Extract integer after the EBE token P.

    MuographyTracking ReadEventFormats.cpp reads this field as TriggerPattern.
    """
    marker = b" P "
    pos = line.find(marker)

    if pos >= 0:
        start = pos + len(marker)
        end = line.find(b" ", start)
        if end < 0:
            end = len(line)

        token = line[start:end].strip()
        try:
            return int(token)
        except ValueError:
            pass

    toks = line.split()
    try:
        i = toks.index(b"P")
        return int(toks[i + 1])
    except (ValueError, IndexError):
        return None


# ----------------------------------------------------------------------
# Topology
# ----------------------------------------------------------------------

def popcount(value):
    try:
        return value.bit_count()
    except AttributeError:
        return bin(value).count("1")


def pattern_info(pattern, detector):
    nbits = detector["n_trigger_bits"]
    mask = (1 << nbits) - 1
    p = pattern & mask

    bits = [i for i in range(nbits) if (p >> i) & 1]
    ntrig = len(bits)

    # Contiguity is based on physical z-order, not merely bit number.
    fired = []
    for bit in bits:
        z = detector["trigger_z"][bit]
        name = detector["trigger_names"][bit]
        fired.append((z, bit, name))

    fired.sort(key=lambda x: (math.isnan(x[0]), x[0], x[1]))

    ordered_bits = [x[1] for x in fired]
    ordered_names = [x[2] for x in fired]
    ordered_z = [x[0] for x in fired]

    # Map every used trigger bit into its rank in detector z-order.
    all_used = [
        (z, bit)
        for bit, z in enumerate(detector["trigger_z"])
        if not math.isnan(z)
    ]
    all_used.sort()
    rank = {bit: i for i, (_, bit) in enumerate(all_used)}

    ranks = sorted(rank[b] for b in bits if b in rank)

    if ranks:
        span_layers = ranks[-1] - ranks[0] + 1
        gaps = span_layers - len(ranks)
        contiguous = gaps == 0
        z_span = (
            max(z for z in ordered_z if not math.isnan(z))
            - min(z for z in ordered_z if not math.isnan(z))
        )
    else:
        span_layers = 0
        gaps = 0
        contiguous = False
        z_span = 0.0

    return {
        "pattern_dec": p,
        "pattern_bin": format(p, f"0{nbits}b"),
        "ntrig": ntrig,
        "bits": bits,
        "ordered_bits": ordered_bits,
        "chambers": "+".join(ordered_names) if ordered_names else "none",
        "contiguous": contiguous,
        "gaps": gaps,
        "span_layers": span_layers,
        "z_span": z_span,
    }


# ----------------------------------------------------------------------
# Analysis
# ----------------------------------------------------------------------

def analyze(label, filenames, detector, examples_per_pattern, progress_every):
    max_patterns = 1 << detector["n_trigger_bits"]

    counts = [0] * max_patterns
    mult_counts = [0] * (detector["n_trigger_bits"] + 1)
    examples = defaultdict(list)
    file_counts = []

    total_parsed = 0
    total_bad = 0
    t0 = time.time()

    print(f"\n=== {label} ===")
    print(f"Input files: {len(filenames)}")
    warn_part_sequence(label, filenames)

    for i_file, filename in enumerate(filenames, 1):
        parsed_this = 0
        bad_this = 0

        print(f"[{i_file}/{len(filenames)}] {filename}", flush=True)

        for source, lineno, line in iter_lines(filename):
            pattern = extract_trigger_pattern(line)

            if pattern is None:
                bad_this += 1
                continue

            mask = max_patterns - 1
            p = pattern & mask
            counts[p] += 1

            ntrig = popcount(p)
            mult_counts[ntrig] += 1

            parsed_this += 1
            total_parsed += 1

            if ntrig in (2, 3) and len(examples[p]) < examples_per_pattern:
                examples[p].append(
                    (
                        source,
                        lineno,
                        line.decode("utf-8", errors="replace").rstrip(),
                    )
                )

            if progress_every and total_parsed % progress_every == 0:
                dt = max(time.time() - t0, 1e-9)
                print(
                    f"  total parsed: {total_parsed:,} "
                    f"({total_parsed / dt:,.0f} events/s)",
                    flush=True,
                )

        file_counts.append((filename, parsed_this, bad_this))
        total_bad += bad_this

        print(
            f"  -> parsed={parsed_this:,}, unparsed={bad_this:,}",
            flush=True,
        )

    dt = time.time() - t0

    print(
        f"{label}: total parsed={total_parsed:,}, "
        f"unparsed={total_bad:,}, elapsed={dt / 60:.1f} min"
    )

    return {
        "label": label,
        "filenames": filenames,
        "counts": counts,
        "mult_counts": mult_counts,
        "examples": examples,
        "file_counts": file_counts,
        "parsed": total_parsed,
        "bad": total_bad,
    }


# ----------------------------------------------------------------------
# Output
# ----------------------------------------------------------------------

def ensure_output_dir(path):
    p = Path(path)
    p.mkdir(parents=True, exist_ok=True)
    return p


def write_file_counts(outdir, prefix, result):
    fn = outdir / f"{prefix}_{result['label']}_files.csv"

    with open(fn, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["file", "parsed_events", "unparsed_lines"])
        w.writerows(result["file_counts"])

    print(f"Wrote {fn}")


def write_run_csv(outdir, prefix, result, detector):
    fn = outdir / f"{prefix}_{result['label']}.csv"
    total = result["parsed"]

    with open(fn, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "pattern_dec",
            "pattern_bin",
            "ntrig",
            "trigger_bits",
            "chambers_z_order",
            "contiguous",
            "gaps",
            "span_layers",
            "z_span",
            "count",
            "fraction_all_events",
            "fraction_within_multiplicity",
        ])

        rows = []

        for p, count in enumerate(result["counts"]):
            if count == 0:
                continue

            info = pattern_info(p, detector)
            n = info["ntrig"]

            frac_all = count / total if total else 0.0
            frac_n = (
                count / result["mult_counts"][n]
                if result["mult_counts"][n]
                else 0.0
            )

            rows.append((n, -count, p, info, count, frac_all, frac_n))

        rows.sort()

        for _, _, _, info, count, frac_all, frac_n in rows:
            w.writerow([
                info["pattern_dec"],
                info["pattern_bin"],
                info["ntrig"],
                ",".join(map(str, info["bits"])),
                info["chambers"],
                int(info["contiguous"]),
                info["gaps"],
                info["span_layers"],
                f"{info['z_span']:.12g}",
                count,
                f"{frac_all:.12g}",
                f"{frac_n:.12g}",
            ])

    print(f"Wrote {fn}")


def safe_ratio(num, den):
    return num / den if den else math.nan


def write_comparison_csv(outdir, prefix, ref, test, detector):
    fn = outdir / f"{prefix}_comparison.csv"

    with open(fn, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "pattern_dec",
            "pattern_bin",
            "ntrig",
            "trigger_bits",
            "chambers_z_order",
            "contiguous",
            "gaps",
            "span_layers",
            "z_span",
            f"{ref['label']}_count",
            f"{ref['label']}_fraction_all",
            f"{ref['label']}_fraction_within_multiplicity",
            f"{test['label']}_count",
            f"{test['label']}_fraction_all",
            f"{test['label']}_fraction_within_multiplicity",
            f"enrichment_{test['label']}_vs_{ref['label']}_all_fraction",
            f"enrichment_{test['label']}_vs_{ref['label']}_within_multiplicity",
        ])

        rows = []

        for p in range(len(ref["counts"])):
            c_ref = ref["counts"][p]
            c_test = test["counts"][p]

            if not (c_ref or c_test):
                continue

            info = pattern_info(p, detector)
            n = info["ntrig"]

            f_ref = c_ref / ref["parsed"] if ref["parsed"] else 0.0
            f_test = c_test / test["parsed"] if test["parsed"] else 0.0

            g_ref = (
                c_ref / ref["mult_counts"][n]
                if ref["mult_counts"][n]
                else 0.0
            )
            g_test = (
                c_test / test["mult_counts"][n]
                if test["mult_counts"][n]
                else 0.0
            )

            enrich_all = safe_ratio(f_test, f_ref)
            enrich_mult = safe_ratio(g_test, g_ref)

            priority = 0 if n in (2, 3) else 1

            rows.append(
                (
                    priority,
                    n,
                    -c_test,
                    p,
                    info,
                    c_ref,
                    f_ref,
                    g_ref,
                    c_test,
                    f_test,
                    g_test,
                    enrich_all,
                    enrich_mult,
                )
            )

        rows.sort()

        for row in rows:
            (
                _,
                _,
                _,
                _,
                info,
                c_ref,
                f_ref,
                g_ref,
                c_test,
                f_test,
                g_test,
                enrich_all,
                enrich_mult,
            ) = row

            w.writerow([
                info["pattern_dec"],
                info["pattern_bin"],
                info["ntrig"],
                ",".join(map(str, info["bits"])),
                info["chambers"],
                int(info["contiguous"]),
                info["gaps"],
                info["span_layers"],
                f"{info['z_span']:.12g}",
                c_ref,
                f"{f_ref:.12g}",
                f"{g_ref:.12g}",
                c_test,
                f"{f_test:.12g}",
                f"{g_test:.12g}",
                "" if math.isnan(enrich_all) else f"{enrich_all:.12g}",
                "" if math.isnan(enrich_mult) else f"{enrich_mult:.12g}",
            ])

    print(f"Wrote {fn}")


def write_examples(outdir, prefix, ref, test, detector):
    fn = outdir / f"{prefix}_examples.txt"

    with open(fn, "w") as fh:
        for result in (ref, test):
            fh.write(f"===== {result['label']} =====\n\n")

            patterns = [
                p
                for p in range(len(result["counts"]))
                if result["counts"][p]
                and pattern_info(p, detector)["ntrig"] in (2, 3)
            ]

            patterns.sort(key=lambda p: result["counts"][p], reverse=True)

            for p in patterns:
                info = pattern_info(p, detector)

                fh.write(
                    f"Pattern {info['pattern_dec']} "
                    f"({info['pattern_bin']}) "
                    f"N={info['ntrig']} "
                    f"{info['chambers']} "
                    f"contiguous={info['contiguous']} "
                    f"count={result['counts'][p]:,}\n"
                )

                for source, lineno, line in result["examples"].get(p, []):
                    fh.write(f"  [{source}:{lineno}]\n")
                    fh.write(f"  {line}\n")

                fh.write("\n")

    print(f"Wrote {fn}")


def print_detector(detector):
    print("\nDetector trigger mapping from DB:")
    for bit, (name, z, chamber_index) in enumerate(
        zip(
            detector["trigger_names"],
            detector["trigger_z"],
            detector["assign_trigger"],
        )
    ):
        print(
            f"  bit {bit:2d} -> chamber index {chamber_index:2d} "
            f"-> {name:16s}  z={z:g}"
        )


def print_summary(result, detector):
    print(f"\n--- {result['label']} trigger multiplicities ---")

    total = result["parsed"]

    for n, count in enumerate(result["mult_counts"]):
        if count:
            print(
                f"N={n}: {count:>12,} "
                f"{100.0 * count / total:9.4f}%"
            )

    for n in (2, 3):
        denom = result["mult_counts"][n]

        contig = 0
        noncontig = 0

        for p, count in enumerate(result["counts"]):
            info = pattern_info(p, detector)

            if info["ntrig"] != n:
                continue

            if info["contiguous"]:
                contig += count
            else:
                noncontig += count

        if denom:
            print(
                f"{result['label']} N={n}: "
                f"contiguous={contig:,} ({100*contig/denom:.3f}%), "
                f"non-contiguous={noncontig:,} ({100*noncontig/denom:.3f}%)"
            )


# ----------------------------------------------------------------------
# CLI
# ----------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(
        description=(
            "MuographyTracking-compatible trigger-pattern comparison "
            "for two EBE datasets."
        )
    )

    ap.add_argument(
        "-b",
        "--basic-info",
        required=True,
        help="MuographyTracking detector .db file, e.g. Settings/Mts41.db",
    )

    ap.add_argument(
        "--reference",
        nargs="+",
        required=True,
        help="Reference EBE file(s) or quoted glob(s), e.g. Run103",
    )

    ap.add_argument(
        "--test",
        nargs="+",
        required=True,
        help="Test/background EBE file(s) or quoted glob(s), e.g. Run113",
    )

    ap.add_argument(
        "--reference-label",
        default="Reference",
        help="Reference label used in output filenames",
    )

    ap.add_argument(
        "--test-label",
        default="Test",
        help="Test label used in output filenames",
    )

    ap.add_argument(
        "-r",
        "--results",
        default="trigger_patterns",
        help="Output filename prefix",
    )

    ap.add_argument(
        "-rd",
        "--dir-results",
        default="Results/TriggerPatterns",
        help="Output directory",
    )

    ap.add_argument(
        "--examples",
        type=int,
        default=2,
        help="Number of raw N=2/3 examples saved per pattern",
    )

    ap.add_argument(
        "--progress-every",
        type=int,
        default=5_000_000,
        help="Progress print interval in parsed events; 0 disables",
    )

    args = ap.parse_args()

    detector = parse_detector_db(args.basic_info)
    print_detector(detector)

    reference_files = expand_patterns(args.reference)
    test_files = expand_patterns(args.test)

    if not reference_files:
        sys.exit("No reference EBE files matched.")
    if not test_files:
        sys.exit("No test EBE files matched.")

    outdir = ensure_output_dir(args.dir_results)

    print("\nReference files:")
    for x in reference_files:
        print(f"  {x}")

    print("\nTest files:")
    for x in test_files:
        print(f"  {x}")

    ref = analyze(
        args.reference_label,
        reference_files,
        detector,
        args.examples,
        args.progress_every,
    )

    test = analyze(
        args.test_label,
        test_files,
        detector,
        args.examples,
        args.progress_every,
    )

    write_file_counts(outdir, args.results, ref)
    write_file_counts(outdir, args.results, test)

    write_run_csv(outdir, args.results, ref, detector)
    write_run_csv(outdir, args.results, test, detector)

    write_comparison_csv(outdir, args.results, ref, test, detector)
    write_examples(outdir, args.results, ref, test, detector)

    print_summary(ref, detector)
    print_summary(test, detector)

    print("\nDone.")


if __name__ == "__main__":
    main()

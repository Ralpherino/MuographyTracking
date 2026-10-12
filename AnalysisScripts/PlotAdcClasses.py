#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Plot normalized pedestal-corrected ADC distributions from AdcEventClasses.py.

Plotting is deliberately done after the streamed EBE analysis, so figures can
be revised without scanning the raw event files again. Each figure contains
only one detector run and one chamber-activity selection.
"""
import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

STYLES = {
    "no_accepted_track": "No accepted track",
    "clean_track": "Clean track",
    "clean_plus_single_strip": "Clean track + isolated extra strip",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--histograms", default="Results/adc_histograms.csv")
    parser.add_argument("-o", "--output", default="Results")
    parser.add_argument("--x-min", type=float, default=-500)
    parser.add_argument("--x-max", type=float, default=5000)
    args = parser.parse_args()
    if args.x_min >= args.x_max:
        parser.error("X-axis minimum must be below maximum")

    groups = defaultdict(Counter)
    for row in csv.DictReader(open(args.histograms, newline="")):
        if row["class"] not in STYLES or row["scope"] not in ("hit", "quiet"):
            continue
        k = (row["dataset"], row["scope"], row["class"])
        groups[k][float(row["adc_bin_low"])] += int(row["count"])

    output = Path(args.output)
    output.mkdir(parents=True, exist_ok=True)
    for run in sorted({key[0] for key in groups}):
        for scope in ("hit", "quiet"):
            fig, ax = plt.subplots(figsize=(9, 5.4))
            shown = 0
            for cls, label in STYLES.items():
                h = groups.get((run, scope, cls), Counter())
                if not h:
                    continue
                total = sum(h.values())
                xs = [x for x in sorted(h) if args.x_min <= x <= args.x_max]
                ys = [h[x] / total for x in xs]
                ax.plot(xs, ys, label=f"{label} (N={total:,} chamber readings)", linewidth=1.35)
                shown += 1
            if not shown:
                plt.close(fig)
                continue
            ax.set_xlim(args.x_min, args.x_max)
            ax.set_xlabel("Pedestal-corrected chamber ADC [ADC units]")
            ax.set_ylabel("Fraction of chamber readings / histogram bin")
            ax.set_title(f"{run}: {'hit-bearing' if scope=='hit' else 'quiet'} chambers")
            ax.grid(alpha=.25)
            ax.legend(fontsize=9)
            fig.tight_layout()
            path = output / f"adc_{run}_{scope}.png"
            fig.savefig(path, dpi=160)
            plt.close(fig)
            print("Wrote", path)


if __name__ == "__main__":
    main()

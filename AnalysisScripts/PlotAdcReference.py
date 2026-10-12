#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Overlay the previously calculated chamber ADC reference distributions."""
import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

SELECTIONS = [
    ("Run103", "contiguous", "compatible", "Run103: contiguous, line-compatible"),
    ("Run113", "gapped", "incompatible", "Run113: gapped, line-incompatible"),
    ("Run113", "gapped", "missing_xy", "Run113: gapped, missing X/Y"),
    ("Run103_6Layer", "sixlayer", "quality_track", "Run103: quality six-layer tracks"),
]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--full", required=True, help="Existing full-statistics ADC histogram CSV")
    p.add_argument("--tracks", required=True, help="Existing quality-track ADC histogram CSV")
    p.add_argument("-o", "--output", default="Results/adc_reference_distributions.png")
    a = p.parse_args()
    hists = defaultdict(Counter)
    totals = Counter()
    for filename in (a.full, a.tracks):
        with open(filename, newline="") as fh:
            for row in csv.DictReader(fh):
                if row["scope"] != "triggered_chamber":
                    continue
                k = (row["dataset"], row["topology"], row["geometry"])
                n = int(row["count"])
                totals[k] += n
                if row["bin_low"] and row["bin_high"] and not row["special_bin"]:
                    x = (float(row["bin_low"]) + float(row["bin_high"])) / 2.
                    hists[k][x] += n

    fig, ax = plt.subplots(figsize=(10.5,6.4))
    for run, topo, geo, label in SELECTIONS:
        k = (run,topo,geo)
        if not totals[k]:
            raise ValueError(f"No histogram entries for {k}")
        xs = [x for x in sorted(hists[k]) if -300 <= x <= 5000]
        ys = [hists[k][x]/totals[k] for x in xs]
        ax.plot(xs,ys,lw=1.55,label=label)
    ax.set_xlabel("Pedestal-corrected ADC [units]")
    ax.set_ylabel("Fraction of triggered-chamber ADC readings / 8 ADC-unit bin")
    ax.set_xlim(-300,5000)
    ax.set_yscale("log")
    ax.set_ylim(bottom=1e-6)
    ax.grid(alpha=.25)
    ax.legend(fontsize=9)
    ax.set_title("Previously measured MTS-41 ADC responses (different selections)")
    fig.tight_layout()
    dest=Path(a.output)
    dest.parent.mkdir(parents=True,exist_ok=True)
    fig.savefig(dest,dpi=175)
    print("Wrote",dest)


if __name__ == "__main__":
    main()

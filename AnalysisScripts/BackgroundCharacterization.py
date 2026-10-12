#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""
BackgroundCharacterization.py

This script characterizes the Ntrigger=3 populations identified in
using the SAME raw-hit parsing, clusterization, trigger topology, and
three-layer line-compatibility code from HitTopologyAnalysis.py.

It does not assign a particle identity.  It separates events by:

    dataset   : reference / test labels supplied on the command line
    topology  : contiguous / gapped trigger layers
    geometry  : compatible / incompatible / missing_xy

and accumulates:
    - ADC raw and pedestal-corrected distributions per chamber
    - hit multiplicities
    - raw and reconstruction-retained cluster multiplicities/sizes
    - chamber activity/occupancy
    - 1D cluster-position distributions in X and Y
    - a small event-level feature sample for inspection / future ML

Primary comparison:
    Run103 + contiguous + compatible
        vs
    Run113 + gapped + incompatible

The Run113 + gapped + missing_xy population is kept separate because its
cluster incompleteness is part of the selection itself.

Recommended repository location:
    MuographyTracking/AnalysisScripts/BackgroundCharacterization.py

The script expects HitTopologyAnalysis.py to be in the same directory.
"""

import argparse
import csv
import math
import sys
import time
from collections import Counter, defaultdict
from pathlib import Path

# Import logic from the sibling repository utility.
SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

try:
    import HitTopologyAnalysis as ht
except ImportError as exc:
    raise SystemExit(
        "Cannot import HitTopologyAnalysis.py. Put both scripts in "
        "MuographyTracking/AnalysisScripts/."
    ) from exc


# ---------------------------------------------------------------------------
# Additional configuration needed
# ---------------------------------------------------------------------------

def parse_adc_and_hist_config(db_path, ap_path):
    """Read ADC mapping/pedestals/scales and ADC histogram settings."""
    lines = Path(db_path).read_text(errors="replace").splitlines()

    n_adcs = 0
    adc_assign = []
    chamber_adc = []

    for raw in lines:
        s = raw.strip()
        if not s or s.startswith("#"):
            continue
        t = s.split()

        if t[0] == "NAdcs":
            n_adcs = int(t[1])
            try:
                j = t.index("Assign")
                adc_assign = list(map(int, t[j+1:j+1+n_adcs]))
            except ValueError:
                adc_assign = [-1] * n_adcs

        elif t[0] == "ChamberBasics":
            pedestal = 0.0
            scale = 1.0
            name = ""
            i = 1
            while i < len(t):
                key = t[i]
                i += 1
                if key == "End":
                    break
                if key == "Name":
                    name = t[i]; i += 1
                elif key == "Size":
                    i += 2
                elif key in ("PosZ", "PosX", "PosY", "PadSizeX", "PadSizeY"):
                    i += 1
                elif key == "PadSizeXY":
                    i += 2
                elif key == "AdcPedestal":
                    pedestal = float(t[i]); i += 1
                elif key == "AdcScale":
                    scale = float(t[i]); i += 1
                elif key == "SideGrow":
                    i += 2
                elif key == "Flips":
                    nx = int(t[i]); i += 1 + nx
                    ny = int(t[i]); i += 1 + ny
                elif key in ("MisPositionX", "MisPositionY"):
                    i += 3
                elif key in ("BrokenChannelsX", "BrokenChannelsY"):
                    n = int(t[i]); i += 1 + n
                else:
                    raise ValueError(
                        f"Unsupported ChamberBasics token while parsing ADC config: {key}"
                    )
            chamber_adc.append({
                "name": name,
                "pedestal": pedestal,
                "scale": scale,
            })

    if n_adcs > 0 and not adc_assign:
        adc_assign = list(range(n_adcs))

    hist = {
        "AdcMin": -1000.0,
        "AdcMax": 5000.0,
        "AdcBinSize": 8.0,
    }

    if ap_path:
        for raw in Path(ap_path).read_text(errors="replace").splitlines():
            s = raw.strip()
            if not s or s.startswith("#"):
                continue
            t = s.split()
            if len(t) < 2:
                continue
            if t[0] in hist:
                hist[t[0]] = float(t[1])

    return {
        "n_adcs": n_adcs,
        "adc_assign": adc_assign,
        "chamber_adc": chamber_adc,
        **hist,
    }


def parse_adc_values(tokens, adc_cfg, nch):
    """
    Extract raw ADC values from an EBE line and map ADC channels to chambers.

    Mirrors Tracking.cpp AssignData() and ChamberClass::GoodEvent():
        Adc = (AdcRaw - AdcPedestal) * AdcScale
        if AdcRaw == 0: Adc = 0
    """
    n_adcs = adc_cfg["n_adcs"]
    raw_by_ch = [0] * nch
    corrected_by_ch = [0.0] * nch

    if n_adcs <= 0:
        return raw_by_ch, corrected_by_ch

    try:
        i = tokens.index(b"Adc") + 1
    except ValueError:
        # The C++ reader does not verify the marker text, but current EBE
        # examples use "Adc". Treat absence as malformed.
        return None, None

    try:
        vals = [int(v) for v in tokens[i:i+n_adcs]]
    except (ValueError, IndexError):
        return None, None

    if len(vals) != n_adcs:
        return None, None

    for iadc, raw in enumerate(vals):
        if iadc >= len(adc_cfg["adc_assign"]):
            continue
        ch = adc_cfg["adc_assign"][iadc]
        if ch < 0 or ch >= nch:
            continue

        raw_by_ch[ch] = raw

        if ch < len(adc_cfg["chamber_adc"]):
            ped = adc_cfg["chamber_adc"][ch]["pedestal"]
            scale = adc_cfg["chamber_adc"][ch]["scale"]
        else:
            ped, scale = 0.0, 1.0

        corrected_by_ch[ch] = 0.0 if raw == 0 else (raw - ped) * scale

    return raw_by_ch, corrected_by_ch


# ---------------------------------------------------------------------------
# Streaming accumulators
# ---------------------------------------------------------------------------

def group_name(dataset, topology, geometry):
    return f"{dataset}__{topology}__{geometry}"


class GroupAccumulator:
    def __init__(self, nch, adc_min, adc_max, adc_bin):
        self.nch = nch
        self.events = 0

        self.sum_event_hits_x = 0
        self.sum_event_hits_y = 0
        self.sum_active_any = 0
        self.sum_active_xy = 0
        self.sum_extra_active = 0
        self.sum_event_clusters_x = 0
        self.sum_event_clusters_y = 0

        # Raw cluster quantities BEFORE ClusterSizeLimit / NClustersLimit.
        # These are essential for interpreting high-occupancy/missing_xy events.
        self.sum_raw_clusters_x = 0
        self.sum_raw_clusters_y = 0
        self.sum_oversize_axes_x = 0
        self.sum_oversize_axes_y = 0
        self.sum_overlimit_axes_x = 0
        self.sum_overlimit_axes_y = 0

        self.event_hits_x_hist = Counter()
        self.event_hits_y_hist = Counter()
        self.active_any_hist = Counter()
        self.active_xy_hist = Counter()
        self.extra_active_hist = Counter()
        self.event_clusters_x_hist = Counter()
        self.event_clusters_y_hist = Counter()
        self.event_raw_clusters_x_hist = Counter()
        self.event_raw_clusters_y_hist = Counter()
        self.event_oversize_axes_x_hist = Counter()
        self.event_oversize_axes_y_hist = Counter()
        self.event_overlimit_axes_x_hist = Counter()
        self.event_overlimit_axes_y_hist = Counter()

        self.ch = []
        for _ in range(nch):
            self.ch.append({
                "events": 0,
                "triggered": 0,
                "active_x": 0,
                "active_y": 0,
                "active_xy": 0,
                "active_any": 0,
                "sum_hits_x": 0,
                "sum_hits_y": 0,
                "sum_clusters_x": 0,
                "sum_clusters_y": 0,
                "sum_raw_clusters_x": 0,
                "sum_raw_clusters_y": 0,
                "oversize_axis_x": 0,
                "oversize_axis_y": 0,
                "overlimit_axis_x": 0,
                "overlimit_axis_y": 0,
                "sum_adc_raw": 0.0,
                "sum_adc_corr": 0.0,
                "adc_raw_n": 0,
                "adc_corr_n": 0,
            })

        self.cluster_size = defaultdict(Counter)      # retained/reconstruction clusters
        self.raw_cluster_size = defaultdict(Counter)  # before cluster cuts
        self.cluster_pos = defaultdict(Counter)       # retained cluster positions
        self.adc_hist = defaultdict(Counter)          # ch -> adc_bin_index / under / over
        self.adc_triggered_hist = defaultdict(Counter)
        self.adc_active_hist = defaultdict(Counter)

        self.adc_min = adc_min
        self.adc_max = adc_max
        self.adc_bin = adc_bin

    def adc_bin_key(self, value):
        if value < self.adc_min:
            return "UNDER"
        if value >= self.adc_max:
            return "OVER"
        return int(math.floor((value - self.adc_min) / self.adc_bin))

    def add_event(self, pattern, prep, trig_chambers, raw_adc, corr_adc, ap):
        self.events += 1
        trig_set = set(trig_chambers)

        total_hx = sum(len(c["hx"]) for c in prep)
        total_hy = sum(len(c["hy"]) for c in prep)
        active_any = sum(bool(c["hx"] or c["hy"]) for c in prep)
        active_xy = sum(bool(c["hx"] and c["hy"]) for c in prep)
        extra_active = sum(
            i not in trig_set and bool(c["hx"] or c["hy"])
            for i, c in enumerate(prep)
        )
        total_cx = sum(len(c["cx"]) for c in prep)
        total_cy = sum(len(c["cy"]) for c in prep)

        # Reconstruct the raw clusters from the corrected hit lists BEFORE
        # ClusterSizeLimit and NClustersLimit are applied.
        raw_info = []
        total_raw_cx = 0
        total_raw_cy = 0
        oversize_axes_x = 0
        oversize_axes_y = 0
        overlimit_axes_x = 0
        overlimit_axes_y = 0

        for c in prep:
            rcx = ht.clusterize(c["hx"])
            rcy = ht.clusterize(c["hy"])

            total_raw_cx += len(rcx)
            total_raw_cy += len(rcy)

            ox = any(size > ap["ClusterSizeLimit"] for _, size in rcx)
            oy = any(size > ap["ClusterSizeLimit"] for _, size in rcy)

            # NClustersLimit is evaluated AFTER ClusterBreak in the repository.
            pcx = ht.cluster_break(rcx, ap["ClusterSizeLimit"], ap["ClusterBreak"])
            pcy = ht.cluster_break(rcy, ap["ClusterSizeLimit"], ap["ClusterBreak"])
            lx = len(pcx) > ap["NClustersLimit"]
            ly = len(pcy) > ap["NClustersLimit"]

            oversize_axes_x += int(ox)
            oversize_axes_y += int(oy)
            overlimit_axes_x += int(lx)
            overlimit_axes_y += int(ly)

            raw_info.append((rcx, rcy, ox, oy, lx, ly))

        self.sum_event_hits_x += total_hx
        self.sum_event_hits_y += total_hy
        self.sum_active_any += active_any
        self.sum_active_xy += active_xy
        self.sum_extra_active += extra_active
        self.sum_event_clusters_x += total_cx
        self.sum_event_clusters_y += total_cy
        self.sum_raw_clusters_x += total_raw_cx
        self.sum_raw_clusters_y += total_raw_cy
        self.sum_oversize_axes_x += oversize_axes_x
        self.sum_oversize_axes_y += oversize_axes_y
        self.sum_overlimit_axes_x += overlimit_axes_x
        self.sum_overlimit_axes_y += overlimit_axes_y

        self.event_hits_x_hist[total_hx] += 1
        self.event_hits_y_hist[total_hy] += 1
        self.active_any_hist[active_any] += 1
        self.active_xy_hist[active_xy] += 1
        self.extra_active_hist[extra_active] += 1
        self.event_clusters_x_hist[total_cx] += 1
        self.event_clusters_y_hist[total_cy] += 1
        self.event_raw_clusters_x_hist[total_raw_cx] += 1
        self.event_raw_clusters_y_hist[total_raw_cy] += 1
        self.event_oversize_axes_x_hist[oversize_axes_x] += 1
        self.event_oversize_axes_y_hist[oversize_axes_y] += 1
        self.event_overlimit_axes_x_hist[overlimit_axes_x] += 1
        self.event_overlimit_axes_y_hist[overlimit_axes_y] += 1

        for ich, c in enumerate(prep):
            d = self.ch[ich]
            d["events"] += 1

            triggered = ich in trig_set
            ax = bool(c["hx"])
            ay = bool(c["hy"])
            axy = ax and ay
            aa = ax or ay

            rcx, rcy, ox, oy, lx, ly = raw_info[ich]

            d["triggered"] += int(triggered)
            d["active_x"] += int(ax)
            d["active_y"] += int(ay)
            d["active_xy"] += int(axy)
            d["active_any"] += int(aa)
            d["sum_hits_x"] += len(c["hx"])
            d["sum_hits_y"] += len(c["hy"])
            d["sum_clusters_x"] += len(c["cx"])
            d["sum_clusters_y"] += len(c["cy"])
            d["sum_raw_clusters_x"] += len(rcx)
            d["sum_raw_clusters_y"] += len(rcy)
            d["oversize_axis_x"] += int(ox)
            d["oversize_axis_y"] += int(oy)
            d["overlimit_axis_x"] += int(lx)
            d["overlimit_axis_y"] += int(ly)

            if raw_adc is not None:
                d["sum_adc_raw"] += raw_adc[ich]
                d["adc_raw_n"] += 1
            if corr_adc is not None:
                d["sum_adc_corr"] += corr_adc[ich]
                d["adc_corr_n"] += 1

                k = self.adc_bin_key(corr_adc[ich])
                self.adc_hist[ich][k] += 1
                if triggered:
                    self.adc_triggered_hist[ich][k] += 1
                if aa:
                    self.adc_active_hist[ich][k] += 1

            for center, size in rcx:
                self.raw_cluster_size[(ich, "X")][int(size)] += 1
            for center, size in rcy:
                self.raw_cluster_size[(ich, "Y")][int(size)] += 1

            for gpos, size, local in c["cx"]:
                self.cluster_size[(ich, "X")][int(size)] += 1
                self.cluster_pos[(ich, "X")][int(round(local * 2.0))] += 1

            for gpos, size, local in c["cy"]:
                self.cluster_size[(ich, "Y")][int(size)] += 1
                self.cluster_pos[(ich, "Y")][int(round(local * 2.0))] += 1



# ---------------------------------------------------------------------------
# Analysis
# ---------------------------------------------------------------------------

def classify_geometry(fx, fy, chi2_max):
    if fx is None or fy is None:
        return "missing_xy"
    if fx[0] <= chi2_max and fy[0] <= chi2_max:
        return "compatible"
    return "incompatible"


def analyze_dataset(
    dataset,
    files,
    det,
    ap,
    adc_cfg,
    ntrig,
    sample_every,
    resolution_mm,
    chi2_max,
    progress_every,
    save_event_every,
    event_writer,
):
    groups = {}
    selected = 0
    analyzed = 0
    scanned = 0
    adc_bad = 0
    ch_bad = 0
    t0 = time.time()

    ht.warn_parts(dataset, files)
    print(f"\n=== {dataset} ===")

    for ifile, fn in enumerate(files, 1):
        print(f"[{ifile}/{len(files)}] {fn}", flush=True)

        for source, lineno, line in ht.lines_from(fn):
            scanned += 1
            tokens, pattern = ht.parse_p_and_tokens(line, det["nbits"])
            if pattern is None:
                continue

            nt = pattern.bit_count() if hasattr(pattern, "bit_count") else bin(pattern).count("1")
            if nt != ntrig:
                continue

            selected += 1
            if sample_every > 1 and (selected - 1) % sample_every != 0:
                continue

            analyzed += 1

            raw_ch = ht.parse_ch_payload(tokens, det)
            if raw_ch is None:
                ch_bad += 1
                continue

            prep = ht.prepare(raw_ch, det, ap)
            trig = ht.trig_ch(pattern, det)

            fx = ht.best_fit(trig, prep, det, "x", resolution_mm)
            fy = ht.best_fit(trig, prep, det, "y", resolution_mm)

            topology = "contiguous" if ht.contiguous(pattern, det) else "gapped"
            geometry = classify_geometry(fx, fy, chi2_max)
            gkey = (dataset, topology, geometry)

            if gkey not in groups:
                groups[gkey] = GroupAccumulator(
                    det["nch"],
                    adc_cfg["AdcMin"],
                    adc_cfg["AdcMax"],
                    adc_cfg["AdcBinSize"],
                )

            raw_adc, corr_adc = parse_adc_values(tokens, adc_cfg, det["nch"])
            if raw_adc is None:
                adc_bad += 1

            groups[gkey].add_event(
                pattern, prep, trig,
                raw_adc, corr_adc, ap
            )

            if (
                event_writer is not None
                and save_event_every > 0
                and (analyzed - 1) % save_event_every == 0
            ):
                trig_set = set(trig)
                event_writer.writerow({
                    "dataset": dataset,
                    "source": source,
                    "line": lineno,
                    "event_id": tokens[0].decode(errors="replace"),
                    "pattern_dec": pattern,
                    "pattern_bin": format(pattern, f"0{det['nbits']}b"),
                    "topology": topology,
                    "geometry": geometry,
                    "chi2x": "" if fx is None else fx[0],
                    "chi2y": "" if fy is None else fy[0],
                    "hits_x_total": sum(len(c["hx"]) for c in prep),
                    "hits_y_total": sum(len(c["hy"]) for c in prep),
                    "clusters_x_total": sum(len(c["cx"]) for c in prep),
                    "clusters_y_total": sum(len(c["cy"]) for c in prep),
                    "raw_clusters_x_total": sum(len(ht.clusterize(c["hx"])) for c in prep),
                    "raw_clusters_y_total": sum(len(ht.clusterize(c["hy"])) for c in prep),
                    "oversize_axes_x": sum(
                        any(s > ap["ClusterSizeLimit"] for _, s in ht.clusterize(c["hx"]))
                        for c in prep
                    ),
                    "oversize_axes_y": sum(
                        any(s > ap["ClusterSizeLimit"] for _, s in ht.clusterize(c["hy"]))
                        for c in prep
                    ),
                    "overlimit_axes_x": sum(
                        len(ht.cluster_break(ht.clusterize(c["hx"]), ap["ClusterSizeLimit"], ap["ClusterBreak"]))
                        > ap["NClustersLimit"]
                        for c in prep
                    ),
                    "overlimit_axes_y": sum(
                        len(ht.cluster_break(ht.clusterize(c["hy"]), ap["ClusterSizeLimit"], ap["ClusterBreak"]))
                        > ap["NClustersLimit"]
                        for c in prep
                    ),
                    "active_any": sum(bool(c["hx"] or c["hy"]) for c in prep),
                    "active_xy": sum(bool(c["hx"] and c["hy"]) for c in prep),
                    "extra_active": sum(
                        i not in trig_set and bool(c["hx"] or c["hy"])
                        for i, c in enumerate(prep)
                    ),
                    "adc_corr_sum": "" if corr_adc is None else sum(corr_adc),
                    "adc_corr_triggered_sum": "" if corr_adc is None else sum(corr_adc[i] for i in trig),
                    "adc_corr_max": "" if corr_adc is None else max(corr_adc),
                })

            if progress_every and analyzed % progress_every == 0:
                dt = max(time.time() - t0, 1e-9)
                print(
                    f"  analyzed={analyzed:,} Ntrig={ntrig} events; "
                    f"scanned={scanned:,}; {scanned/dt:,.0f} lines/s",
                    flush=True,
                )

    dt = time.time() - t0
    print(
        f"{dataset}: scanned={scanned:,}, Ntrig={ntrig}={selected:,}, "
        f"analyzed={analyzed:,}, bad Ch={ch_bad:,}, bad ADC={adc_bad:,}, "
        f"elapsed={dt/60:.1f} min"
    )

    return groups


# ---------------------------------------------------------------------------
# CSV writers
# ---------------------------------------------------------------------------

def write_class_summary(path, groups):
    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "dataset", "topology", "geometry", "events",
            "mean_hits_x", "mean_hits_y",
            "mean_clusters_x", "mean_clusters_y",
            "mean_raw_clusters_x", "mean_raw_clusters_y",
            "mean_oversize_axes_x", "mean_oversize_axes_y",
            "mean_overlimit_axes_x", "mean_overlimit_axes_y",
            "mean_active_any", "mean_active_xy",
            "mean_extra_active",
        ])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            n = g.events
            if not n:
                continue
            w.writerow([
                dataset, topology, geometry, n,
                g.sum_event_hits_x / n,
                g.sum_event_hits_y / n,
                g.sum_event_clusters_x / n,
                g.sum_event_clusters_y / n,
                g.sum_raw_clusters_x / n,
                g.sum_raw_clusters_y / n,
                g.sum_oversize_axes_x / n,
                g.sum_oversize_axes_y / n,
                g.sum_overlimit_axes_x / n,
                g.sum_overlimit_axes_y / n,
                g.sum_active_any / n,
                g.sum_active_xy / n,
                g.sum_extra_active / n,
            ])


def write_chamber_summary(path, groups, det):
    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "dataset", "topology", "geometry",
            "chamber_index", "chamber_name", "z",
            "events",
            "trigger_fraction",
            "active_x_fraction", "active_y_fraction",
            "active_xy_fraction", "active_any_fraction",
            "mean_hits_x", "mean_hits_y",
            "mean_clusters_x", "mean_clusters_y",
            "mean_raw_clusters_x", "mean_raw_clusters_y",
            "oversize_axis_x_fraction", "oversize_axis_y_fraction",
            "overlimit_axis_x_fraction", "overlimit_axis_y_fraction",
            "mean_adc_raw", "mean_adc_corrected",
        ])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            for ich, d in enumerate(g.ch):
                n = d["events"]
                if not n:
                    continue
                cfg = det["ch"][ich]
                w.writerow([
                    dataset, topology, geometry,
                    ich, cfg["name"], cfg["z"], n,
                    d["triggered"] / n,
                    d["active_x"] / n,
                    d["active_y"] / n,
                    d["active_xy"] / n,
                    d["active_any"] / n,
                    d["sum_hits_x"] / n,
                    d["sum_hits_y"] / n,
                    d["sum_clusters_x"] / n,
                    d["sum_clusters_y"] / n,
                    d["sum_raw_clusters_x"] / n,
                    d["sum_raw_clusters_y"] / n,
                    d["oversize_axis_x"] / n,
                    d["oversize_axis_y"] / n,
                    d["overlimit_axis_x"] / n,
                    d["overlimit_axis_y"] / n,
                    d["sum_adc_raw"] / d["adc_raw_n"] if d["adc_raw_n"] else "",
                    d["sum_adc_corr"] / d["adc_corr_n"] if d["adc_corr_n"] else "",
                ])


def write_integer_hists(path, groups):
    kinds = [
        ("hits_x_total", "event_hits_x_hist"),
        ("hits_y_total", "event_hits_y_hist"),
        ("clusters_x_total", "event_clusters_x_hist"),
        ("clusters_y_total", "event_clusters_y_hist"),
        ("raw_clusters_x_total", "event_raw_clusters_x_hist"),
        ("raw_clusters_y_total", "event_raw_clusters_y_hist"),
        ("oversize_axes_x", "event_oversize_axes_x_hist"),
        ("oversize_axes_y", "event_oversize_axes_y_hist"),
        ("overlimit_axes_x", "event_overlimit_axes_x_hist"),
        ("overlimit_axes_y", "event_overlimit_axes_y_hist"),
        ("active_any", "active_any_hist"),
        ("active_xy", "active_xy_hist"),
        ("extra_active", "extra_active_hist"),
    ]

    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["dataset", "topology", "geometry", "quantity", "value", "count"])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            for quantity, attr in kinds:
                counter = getattr(g, attr)
                for value, count in sorted(counter.items()):
                    w.writerow([dataset, topology, geometry, quantity, value, count])


def write_cluster_size_hists(path, groups, det):
    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "dataset", "topology", "geometry",
            "chamber_index", "chamber_name",
            "axis", "cluster_size", "count",
        ])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            for (ich, axis), counter in sorted(g.cluster_size.items()):
                for size, count in sorted(counter.items()):
                    w.writerow([
                        dataset, topology, geometry,
                        ich, det["ch"][ich]["name"],
                        axis, size, count,
                    ])



def write_raw_cluster_size_hists(path, groups, det):
    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "dataset", "topology", "geometry",
            "chamber_index", "chamber_name",
            "axis", "raw_cluster_size", "count",
        ])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            for (ich, axis), counter in sorted(g.raw_cluster_size.items()):
                for size, count in sorted(counter.items()):
                    w.writerow([
                        dataset, topology, geometry,
                        ich, det["ch"][ich]["name"],
                        axis, size, count,
                    ])


def write_cluster_position_hists(path, groups, det):
    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "dataset", "topology", "geometry",
            "chamber_index", "chamber_name", "z",
            "axis", "local_center", "global_center_mm", "count",
        ])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            for (ich, axis), counter in sorted(g.cluster_pos.items()):
                cfg = det["ch"][ich]
                pad = cfg["padx"] if axis == "X" else cfg["pady"]
                pos0 = cfg["px"] if axis == "X" else cfg["py"]

                for twice_center, count in sorted(counter.items()):
                    local = twice_center / 2.0
                    global_mm = (local + 0.5) * pad + pos0
                    w.writerow([
                        dataset, topology, geometry,
                        ich, cfg["name"], cfg["z"],
                        axis, local, global_mm, count,
                    ])


def adc_bin_description(key, g):
    if key == "UNDER":
        return "", g.adc_min, f"<{g.adc_min:g}"
    if key == "OVER":
        return g.adc_max, "", f">={g.adc_max:g}"

    low = g.adc_min + int(key) * g.adc_bin
    high = low + g.adc_bin
    return low, high, ""


def write_adc_hists(path, groups, det):
    scopes = [
        ("all", "adc_hist"),
        ("triggered_chamber", "adc_triggered_hist"),
        ("hit_active_chamber", "adc_active_hist"),
    ]

    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "dataset", "topology", "geometry",
            "chamber_index", "chamber_name",
            "scope",
            "bin_low", "bin_high", "special_bin", "count",
        ])

        for (dataset, topology, geometry), g in sorted(groups.items()):
            for scope, attr in scopes:
                hist_by_ch = getattr(g, attr)
                for ich, counter in sorted(hist_by_ch.items()):
                    def sort_key(x):
                        if x == "UNDER":
                            return -10**18
                        if x == "OVER":
                            return 10**18
                        return int(x)

                    for key in sorted(counter, key=sort_key):
                        low, high, special = adc_bin_description(key, g)
                        w.writerow([
                            dataset, topology, geometry,
                            ich, det["ch"][ich]["name"],
                            scope,
                            low, high, special, counter[key],
                        ])


def print_primary(groups, ref_label, test_label):
    print("\n=== Primary populations ===")

    primary = [
        (ref_label, "contiguous", "compatible",
         "muon-like reference"),
        (test_label, "gapped", "incompatible",
         "background-enriched, testable"),
        (test_label, "gapped", "missing_xy",
         "background-enriched, incomplete X/Y"),
        (test_label, "contiguous", "compatible",
         "Run113 muon-like control"),
    ]

    for dataset, topology, geometry, desc in primary:
        g = groups.get((dataset, topology, geometry))
        if g is None:
            print(f"{desc:36s}: 0")
            continue
        print(
            f"{desc:36s}: {g.events:,} events; "
            f"<NhitsX>={g.sum_event_hits_x/g.events:.2f}, "
            f"<NhitsY>={g.sum_event_hits_y/g.events:.2f}, "
            f"<NextraActive>={g.sum_extra_active/g.events:.2f}"
        )


def main():
    ap = argparse.ArgumentParser(
        description="Background characterization for MuographyTracking"
    )

    ap.add_argument("-b", "--basic-info", required=True)
    ap.add_argument("-ap", "--analysis-parameters", default=None)

    ap.add_argument("--reference", nargs="+", required=True)
    ap.add_argument("--test", nargs="+", required=True)
    ap.add_argument("--reference-label", default="Run103")
    ap.add_argument("--test-label", default="Run113")

    ap.add_argument("--ntrig", type=int, default=3)

    ap.add_argument(
        "--sample-every", type=int, default=100,
        help="Analyze every Nth selected Ntrigger event. 100 = 1%% exploratory; 1 = full."
    )

    ap.add_argument("--resolution-mm", type=float, default=3.6)
    ap.add_argument("--chi2-max", type=float, default=None)

    ap.add_argument(
        "--save-event-every", type=int, default=100,
        help="Save one event-level feature row every N analyzed events; 0 disables."
    )

    ap.add_argument("--progress-every", type=int, default=100000)

    ap.add_argument("-r", "--results", default="task3")
    ap.add_argument("-rd", "--dir-results", default="Results/Task3")

    args = ap.parse_args()

    det = ht.parse_db(args.basic_info)
    apar = ht.parse_ap(args.analysis_parameters)
    adc_cfg = parse_adc_and_hist_config(
        args.basic_info, args.analysis_parameters
    )

    chi2_max = (
        args.chi2_max
        if args.chi2_max is not None
        else apar["TrackKhi2Max"]
    )

    ref_files = ht.expand(args.reference)
    test_files = ht.expand(args.test)

    if not ref_files:
        raise SystemExit("No reference files matched.")
    if not test_files:
        raise SystemExit("No test files matched.")

    outdir = Path(args.dir_results)
    outdir.mkdir(parents=True, exist_ok=True)

    print("Configuration:")
    print(f"  Ntrigger      = {args.ntrig}")
    print(f"  sample_every  = {args.sample_every}")
    print(f"  resolution    = {args.resolution_mm} mm")
    print(f"  chi2_max      = {chi2_max}")
    print(
        f"  ADC histogram = [{adc_cfg['AdcMin']}, {adc_cfg['AdcMax']}) "
        f"in {adc_cfg['AdcBinSize']} ADC bins"
    )

    event_path = outdir / f"{args.results}_event_features_sample.csv"
    event_fields = [
        "dataset", "source", "line", "event_id",
        "pattern_dec", "pattern_bin",
        "topology", "geometry",
        "chi2x", "chi2y",
        "hits_x_total", "hits_y_total",
        "clusters_x_total", "clusters_y_total",
        "raw_clusters_x_total", "raw_clusters_y_total",
        "oversize_axes_x", "oversize_axes_y",
        "overlimit_axes_x", "overlimit_axes_y",
        "active_any", "active_xy", "extra_active",
        "adc_corr_sum", "adc_corr_triggered_sum", "adc_corr_max",
    ]

    with open(event_path, "w", newline="") as event_fh:
        event_writer = csv.DictWriter(event_fh, fieldnames=event_fields)
        event_writer.writeheader()

        ref_groups = analyze_dataset(
            args.reference_label, ref_files, det, apar, adc_cfg,
            args.ntrig, args.sample_every,
            args.resolution_mm, chi2_max,
            args.progress_every,
            args.save_event_every,
            event_writer,
        )

        test_groups = analyze_dataset(
            args.test_label, test_files, det, apar, adc_cfg,
            args.ntrig, args.sample_every,
            args.resolution_mm, chi2_max,
            args.progress_every,
            args.save_event_every,
            event_writer,
        )

    groups = {}
    groups.update(ref_groups)
    groups.update(test_groups)

    write_class_summary(
        outdir / f"{args.results}_class_summary.csv",
        groups
    )
    write_chamber_summary(
        outdir / f"{args.results}_chamber_summary.csv",
        groups, det
    )
    write_integer_hists(
        outdir / f"{args.results}_event_multiplicity_hist.csv",
        groups
    )
    write_cluster_size_hists(
        outdir / f"{args.results}_cluster_size_hist.csv",
        groups, det
    )
    write_raw_cluster_size_hists(
        outdir / f"{args.results}_raw_cluster_size_hist.csv",
        groups, det
    )
    write_cluster_position_hists(
        outdir / f"{args.results}_cluster_position_hist.csv",
        groups, det
    )
    write_adc_hists(
        outdir / f"{args.results}_adc_hist.csv",
        groups, det
    )

    print_primary(groups, args.reference_label, args.test_label)

    print("\nWrote:")
    for fn in [
        f"{args.results}_class_summary.csv",
        f"{args.results}_chamber_summary.csv",
        f"{args.results}_event_multiplicity_hist.csv",
        f"{args.results}_cluster_size_hist.csv",
        f"{args.results}_raw_cluster_size_hist.csv",
        f"{args.results}_cluster_position_hist.csv",
        f"{args.results}_adc_hist.csv",
        f"{args.results}_event_features_sample.csv",
    ]:
        print(f"  {outdir / fn}")

    print("\n Note:")
    print(
        "Do not interpret 'background-enriched' as a particle identity. "
        "The first comparison is detector-response morphology: ADC, hit/cluster "
        "multiplicity, chamber occupancy, and spatial distributions."
    )


if __name__ == "__main__":
    main()

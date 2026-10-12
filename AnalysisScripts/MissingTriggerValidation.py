#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Test for geometrically aligned hits in a missing trigger layer.

For events with exactly three trigger bits spanning four adjacent z-ordered
chambers (one internal missing trigger), fit X(z) and Y(z) using ONLY the
three triggered layers. Among line-compatible fits, extrapolate to the
non-triggered intermediate chamber and look for nearby X and Y clusters.

A within-trigger-pattern shuffled-event control gives the chance alignment
rate, preserving the observed 2D strip-cluster content of the test layer.
This does NOT measure gamma purity or prove a muon/missed trigger.
"""
import argparse
import csv
import math
import random
import sys
from collections import defaultdict
from pathlib import Path

# Import a bundled, version-pinned copy of the previously validated decoder.
# This avoids silently using a different copy in the user's working tree.
sys.path.insert(0, str(Path(__file__).parent / '_missing_trigger_support'))
import HitTopologyAnalysis as ht


def newstat():
    return dict(selected=0, unparsed=0, testable=0, line_compatible=0, compatible=0,
                missing_raw_any=0, missing_raw_xy=0,
                missing_cluster_xy=0, match_x=0, match_y=0, match_xy=0)


def match(centers, expected_strip, radius):
    return any(abs(c[2] - expected_strip) <= radius for c in centers)


def missing_internal(pattern, detector):
    chambers = ht.trig_ch(pattern, detector)
    if len(chambers) != 3 or len(set(chambers)) != 3:
        return None
    zorder = sorted(range(detector['nch']), key=lambda c: detector['ch'][c]['z'])
    rank = {c: i for i, c in enumerate(zorder)}
    rr = sorted(rank[c] for c in chambers)
    if rr[-1] - rr[0] != 3:
        return None
    missing = sorted(set(range(rr[0], rr[-1] + 1)) - set(rr))
    return (chambers, zorder[missing[0]]) if len(missing) == 1 else None


def write_csv(path, rows, fields):
    with open(path, 'w', newline='') as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)


def analyze(args):
    if args.sample_mod < 1 or args.match_window_strips <= 0:
        raise SystemExit('sample-mod and match-window-strips must be positive')
    if args.null_repeats < 1 or args.max_null_pool < 2:
        raise SystemExit('null-repeats >=1 and max-null-pool >=2 required')

    det = ht.parse_db(args.detector_db)
    ap = ht.parse_ap(args.analysis_parameters)
    cut = args.chi2_max if args.chi2_max is not None else ap['TrackKhi2Max']
    files = ht.expand(args.ebe)
    if not files:
        raise SystemExit('No EBE files matched --ebe argument')
    ht.warn_parts(args.label, files)
    print(f'[{args.label}] Files: {len(files)}, sample event ID % {args.sample_mod} == 0', flush=True)
    print(f'[{args.label}] Fit cuts: both chi2/ndf <= {cut:g}; match radius {args.match_window_strips:g} strips', flush=True)
    print('Only Ntrigger=3, physical z-span=4, one internal missing trigger', flush=True)

    by_pattern = defaultdict(newstat)
    by_missing = defaultdict(newstat)
    grand = newstat()
    pools = defaultdict(list)
    pool_nseen = defaultdict(int)
    examples = []
    rng = random.Random(args.seed)
    scanned = 0
    for file_idx, fn in enumerate(files, 1):
        print(f'  [{file_idx}/{len(files)}] {fn}', flush=True)
        for _, line_no, line in ht.lines_from(fn):
            scanned += 1
            if args.max_scanned and scanned > args.max_scanned:
                break
            tok, pat = ht.parse_p_and_tokens(line, det['nbits'])
            if pat is None or pat.bit_count() != 3:
                continue
            candidate = missing_internal(pat, det)
            if candidate is None:
                continue
            try:
                event_id = int(tok[0])
            except (ValueError, IndexError):
                continue
            if event_id % args.sample_mod:
                continue
            trigs, missing = candidate
            keys = (grand, by_pattern[pat], by_missing[missing])
            for d in keys:
                d['selected'] += 1
            raw = ht.parse_ch_payload(tok, det)
            if raw is None:
                for d in keys:
                    d['unparsed'] += 1
                continue
            prep = ht.prepare(raw, det, ap)
            fx = ht.best_fit(trigs, prep, det, 'x', args.resolution_mm)
            fy = ht.best_fit(trigs, prep, det, 'y', args.resolution_mm)
            if fx is None or fy is None:
                continue
            for d in keys:
                d['testable'] += 1
            if not (fx[0] <= cut and fy[0] <= cut):
                continue
            for d in keys:
                d['line_compatible'] += 1
            cfg = det['ch'][missing]
            z = cfg['z']
            ex_global = fx[1] * z + fx[2]
            ey_global = fy[1] * z + fy[2]
            # Inverse of the coordinate transform used by the reference analysis.
            ex = (ex_global - cfg['px']) / cfg['padx'] - 0.5
            ey = (ey_global - cfg['py']) / cfg['pady'] - 0.5
            # Respect the target chamber's active-area crossing criterion:
            # local cluster/strip coordinate strictly between 0 and Size-1.
            if not (0 < ex < cfg['sx'] - 1 and 0 < ey < cfg['sy'] - 1):
                continue
            for d in keys:
                d['compatible'] += 1
            xs, ys = raw[missing]
            cx, cy = prep[missing]['cx'], prep[missing]['cy']
            mx = match(cx, ex, args.match_window_strips)
            my = match(cy, ey, args.match_window_strips)
            counts = dict(missing_raw_any=bool(xs or ys), missing_raw_xy=bool(xs and ys),
                          missing_cluster_xy=bool(cx and cy), match_x=mx, match_y=my, match_xy=mx and my)
            for d in keys:
                for key, val in counts.items():
                    d[key] += int(val)
            # Reservoir sampling, stratified by the EXACT 3-trigger mask.
            pool_nseen[pat] += 1
            pool = pools[pat]
            entry = (ex, ey, tuple(v[2] for v in cx), tuple(v[2] for v in cy))
            if len(pool) < args.max_null_pool:
                pool.append(entry)
            else:
                index = rng.randrange(pool_nseen[pat])
                if index < args.max_null_pool:
                    pool[index] = entry
            if len(examples) < args.examples_limit:
                examples.append(dict(run=args.label, event_id=event_id, source=fn, source_line=line_no,
                                     trigger_mask=pat, missing_chamber=missing,
                                     chi2_x=fx[0], chi2_y=fy[0], pred_x_strip=round(ex, 3),
                                     pred_y_strip=round(ey, 3), raw_x=len(xs), raw_y=len(ys),
                                     match_x=int(mx), match_y=int(my), match_xy=int(mx and my),
                                     missing_x_cluster_centers=' '.join(f'{v[2]:g}' for v in cx),
                                     missing_y_cluster_centers=' '.join(f'{v[2]:g}' for v in cy)))
            if args.progress and grand['selected'] % args.progress == 0:
                print(f'    Selected {grand["selected"]:,}; line-compatible {grand["compatible"]:,}; scanned {scanned:,}', flush=True)
        if args.max_scanned and scanned > args.max_scanned:
            break

    null = {}
    for pattern, pool in pools.items():
        n = len(pool)
        if n < 2:
            null[pattern] = (None, None, n)
            continue
        rates = []
        for _ in range(args.null_repeats):
            # A circular derangement: each tested trajectory receives X/Y
            # clusters from another event with the SAME trigger pattern.
            offset = rng.randint(1, n - 1)
            nmatch = 0
            for i, (ex, ey, _, _) in enumerate(pool):
                _, _, cx, cy = pool[(i + offset) % n]
                if any(abs(x - ex) <= args.match_window_strips for x in cx) and any(
                    abs(y - ey) <= args.match_window_strips for y in cy):
                    nmatch += 1
            rates.append(nmatch / n)
        mean = sum(rates) / len(rates)
        sd = math.sqrt(sum((x - mean) ** 2 for x in rates) / (len(rates) - 1)) if len(rates) > 1 else 0.0
        null[pattern] = (mean, sd, n)

    out = Path(args.outdir)
    out.mkdir(parents=True, exist_ok=True)
    fields = ['run','scope','pattern','pattern_bin','triggered_chambers','missing_chamber','selected','unparsed',
              'testable','line_compatible','compatible','missing_raw_any','missing_raw_xy',
              'missing_cluster_xy','match_x','match_y','match_xy',
              'match_fraction_given_compatible','null_match_fraction','null_repeat_sd',
              'null_pool_size','observed_minus_null']
    rows = []
    for pattern in sorted(by_pattern):
        d = by_pattern[pattern]
        trigs, missing = missing_internal(pattern, det)
        rate, sd, n = null.get(pattern, (None, None, 0))
        obs = d['match_xy'] / d['compatible'] if d['compatible'] else None
        row = dict(run=args.label, scope='pattern', pattern=pattern,
                   pattern_bin=format(pattern, f'0{det["nbits"]}b'),
                   triggered_chambers=' '.join(map(str, trigs)), missing_chamber=missing, **d,
                   match_fraction_given_compatible=obs,
                   null_match_fraction=rate, null_repeat_sd=sd, null_pool_size=n,
                   observed_minus_null=(obs - rate if obs is not None and rate is not None else None))
        rows.append(row)
    write_csv(out / f'{args.label}_missing_trigger_by_pattern.csv', rows, fields)

    chamber_rows = []
    for missing, d in sorted(by_missing.items()):
        chamber_rows.append(dict(run=args.label, missing_chamber=missing,
                                 chamber_name=det['ch'][missing]['name'], **d,
                                 match_fraction_given_compatible=(d['match_xy']/d['compatible'] if d['compatible'] else None)))
    write_csv(out / f'{args.label}_missing_trigger_by_chamber.csv', chamber_rows,
              ['run','missing_chamber','chamber_name', *newstat().keys(), 'match_fraction_given_compatible'])

    valid = [r for r in rows if r['null_match_fraction'] is not None]
    weight = sum(r['compatible'] for r in valid)
    expected = sum(r['compatible'] * r['null_match_fraction'] for r in valid)
    total_null = expected / weight if weight else None
    obs = grand['match_xy'] / grand['compatible'] if grand['compatible'] else None
    summary = dict(run=args.label, files=len(files), scanned=scanned, sample_mod=args.sample_mod,
                   chi2_max=cut, resolution_mm=args.resolution_mm,
                   match_window_strips=args.match_window_strips, **grand,
                   match_fraction_given_compatible=obs,
                   null_match_fraction_weighted=total_null,
                   null_expected_matches=expected if weight else None,
                   compatible_events_with_null=weight,
                   observed_minus_null=(obs-total_null if obs is not None and total_null is not None else None),
                   note='Counts conditional on line-compatible 3-trigger span-4 events; not muon or gamma purity')
    write_csv(out / f'{args.label}_missing_trigger_summary.csv', [summary], list(summary))
    ef = ['run','event_id','source','source_line','trigger_mask','missing_chamber','chi2_x','chi2_y',
          'pred_x_strip','pred_y_strip','raw_x','raw_y','match_x','match_y','match_xy',
          'missing_x_cluster_centers','missing_y_cluster_centers']
    write_csv(out / f'{args.label}_missing_trigger_examples.csv', examples, ef)
    print(f'[{args.label}] Scanned {scanned:,} events; sampled span-4 3-trigger {grand["selected"]:,}', flush=True)
    print(f'[{args.label}] Testable: {grand["testable"]:,}; line-compatible: {grand["line_compatible"]:,}; inside target chamber: {grand["compatible"]:,}', flush=True)
    print(f'[{args.label}] Aligned XY in non-triggered intermediate chamber: {grand["match_xy"]:,}', flush=True)
    print(f'[{args.label}] Observed conditional fraction: {obs}', flush=True)
    print(f'[{args.label}] Shuffled null conditional fraction: {total_null}', flush=True)
    print(f'[{args.label}] Results in {out}', flush=True)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    p.add_argument('--label', default='Run113')
    p.add_argument('-b', '--detector-db', required=True)
    p.add_argument('-ap', '--analysis-parameters', required=True)
    p.add_argument('--ebe', nargs='+', required=True)
    p.add_argument('--sample-mod', type=int, default=20, help='Event ID modulo sampling; 20 = 5 percent')
    p.add_argument('--max-scanned', type=int, default=0, help='Optional quick-test limit on raw lines; 0 = all')
    p.add_argument('--resolution-mm', type=float, default=3.6)
    p.add_argument('--chi2-max', type=float, default=None, help='Default follows .ap TrackKhi2Max')
    p.add_argument('--match-window-strips', type=float, default=2.6,
                   help='Maximum |predicted position minus cluster center|, in strips')
    p.add_argument('--max-null-pool', type=int, default=5000)
    p.add_argument('--null-repeats', type=int, default=20)
    p.add_argument('--examples-limit', type=int, default=30)
    p.add_argument('--seed', type=int, default=20261012)
    p.add_argument('--progress', type=int, default=100000)
    p.add_argument('-o', '--outdir', default='Results')
    analyze(p.parse_args())


if __name__ == '__main__':
    main()

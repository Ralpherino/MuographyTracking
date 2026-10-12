#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Mixed-event accidental 3-layer compatibility null.

Requires HitTopologyAnalysis.py in the same directory or available through
PYTHONPATH. For each Ntrigger=3 trigger pattern in each run:
  * deterministically thin events with --sample-every;
  * count observed fully-testable X/Y 3-layer candidates and line-compatible ones;
  * build a random reservoir of *testable* events per trigger pattern;
  * form synthetic events by taking chamber 1 from one event, chamber 2
    from another, and chamber 3 from a third distinct event;
  * preserve each chamber's paired X/Y cluster lists and all chamber-specific
    multiplicity/position distributions; break between-chamber alignment;
  * apply EXACT SAME least-squares geometry test.

A mixed-event rate measures a conditional accidental-alignment baseline under
independent layers. It is NOT a gamma fraction or proof of particle identity.
Do not mix runs. Do not mix trigger patterns. Do not mix testable and
untestable events. The Monte Carlo uncertainty is conditional on the stored
reservoir; reservoir sampling/systematics require independent checks.
"""
import argparse
import csv
import math
import random
import sys
from collections import defaultdict
from pathlib import Path

try:
    import HitTopologyAnalysis as ht
except ImportError as e:
    raise SystemExit('HitTopologyAnalysis.py must be in the same folder or on PYTHONPATH') from e


def passing(fx, fy, cut):
    return fx is not None and fy is not None and fx[0] <= cut and fy[0] <= cut


def classify_event(pat, prepared, det, res, cut):
    fired=ht.trig_ch(pat,det)
    fx=ht.best_fit(fired,prepared,det,'x',res)
    fy=ht.best_fit(fired,prepared,det,'y',res)
    return fx is not None and fy is not None, passing(fx,fy,cut)


def summarize_one_run(label, paths, det, apar, sample_every, max_pool, trials,
                      seed, resolution, cut, progress=0):
    rng=random.Random(seed)
    counters=defaultdict(lambda:dict(seen=0,testable=0,compatible=0,pool=[]))
    event_total=0;selected=0;selected_sampled=0;bad_ch=0
    ht.warn_parts(label,paths)
    print('\n---',label,'---',flush=True)
    for ifile,fn in enumerate(paths,1):
        print(f'[{ifile}/{len(paths)}] {fn}',flush=True)
        for src,line_num,line in ht.lines_from(fn):
            event_total+=1
            tokens,pat=ht.parse_p_and_tokens(line,det['nbits'])
            if pat is None or pat.bit_count()!=3:continue
            selected+=1
            if sample_every>1 and (selected-1)%sample_every!=0:continue
            selected_sampled+=1
            raw=ht.parse_ch_payload(tokens,det)
            if raw is None:
                bad_ch+=1;continue
            prep=ht.prepare(raw,det,apar)
            trigs=ht.trig_ch(pat,det)
            d=counters[pat];d['seen']+=1
            usable,ok=classify_event(pat,prep,det,resolution,cut)
            if usable:
                d['testable']+=1
                if ok:d['compatible']+=1
                # Save per-chamber *pairs* of X and Y cluster arrays.
                # `best_fit` needs only global positions and local centers;
                # retain the original tuple (global, size, center).
                record=tuple((prep[c]['cx'],prep[c]['cy']) for c in trigs)
                pool=d['pool']
                if len(pool)<max_pool:pool.append(record)
                else:
                    z=rng.randrange(d['testable'])
                    if z<max_pool:pool[z]=record
            if progress and selected_sampled%progress==0:
                print(f'  sampled {selected_sampled:,}, tested {sum(v["testable"] for v in counters.values()):,}',flush=True)

    if bad_ch:
        print(f'WARNING: {bad_ch} sampled selected events lacked parseable Ch; excluded',file=sys.stderr)
    print(f'Parsed {event_total:,} lines, Ntrig3={selected:,}, sampled={selected_sampled:,}, unparsed Ch={bad_ch:,}',flush=True)
    rows=[]; summary=defaultdict(lambda:dict(patterns=0,observed_testable=0,observed_compatible=0,
                                              mixed_trials=0,mixed_success=0,weighted_expected=0.0,weight_total=0))
    for pat in sorted(counters):
        d=counters[pat];pool=d['pool'];n=len(pool)
        topo='contiguous' if ht.contiguous(pat,det) else 'gapped'
        successes=0;nt=0
        if n>=3:
            firing=ht.trig_ch(pat,det)
            for _ in range(trials):
                # each triggered chamber must come from a different real event
                i,j,k=rng.sample(range(n),3)
                fabricated=[{'cx':[],'cy':[]} for _ in range(det['nch'])]
                for c,source_record,idx in zip(firing,[pool[i],pool[j],pool[k]],range(3)):
                    cluster_x,cluster_y=source_record[idx]
                    fabricated[c]['cx']=cluster_x
                    fabricated[c]['cy']=cluster_y
                good,passcut=classify_event(pat,fabricated,det,resolution,cut)
                assert good  # because each source event was testable
                nt+=1;successes+=int(passcut)
        null=successes/nt if nt else None
        obs=d['compatible']/d['testable'] if d['testable'] else None
        rows.append(dict(dataset=label,pattern=pat,pattern_binary=format(pat,f'0{det["nbits"]}b'),
                         topology=topo,selected_sampled=d['seen'],testable=d['testable'],
                         observed_compatible=d['compatible'],observed_fraction=obs,
                         reservoir_size=n,mixed_trials=nt,mixed_compatible=successes,
                         mixed_fraction=null,observed_over_null=(obs/null if null and obs is not None else '')))
        g=summary[topo];g['patterns']+=1;g['observed_testable']+=d['testable'];
        g['observed_compatible']+=d['compatible'];g['mixed_trials']+=nt;g['mixed_success']+=successes
        if null is not None:
            g['weighted_expected']+=d['testable']*null
            g['weight_total']+=d['testable']
    # `overall` combines all pattern groups; weighted by observed testable counts.
    all_summary=summary['all']
    for topo in ['contiguous','gapped']:
        d=summary[topo]
        for key in all_summary:dval=d[key];all_summary[key]+=dval
    sums=[]
    for topo in ['contiguous','gapped','all']:
        g=summary[topo];obs=(g['observed_compatible']/g['observed_testable'] if g['observed_testable'] else '')
        nul=(g['weighted_expected']/g['weight_total'] if g['weight_total'] else '')
        sums.append(dict(dataset=label,topology=topo,testable_sampled=g['observed_testable'],
                         observed_compatible=g['observed_compatible'],observed_fraction=obs,
                         mixed_fraction_weighted=nul,mixed_trials_total=g['mixed_trials'],
                         patterns_total=g['patterns'],patterns_in_null=sum(1 for r in rows if r['topology']==topo and r['mixed_trials']>0) if topo!='all' else sum(r['mixed_trials']>0 for r in rows),
                         observed_over_mixed=(obs/nul if obs!='' and nul else '')))
    return rows,sums


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('-b','--detector-db',required=True)
    p.add_argument('-ap','--analysis-parameters',required=True)
    p.add_argument('--reference',nargs='+',required=True)
    p.add_argument('--test',nargs='+',required=True)
    p.add_argument('--reference-label',default='Run103')
    p.add_argument('--test-label',default='Run113')
    p.add_argument('--sample-every',type=int,default=100)
    p.add_argument('--max-pool-per-pattern',type=int,default=1500)
    p.add_argument('--trials-per-pattern',type=int,default=3000)
    p.add_argument('--resolution-mm',type=float,default=3.6)
    p.add_argument('--chi2-max',type=float,default=None)
    p.add_argument('--seed',type=int,default=20261010)
    p.add_argument('--progress-every',type=int,default=100000)
    p.add_argument('-o','--outdir',default='Results/Validation/MixedEventNull')
    a=p.parse_args()
    if a.sample_every<1 or a.max_pool_per_pattern<3 or a.trials_per_pattern<1:
        p.error('sample-every >=1, max-pool-per-pattern >=3, trials-per-pattern >=1')
    det=ht.parse_db(a.detector_db);apar=ht.parse_ap(a.analysis_parameters)
    cut=a.chi2_max if a.chi2_max is not None else apar['TrackKhi2Max']
    results=[];summary=[]
    for i,(label,pat) in enumerate([(a.reference_label,a.reference),(a.test_label,a.test)]):
        files=ht.expand(pat)
        if not files:raise SystemExit(f'No EBE files matched for {label}: {pat}')
        r,s=summarize_one_run(label,files,det,apar,a.sample_every,a.max_pool_per_pattern,
                              a.trials_per_pattern,a.seed+i,a.resolution_mm,cut,a.progress_every)
        results+=r;summary+=s
    out=Path(a.outdir);out.mkdir(parents=True,exist_ok=True)
    for name,rows in [('mixed_by_pattern.csv',results),('mixed_summary.csv',summary)]:
        with (out/name).open('w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=rows[0].keys());w.writeheader();w.writerows(rows)
    print('\nObserved vs mixed-null probability, conditioning on Ntrigger=3 and testable 3-layer X/Y clusters:')
    for r in summary:
        print(r['dataset'],r['topology'], 'observed=',r['observed_fraction'], 'null=',r['mixed_fraction_weighted'],
              'n=',r['testable_sampled'])
    print('Results:',out)

if __name__=='__main__':main()

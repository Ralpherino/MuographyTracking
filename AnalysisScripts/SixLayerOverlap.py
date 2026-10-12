#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Event-ID overlap between a MuographyTracking -t yes track list (.tl) and
the three-trigger candidate classes.

The .tl contains *accepted* TrackMain tracks. Verify the accompanying .res
provenance has TrackingNHits >= 6 and matches the detector/analysis settings.
This output is an independent high-quality-track acceptance CROSS-CHECK,
NOT an estimator of muon purity, which requires particle truth/control data.

The join is streaming and requires strictly increasing unique event IDs in
both EBE and .tl. It deliberately errors if that condition is violated.
"""
import argparse,csv,json,sys
from collections import defaultdict
from pathlib import Path
import HitTopologyAnalysis as ht


def iter_tracklist(path,cut):
    last=-1
    with open(path,errors='replace') as fh:
        for line_num,line in enumerate(fh,1):
            if not line.strip() or line.startswith('#'):continue
            t=line.split()
            if len(t)<10:
                raise ValueError(f'Unexpected .tl format line {line_num}: {line[:150]}')
            try:
                eid=int(t[0]);cx=float(t[8]);cy=float(t[9])
            except ValueError as e:
                raise ValueError(f'Cannot parse event ID or chi2 on .tl line {line_num}: {line[:150]}') from e
            if eid<=last:
                raise ValueError(f'.tl event IDs are not strictly increasing near {eid}. '
                                 'Do not join ambiguous/reset event IDs; process separate files or fix the IDs.')
            last=eid
            # The original .tl already contains only accepted TrackMain tracks,
            # but preserve its observed chi2 as a separate diagnostic.
            yield eid,cx,cy


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('-b','--detector-db',required=True)
    p.add_argument('-ap','--analysis-parameters',required=True)
    p.add_argument('--ebe',nargs='+',required=True)
    p.add_argument('--tracks',required=True,help='Output of Tracking.run -t yes (.tl)')
    p.add_argument('--label',required=True)
    p.add_argument('--sample-every',type=int,default=100)
    p.add_argument('--resolution-mm',type=float,default=3.6)
    p.add_argument('--chi2-max',type=float,default=None)
    p.add_argument('-o','--outdir',default='Results/Validation/SixLayer')
    a=p.parse_args()
    if a.sample_every<1:p.error('sample-every must be >=1')
    d=ht.parse_db(a.detector_db);ap=ht.parse_ap(a.analysis_parameters)
    cut=a.chi2_max if a.chi2_max is not None else ap['TrackKhi2Max']
    filenames=ht.expand(a.ebe)
    if not filenames:raise SystemExit('No EBE files matched')
    out=Path(a.outdir);out.mkdir(parents=True,exist_ok=True)
    ht.warn_parts(a.label,filenames)
    tracks=iter_tracklist(a.tracks,cut)
    next_track=next(tracks,None)
    count_trk=0;count_events=0;selected=0;sampled=0;prev_id=-1
    table=defaultdict(lambda:[0,0]);bad=0
    for ifile,fn in enumerate(filenames,1):
        print(f'[{ifile}/{len(filenames)}] {fn}',flush=True)
        for source,lineno,line in ht.lines_from(fn):
            toks,pat=ht.parse_p_and_tokens(line,d['nbits'])
            if pat is None:continue
            try:eid=int(toks[0])
            except ValueError:continue
            if eid<=prev_id:
                raise ValueError(f'EBE IDs not strictly increasing at {source}:{lineno}: {eid} <= {prev_id}. '
                                 'Per-run event-ID matching is ambiguous.')
            prev_id=eid;count_events+=1
            while next_track is not None and next_track[0]<eid:
                raise ValueError(f'.tl track event ID {next_track[0]} missing from EBE sequence before ID {eid}. '
                                 'Inputs may use different event lists or an inconsistent scan ordering.')
            accepted=next_track is not None and next_track[0]==eid
            if accepted:
                count_trk+=1
                next_track=next(tracks,None)
            if pat.bit_count()!=3:continue
            selected+=1
            if (selected-1)%a.sample_every:continue
            sampled+=1
            raw=ht.parse_ch_payload(toks,d)
            if raw is None:
                bad+=1;continue
            prep=ht.prepare(raw,d,ap)
            fired=ht.trig_ch(pat,d)
            fx=ht.best_fit(fired,prep,d,'x',a.resolution_mm)
            fy=ht.best_fit(fired,prep,d,'y',a.resolution_mm)
            geo='missing_xy' if fx is None or fy is None else ('compatible' if fx[0]<=cut and fy[0]<=cut else 'incompatible')
            topo='contiguous' if ht.contiguous(pat,d) else 'gapped'
            v=table[(topo,geo)];v[0]+=1;v[1]+=int(accepted)
    if next_track is not None:
        raise ValueError(f'.tl contains event ID {next_track[0]} beyond the scanned EBE range. Did you load all files?')
    rows=[]
    for (topo,geo),(n,accepted) in sorted(table.items()):
        rows.append(dict(dataset=a.label,topology=topo,geometry=geo,events_sampled=n,
                         accepted_6layer_tracks=accepted,acceptance_fraction=accepted/n if n else ''))
    with (out/f'{a.label}_six_layer_overlap.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
    (out/f'{a.label}_six_layer_manifest.json').write_text(json.dumps({
        'raw_events':count_events,'track_entries_matched':count_trk,
        'ntrigger3':selected,'ntrigger3_sampled':sampled,
        'bad_sampled_ch':bad,'ap':a.analysis_parameters,'db':a.detector_db,
        'important':'Accepted >=6-layer track acceptance proxy only; not literal muon purity. '
                    'Record independent detector conditions and matched source files.'},indent=2))
    print(f'Matched {count_trk:,} accepted tracks against {count_events:,} events.')
    print('Overlap table:')
    for r in rows:print(r)
    print('Output:',out)

if __name__=='__main__':main()

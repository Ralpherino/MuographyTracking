#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Track-matched ADC and extra-strip study for MTS-41 EBE and TrackMain .tl files.

Scientific scope
----------------
A raw hit is a strip address, and one cluster may include several strip hits.
'one_extra_strip' here means EXACTLY ONE separate unassigned singleton raw
cluster (one raw strip address), after assigning one cluster per axis/chamber
to a qualifying reconstructed track. Unresolved/merged strips cannot be counted.

'Track' means accepted by the provided TrackMain .tl file, NOT MC truth.
'No_track' means absent from that .tl file, NOT proof that no charged particle
traversed the detector. Track positions from .tl are used as approximate
association references; raw clustering here does not reproduce the C++
PositionSmoother exactly. Explicitly report unmatched/ambiguous categories.

The .ebe/.tl streaming merge requires files in increasing event ID order.
"""
import argparse
import csv
import glob
import math
import statistics
import sys
from collections import Counter, defaultdict
from pathlib import Path

import HitTopologyAnalysis as ht
from BackgroundCharacterization import parse_adc_and_hist_config, parse_adc_values


def ordered_ebe(globs):
    seen=set(); out=[]
    for pattern in globs:
        matches=glob.glob(pattern)
        if not matches and Path(pattern).is_file(): matches=[pattern]
        for f in matches:
            if f not in seen: out.append(f); seen.add(f)
    def part_sort(s):
        import re
        m=re.search(r'\.part(\d+)\.',s)
        return (int(m.group(1)) if m else -1, s)
    out.sort(key=part_sort)
    if not out: raise ValueError('No EBE input files matched')
    return out


def track_rows(path):
    last=-1
    with open(path,'rt',buffering=1024*1024,errors='replace') as f:
        for line_no,line in enumerate(f,1):
            if not line.strip() or line.startswith('#'): continue
            t=line.split()
            if len(t)<10: raise ValueError(f'Unexpected track row {path}:{line_no}: {line[:100]!r}')
            try:
                eid=int(t[0])
                if t[2]!='TrackMain':
                    raise ValueError(f'Expected TrackMain in .tl; got {t[2]}')
                track=tuple(float(v) for v in t[4:10]) # mx,my,x(z=0),y(z=0),chi2x,chi2y
            except (ValueError,IndexError) as e:
                raise ValueError(f'Bad .tl record at {path}:{line_no}') from e
            if eid <= last:
                raise ValueError(f'.tl must have strictly increasing event IDs ({last} -> {eid} at line {line_no})')
            last=eid
            yield eid,track


def classify_track(raw,det,ap,track,window_mm,min_layers):
    """Count raw, corrected-strip groups not assigned to an existing 1-D track.

    The reconstructed .tl track is *not* rebuilt from these clusters. We only
    associate its reference line to each raw axis; loss/splitting means event
    classification is ambiguous rather than clean/extra.
    """
    mx,my,x0,y0,qx,qy=track
    matched=[0,0]; unmatched=[]; oversize=False
    matched_chambers=set()
    track_chambers=set()
    for ich, ((rx,ry),cfg) in enumerate(zip(raw,det['ch'])):
        z=cfg['z']
        for axis,hh,ref,pad,mis,broken,size in (
            ('x',rx,mx*z+x0,cfg['padx'],cfg['misx'],cfg['brokenx'],cfg['sx']),
            ('y',ry,my*z+y0,cfg['pady'],cfg['misy'],cfg['brokeny'],cfg['sy'])):
            hits=ht.correct(hh,size,mis,broken)
            clusters=ht.clusterize(hits)
            # Conservative: oversize raw groups cause ambiguity, even if
            # tracking C++ may break them into several reconstructed clusters.
            if any(k[1]>ap['ClusterSizeLimit'] for k in clusters): oversize=True
            # If C++ cut drops all clusters on an axis, cannot reliably match.
            after=ht.cluster_break(clusters,ap['ClusterSizeLimit'],ap['ClusterBreak'])
            if len(after)>ap['NClustersLimit']: oversize=True
            if not clusters: continue
            candidates=[]
            for j,(c,s) in enumerate(clusters):
                pos=(c+0.5)*pad+(cfg['px'] if axis=='x' else cfg['py'])
                candidates.append((abs(pos-ref),j))
            best_d,best_j=min(candidates)
            # A cluster can be very wide; use a fixed window in physical mm,
            # with source field corrections respected where represented in .db.
            if best_d<=window_mm:
                matched[0 if axis=='x' else 1]+=1
                matched_chambers.add(ich)
                track_chambers.add(ich)
                for j,(_center,size_cl) in enumerate(clusters):
                    if j!=best_j:
                        unmatched.append((ich,axis,round(_center,2),int(size_cl)))
            else:
                for center,size_cl in clusters:
                    unmatched.append((ich,axis,round(center,2),int(size_cl)))
    if oversize or min(matched)<min_layers:
        return 'track_ambiguous',unmatched,matched,track_chambers
    if len(unmatched)==0: category='clean_track'
    elif len(unmatched)==1 and unmatched[0][3]==1: category='one_extra_strip'
    elif len(unmatched)==1: category='one_extra_cluster'
    else: category='multiple_extra_clusters'
    return category,unmatched,matched,track_chambers


class Moments:
    __slots__=('n','sum','sumsq','hist')
    def __init__(self): self.n=0;self.sum=0.0;self.sumsq=0.0;self.hist=Counter()
    def add(self,x,step=50):
        self.n+=1; self.sum+=x; self.sumsq+=x*x
        self.hist[int(math.floor(x/step))]+=1
    def stats(self):
        if not self.n: return ('','','')
        avg=self.sum/self.n
        std=math.sqrt(max(0.0,(self.sumsq-self.n*avg*avg)/(self.n-1))) if self.n>1 else 0.0
        halfway=(self.n+1)//2; acc=0; median=''
        for i,c in sorted(self.hist.items()):
            acc+=c
            if acc>=halfway: median=i*50+25;break
        return (round(avg,3),round(std,3),median)


def make_csv(path,header,rows):
    with open(path,'w',newline='') as f:
        w=csv.writer(f);w.writerow(header);w.writerows(rows)


def read_ebe_and_tracks(args,det,ap,adc_cfg):
    tr=track_rows(args.tracks)
    current=next(tr,None)
    stats=Counter(); breakdown=Counter()
    adc=defaultdict(Moments)
    adc_raw=defaultdict(Moments)
    quiet_baseline=defaultdict(Moments)
    examples=defaultdict(list)
    last_eid=-1
    stop=False
    files=ordered_ebe(args.ebe)
    print(f'[{args.label}] {len(files)} EBE file(s), sample event_id % {args.event_mod} = {args.event_remainder}',flush=True)
    for fn in files:
        print(f'  scanning {fn}',flush=True)
        with open(fn,'rb',buffering=2**20) as f:
            for n,line in enumerate(f,1):
                stats['scanned']+=1
                if args.max_scanned and stats['scanned']>args.max_scanned:
                    stop=True;break
                try: eid=int(line.split(None,1)[0])
                except (ValueError,IndexError): stats['malformed_id']+=1;continue
                if eid<=last_eid: raise ValueError(f'EBE event IDs not increasing ({last_eid} -> {eid} in {fn}:{n})')
                last_eid=eid
                if eid%args.event_mod!=args.event_remainder:continue
                stats['sampled']+=1
                while current is not None and current[0]<eid:
                    current=next(tr,None)
                track=current[1] if current is not None and current[0]==eid else None
                tokens,pat=ht.parse_p_and_tokens(line,det['nbits'])
                if pat is None: stats['bad_trigger']+=1;continue
                raw=ht.parse_ch_payload(tokens,det)
                ar,ac=parse_adc_values(tokens,adc_cfg,det['nch'])
                if raw is None or ar is None:stats['bad_payload']+=1;continue
                if track is None:
                    category='no_track'; leftover=[];matched=[0,0];track_ch=set()
                elif max(track[4],track[5])>args.max_axis_chi2:
                    category='track_loose_quality';leftover=[];matched=[0,0];track_ch=set()
                else:
                    category,leftover,matched,track_ch=classify_track(raw,det,ap,track,args.match_window_mm,args.min_matched_layers)
                stats['parsed']+=1;stats[category]+=1
                ntrig=pat.bit_count()
                breakdown[(category,ntrig,pat)]+=1
                trigset=set(ht.trig_ch(pat,det))
                allquiet=[]
                for c,(xy,r,corr) in enumerate(zip(raw,ar,ac)):
                    hit=bool(xy[0] or xy[1]); trig=c in trigset
                    status=('trigger' if trig else 'no_trigger')+'_'+('hit' if hit else 'quiet')
                    for label in ('all',status):
                        adc[(category,c,label)].add(corr)
                        adc_raw[(category,c,label)].add(r,step=50)
                    if not hit and not trig: allquiet.append(corr)
                if len(allquiet)>=args.min_quiet_channels:
                    quiet_baseline[category].add(statistics.median(allquiet))
                if len(examples[category])<args.max_examples:
                    examples[category].append([
                        args.label,category,eid,fn,n,pat,format(pat,f'0{det["nbits"]}b'),ntrig,
                        *(('' if track is None else v) for v in ((track or ('','','','','','')))),
                        matched[0],matched[1],len(leftover),
                        ';'.join(f'{c}:{axis}:{ctr}:{size}' for c,axis,ctr,size in leftover),
                        ';'.join(str(c) for c in sorted(track_ch)),
                        len(allquiet),round(statistics.median(allquiet),3) if allquiet else ''
                    ])
                if args.progress_every and stats['sampled']%args.progress_every==0:
                    print(f'    scanned {stats["scanned"]:,}, sampled {stats["sampled"]:,}, tracks {stats["clean_track"]+stats["one_extra_strip"]:,}',flush=True)
        if stop:break
    if stats['scanned']==0:raise ValueError('No EBE lines scanned')
    return stats,breakdown,adc,adc_raw,quiet_baseline,examples


def save_outputs(args,result):
    stats,pattern,adc,adc_raw,quiet,examples=result
    dest=Path(args.output);dest.mkdir(parents=True,exist_ok=True)
    make_csv(dest/f'{args.label}_event_summary.csv', ['dataset','category','events_sampled','total_ebe_scanned','event_mod','max_axis_chi2','match_window_mm','min_matched_layers'],
        [[args.label,k,v,stats['scanned'],args.event_mod,args.max_axis_chi2,args.match_window_mm,args.min_matched_layers]
         for k,v in sorted(stats.items()) if k in ('no_track','track_loose_quality','track_ambiguous','clean_track','one_extra_strip','one_extra_cluster','multiple_extra_clusters')])
    make_csv(dest/f'{args.label}_pattern_summary.csv', ['dataset','category','ntrigger','pattern','events'],
        [[args.label,*key,n] for key,n in sorted(pattern.items())])
    make_csv(dest/f'{args.label}_adc_summary.csv',
        ['dataset','category','chamber_index','selection','events','corrected_adc_mean','corrected_adc_sd','corrected_adc_median_50bin_approx','raw_adc_mean','raw_adc_sd'],
        [[args.label,category,ch,selection,m.n,*m.stats(),*adc_raw[(category,ch,selection)].stats()[:2]]
         for (category,ch,selection),m in sorted(adc.items())])
    make_csv(dest/f'{args.label}_adc_hist.csv',
        ['dataset','category','chamber_index','selection','corrected_adc_bin_low','corrected_adc_bin_high','count'],
        [[args.label,category,ch,selection,i*50,(i+1)*50,n]
         for (category,ch,selection),m in sorted(adc.items()) for i,n in sorted(m.hist.items())])
    make_csv(dest/f'{args.label}_quiet_baseline.csv',
        ['dataset','category','events_with_sufficient_quiet_channels','median_corrected_quiet_adc_mean','median_corrected_quiet_adc_sd','median_corrected_quiet_adc_50bin_approx'],
        [[args.label,cat,v.n,*v.stats()] for cat,v in sorted(quiet.items())])
    head=['dataset','category','event_id','source','source_line','pattern','pattern_bin','ntrigger',
          'slope_x','slope_y','x_at_z0','y_at_z0','chi2x','chi2y','matched_x','matched_y','extra_cluster_count','extra_clusters_ch_axis_center_size','track_matched_chambers','quiet_untriggered_chambers','quiet_adc_median']
    make_csv(dest/f'{args.label}_examples.csv',head,[row for k,rows in sorted(examples.items()) for row in rows])
    print(f'[{args.label}] categories:',{k:stats[k] for k in ('no_track','track_loose_quality','track_ambiguous','clean_track','one_extra_strip','one_extra_cluster','multiple_extra_clusters')},flush=True)
    print(f'[{args.label}] outputs in {dest.resolve()}',flush=True)


def main():
    p=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--label',required=True,help='Run103 or Run113')
    p.add_argument('-b','--db',required=True,help='Mts41.db, matching the tracking reconstruction')
    p.add_argument('-ap','--ap',required=True,help='Mts41.ap used for this .tl reconstruction')
    p.add_argument('--tracks',required=True,help='Accepted TrackMain .tl for the SAME EBE files')
    p.add_argument('--ebe',required=True,nargs='+',help='One or more .ebe files/globs, part order is sorted')
    p.add_argument('-o','--output',default='Results',help='Output folder')
    p.add_argument('--event-mod',type=int,default=20,help='Examine event IDs modulo N (20=5%% sample)')
    p.add_argument('--event-remainder',type=int,default=0)
    p.add_argument('--max-axis-chi2',type=float,default=2.5)
    p.add_argument('--match-window-mm',type=float,default=8.0,help='Spatial tolerance for raw cluster–track matching, each axis')
    p.add_argument('--min-matched-layers',type=int,default=6,help='Require this many cluster matches IN EACH X AND Y projection')
    p.add_argument('--min-quiet-channels',type=int,default=3)
    p.add_argument('--max-examples',type=int,default=40)
    p.add_argument('--max-scanned',type=int,default=0,help='Debug smoke-test EBE lines; 0 means unlimited')
    p.add_argument('--progress-every',type=int,default=50000)
    args=p.parse_args()
    if args.event_mod<1 or not 0<=args.event_remainder<args.event_mod: p.error('invalid event modulo/remainder')
    if args.min_matched_layers<1: p.error('min matched layers must be >0')
    det=ht.parse_db(args.db);ap=ht.parse_ap(args.ap)
    adc_cfg=parse_adc_and_hist_config(args.db,args.ap)
    if adc_cfg['n_adcs']!=det['nch']:print('WARNING: ADC channels != chamber count, check map',file=sys.stderr)
    result=read_ebe_and_tracks(args,det,ap,adc_cfg)
    save_outputs(args,result)

if __name__=='__main__':main()

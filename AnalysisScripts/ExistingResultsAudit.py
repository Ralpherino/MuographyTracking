#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Compare MuographyTracking .res provenance, HV/THP,
reconstructed-track ADC controls, and the sn16 position anomaly.

Only standard library. Run from anywhere.
"""
import argparse, csv, json, math
from collections import defaultdict
from pathlib import Path


def write_csv(path, fields, rows):
    with open(path, 'w', newline='') as f:
        w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)


def parse_result(path):
    chambers=[]; current=None; tab=None; mode=None; hist_header=None
    hv=[]; thp=[]; settings={}; adc=defaultdict(lambda:defaultdict(list))
    hitx=defaultdict(list); chunk=[]; section=None
    def process(line):
        nonlocal current, tab, mode, hist_header, section, chunk
        s=line.strip()
        if s=='!Chamber':
            current={}; chambers.append(current);tab=None;mode=None;section=None;return
        if s=='!Detector':
            current=None;tab=None;mode=None;section=None;return
        if s=='#AnalysisParameters':
            section='analysis'; settings[section]=[]; tab=None;mode=None;return
        if s=='#DetectorBasicsFile':
            section='detector';settings[section]=[];tab=None;mode=None;return
        if section in ('analysis','detector'):
            if s:settings[section].append(' '.join(s.split()))
            elif section=='analysis':section=None
            return
        if s.startswith('#HvTimeDev NHists'):tab='hv';mode='header';return
        if s.startswith('#ThpTimeDev NHists'):tab='thp';mode='header';return
        if s.startswith('#HitXDep NHists') and current is not None:
            tab='hitx';mode='header';return
        if s.startswith('#AdcDep NHists') and current is not None:
            tab='adc';mode='header';return
        if not s:
            tab=None;mode=None;return
        if tab in ('hv','thp'):
            if s.startswith('#Hists'):mode='values';return
            if mode!='values':return
            z=s.split()
            if len(z)>=6:
                try:
                    count=float(z[2]); sums=[float(x) for x in z[3:6]]
                    if count>0:
                        (hv if tab=='hv' else thp).append(dict(time=z[1],count=count,values=[v/count for v in sums],sums=sums))
                except ValueError:pass
            return
        if tab=='hitx':
            if s.startswith('X '):mode='values';return
            if mode!='values':return
            t=s.split()
            if len(t)>1:
                try: hitx[current['Name']].append((float(t[0]),float(t[1])))
                except (ValueError,KeyError):pass
            return
        if tab=='adc':
            if s.startswith('X '): mode='bare';hist_header=s.split()[1:];return
            if s.startswith('#Hists'):mode='sparse';hist_header=s.split()[1:];return
            if mode!='sparse':return
            z=s.split()
            if len(z)>2 and current and 'Name' in current:
                try:
                    x=float(z[0]); values=[float(v) for v in z[1:]]
                    for j,key in enumerate(hist_header[:len(values)]):
                        if key in ('TrackCross','ClusterOk') and values[j]>0:
                            adc[current['Name']][key].append((x,values[j]))
                except ValueError:pass
            return
        if current is not None and s.split()[0] == 'Name':current['Name']=s.split()[1]
        elif current is not None and s.split()[0] == 'PositionXYZ':current['PositionXYZ']=' '.join(s.split()[1:])
        elif current is not None and s.split()[0] == 'AdcPedestal':
            t=s.split();current['AdcPedestal']=t[1];current['AdcScale']=t[3]
        elif current is not None and s.split()[0] == 'Gain':current['Gain']=float(s.split()[1])
        elif current is not None and s.split()[0] == 'Efficiency':
            t=s.split();current['Efficiency']=float(t[1]);current['TriggerEfficiency']=float(t[3])
    with open(path,errors='replace') as f:
        for line in f:process(line)
    return dict(path=path,chambers=chambers,hv=hv,thp=thp,settings=settings,adc=adc,hitx=hitx)


def weighted_group(data):
    if not data:return None
    n=sum(v['count'] for v in data)
    avg=[sum(v['sums'][j] for v in data)/n for j in range(3)]
    lo=[min(v['values'][j] for v in data) for j in range(3)]
    hi=[max(v['values'][j] for v in data) for j in range(3)]
    return n,avg,lo,hi


def median_hist(pairs):
    n=sum(weight for _,weight in pairs)
    if not n:return None
    cumul=0
    for x,count in sorted(pairs):
        cumul+=count
        if cumul >= n/2:return x
    return pairs[-1][0]


def sn16_table(path):
    counts=defaultdict(list)
    if not path or not Path(path).exists():return []
    with open(path,newline='') as f:
        for row in csv.DictReader(f):
            if row['chamber_name'].endswith('sn16') and row['axis']=='X':
                key=(row['dataset'],row['topology'],row['geometry'])
                counts[key].append((float(row['local_center']),int(row['count'])))
    rows=[]
    for (run,topo,geo),positions in sorted(counts.items()):
        tot=sum(n for _,n in positions)
        if tot<1:continue
        edge=sum(n for x,n in positions if x<8 or x>=72)
        top5=sum(sorted((n for _,n in positions),reverse=True)[:5])
        rows.append(dict(dataset=run,topology=topo,geometry=geo,
                         sn16_x_clusters=tot,edge_fraction=edge/tot,top5_fraction=top5/tot))
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--run103-res',required=True)
    p.add_argument('--run113-res',required=True)
    p.add_argument('--cluster-position',default=None)
    p.add_argument('-o','--outdir',default='Results/Validation/Audit')
    a=p.parse_args();out=Path(a.outdir);out.mkdir(parents=True,exist_ok=True)
    info={k:parse_result(v) for k,v in [('Run103',a.run103_res),('Run113',a.run113_res)]}
    r103,r113=info['Run103'],info['Run113']

    lines103=r103['settings'];lines113=r113['settings']
    conf=[]
    for sec in ['analysis','detector']:
        seq1=lines103.get(sec,[]);seq2=lines113.get(sec,[])
        if seq1==seq2:conf.append(dict(section=sec,status='IDENTICAL',run103='',run113=''))
        else:
            maxn=max(len(seq1),len(seq2))
            for i in range(maxn):
                x=seq1[i] if i<len(seq1) else '<ABSENT>'
                y=seq2[i] if i<len(seq2) else '<ABSENT>'
                if x!=y:conf.append(dict(section=sec,status='DIFFERENT',run103=x,run113=y))
    write_csv(out/'effective_configuration_diff.csv',['section','status','run103','run113'],conf)

    g103={c['Name']:c for c in r103['chambers']};g113={c['Name']:c for c in r113['chambers']}
    gains=[];track_adc=[];raw_hits=[]
    for name in sorted(set(g103)|set(g113)):
        x=g103.get(name,{});y=g113.get(name,{})
        v1=x.get('Gain');v2=y.get('Gain')
        gains.append(dict(chamber=name,pedestal_103=x.get('AdcPedestal',''),pedestal_113=y.get('AdcPedestal',''),
                          scale_103=x.get('AdcScale',''),scale_113=y.get('AdcScale',''),
                          geometry_103=x.get('PositionXYZ',''),geometry_113=y.get('PositionXYZ',''),
                          gain_103=v1,gain_113=v2,gain_relative_change_pct=100*(v2/v1-1) if v1 and v2 else ''))
        for run,d in info.items():
            for scope in ['TrackCross','ClusterOk']:
                pairs=d['adc'][name][scope]
                track_adc.append(dict(dataset=run,chamber=name,scope=scope,median_corrected_adc=median_hist(pairs),entries=sum(n for _,n in pairs)))
            hits=d['hitx'][name];total=sum(y for _,y in hits)
            if total:
                # local detector X extents vary; infer last pad
                xmax=max(x for x,_ in hits)
                edge=sum(n for x,n in hits if x < 0.1*(xmax+1) or x>=0.9*(xmax+1))
                raw_hits.append(dict(dataset=run,chamber=name,hit_x_total=total,hit_x_outer_10pct_fraction=edge/total))

    write_csv(out/'chamber_gain_comparison.csv',list(gains[0]),gains)
    write_csv(out/'six_layer_track_adc_reference.csv',list(track_adc[0]),track_adc)
    write_csv(out/'raw_hit_x_edge_fractions.csv',list(raw_hits[0]),raw_hits)
    for tab in ['hv','thp']:
        rows=[];stats=[]
        names= ['UIseg','UMon','IMon'] if tab=='hv' else ['T','H','P']
        for run,d in info.items():
            for row in d[tab]:
                r={'dataset':run,'timestamp':row['time'],'count':row['count']}
                r.update({name:row['values'][i] for i,name in enumerate(names)})
                rows.append(r)
            summ=weighted_group(d[tab])
            if summ:
                n,avg,lo,hi=summ
                for i,name in enumerate(names):
                    stats.append(dict(dataset=run,quantity=name,events=n,
                                  event_weighted_mean=avg[i],min_time_bin_mean=lo[i],max_time_bin_mean=hi[i]))
        write_csv(out/f'{tab}_timebins.csv', ['dataset','timestamp','count']+names,rows)
        write_csv(out/f'{tab}_summary.csv',list(stats[0]),stats)
    pos=sn16_table(a.cluster_position)
    if pos:write_csv(out/'sn16_x_cluster_edges.csv',list(pos[0]),pos)
    (out/'audit_manifest.json').write_text(json.dumps({
        'inputs':{'Run103':r103['path'],'Run113':r113['path'],'task3_cluster_position':a.cluster_position},
        'config_identical':all(r['status']=='IDENTICAL' for r in conf),
        'notes':[
            'Both .res contain settings used by analysis, not proof hardware settings were unchanged during acquisition.',
            'HV/THP summaries are event-weighted time-bin means, not independent stability calibrations.',
            'The .res Gain is a mean of path-length-normalized track ADC, not standalone gas gain.',
            'The 6-layer track ADC reference comes from TrackCross/ClusterOk .res histograms and is only an aggregate control, not a same-event overlap or purity measurement.',
            'The detector-specific sn16 anomaly cannot be assigned a cause from aggregated outputs alone.'
        ]},indent=2))
    print('Finished. Config identical:',all(r['status']=='IDENTICAL' for r in conf))
    for tab in ['hv','thp']:
        for run,d in info.items():
            q=weighted_group(d[tab]);print(tab,run, [round(x,3) for x in q[1]])
    print('Output:',out.resolve())

if __name__=='__main__':main()

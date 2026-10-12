#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Reads the MuographyTracking detector .db and analysis .ap files, decodes the
raw EBE `Ch` payload, reproduces the basic hit corrections and clusterization,
and tests whether Ntrigger=3 events contain an approximately straight 3-point
candidate in BOTH X and Y.

`line-compatible` is NOT equivalent to a normal reconstructed muon track.
It is only a three-layer geometry test.
"""

import argparse, csv, glob, itertools, math, re, sys, time, zipfile
from collections import defaultdict
from pathlib import Path

PART_RE = re.compile(r"\.part(\d+)\.")
CHI2_EDGES = [0.25,0.5,1,2,4,8,16,32]


def parse_ap(path):
    p={"ClusterSizeLimit":8,"ClusterBreak":0,"NClustersLimit":4,"TrackKhi2Max":5.0}
    if not path: return p
    for raw in Path(path).read_text(errors="replace").splitlines():
        s=raw.strip()
        if not s or s.startswith("#"): continue
        t=s.split()
        if len(t)<2: continue
        if t[0] in ("ClusterSizeLimit","ClusterBreak","NClustersLimit"):
            p[t[0]]=int(float(t[1]))
        elif t[0]=="TrackKhi2Max": p[t[0]]=float(t[1])
    return p


def parse_chamber(tokens, default_pad):
    c={"name":"","sx":None,"sy":None,"z":0.0,"px":0.0,"py":0.0,
       "padx":default_pad,"pady":default_pad,"misx":None,"misy":None,
       "brokenx":set(),"brokeny":set()}
    i=1
    while i<len(tokens):
        k=tokens[i]; i+=1
        if k=="End": break
        if k=="Name": c["name"]=tokens[i]; i+=1
        elif k=="Size": c["sx"]=int(tokens[i]); c["sy"]=int(tokens[i+1]); i+=2
        elif k=="PosZ": c["z"]=float(tokens[i]); i+=1
        elif k=="PosX": c["px"]=float(tokens[i]); i+=1
        elif k=="PosY": c["py"]=float(tokens[i]); i+=1
        elif k=="PadSizeX": c["padx"]=float(tokens[i]); i+=1
        elif k=="PadSizeY": c["pady"]=float(tokens[i]); i+=1
        elif k=="PadSizeXY": c["padx"]=float(tokens[i]); c["pady"]=float(tokens[i+1]); i+=2
        elif k in ("AdcPedestal","AdcScale"): i+=1
        elif k=="SideGrow": i+=2
        elif k=="Flips":
            nx=int(tokens[i]); i+=1+nx
            ny=int(tokens[i]); i+=1+ny
        elif k=="MisPositionX": c["misx"]=(int(tokens[i]),int(tokens[i+1]),int(tokens[i+2])); i+=3
        elif k=="MisPositionY": c["misy"]=(int(tokens[i]),int(tokens[i+1]),int(tokens[i+2])); i+=3
        elif k=="BrokenChannelsX":
            n=int(tokens[i]); i+=1; c["brokenx"]=set(map(int,tokens[i:i+n])); i+=n
        elif k=="BrokenChannelsY":
            n=int(tokens[i]); i+=1; c["brokeny"]=set(map(int,tokens[i:i+n])); i+=n
        else: raise ValueError(f"Unsupported ChamberBasics token: {k}")
    if c["sx"] is None or c["sy"] is None or c["padx"] is None or c["pady"] is None:
        raise ValueError(f"Incomplete chamber configuration: {c['name']}")
    return c


def parse_db(path):
    lines=Path(path).read_text(errors="replace").splitlines()
    nch=None; pad=None; nbits=None; assign=None; clines=[]
    for raw in lines:
        s=raw.strip()
        if not s or s.startswith("#"): continue
        t=s.split()
        if t[0]=="NChambers": nch=int(t[1])
        elif t[0]=="PadSizeMm": pad=float(t[1])
        elif t[0]=="Trigger":
            nbits=int(t[1]); j=t.index("Assign"); assign=list(map(int,t[j+1:j+1+nbits]))
        elif t[0]=="ChamberBasics": clines.append(t)
    if nch is None or nbits is None or assign is None: raise ValueError("Missing NChambers/Trigger in DB")
    ch=[parse_chamber(t,pad) for t in clines]
    if len(ch)!=nch: raise ValueError(f"Expected {nch} chambers, parsed {len(ch)}")
    zorder=sorted(range(nch),key=lambda i:ch[i]["z"]); zrank={c:r for r,c in enumerate(zorder)}
    return {"nch":nch,"nbits":nbits,"assign":assign,"ch":ch,"zrank":zrank}


def expand(pats):
    out=[]; seen=set()
    for p in pats:
        m=sorted(glob.glob(p))
        if not m and Path(p).exists(): m=[p]
        for x in m:
            x=str(Path(x))
            if x not in seen: seen.add(x); out.append(x)
    return out


def warn_parts(label,files):
    parts=[]
    for f in files:
        m=PART_RE.search(Path(f).name)
        if m: parts.append(int(m.group(1)))
    if not parts: return
    parts=sorted(set(parts))
    if parts[0]!=0: print(f"WARNING [{label}]: part00 absent; first part is {parts[0]:02d}",file=sys.stderr)
    miss=sorted(set(range(parts[0],parts[-1]+1))-set(parts))
    if miss: print(f"WARNING [{label}]: missing parts {miss}",file=sys.stderr)


def lines_from(fn):
    p=Path(fn)
    if p.suffix.lower()==".zip":
        with zipfile.ZipFile(p) as z:
            for member in [x for x in z.namelist() if not x.endswith("/")]:
                with z.open(member) as fh:
                    for n,line in enumerate(fh,1): yield f"{p}:{member}",n,line
    else:
        with open(p,"rb",buffering=1024*1024) as fh:
            for n,line in enumerate(fh,1): yield str(p),n,line


def parse_p_and_tokens(line,nbits):
    t=line.split()
    try:
        j=t.index(b"P"); p=int(t[j+1]) & ((1<<nbits)-1); return t,p
    except (ValueError,IndexError): return None,None


def parse_ch_payload(t,det):
    try: i=t.index(b"Ch")+1
    except ValueError: return None
    out=[]
    try:
        for _ in range(det["nch"]):
            nx=int(t[i]); i+=1; xs=list(map(int,t[i:i+nx])); i+=nx
            ny=int(t[i]); i+=1; ys=list(map(int,t[i:i+ny])); i+=ny
            out.append((xs,ys))
    except (ValueError,IndexError): return None
    return out


def correct(hits,size,mis,broken):
    out=[]
    for h in hits:
        if mis and mis[0] <= h < mis[1]: h=max(0,min(size-1,h+mis[2]))
        if h not in broken: out.append(h)
    return sorted(out)


def clusterize(hits):
    if not hits: return []
    out=[]; center=float(hits[0]); size=1
    for a,b in zip(hits[:-1],hits[1:]):
        d=b-a
        if d==1: center+=0.5; size+=1
        elif d==2: center+=1.0; size+=2
        else: out.append([center,size]); center=float(b); size=1
    out.append([center,size]); return out


def cluster_break(cs,limit,mode):
    if mode==0: return [x[:] for x in cs if x[1]<=limit]
    out=[]
    for center,size in cs:
        if size<=limit: out.append([center,size]); continue
        if mode==1: out.append([center,limit])
        elif mode==2:
            s=size/2.0-1; out += [[center-s,limit],[center+s,limit]]
        elif mode==3:
            s=size/2.0-1; out += [[center-s,limit],[center,limit],[center+s,limit]]
        else: raise ValueError(f"Unsupported ClusterBreak={mode}")
    return out


def prepare(raw,det,ap):
    out=[]
    for (xs,ys),cfg in zip(raw,det["ch"]):
        xs=correct(xs,cfg["sx"],cfg["misx"],cfg["brokenx"])
        ys=correct(ys,cfg["sy"],cfg["misy"],cfg["brokeny"])
        cx=cluster_break(clusterize(xs),ap["ClusterSizeLimit"],ap["ClusterBreak"])
        cy=cluster_break(clusterize(ys),ap["ClusterSizeLimit"],ap["ClusterBreak"])
        if len(cx)>ap["NClustersLimit"]: cx=[]
        if len(cy)>ap["NClustersLimit"]: cy=[]
        gx=[((c+0.5)*cfg["padx"]+cfg["px"],s,c) for c,s in cx]
        gy=[((c+0.5)*cfg["pady"]+cfg["py"],s,c) for c,s in cy]
        out.append({"hx":xs,"hy":ys,"cx":gx,"cy":gy})
    return out


def trig_ch(pattern,det):
    return [c for bit,c in enumerate(det["assign"]) if c>=0 and ((pattern>>bit)&1)]


def contiguous(pattern,det):
    c=trig_ch(pattern,det)
    if not c: return False
    r=sorted(det["zrank"][x] for x in c)
    return r[-1]-r[0]+1==len(r)


def fit(z,y):
    n=len(z); sx=sum(z); sy=sum(y); sxx=sum(x*x for x in z); sxy=sum(x*v for x,v in zip(z,y)); syy=sum(v*v for v in y)
    den=n*sxx-sx*sx
    if den==0: return None
    m=(n*sxy-sx*sy)/den; b=(sxx*sy-sxy*sx)/den; e2=max(0.0,syy-m*sxy-b*sy)
    return m,b,e2


def best_fit(chs,prepared,det,axis,res):
    key="cx" if axis=="x" else "cy"; lists=[prepared[c][key] for c in chs]
    if any(not x for x in lists): return None
    z=[det["ch"][c]["z"] for c in chs]; best=None
    for combo in itertools.product(*lists):
        y=[x[0] for x in combo]; f=fit(z,y)
        if not f: continue
        m,b,e2=f; ndf=len(z)-2; q=0.0 if ndf<=0 else e2/(res*res)/ndf
        if best is None or q<best[0]: best=(q,m,b)
    return best


def newstat():
    return {k:0 for k in ["sampled","parsed","badch","allxy","tested","compatible","incompatible","hitsx","hitsy","active_any","active_xy","extra_active"]} | {"sumqX":0.0,"sumqY":0.0}


def qbin(q):
    for e in CHI2_EDGES:
        if q<e:return f"<{e:g}"
    return f">={CHI2_EDGES[-1]:g}"


def analyze(label,files,det,ap,ntrig,sample_every,res,chi2max,examples_n,progress):
    stats=defaultdict(newstat); examples=defaultdict(list); hist=defaultdict(int); file_rows=[]
    scanned=selected=analyzed=0; t0=time.time(); warn_parts(label,files)
    print(f"\n=== {label} ===")
    for fi,fn in enumerate(files,1):
        fs=fs_sel=fs_ana=fs_bad=0; print(f"[{fi}/{len(files)}] {fn}",flush=True)
        for source,lineno,line in lines_from(fn):
            scanned+=1; fs+=1; toks,p=parse_p_and_tokens(line,det["nbits"])
            if p is None: continue
            nt=p.bit_count() if hasattr(p,"bit_count") else bin(p).count("1")
            if nt!=ntrig: continue
            selected+=1; fs_sel+=1
            if sample_every>1 and (selected-1)%sample_every: continue
            analyzed+=1; fs_ana+=1; s=stats[p]; s["sampled"]+=1
            raw=parse_ch_payload(toks,det)
            if raw is None: s["badch"]+=1; fs_bad+=1; continue
            s["parsed"]+=1; prep=prepare(raw,det,ap); tc=trig_ch(p,det); ts=set(tc)
            hx=sum(len(c["hx"]) for c in prep); hy=sum(len(c["hy"]) for c in prep)
            aa=sum(bool(c["hx"] or c["hy"]) for c in prep); axy=sum(bool(c["hx"] and c["hy"]) for c in prep)
            extra=sum(i not in ts and bool(c["hx"] or c["hy"]) for i,c in enumerate(prep))
            s["hitsx"]+=hx; s["hitsy"]+=hy; s["active_any"]+=aa; s["active_xy"]+=axy; s["extra_active"]+=extra
            allxy=all(prep[c]["cx"] and prep[c]["cy"] for c in tc)
            if allxy:s["allxy"]+=1
            fx=best_fit(tc,prep,det,"x",res); fy=best_fit(tc,prep,det,"y",res)
            cat="missing_xy"
            if fx and fy:
                s["tested"]+=1; s["sumqX"]+=fx[0]; s["sumqY"]+=fy[0]
                ok=fx[0]<=chi2max and fy[0]<=chi2max
                cat="compatible" if ok else "incompatible"
                s[cat]+=1
                topo="contiguous" if contiguous(p,det) else "gapped"
                for name,q in [("chi2x",fx[0]),("chi2y",fy[0]),("chi2max",max(fx[0],fy[0]))]: hist[(topo,name,qbin(q))]+=1
            ek=(p,cat)
            if len(examples[ek])<examples_n:
                examples[ek].append({"source":source,"line":lineno,"event_id":toks[0].decode(errors="replace"),"pattern":p,
                    "pattern_bin":format(p,f"0{det['nbits']}b"),"contiguous":int(contiguous(p,det)),"category":cat,
                    "active_any":aa,"active_xy":axy,"extra_active":extra,"hits_x":hx,"hits_y":hy,
                    "chi2x":"" if not fx else fx[0],"chi2y":"" if not fy else fy[0],"raw":line.decode(errors="replace").rstrip()})
            if progress and analyzed%progress==0:
                dt=max(time.time()-t0,1e-9); print(f"  analyzed {analyzed:,}; scanned {scanned:,}; {scanned/dt:,.0f} lines/s",flush=True)
        file_rows.append([fn,fs,fs_sel,fs_ana,fs_bad])
    print(f"{label}: scanned={scanned:,}, selected N={ntrig}: {selected:,}, analyzed={analyzed:,}")
    return {"label":label,"stats":stats,"examples":examples,"hist":hist,"files":file_rows,"scanned":scanned,"selected":selected,"analyzed":analyzed}


def meta(p,det):
    tc=trig_ch(p,det); return format(p,f"0{det['nbits']}b"),"+".join(det["ch"][c]["name"] for c in tc),int(contiguous(p,det))


def write_outputs(outdir,prefix,ref,test,det):
    outdir.mkdir(parents=True,exist_ok=True)
    for r in (ref,test):
        fn=outdir/f"{prefix}_{r['label']}_files.csv"
        with open(fn,"w",newline="") as f:
            w=csv.writer(f); w.writerow(["file","lines_scanned","selected_before_sampling","selected_analyzed","bad_ch_payload"]); w.writerows(r["files"])
        fn=outdir/f"{prefix}_{r['label']}_pattern_summary.csv"
        with open(fn,"w",newline="") as f:
            w=csv.writer(f); w.writerow(["pattern_dec","pattern_bin","chambers","contiguous","sampled","parsed_ch","all_triggered_xy","frac_all_triggered_xy","tested","compatible","incompatible","compatible_fraction_tested","compatible_fraction_all_sampled","mean_hits_x","mean_hits_y","mean_active_any","mean_active_xy","mean_extra_active_any","mean_best_chi2x","mean_best_chi2y"])
            for p,s in sorted(r["stats"].items(),key=lambda kv:-kv[1]["sampled"]):
                b,n,c=meta(p,det); parsed=s["parsed"]; tested=s["tested"]
                w.writerow([p,b,n,c,s["sampled"],parsed,s["allxy"],s["allxy"]/parsed if parsed else 0,tested,s["compatible"],s["incompatible"],s["compatible"]/tested if tested else 0,s["compatible"]/s["sampled"] if s["sampled"] else 0,s["hitsx"]/parsed if parsed else 0,s["hitsy"]/parsed if parsed else 0,s["active_any"]/parsed if parsed else 0,s["active_xy"]/parsed if parsed else 0,s["extra_active"]/parsed if parsed else 0,s["sumqX"]/tested if tested else "",s["sumqY"]/tested if tested else ""])
    fn=outdir/f"{prefix}_comparison.csv"
    with open(fn,"w",newline="") as f:
        w=csv.writer(f); w.writerow(["pattern_dec","pattern_bin","chambers","contiguous",f"{ref['label']}_sampled",f"{ref['label']}_tested",f"{ref['label']}_compatible_fraction_tested",f"{test['label']}_sampled",f"{test['label']}_tested",f"{test['label']}_compatible_fraction_tested","delta_test_minus_reference"])
        for p in sorted(set(ref["stats"])|set(test["stats"])):
            b,n,c=meta(p,det); a=ref["stats"].get(p,newstat()); d=test["stats"].get(p,newstat()); fa=a["compatible"]/a["tested"] if a["tested"] else None; fb=d["compatible"]/d["tested"] if d["tested"] else None
            w.writerow([p,b,n,c,a["sampled"],a["tested"],"" if fa is None else fa,d["sampled"],d["tested"],"" if fb is None else fb,"" if fa is None or fb is None else fb-fa])
    fn=outdir/f"{prefix}_examples.csv"
    fields=["dataset","source","line","event_id","pattern","pattern_bin","contiguous","category","active_any","active_xy","extra_active","hits_x","hits_y","chi2x","chi2y","raw"]
    with open(fn,"w",newline="") as f:
        w=csv.DictWriter(f,fieldnames=fields); w.writeheader()
        for r in (ref,test):
            for vals in r["examples"].values():
                for row in vals: row=dict(row); row["dataset"]=r["label"]; w.writerow(row)
    fn=outdir/f"{prefix}_chi2_hist.csv"
    bins=[f"<{x:g}" for x in CHI2_EDGES]+[f">={CHI2_EDGES[-1]:g}"]
    with open(fn,"w",newline="") as f:
        w=csv.writer(f); w.writerow(["dataset","topology","quantity","bin","count"])
        for r in (ref,test):
            for topo in ("contiguous","gapped"):
                for q in ("chi2x","chi2y","chi2max"):
                    for b in bins:w.writerow([r["label"],topo,q,b,r["hist"].get((topo,q,b),0)])


def summarize(r,det):
    print(f"\n--- {r['label']} ---")
    for want in (1,0):
        vals=[s for p,s in r["stats"].items() if int(contiguous(p,det))==want]
        sampled=sum(s["sampled"] for s in vals); tested=sum(s["tested"] for s in vals); comp=sum(s["compatible"] for s in vals)
        if sampled: print(f"{'contiguous' if want else 'gapped':10s}: sampled={sampled:,}, testable={tested:,} ({100*tested/sampled:.2f}%), compatible/tested={100*comp/tested if tested else 0:.2f}%, compatible/all={100*comp/sampled:.2f}%")


def main():
    a=argparse.ArgumentParser()
    a.add_argument("-b","--basic-info",required=True)
    a.add_argument("-ap","--analysis-parameters",default=None)
    a.add_argument("--reference",nargs="+",required=True); a.add_argument("--test",nargs="+",required=True)
    a.add_argument("--reference-label",default="Run103"); a.add_argument("--test-label",default="Run113")
    a.add_argument("--ntrig",type=int,default=3)
    a.add_argument("--sample-every",type=int,default=100,help="100 = initial 1%% sample; 1 = full scan")
    a.add_argument("--resolution-mm",type=float,default=3.6)
    a.add_argument("--chi2-max",type=float,default=None)
    a.add_argument("--examples-per-category",type=int,default=2)
    a.add_argument("--progress-every",type=int,default=100000)
    a.add_argument("-r","--results",default="toplogy"); a.add_argument("-rd","--dir-results",default="Results/Topology")
    x=a.parse_args(); det=parse_db(x.basic_info); ap=parse_ap(x.analysis_parameters); chi2=ap["TrackKhi2Max"] if x.chi2_max is None else x.chi2_max
    if x.ntrig!=3: print("WARNING: Ntrigger=2 has zero line-fit degrees of freedom; geometry test is not discriminating.",file=sys.stderr)
    print("Detector:")
    for i,c in enumerate(det["ch"]): print(f" ch{i}: {c['name']:14s} z={c['z']:g} size={c['sx']}x{c['sy']} pad=({c['padx']:g},{c['pady']:g})")
    print(f"Settings: ClusterSizeLimit={ap['ClusterSizeLimit']} ClusterBreak={ap['ClusterBreak']} NClustersLimit={ap['NClustersLimit']} chi2max={chi2} resolution={x.resolution_mm} mm sample_every={x.sample_every}")
    rf=expand(x.reference); tf=expand(x.test)
    if not rf or not tf: sys.exit("Reference or test glob matched no files")
    ref=analyze(x.reference_label,rf,det,ap,x.ntrig,x.sample_every,x.resolution_mm,chi2,x.examples_per_category,x.progress_every)
    test=analyze(x.test_label,tf,det,ap,x.ntrig,x.sample_every,x.resolution_mm,chi2,x.examples_per_category,x.progress_every)
    write_outputs(Path(x.dir_results),x.results,ref,test,det); summarize(ref,det); summarize(test,det)
    print("\nDone. 'compatible' means only a 3-layer line candidate in both X and Y, not a normal reconstructed muon track.")

if __name__=="__main__": main()

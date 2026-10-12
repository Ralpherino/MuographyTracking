#!/usr/bin/env python3
"""Finalize operational Run103/Run113 three-trigger background-enriched selection.

Consumes EXISTING aggregated results: no EBE parsing or track reconstruction.
Requires Python 3 standard library; matplotlib is optional for figure generation.
Data sources: BackgroundCharacterization_v2 full class counts, MixedEventNull
10k-trial high-statistics summary, MissingTriggerValidation deterministic 1/20
sample per run. Does not infer gamma purity.
"""
import argparse
import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path

RUNS=('Run103','Run113')
TOPO=('contiguous','gapped')
GEO=('compatible','incompatible','missing_xy')
SCANNED={'Run103':20659362,'Run113':102859424}


def readcsv(p):
    with open(p,newline='') as f:
        return list(csv.DictReader(f))


def writecsv(p,rows,fields):
    with open(p,'w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=fields)
        w.writeheader(); w.writerows(rows)


def pct(a,b):
    return 100*a/b if b else 0.0


def make(input_dir,output_dir):
    input_dir=Path(input_dir);output_dir=Path(output_dir)
    output_dir.mkdir(parents=True,exist_ok=True)
    paths=[input_dir/'background_selection_source_classes.csv',input_dir/'background_selection_source_mixed.csv']
    for run in RUNS:
        paths += [input_dir/f'background_selection_source_{run}_missing_summary.csv',input_dir/f'background_selection_source_{run}_missing_patterns.csv']
    for p in paths:
        if not p.exists(): raise FileNotFoundError(p)
    rows=readcsv(paths[0]); mixed=readcsv(paths[1]);
    bykey={}
    for r in rows:
        key=(r['dataset'],r['topology'],r['geometry'])
        assert key not in bykey, f'duplicate classification: {key}'
        bykey[key]=int(r['events'])
    assert set(bykey)=={(run,topo,geo) for run in RUNS for topo in TOPO for geo in GEO},'Missing class row'
    n3={r:sum(bykey[(r,t,g)] for t in TOPO for g in GEO) for r in RUNS}
    assert n3=={'Run103':5372177,'Run113':74333026},f'Unexpected N3 denominator: {n3}'
    selected={'Run103':{},'Run113':{}}
    cls=[]
    for run in RUNS:
        for topo in TOPO:
            for geo in GEO:
                n=bykey[(run,topo,geo)]
                cls.append(dict(run=run,topology=topo,geometry=geo,events=n,
                                percent_within_three_triggers=round(pct(n,n3[run]),7),
                                percent_within_all_events=round(pct(n,SCANNED[run]),7),
                                interpretation=( 'primary_background_enriched' if topo=='gapped' and geo=='incompatible'
                                               else 'indeterminate_background_like' if topo=='gapped' and geo=='missing_xy'
                                               else 'gapped_track_compatible_control' if topo=='gapped'
                                               else 'contiguous_track_compatible_control' if geo=='compatible'
                                               else 'other_contiguous_control')))
        selected[run]={'primary':bykey[(run,'gapped','incompatible')],
                       'indeterminate':bykey[(run,'gapped','missing_xy')],
                       'gapped_compatible':bykey[(run,'gapped','compatible')]}
    # Stage counts form a branching cut-flow; row order is not a strict linear filtering chain.
    cutflow=[]
    for run in RUNS:
        counts={'all_ebe_events':SCANNED[run],
                'exactly_three_triggers':n3[run],
                'contiguous_three_triggers':sum(bykey[(run,'contiguous',g)] for g in GEO),
                'gapped_three_triggers':sum(bykey[(run,'gapped',g)] for g in GEO),
                'gapped_line_compatible':selected[run]['gapped_compatible'],
                'gapped_line_incompatible_primary':selected[run]['primary'],
                'gapped_insufficient_xy_separate':selected[run]['indeterminate'],
                'contiguous_line_compatible_control':bykey[(run,'contiguous','compatible')]}
        assert counts['contiguous_three_triggers']+counts['gapped_three_triggers']==n3[run]
        assert sum(selected[run].values())==counts['gapped_three_triggers']
        for k,v in counts.items():
            cutflow.append(dict(run=run,stage=k,events=v,
                                fraction_of_all_events=round(v/SCANNED[run],9),
                                fraction_of_three_trigger_events=round(v/n3[run],9)))
    mixmap={(r['dataset'],r['topology']):r for r in mixed}
    for run in RUNS:
        assert (run,'gapped') in mixmap
    missing={}; patrows=[]
    for run in RUNS:
        srows=readcsv(input_dir/f'background_selection_source_{run}_missing_summary.csv')
        assert len(srows)==1; s=srows[0]
        assert s['run']==run
        assert int(s['scanned'])==SCANNED[run]
        p=readcsv(input_dir/f'background_selection_source_{run}_missing_patterns.csv')
        assert len(p)==10,'Expected ten physical span-four masks'
        for k in ('selected','testable','line_compatible','compatible','match_xy'):
            assert sum(int(x[k]) for x in p)==int(s[k]), (run,k)
        assert abs(sum(int(x['compatible'])*float(x['null_match_fraction']) for x in p)-float(s['null_expected_matches']))<1e-7
        missing[run]=s
        for r in p:
            patrows.append(dict(run=run,trigger_mask=int(r['pattern']),binary_mask=r['pattern_bin'],
                                triggered_chambers=r['triggered_chambers'],missing_chamber=int(r['missing_chamber']),
                                selected_sampled=int(r['selected']),compatible_sampled=int(r['compatible']),
                                matching_xy=int(r['match_xy']),matching_fraction=round(float(r['match_fraction_given_compatible']),9),
                                shuffled_fraction=round(float(r['null_match_fraction']),9)))
    # The high-trial-count mixed-event null is a separate 1% sampling analysis.
    nullrows=[]
    for run in RUNS:
        for topo in TOPO:
            rr=mixmap[(run,topo)]
            ntest=int(rr['testable_sampled']); nmatch=int(rr['observed_compatible'])
            assert 0<=nmatch<=ntest
            nullrows.append(dict(run=run,topology=topo,testable_sampled=ntest,
                                 line_compatible_sampled=nmatch,
                                 observed_fraction=round(float(rr['observed_fraction']),9),
                                 mixed_event_fraction=round(float(rr['mixed_fraction_weighted']),9),
                                 observed_to_mixed_ratio=round(float(rr['observed_over_mixed']),5)))
    writecsv(output_dir/'background_selection_classes.csv',cls,
             ['run','topology','geometry','events','percent_within_three_triggers','percent_within_all_events','interpretation'])
    writecsv(output_dir/'background_selection_cutflow.csv',cutflow,
             ['run','stage','events','fraction_of_all_events','fraction_of_three_trigger_events'])
    writecsv(output_dir/'background_selection_missing_patterns.csv',patrows,
             ['run','trigger_mask','binary_mask','triggered_chambers','missing_chamber','selected_sampled','compatible_sampled','matching_xy','matching_fraction','shuffled_fraction'])
    writecsv(output_dir/'background_selection_mixed_null.csv',nullrows,
             ['run','topology','testable_sampled','line_compatible_sampled','observed_fraction','mixed_event_fraction','observed_to_mixed_ratio'])
    a=selected['Run103']['primary']/n3['Run103']; b=selected['Run113']['primary']/n3['Run113']; factor=b/a
    m103=missing['Run103'];m113=missing['Run113']
    p103=int(m103['match_xy'])/int(m103['compatible']);p113=int(m113['match_xy'])/int(m113['compatible'])
    lines=['# Final background-enriched selection — MTS-41 Run103 versus Run113','',
           '**Status:** Original supervisor Task 5, empirical selection finalized. This report does not identify individual photons and does not measure gamma purity or efficiency.','',
           '## Scientific purpose','',
           'Construct an operational **background-enriched** sample from sparse, non-consecutive three-chamber trigger events, and distinguish incompatible trajectories from incomplete information. Use Run103 as a muon-like reference and Run113 as a background-enriched data-taking condition. Neither run is a particle-pure truth sample.','',
           '## Reproducible event definition','',
           'For each raw EBE event:','',
           '1. Read the 8-bit `P` trigger mask using chamber assignments and physical z positions in the matching MTS-41 `.db`. **Exactly three triggered chambers** are required. An active chamber is not the same as a triggered chamber.','2. Apply the established chamber X/Y hit corrections and clustering using the matching `.ap` configuration. The full-classification analysis used: `ClusterSizeLimit=8`, `ClusterBreak=3`, `NClustersLimit=4`; raw clusters exceeding the limits can leave insufficient usable clusters.','3. Sort the triggered chambers by physical z. Classify **contiguous** if their z-order indices span exactly 3 layers; otherwise classify **gapped**.','4. In the three triggered chambers, independently select the best X(z) and Y(z) straight-line cluster combinations; use a nominal position resolution of **3.6 mm** and one degree of freedom for each three-point fit.','5. **Compatible:** both fits exist and each satisfies `χ²/ndf ≤ 4`. **Incompatible:** both fits exist but at least one has `χ²/ndf > 4`. **Insufficient X/Y (`missing_xy`):** one or both fits cannot be made with the processed clusters. The last outcome is *not* treated as a failed-trajectory measurement.','',
           '### Final labels','',
           '| Label | Topology | Geometry | Use |','|---|---|---|---|',
           '| `primary_background_enriched` | Gapped | Incompatible | Primary observable background-like selection. |',
           '| `indeterminate_background_like` | Gapped | Insufficient X/Y | Report separately; not included in primary selection. |',
           '| `gapped_track_compatible_control` | Gapped | Compatible | Track-compatible gapped control; may include missed triggers. |',
           '| `contiguous_track_compatible_control` | Contiguous | Compatible | Muon-like reference/control; also compare across runs. |',
           '| Other contiguous categories | Contiguous | Incompatible or insufficient X/Y | Auxiliary controls, not in primary sample. |','',
           '### Pseudocode','',
           '```python',
           'if popcount(trigger_mask) != 3: excluded_from_this_selection()',
           'else:',
           '    topo = "contiguous" if physical_z_span_in_layers == 3 else "gapped"',
           '    if fit_x is None or fit_y is None: geo = "missing_xy"',
           '    elif fit_x.chi2_ndf <= 4 and fit_y.chi2_ndf <= 4: geo = "compatible"',
           '    else: geo = "incompatible"',
           '    primary = (topo == "gapped" and geo == "incompatible")',
           '    indeterminate = (topo == "gapped" and geo == "missing_xy")',
           '```','',
           '## Full-run event counts (exclusive classes)','',
           '| Class | Run103 | Run113 |','|---|---:|---:|']
    for t in TOPO:
        for g in GEO:
            lines.append(f'| {t} / {g} | {bykey[("Run103",t,g)]:,} | {bykey[("Run113",t,g)]:,} |')
    lines+=['',f'Run103: **{SCANNED["Run103"]:,}** scanned EBE events, of which **{n3["Run103"]:,}** have three triggers ({pct(n3["Run103"],SCANNED["Run103"]):.3f}%).',
            f'Run113: **{SCANNED["Run113"]:,}** scanned EBE events, of which **{n3["Run113"]:,}** have three triggers ({pct(n3["Run113"],SCANNED["Run113"]):.3f}%).','',
            f'Primary selection among three-trigger events: Run103 **{pct(selected["Run103"]["primary"],n3["Run103"]):.3f}%** versus Run113 **{pct(selected["Run113"]["primary"],n3["Run113"]):.3f}%**. The Run113/Run103 *topology prevalence ratio* is **{factor:.2f}×**. This is not a gamma-purity enhancement factor.','',
            f'Indeterminate gapped events: Run103 **{pct(selected["Run103"]["indeterminate"],n3["Run103"]):.3f}%**; Run113 **{pct(selected["Run113"]["indeterminate"],n3["Run113"]):.3f}%** of three-trigger events; they remain separate because missing fit information can result from cluster filtering, not just physical absence of hits.','',
            '## Independent mixed-event null for geometrical compatibility','',
            'The mixed-event study exchanges chamber-level X/Y cluster lists among otherwise testable events with the **same exact trigger mask**, retaining per-chamber occupancy/position structure while destroying inter-chamber alignment. Rates below refer to testable events in its deterministic sampling study, not the full-run event count.','',
            '| Run / topology | Observed 3-layer compatible | Mixed-event expectation | Observed/null |','|---|---:|---:|---:|']
    for r in nullrows:
        lines.append(f'| {r["run"]} / {r["topology"]} | {100*r["observed_fraction"]:.3f}% | {100*r["mixed_event_fraction"]:.3f}% | {r["observed_to_mixed_ratio"]:.2f}× |')
    lines+=['',
            'In particular, Run113 **gapped** patterns have an observed compatibility fraction only modestly above the constructed accidental baseline. This supports a predominantly weakly correlated topology *under this test*, but cannot determine individual event identities or prove that the apparent excess is physically gamma-induced.','',
            '## Independent validation of an untriggered intermediate chamber','',
            'This separate deterministic 1-in-20 analysis concerns **only** three-trigger patterns spanning exactly four z-ordered chambers (one untriggered intermediate layer). Fit X/Y trajectories in triggered layers without using the missing chamber; require both χ²/ndf ≤ 4, extrapolation inside its active area, and cluster-center matches within **2.6 strips** in X and Y. Compare to a within-pattern shuffled-event null.','',
            '| Stage or metric | Run103 | Run113 |','|---|---:|---:|',
            f'| Sampled three-trigger span-four events | {int(m103["selected"]):,} | {int(m113["selected"]):,} |',
            f'| Testable X/Y fits | {int(m103["testable"]):,} | {int(m113["testable"]):,} |',
            f'| Compatible trajectories crossing missing layer | {int(m103["compatible"]):,} | {int(m113["compatible"]):,} |',
            f'| X/Y cluster match in missing layer | {int(m103["match_xy"]):,} | {int(m113["match_xy"]):,} |',
            f'| Match fraction among compatible trajectories | {100*p103:.2f}% | {100*p113:.2f}% |',
            f'| Shuffled-control match fraction | {100*float(m103["null_match_fraction_weighted"]):.3f}% | {100*float(m113["null_match_fraction_weighted"]):.3f}% |',
            f'| Matches among all sampled span-four events | {pct(int(m103["match_xy"]),int(m103["selected"])):.3f}% | {pct(int(m113["match_xy"]),int(m113["selected"])):.3f}% |','',
            'The intermediate-chamber match fraction is higher for Run103 within *each* of the ten exact eligible masks. Nevertheless, only a small fraction of the entire Run113 span-four sample shows the combined compatible-and-matched signature. **Do not equate the unmatched majority with identified gamma events.** An untriggered chamber with aligned hits may reflect a trigger failure, charged-particle passage, or other correlated detector activity.','',
            '## Limitations and scientific claims','',
            '- `Background-enriched` means enriched in the **defined topology** relative to Run103; Run103 is not a pure muon truth dataset, and Run113 is not a gamma-labeled truth dataset. Do not report gamma purity, gamma efficiency, contamination fraction or flux from these counts.','- The test is conditioned on exactly three triggered chambers and thus is not a measurement of the entire detector event distribution or a gamma selection across all multiplicities.','- `missing_xy` can be caused by noisy clusters, lost clusters due to `ClusterSizeLimit` and `NClustersLimit`, broken channels or other reconstruction failures; it is indeterminate rather than demonstrably incompatible.','- X and Y best fits are selected independently and the Python reconstruction is an approximation of the C++ tracking code; the selection thresholds and configurations must be held fixed when comparing runs.','- High cluster multiplicity increases the number of possible fitting combinations and can cause incidental compatibility; mixed-event controls quantify an operational accidental baseline, not all detector systematics.','- Trigger and raw-hit chamber activities are different quantities; a non-triggered intermediate layer can still carry strips or clusters.','- Detector acceptance, changing background composition, electronics/pedestal conditions and position-dependent chamber efficiencies can influence run-to-run population differences.','- The mixture/null results and missing-trigger tests use subsamples of the raw datasets; their rates have sampling and model systematic uncertainties. The full six-class counts are from the full EBE scans.','- The physical chamber-name mapping should be checked against the relevant `.db` rather than inferring serial-number order from a `StdDump` header.','',
            '## Conclusions suitable for thesis','',
            'The MTS-41 background-suppression study defines a reproducible three-trigger background-enriched selection using non-consecutive chamber triggers and the failure of a testable straight-line trajectory to satisfy independent X and Y χ² requirements. This selection comprises **28,937,174** events in Run113 and **144,188** events in Run103, or **38.929%** and **2.684%** of the respective three-trigger samples. Events with insufficient X/Y information are reported as a distinct indeterminate population. Mixed-event comparisons indicate that most line-compatible gapped Run113 configurations are consistent in rate with a weak-correlation baseline, while targeted missing-trigger tests demonstrate a small population containing spatially correlated hits in untriggered intermediate chambers. Thus Run113 is strongly enriched in non-track-like detector *topologies*, but no individual gamma identification or gamma-purity measurement follows from these data alone.','',
            '## Provenance and reproduction','',
            'Source class counts: `background_selection_source_classes.csv`, from the existing full-run `BackgroundCharacterization_v2.py` analysis.',
            'Mixed-event expectation: `background_selection_source_mixed.csv`, high-trial `MixedEventNull.py` output.',
            'Intermediate-chamber sample: `background_selection_source_Run103_missing_summary.csv`, `background_selection_source_Run113_missing_summary.csv` and their matching pattern tables, from `MissingTriggerValidation.py`.','',
            'Regenerate the derived tables and this document with `python3 AnalysisScripts/FinalizeBackgroundSelection.py --input-dir Results --output-dir Results` after copying this package into the repository. No raw EBE scan is required to reproduce these aggregated figures.']
    (output_dir/'background_selection_report.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    config=dict(analysis='operational_three_trigger_background_enriched_selection',
                selection='NTrigger=3 AND noncontiguous z-ranked trigger chambers AND both X/Y fits present AND at least one chi2/ndf>4',
                labels={'primary':'gapped/incompatible','indeterminate':'gapped/missing_xy','control':'contiguous/compatible and gapped/compatible'},
                resolution_mm=3.6,chi2_ndf_max_per_projection=4.0,cluster_size_limit=8,cluster_break=3,max_clusters_per_axis=4,
                missing_trigger_validation={'event_id_mod':20,'z_span_layers':4,'position_match_radius_strips':2.6},
                n_scanned=SCANNED,n_three_trigger=n3,
                input_sha256={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
                caveat='Three-layer proxy reconstruction; no gamma truth labels, purity, or gamma-selection efficiency.')
    (output_dir/'background_selection_manifest.json').write_text(json.dumps(config,indent=2)+'\n')
    # Optional compact main-result figure. Values are fractions of NTrigger=3.
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        fig,ax=plt.subplots(figsize=(9.5,3.8))
        y=[1,0]
        for ci,(t,g) in enumerate((('contiguous','compatible'),('contiguous','incompatible'),('contiguous','missing_xy'),('gapped','compatible'),('gapped','incompatible'),('gapped','missing_xy'))):
            vals=[100*bykey[(run,t,g)]/n3[run] for run in RUNS]
            if ci==0: bottoms=[0,0]
            ax.barh(y,vals,left=bottoms,height=0.62,label=f'{t} / {g}')
            bottoms=[x+z for x,z in zip(bottoms,vals)]
        ax.set_yticks(y,['Run103','Run113']); ax.set_xlim(0,100)
        ax.set_xlabel('Fraction of all events with exactly three trigger bits (%)')
        ax.set_title('Exclusive trigger-topology and geometric-fit classes')
        ax.legend(loc='upper center',bbox_to_anchor=(0.5,-0.18),ncol=3,fontsize=8)
        ax.grid(axis='x',alpha=0.2);ax.set_axisbelow(True)
        fig.tight_layout();fig.savefig(output_dir/'background_selection_classes.png',dpi=190,bbox_inches='tight')
        plt.close(fig)
    except ImportError:
        print('matplotlib not installed: skipping optional PNG plot')
    print(f'Validated full-event cutflow: Run103 N3={n3["Run103"]:,}; Run113 N3={n3["Run113"]:,}')
    print(f'Primary selection: Run103={selected["Run103"]["primary"]:,}; Run113={selected["Run113"]["primary"]:,}; topology prevalence ratio={factor:.3f}')
    print(f'Outputs written in {output_dir.resolve()}')


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input-dir',default='Results',help='Location of the six packaged source CSV files')
    p.add_argument('--output-dir',default='Results')
    a=p.parse_args();make(a.input_dir,a.output_dir)

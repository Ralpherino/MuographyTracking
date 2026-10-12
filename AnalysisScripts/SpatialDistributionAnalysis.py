#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Summarize chamber occupancy and retained-cluster spatial profiles.

Inputs: precomputed, full Ntrigger=3 class/chamber/cluster-position CSVs from
BackgroundCharacterization_v2.py. No EBE rereading and no track reconstruction.

Scientific limits:
- Classifies EVENTS using the existing three-trigger topology/geometry labels;
  it does not identify the particle that produced an individual cluster.
- 1D X/Y distributions are marginal profiles; do not combine into a physical
  joint 2D hit map.
- Position histogram includes only retained clusters after the source analysis
  cluster-processing rules. Chamber activation is based on corrected strip hits.
- Edge fraction is geometric location within a chamber, not a noise diagnostic.
"""
import argparse
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import PowerNorm

SELECTED = [
    ('Run103', 'contiguous', 'compatible'),
    ('Run113', 'contiguous', 'compatible'),
    ('Run113', 'gapped', 'incompatible'),
    ('Run113', 'gapped', 'missing_xy'),
]
LABELS = [
    'Run103: contiguous / compatible',
    'Run113: contiguous / compatible',
    'Run113: gapped / incompatible',
    'Run113: gapped / missing XY',
]


def parse_args():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--class-summary', default='Results/spatial_class_input.csv')
    p.add_argument('--chamber-summary', default='Results/spatial_chamber_input.csv')
    p.add_argument('--cluster-hist', default='Results/spatial_cluster_input.csv')
    p.add_argument('-o', '--outdir', default='Results')
    p.add_argument('--bins', type=int, default=32)
    p.add_argument('--edge-fraction', type=float, default=0.1)
    return p.parse_args()


def label_name(key):
    return '/'.join(key)


def line_for_key(df, key):
    run, topo, geom = key
    return df[(df.dataset == run) & (df.topology == topo) & (df.geometry == geom)]


def check_inputs(classes, chambers, hist):
    fields = {
        'class summary': (classes, {'dataset', 'topology', 'geometry', 'events', 'mean_extra_active'}),
        'chamber summary': (chambers, {'dataset', 'topology', 'geometry', 'chamber_index', 'events', 'active_any_fraction', 'active_xy_fraction'}),
        'cluster histogram': (hist, {'dataset', 'topology', 'geometry', 'chamber_index', 'axis', 'local_center', 'count'}),
    }
    for name, (df, required) in fields.items():
        missing = required - set(df.columns)
        if missing:
            raise ValueError(f'{name}: missing columns {sorted(missing)}')
    if (hist['count'] < 0).any() or (hist['local_center'] < 0).any():
        raise ValueError('Negative positions/counts in histograms')
    if (classes['events'] <= 0).any():
        raise ValueError('Class summary contains nonpositive events')
    keycols = ['dataset', 'topology', 'geometry']
    for _, ch in chambers.groupby(keycols):
        if len(ch) != len(ch['chamber_index'].unique()):
            raise ValueError('Duplicate chamber in chamber summary')
        c = line_for_key(classes, tuple(ch.iloc[0][keycols]))
        if len(c) != 1 or (ch.events != c.events.iloc[0]).any():
            raise ValueError('Chamber event counts do not match class summary')
    # Check histogram totals correspond to the retained cluster totals used in the class summary.
    sums = hist.groupby(keycols + ['axis'])['count'].sum()
    for row in classes.itertuples(index=False):
        k = (row.dataset, row.topology, row.geometry)
        for axis in ('X', 'Y'):
            observed = sums.get(k + (axis,), 0)
            expected = row.events * (row.mean_clusters_x if axis == 'X' else row.mean_clusters_y)
            if abs(observed - expected) > max(2, 1e-6 * expected):
                raise ValueError(f'Cluster position histogram incompatible with class-summary totals: {k}/{axis} {observed} vs {expected}')


def make_outputs(classes, chambers, hist, args):
    odir = Path(args.outdir)
    odir.mkdir(parents=True, exist_ok=True)
    groupcols = ['dataset', 'topology', 'geometry']
    # Determine chamber axis dimensions from supplied position-grid maxima.
    # These specific input histograms include positions 0 through N-1 and
    # half-strip centers; fail rather than infer an incomplete geometry.
    dimensions = {}
    for (ch, axis), group in hist.groupby(['chamber_index', 'axis']):
        lo, hi = group.local_center.min(), group.local_center.max()
        if lo != 0 or hi < 3 or abs(hi-round(hi)) > 1e-6:
            raise ValueError(f'Cannot establish complete local strip dimension for chamber {ch} {axis}; require --source geometry support')
        dimensions[(int(ch), axis)] = int(hi) + 1

    h = hist.copy()
    h['axis_size'] = [dimensions[(int(i), str(a))] for i, a in zip(h.chamber_index, h.axis)]
    h['u'] = (h.local_center + 0.5) / h.axis_size
    h['spatial_bin'] = np.minimum(np.floor(h.u * args.bins).astype(int), args.bins - 1)
    if ((h.u < 0) | (h.u > 1)).any():
        raise ValueError('Cluster positions outside configured active width')
    h['edge'] = (h.u < args.edge_fraction) | (h.u >= 1 - args.edge_fraction)
    h['center'] = (h.u >= .4) & (h.u < .6)

    profile = h.groupby(groupcols + ['chamber_index', 'chamber_name', 'axis', 'spatial_bin'], as_index=False)['count'].sum()
    totals = profile.groupby(groupcols + ['chamber_index', 'axis'])['count'].transform('sum')
    profile['fraction_in_chamber_axis'] = profile['count'] / totals
    evcounts = classes.set_index(groupcols)['events']
    profile['events_in_class'] = [int(evcounts.loc[tuple(r)]) for r in profile[groupcols].itertuples(index=False, name=None)]
    profile['clusters_per_event_in_bin'] = profile['count'] / profile.events_in_class
    profile['u_bin_start'] = profile.spatial_bin / args.bins
    profile['u_bin_end'] = (profile.spatial_bin + 1) / args.bins
    profile = profile.sort_values(groupcols + ['chamber_index','axis','spatial_bin'])
    profile.to_csv(odir / 'spatial_profile_bins.csv', index=False)

    rows = []
    for (run,topo,geom,ch,name,axis), group in h.groupby(groupcols + ['chamber_index','chamber_name','axis']):
        total = group['count'].sum()
        if total == 0:
            continue
        n_ev = int(evcounts.loc[(run,topo,geom)])
        rows.append(dict(dataset=run,topology=topo,geometry=geom,chamber_index=ch,chamber_name=name,axis=axis,
                         events=n_ev,axis_strips=dimensions[(int(ch),str(axis))],retained_clusters=int(total),
                         retained_clusters_per_event=total/n_ev,
                         edge_10pct_fraction=group.loc[group.edge,'count'].sum()/total,
                         central_20pct_fraction=group.loc[group.center,'count'].sum()/total,
                         edge_width_fraction=args.edge_fraction,
                         edge_clusters=int(group.loc[group.edge,'count'].sum())))
    edges=pd.DataFrame(rows).sort_values(groupcols+['chamber_index','axis'])
    edges.to_csv(odir/'spatial_edge_summary.csv', index=False)

    activity=chambers.merge(classes[groupcols + ['mean_extra_active']],on=groupcols,validate='many_to_one').sort_values(groupcols+['chamber_index'])
    activity.to_csv(odir/'spatial_activity.csv',index=False)
    plot_profiles(profile, activity, args, odir)
    plot_sn16(profile, odir)
    make_report(classes, activity, edges, odir, args)
    print(f'Classified {len(classes)} groups; wrote {len(profile)} profile bins and {len(edges)} chamber/axis summaries to {odir}')


def plot_profiles(profile, activity, args, odir):
    available = [(key, label) for key,label in zip(SELECTED,LABELS) if not line_for_key(activity,key).empty]
    ncols=len(available)
    if not ncols:
        return
    arrays=[]
    for axis in ['X','Y']:
        for key,label in available:
            g=line_for_key(profile,key)
            g=g[g.axis==axis]
            mat=np.zeros((8,args.bins))
            for row in g.itertuples(index=False):
                if int(row.chamber_index)<8:
                    mat[int(row.chamber_index),int(row.spatial_bin)]=100*row.fraction_in_chamber_axis
            arrays.append(mat)
    vmax=max(a.max() for a in arrays)
    fig, axs = plt.subplots(2, ncols, figsize=(4.5*ncols,8), sharex=True, sharey=True,constrained_layout=True)
    axs=np.atleast_2d(axs)
    ref=None
    for i,axis in enumerate(['X','Y']):
        for j,(key,label) in enumerate(available):
            ax=axs[i,j];a=arrays[i*ncols+j]
            ref=ax.imshow(a,origin='lower',aspect='auto',interpolation='nearest',extent=(0,1,-.5,7.5),
                          norm=PowerNorm(gamma=.5,vmin=0,vmax=vmax))
            ax.set_title(label,fontsize=10)
            ax.set_xlabel(f'Normalized {axis} coordinate')
            if j==0:
                ax.set_ylabel(f'{axis}-cluster distribution\nChamber index')
            ax.set_yticks(range(8))
            ch=line_for_key(activity,key).set_index('chamber_index')
            if len(ch)==8:
                ax.set_yticklabels([f"{k}: {str(ch.loc[k,'chamber_name']).split('_')[-1]}" for k in range(8)])
    cbar=fig.colorbar(ref,ax=axs.ravel().tolist(),fraction=.016,pad=.01)
    cbar.set_label('Fraction of retained clusters in chamber/axis bin (%)')
    fig.suptitle('Class-conditioned 1D spatial profiles (not reconstructed 2D hit maps)',fontsize=13)
    fig.savefig(odir/'spatial_profiles.png',dpi=180,bbox_inches='tight')
    plt.close(fig)

    fig,axs=plt.subplots(1,2,figsize=(13,4.9),sharex=True,sharey=True,constrained_layout=True)
    for key,label in available:
        group=line_for_key(activity,key).sort_values('chamber_index')
        axs[0].plot(group.chamber_index,100*group.active_any_fraction,marker='o',label=label)
        axs[1].plot(group.chamber_index,100*group.active_xy_fraction,marker='o',label=label)
    for ax,title in zip(axs,['At least one raw X or Y strip','Both raw X and Y strips']):
        ax.set_title(title);ax.set_xlabel('Configured chamber index');ax.set_xticks(range(8));ax.grid(alpha=.25)
    axs[0].set_ylabel('Fraction of events in selected class (%)')
    legend_handles, legend_labels = axs[0].get_legend_handles_labels()
    fig.legend(legend_handles, legend_labels, loc='outside lower center', ncol=2, fontsize=9)
    fig.suptitle('Chamber activity conditional on selected event class',fontsize=13)
    fig.savefig(odir/'spatial_activity.png',dpi=180,bbox_inches='tight')
    plt.close(fig)


def plot_sn16(profile, odir):
    fig,ax=plt.subplots(figsize=(8,4.6),constrained_layout=True)
    found=False
    for key,label in zip(SELECTED,LABELS):
        sub=line_for_key(profile,key)
        sub=sub[(sub.chamber_index==7)&(sub.axis=='X')].sort_values('spatial_bin')
        if len(sub):
            ax.plot((sub.u_bin_start+sub.u_bin_end)/2,100*sub.fraction_in_chamber_axis,marker='.',lw=1.8,label=label)
            found=True
    if found:
        ax.set(xlabel='Normalized X coordinate within chamber 7',ylabel='Fraction of retained chamber X clusters / bin (%)',
               title='Chamber 7 (sn16 in analysis configuration): X-cluster positions')
        ax.axvspan(0,.1,alpha=.10);ax.axvspan(.9,1,alpha=.10)
        ax.grid(alpha=.25);ax.legend(fontsize=8)
        fig.savefig(odir/'spatial_sn16_x.png',dpi=180,bbox_inches='tight')
    plt.close(fig)


def make_report(classes, activity, edges, odir, args):
    def pct(x):return f'{100*x:.2f}%'
    def found(k):return not line_for_key(classes,k).empty
    lines=[
        '# Spatial distribution of three-trigger event classes',
        '',
        '## Scope and input provenance',
        '',
        'Calculated from **existing full-population Ntrigger=3 aggregate files** produced by `BackgroundCharacterization_v2.py`:',
        '`spatial_class_input.csv`, `spatial_chamber_input.csv`, `spatial_cluster_input.csv`.',
        'No EBE reread or new track reconstruction was performed. The published Run103 and Run113 total-event populations are **not** the denominator here:',
        'all event fractions below are conditional on the specified Ntrigger=3 event class.',
        '',
        'The retained cluster-position histogram reflects the source processing after hit corrections, cluster size/break and multiplicity restrictions. Its counts differ from raw strip-hit occupancy.',
        'The 1D X/Y marginal histograms **cannot** be multiplied or combined into a genuine joint 2D spatial distribution.',
        '',
        '## Analysis populations',
        '',
        '| Run and class | Events | Mean active chambers (any X/Y) | Mean active nontriggered chambers |',
        '|---|---:|---:|---:|',
    ]
    for key,label in zip(SELECTED,LABELS):
        if not found(key):continue
        sub=line_for_key(classes,key).iloc[0]
        a=line_for_key(activity,key)
        avg=(a.active_any_fraction).sum()
        lines.append(f'| {label} | {int(sub.events):,} | {avg:.3f} | {sub.mean_extra_active:.3f} |')
    lines+=['','`mean_active_nontriggered_chambers` counts chambers with at least one raw X or Y hit that are *not* present in the event trigger bit mask; it does not count extra raw hits or prove an additional particle.',
            '','## Per-chamber spatial statistics','',
            'The companion `spatial_activity.csv` gives chamber trigger fraction, active X, active Y, active XY and active-any probabilities for each event class.',
            '`spatial_profile_bins.csv` gives 32-bin X/Y retained-cluster distributions using normalized chamber coordinate',
            r'\(u=(x_{\mathrm{strip}}+0.5)/N_{\mathrm{strips}}\), and likewise for Y.',
            'Each chamber/axis distribution is independently normalized to unit total, which compares spatial **shape**, not absolute rates.',
            'The `clusters_per_event_in_bin` column uses the number of events in that event class as denominator and is a cluster multiplicity, **not** an event probability.',
            '','## Edge-region diagnostic','',
            f'An edge cluster has normalized position $u < {args.edge_fraction:g}$ or $u >= {1-args.edge_fraction:g}$.',
            'Edge fractions refer to the *retained* cluster centers. Previous raw-strip edge fractions across **all** events are not directly comparable.',
            '','| Class | sn16 X retained-cluster edge fraction | sn16 Y retained-cluster edge fraction |',
            '|---|---:|---:|']
    for key,label in zip(SELECTED,LABELS):
        e=line_for_key(edges,key)
        ex=e[(e.chamber_index==7)&(e.axis=='X')]
        ey=e[(e.chamber_index==7)&(e.axis=='Y')]
        if ex.empty or ey.empty:continue
        lines.append(f'| {label} | {pct(ex.iloc[0].edge_10pct_fraction)} | {pct(ey.iloc[0].edge_10pct_fraction)} |')
    lines+=['',
            'The Run103 sn16 X-edge concentration is pronounced even within the nominally line-compatible three-trigger reference class.',
            'This finding does not by itself distinguish a readout artifact from physically induced hit patterns, and edge exclusion is a sensitivity selection only.',
            '',
            '## Files and interpretation',
            '',
            '- `spatial_activity.png`: chamber-by-chamber raw-hit activity probabilities within each selected event class.',
            '- `spatial_profiles.png`: eight-chamber X/Y **one-dimensional** retained-cluster spatial profiles.',
            '- `spatial_sn16_x.png`: cluster-center distribution in the previously identified sn16 edge-sensitive axis.',
            '- `spatial_edge_summary.csv`: edge fractions for all event classes, chambers and axes.',
            '',
            '## Limitations / completion conditions for the original spatial task',
            '',
            '1. **Class-conditional map, not interaction-origin map:** retained clusters in gapped/incompatible events include the clusters that formed triggers and any additional clusters. These data do not identify a gamma-specific hit.',
            '2. **Topology/acceptance confounding:** Run103 contiguous/compatible and Run113 gapped/incompatible populations select different chamber combinations. Compare the *same* class across runs and different classes within a run before invoking detector-origin differences.',
            '3. **Only marginal positions:** the existing histograms cannot show whether X and Y coordinates in a given chamber belong to the same interaction or event. A true joint X–Y map requires event-level data.',
            '4. **sn16 caveat:** the histogram predates the edge-mask sensitivity pass; the edge-mask experiment changed event classes, not the underlying raw cluster positions in the precomputed three-trigger categories.',
            '5. **Physical chamber order:** plotted serial numbers follow the `Mts41.db` interpretation used by the previous analysis. The different wording/order in the acquisition StdDump header should be reconciled before reporting physical top/bottom positions as measured facts.',
            '',
            '**Status:** Original spatial task: full-dataset three-trigger spatial occupancy and X/Y retained-cluster profiles completed. A true 2D map of *unmatched* background-candidate hits cannot be obtained from these aggregate CSVs alone. Only perform that targeted EBE pass if required to answer the supervisor’s intended spatial-location question.',
            '']
    (odir/'spatial_summary.md').write_text('\n'.join(lines))


def main():
    args=parse_args()
    if args.bins < 4 or args.bins > 256 or not (0 < args.edge_fraction < .5):
        raise SystemExit('Expected 4 <= --bins <= 256 and 0 < --edge-fraction < 0.5')
    classes=pd.read_csv(args.class_summary)
    chambers=pd.read_csv(args.chamber_summary)
    hist=pd.read_csv(args.cluster_hist)
    check_inputs(classes,chambers,hist)
    make_outputs(classes,chambers,hist,args)

if __name__=='__main__':
    main()

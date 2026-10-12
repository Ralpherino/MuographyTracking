# MTS-41 Python analysis scripts

Standalone analyses for trigger patterns, gaseous-detector hit topology, chamber ADC distributions, and reconstructed-track control samples in the MuographyTracking workflow. These files do not alter the C++ tracking pipeline.

## Requirements

Python 3 and the standard library are sufficient for most scripts. Install `matplotlib` for plotting and `numpy pandas matplotlib` for `SpatialDistributionAnalysis.py`.

Keep the scripts together in `AnalysisScripts/`. Several modules import `HitTopologyAnalysis.py`, `BackgroundCharacterization.py`, and `AdcEventClasses.py` from this directory.

## Validation

From the repository root:

```bash
python3 -m compileall -q AnalysisScripts
python3 -m unittest discover -s AnalysisScripts -p 'Test*.py' -v
python3 AnalysisScripts/HitTopologyAnalysis.py --help
```

## Included modules

- Trigger and hit topology: `TriggerPatternAnalysis.py`, `HitTopologyAnalysis.py`, `MissingTriggerValidation.py`, `MixedEventNull.py`, `ManualEbeAudit.py`
- Background characterization and selection: `BackgroundCharacterization.py`, `FinalizeBackgroundSelection.py`, `SpatialDistributionAnalysis.py`
- Six-layer reference studies: `SixLayerOverlap.py`, `SixLayerReferenceSummary.py`
- ADC and extra-strip analysis: `AdcEventClasses.py`, `AdcExtraHitAnalysis.py`, `EdgeControlledAdcAnalysis.py`, `PlotAdcClasses.py`, `PlotAdcReference.py`
- Result auditing and tests: `ExistingResultsAudit.py`, `TestAdcEventClasses.py`

Individual scripts require corresponding run-specific detector `.db` and analysis `.ap` files, EBE/`.tl`/`.res` event products, or previously generated CSV inputs. See their command-line `--help` and module documentation. Some selection summaries have hard-coded Run103/Run113 denominators and must not be reused for different runs without validation.

**Scientific limitation:** an accepted reconstructed track is not MC truth. Trigger topology, geometry, strip occupancy, ADC readings, and mixed-event null rates do not by themselves measure gamma purity or identify individual particles. Keep ambiguous and missing-X/Y categories separate.

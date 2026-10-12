#////////////////////////////////////////////////////////////////////////
#//  Trigger Pattern Analysis - Implementaion
#//      version : 2026-10-09
#//      written by : R.Hage
#////////////////////////////////////////////////////////////////////////
#!/usr/bin/env python3
"""Self-contained checks for event matching and ADC decoding."""
import tempfile
import unittest
from pathlib import Path

import AdcEventClasses as adc


class EventSelectionTests(unittest.TestCase):
    def setUp(self):
        self.det = {"nch": 8, "ch": [
            {"sx": 128, "sy": 128, "misx": None, "misy": None,
             "brokenx": set(), "brokeny": set(), "padx": 4., "pady": 4.,
             "px": 0., "py": 0., "z": 40.*i}
            for i in range(8)]}
        self.track = (123, 0., 0., 102., 102., .2, .3)
        self.raw = [([25], [25]) if i < 6 else ([], []) for i in range(8)]

    def classify(self, raw=None, track=None):
        return adc.classify_track(self.raw if raw is None else raw,
                                  self.det, self.track if track is None else track,
                                  max_chi2=2.5, min_pairs=6, match_mm=15.,
                                  max_cluster_size=8)

    def test_clean(self):
        self.assertEqual(self.classify()[0], "clean_track")

    def test_single_extra_strip(self):
        raw = [(x[:], y[:]) for x, y in self.raw]
        raw[2][0].insert(0, 2)
        category, loc, paired = self.classify(raw)
        self.assertEqual(category, "clean_plus_single_strip")
        self.assertEqual(loc, (2, "X"))
        self.assertEqual(paired, 6)

    def test_two_extra_strips(self):
        raw = [(x[:], y[:]) for x, y in self.raw]
        raw[2][0].insert(0, 2)
        raw[3][1].insert(0, 2)
        self.assertEqual(self.classify(raw)[0], "other_accepted_track")

    def test_incomplete_track(self):
        raw = [(x[:], y[:]) for x, y in self.raw]
        raw[2] = ([], [])
        self.assertEqual(self.classify(raw)[0], "other_accepted_track")

    def test_nearby_ambiguous_cluster(self):
        raw = [(x[:], y[:]) for x, y in self.raw]
        raw[2][0].append(28)  # Separate cluster, but still within the matching window.
        self.assertEqual(self.classify(raw)[0], "other_accepted_track")

    def test_bad_track_quality(self):
        self.assertEqual(self.classify(track=(123, 0., 0., 102., 102., 2.6, .3))[0],
                         "other_accepted_track")

    def test_adc_mapping_and_pedestal(self):
        cfg = {"n_adc": 3, "mapping": [2, 0, 1],
               "ped": [1500., 1400., 1300.], "scale": [1., 2., .5]}
        raw, corr = adc.get_adc([b"Adc", b"1500", b"0", b"1440"], cfg, 3)
        self.assertEqual(raw, [0, 1440, 1500])
        self.assertEqual(corr, [0., 80., 100.])

    def test_track_order_validation(self):
        with tempfile.TemporaryDirectory() as folder:
            p = Path(folder) / "tracks.tl"
            p.write_text("1 0 TrackMain 0 0 0 100 100 .3 .4\n"
                         "2 0 TrackMain 0 0 0 100 100 .3 .4\n")
            self.assertEqual([x[0] for x in adc.track_records([str(p)])], [1, 2])
            p.write_text("2 0 TrackMain 0 0 0 100 100 .3 .4\n"
                         "1 0 TrackMain 0 0 0 100 100 .3 .4\n")
            with self.assertRaises(ValueError):
                list(adc.track_records([str(p)]))


if __name__ == "__main__":
    unittest.main(verbosity=2)

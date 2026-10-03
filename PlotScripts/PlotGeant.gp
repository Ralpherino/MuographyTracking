# Gnuplot Script for ChamberResults visualisation for Geant4 studies
# Header for specific file is needed
#	version: 2019-06-14
#	written by: G.Hamar

# General settings
set style data his
set view map;
set bars small
set timefmt "%Y-%m-%d_%H:%M:%S"
set print "-"

# Use info from header
MpY=2
MpX=int((NChambers+0.01)/MpY+0.99)
# Set plots type and filename
PdfFileName = Filenaming.".pdf"
set term pdf noenhanced size 6,4
set out PdfFileName

# Colours
CAll=0; CTrack=1; CEffTrack=3; CEffTrig=4; CAdc=0; CAdcScaled=5;
CSideX=1; CSideY=3;

# Id List for Chambers
IdAll=0; IdTrackCross=1; IdHitOk=2; IdTriggerOk=3; IdHitFound=4;
IdEff=5; IdTrackAdcSum=6; IdTrackAdcScaledSum=7; IdShiftXSum=8; IdShiftYSum=9;
IdFiducial=10; IdFiducialNew=11;
IdNTriggers=12; IdTriggerRate=13;
IdCluAllRaw=0; IdCluAll=1; IdCluOkRaw=2; IdCluOk=3;
# Id List for Detector
IdDetTrackCross=0; IdDetTriggerEff=1; IdDetTrackingEff=2; IdDetAcceptance=3; IdDetEffArea=4; IdDetFlux=5;
IdRateNEvents=0; IdRateNTracks=2; IdRateEventRate=2; IdRateTrackRate=3;
# Id List for THP
IdThpNEvents=0; IdThpSumTemperature=1; IdThpSumHumidity=2; IdThpSumPressure=3;
# Id List for HV
IdHvNEvents=0; IdHvSumUIseg=1; IdHvSumUMon=2; IdHvSumIMon=3;
# Column functions
C1(x)=column(x+2); C2(x)=column(x+3);

########################################################################
## Detector level plots
print "Preparing plots for the Detector ..."

# General ?
set title "Detector summary info"
unset xlabel; unset xtics; unset ylabel; unset ytics
plot [0:5][0.1:6] for [c=1:4] ResultsFile i IndexDetectorGeneral u (c):(5-$0):(column(c)) with labels t ""
set xtics auto; set ytics auto;

# AngleDep
set cbrange [*:*]
set title DetectorName." - Track Counts (AngleDep)"
set xlabel "MX []"; set xtics auto; set xrange [-SlopeMax:SlopeMax]
set ylabel "MY []"; set ytics auto; set yrange [-SlopeMax:SlopeMax]
unset key
plot ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTrackCross)) w image
set logscale zcb
plot ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTrackCross)) w image
set xrange [*:*]; set yrange [*:*]; set key; unset logscale zcb


########################################################################
### Chambers together
print "Preparing plots for the set of Chambers ..."
PlotAdcDiff=0.2;

# Position Dep Efficiencies
set xlabel "X [PadSizeX]"; set xtics 16; unset xlabel; unset xtics; set xrange [-1:SizeX];
set ylabel "Y [PadSizeY]"; set ytics 16; unset ylabel; unset ytics; set yrange [-1:SizeY];
unset key
set cbrange [*:*];
set colorbox; unset key
set cbtics font ",11" format "%4.0f"
# Track Cross PosDep
set multiplot layout MpY,MpX title "Track Cross Maps (Lin)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][1:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)) w image t ""
	}
unset multiplot
set logscale zcb
set multiplot layout MpY,MpX title "Track Cross Maps (Log)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][0.5:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)) w image t ""
	}
unset multiplot
unset logscale zcb
# Average Adc PosDep
set cbrange [0.8:1.2];
set colorbox; unset key
set cbtics font ",11" format "%4.0f"
set multiplot layout MpY,MpX title "Gain Maps (PosDep)"
set cbrange [GainAverage*(1.0-PlotAdcDiff):GainAverage*(1.0+PlotAdcDiff)]; unset colorbox; unset key
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdTrackAdcScaledSum)/C2(IdTrackCross):0) w image
	}
unset multiplot
#PosDep-
set xrange [*:*]; set xlabel; set xtics auto
set yrange [*:*]; set ylabel; set ytics auto
set cbrange [*:*]; set cbtics auto; set colorbox; set key

# HitFrequencies
set multiplot layout MpY,MpX title "Hit Frequency (Lin)"
unset xlabel; set xtics 16; unset mxtics; set format x "";
unset ylabel; unset ytics; unset key
set grid xtics
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [-1:][0:] ResultsFile i NIndexes*NChamber+IndexHitXDep u 1:2 lc CSideX, \
		ResultsFile i NIndexes*NChamber+IndexHitYDep u 1:2 lc CSideY
	}
unset multiplot
set logscale y
set multiplot layout MpY,MpX title "Hit Frequency (Log)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [-1:][0.5:] ResultsFile i NIndexes*NChamber+IndexHitXDep u 1:2 lc CSideX, \
		ResultsFile i NIndexes*NChamber+IndexHitYDep u 1:2 lc CSideY
	}
unset multiplot
# Clusters
set multiplot layout MpY,MpX title "GoodCluster Positions (Log)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][0.5:] ResultsFile i NIndexes*NChamber+IndexClusterPosX u 1:(C1(IdCluOk)) lc CSideX,\
		ResultsFile i NIndexes*NChamber+IndexClusterPosY u 1:(C1(IdCluOk)) lc CSideY
	}
unset multiplot

# Chambers together
set format x "%g"; set ytics auto; set key top right;
set title "Hit Frequency X"
plot [-1:][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexHitXDep u 1:2 lc NChamber t word(ChamberNameList,NChamber+1)
set title "Hit Frequency Y"
plot [-1:][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexHitYDep u 1:2 lc NChamber t word(ChamberNameList,NChamber+1)
set title "Good Clusters X"
plot [-1:][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexClusterPosX u 1:(C1(IdCluOk)) lc NChamber t word(ChamberNameList,NChamber+1)
set title "Good Clusters Y"
plot [-1:][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexClusterPosY u 1:(C1(IdCluOk)) lc NChamber t word(ChamberNameList,NChamber+1)




# Reset
set format x "%g";
set xrange [*:*]; set xlabel; set xtics auto
set yrange [*:*]; set ylabel; set ytics auto
set cbrange [*:*]; set cbtics auto; set colorbox; set key

# Adc Hists
if (GainAverage>2) {
	set title "ADC histograms"
	set xlabel "Signal Charge [adc]"; set xtics auto; set mxtics; set logscale x; #(10,20,50,100,200,500,1000,2000,5000); 
	set ylabel "N.Hits"; 	set ytics auto; set mytics; set logscale y;
	plot [20:5000][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTrackCross)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1)
	set xlabel "Signal Charge Scaled [adc]";
	plot [20:5000][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTrackAdcScaledSum)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1)
	unset logscale x; unset logscale y;
	}

print "Plots stored in ".PdfFileName

set term x11; set out
exit



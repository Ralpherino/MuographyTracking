# Gnuplot Script for ChamberResults visualisation
# Header for specific file is needed
#	version: 2019-02-25
#	written by: G.Hamar

# General settings
set style data his
set view map;
set bars small
set timefmt "%Y-%m-%d_%H:%M:%S"
set print "-"

PlotTitleAdd=Filenaming." : "

# Use info from header
MpY=int(sqrt(NChambers));
if (SizeX*3<SizeY && MpY>1) MpY=int(MpY/2); # for the BoreHoleDetectors
#if (NChambers>8) { MpY=3; }
MpX=int((NChambers+0.01)/MpY+0.98)

# Colours
CAll=0; CTrack=1; CEffTrack=3; CEffTrig=4; CAdc=0; CAdcScaled=5;
CSideX=1; CSideY=3;

# Id List for Chambers
IdAll=0; IdTrackCross=1; IdTriggerOk=2; IdClusterOk=3; IdHitFound=4;
IdTrackingEff=5; IfTriggerEff=6; 
IdTrackAdcSum=7; IdTrackAdcScaledSum=8; IdShiftXSum=9; IdShiftYSum=10;
IdFiducial=11; IdFiducialNew=12;
IdNTriggers=13; IdTriggerRate=14;
IdCluAllRaw=0; IdCluAll=1; IdCluOkRaw=2; IdCluOk=3;
# Id List for Detector
IdDetTrackCross=0; IdDetTriggerEff=1; IdDetTrackingEff=2; IdDetAcceptance=3; IdDetEffArea=4; IdDetFlux=5;
IdRateNEvents=0; IdRateNTracks=2; IdRateEventRate=2; IdRateTrackRate=3;
# Id List for THP
IdThpNEvents=0; IdThpSumTemperature=1; IdThpSumHumidity=2; IdThpSumPressure=3;
# Id List for HV
IdHvNEvents=0; IdHvSumUIseg=1; IdHvSumUMon=2; IdHvSumIMon=3;
IdHvN=0; IdHvI1=1; IdHvI2=2; IdHvI3=3;
# Column functions
C1(x)=column(x+2); C2(x)=column(x+3);



# Set plots type and filename
PdfFileName = Filenaming.".pdf"
set term pdf noenhanced size 6,4
set out PdfFileName


########################################################################
## Detector level plots
print "Preparing plots for the Detector ..."

# General ?
set title PlotTitleAdd."Detector summary info"
unset xlabel; unset xtics; unset ylabel; unset ytics
plot [0:5][-7.9:-1.1] for [c=1:4] ResultsFile i IndexDetectorGeneral u (c):(0-$0):(column(c)) with labels t ""
set xtics auto; set ytics auto;

# AngleDep
set cbrange [*:*]
set title PlotTitleAdd.DetectorName." - Track Counts (AngleDep)"
set xlabel "MX []"; set xtics auto; set xrange [-SlopeMax:SlopeMax]
set ylabel "MY []"; set ytics auto; set yrange [-SlopeMax:SlopeMax]
unset key
plot ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTrackCross)) w image
set cbrange [0.8:1.0]
set title PlotTitleAdd.DetectorName." - Tracking Efficiency Map (AngleDep)"
plot [][][0.1:] ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTrackingEff)) w image
set title PlotTitleAdd.DetectorName." - Trigger Efficiency Map (AngleDep)"
plot [][][0.1:] ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTriggerEff)) w image
set cbrange [*:*]
set title PlotTitleAdd.DetectorName." - Flux (AngleDep)"
plot ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetEffArea)>200 && C2(IdDetTrackCross)>10?C2(IdDetFlux):0) w image
set xrange [*:*]; set yrange [*:*]; set key

########################################################################
### Chambers together
print "Preparing plots for the set of Chambers ..."
PlotMinEff=0.5;
PlotAdcDiff=0.2;
# Position Dep Efficiencies
set xlabel "X [PadSizeX]"; set xtics 16; unset xlabel; unset xtics; set xrange [-1:SizeX];
set ylabel "Y [PadSizeY]"; set ytics 16; unset ylabel; unset ytics; set yrange [-1:SizeY];
set cbrange [PlotMinEff:1.0]; set cbtics 0.05; unset colorbox; unset key
set cbtics font ",11" format "%0.2f"
## Tracking Efficiency PosDep
#set multiplot layout MpY,MpX title "Track Cross Maps (PosDep)"
#do for [NChamber=0:NChambers-1] {
#	set title word(ChamberNameList,NChamber+1)
#	plot [][][0:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)) w image t ""
#	}
#unset multiplot
# Tracking Efficiency PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Tracking Efficiency Maps (PosDep)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][PlotMinEff:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackingEff)) w image t ""
	}
unset multiplot
# HitFound Efficiency PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Hit Efficiency Maps (PosDep)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][PlotMinEff:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdHitFound)/C2(IdTrackCross):0) w image t ""
	}
unset multiplot
# Trigger Efficiency PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Trigger Efficiency Maps (PosDep)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][PlotMinEff:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdTriggerOk)/C2(IdTrackCross):0) w image t ""
	}
unset multiplot
## Fiducials PosDep
#set multiplot layout MpY,MpX title PlotTitleAdd."Fiducials (PosDep)"
#set cbrange [0:3]; unset colorbox; unset key
#do for [NChamber=0:NChambers-1] {
#	set title word(ChamberNameList,NChamber+1)
#	plot ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdFiducial)+C2(IdFiducialNew)*2) w image
#	}
#unset multiplot
# Gain PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Gain Maps (PosDep)"
set cbrange [GainAverage*(1.0-PlotAdcDiff):GainAverage*(1.0+PlotAdcDiff)]; unset colorbox; unset key
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdTrackAdcScaledSum)/C2(IdTrackCross):0) w image
	}
unset multiplot
# Shift in X and Y
set multiplot layout MpY,MpX title PlotTitleAdd."Average Shift in X (PosDep)"
set cbrange [-1:1]; unset colorbox; unset key
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdShiftXSum)/C2(IdTrackCross):0) w image
	}
unset multiplot
set multiplot layout MpY,MpX title PlotTitleAdd."Average Shift in Y (PosDep)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdShiftYSum)/C2(IdTrackCross):0) w image
	}
unset multiplot


############### MtCern

# MtCern: BeamPos from Hits
set xrange [*:*]; set yrange [*:*]; unset title
set xrange [-400:400]; set xtics 50; set mxtics 5; set format x "%g";
set ylabel "N.Hits"; set ytics auto;
set key out right top vertical opaque maxcol 1
set grid xtics
array PadSizeX[8]=[8,4,8,4,12,12,12,12]; array PadSizeY[8]=[4,8,4,8,12,12,12,12]; array ShiftX[8]; array ShiftY[8];
#do for [n=1:8] { PadSizeY[n]=PadSizeX[n+n%2*2-1]; ShiftX[n]=-64+32*((n+0)%2); ShiftY[n]=-64+32*((n+1)%2); }
do for [n=1:4] { ShiftX[n]=-64+32*((n+0)%2); ShiftY[n]=-64+32*((n+1)%2); }
do for [n=5:8] { ShiftX[n]=-32; ShiftY[n]=-32; }
#do for [n=1:8] { print n,PadSizeX[n],PadSizeY[n],ShiftX[n],ShiftY[n]; }
set multiplot layout 2,1 title PlotTitleAdd."BeamPosition : X"; set xlabel "X [mm]"; 
plot for [n=1:4] ResultsFile i NIndexes*(n-1)+IndexHitXDep u (($1+ShiftX[n])*PadSizeX[n]):2 w l lc n t "Ch-".(n-1)
plot for [n=5:8] ResultsFile i NIndexes*(n-1)+IndexHitXDep u (($1+ShiftX[n])*PadSizeX[n]):2 w l lc n t "Ch-".(n-1)
unset multiplot
set multiplot layout 2,1 title PlotTitleAdd."BeamPosition : Y"; set xlabel "Y [mm]"; 
plot for [n=1:4] ResultsFile i NIndexes*(n-1)+IndexHitYDep u (($1+ShiftY[n])*PadSizeY[n]):2 w l lc n t "Ch-".(n-1)
plot for [n=5:8] ResultsFile i NIndexes*(n-1)+IndexHitYDep u (($1+ShiftY[n])*PadSizeY[n]):2 w l lc n t "Ch-".(n-1)
unset multiplot
set xtics 5; set mxtics 5; set xrange [-50:50];
set multiplot layout 2,1 title PlotTitleAdd."BeamPosition : X "; set xlabel "X [mm]"
plot for [n=1:4] ResultsFile i NIndexes*(n-1)+IndexHitXDep u (($1+ShiftX[n])*PadSizeX[n]):2 w l lc n  t "Ch-".(n-1)
plot for [n=5:8] ResultsFile i NIndexes*(n-1)+IndexHitXDep u (($1+ShiftX[n])*PadSizeX[n]):2 w l lc n  t "Ch-".(n-1)
unset multiplot
set multiplot layout 2,1 title PlotTitleAdd."BeamPosition : Y "; set xlabel "Y [mm]"; 
plot for [n=1:4] ResultsFile i NIndexes*(n-1)+IndexHitYDep u (($1+ShiftY[n])*PadSizeY[n]):2 w l lc n  t "Ch-".(n-1)
plot for [n=5:8] ResultsFile i NIndexes*(n-1)+IndexHitYDep u (($1+ShiftY[n])*PadSizeY[n]):2 w l lc n  t "Ch-".(n-1)
unset multiplot






#PosDep-
set xrange [*:*]; set xlabel; set xtics autofreq
set yrange [*:*]; set ylabel; set ytics autofreq
set cbrange [*:*]; set cbtics auto; set colorbox; set key

# HitFrequencies
set multiplot layout MpY,MpX title PlotTitleAdd."Hit Frequency"
unset xlabel; set xtics 16; unset mxtics; set format x "";
unset ylabel; unset ytics; unset key
set grid xtics
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [-1:][0:] ResultsFile i NIndexes*NChamber+IndexHitXDep u 1:2 lc CSideX, \
		ResultsFile i NIndexes*NChamber+IndexHitYDep u 1:2 lc CSideY
	}
unset multiplot
# Clusters
set multiplot layout MpY,MpX title PlotTitleAdd."GoodCluster Positions"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][0:] ResultsFile i NIndexes*NChamber+IndexClusterPosX u 1:(C1(IdCluOk)) lc CSideX, \
		ResultsFile i NIndexes*NChamber+IndexClusterPosY u 1:(C1(IdCluOk)) lc CSideY
	}
unset multiplot
# Noise map
set multiplot layout MpY,MpX title PlotTitleAdd."Noise performance"
Scale1=30*EventRate/TrackRate; Scale2=1000;
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][0:20] ResultsFile i NIndexes*NChamber+IndexHitXDep u 1:(Scale1*$4/$2) lc CSideX,\
		ResultsFile i NIndexes*NChamber+IndexHitXDep u 1:(Scale2*$4/$5) lc CSideX,\
		ResultsFile i NIndexes*NChamber+IndexHitYDep u 1:(Scale1*$4/$2) lc CSideY,\
		ResultsFile i NIndexes*NChamber+IndexHitYDep u 1:(Scale2*$4/$5) lc CSideY
	}
unset multiplot

# Resolution
set multiplot layout MpY,MpX title PlotTitleAdd."Resolution"
set xtics 1.0; set mxtics 2; set grid x y;
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][0:] ResultsFile i NIndexes*NChamber+IndexResolutionX u 1:2 lc CSideX, \
		ResultsFile i NIndexes*NChamber+IndexResolutionY u 1:2 lc CSideY
	}
unset multiplot;

# Reset
set format x "%g";
set xrange [*:*]; set xlabel; set xtics auto
set yrange [*:*]; set ylabel; set ytics auto
set cbrange [*:*]; set cbtics auto; set colorbox; set key

# TimeDep : Efficiency, Gain
set xdata time; set format x "%b-%d\n%H:%M"
unset xlabel; set xtics auto; set grid x y;
set title PlotTitleAdd."Run start marks"
set ylabel "\nRun Names\n"; unset ytics; 
#stats ResultsFile i NIndexes*0+IndexTimeDep u 2
if (exists("UseTimeMarks")==1) {
	plot [][0:2] ResultsFile i (IndexDetectorThp+NHvs+NThps) u 1:(1):2 with labels rot by 90 t ""
	}
set key out left bottom horizontal opaque maxcol 3
set ylabel "Efficiency"; set ytics auto
set title PlotTitleAdd."Tracking Efficiency  time evolution"
plot for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdClusterOk)/C2(IdTrackCross)) w l lw 2 lc NChamber t "" ,\
	for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdClusterOk)/C2(IdTrackCross)):(C2(IdTrackCross)/2<C2(IdClusterOk)?sqrt(C2(IdTrackCross)-C2(IdClusterOk))/C2(IdTrackCross):sqrt(C2(IdClusterOk))/C2(IdTrackCross)) w e ps 0.5 lc NChamber t word(ChamberNameList,NChamber+1) \
#	, ResultsFile i IndexDetectorTimeDep u 2:(C2(IdDetTrackingEff)) w l lw 4 lc 1 t "Detector"
set title PlotTitleAdd."Trigger Efficiency  time evolution"
plot for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerOk)/C2(IdTrackCross)) w l lw 2 lc NChamber t "" ,\
	for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerOk)/C2(IdTrackCross)):(C2(IdTrackCross)/2<C2(IdTriggerOk)?sqrt(C2(IdTrackCross)-C2(IdTriggerOk))/C2(IdTrackCross):sqrt(C2(IdTriggerOk))/C2(IdTrackCross)) w e ps 0.5 lc NChamber t word(ChamberNameList,NChamber+1)
set title PlotTitleAdd."Trigger Rate time evolution"
set ylabel "Trigger Rate [Hz]"
plot [][0:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerRate)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1) \
	, ResultsFile i IndexDetectorRate u 2:(C2(IdRateEventRate)) w l lw 4 lc 0 t "Event rate" \
	, ResultsFile i IndexDetectorRate u 2:(C2(IdRateEventRate)):(C2(IdRateEventRate)/sqrt(C2(IdRateNEvents))) w e ps 0 lw 2 lc 0 t "" \
	, ResultsFile i IndexDetectorRate u 2:(C2(IdRateTrackRate)) w l lw 5 lc 1 t "Track rate"
	#, ResultsFile i IndexDetectorRate u 2:(C2(IdRateTrackRate)):(C2(IdRateTrackRate)/sqrt(C2(IdRateNTracks))) w e ps 0 lw 2 lc 1 t ""
set title PlotTitleAdd."Gain time evolution"
set ylabel "Gain [a.u.]"
plot for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTrackAdcScaledSum)/C2(IdTrackCross)) w l lw 2 lc NChamber t "",\
	for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTrackAdcScaledSum)/C2(IdTrackCross)):(C2(IdTrackAdcScaledSum)/(C2(IdTrackCross)**1.5)) w e ps 0.5 lc NChamber t word(ChamberNameList,NChamber+1)
set xdata; set format x "%g"; unset grid;
set key default; set key vertical

# THP
set title PlotTitleAdd."Environmental parameters time evolution"
unset xlabel; set xtics auto; set grid xtics
set xdata time; set format x "%b-%d\n%H:%M"
set ylabel "Temperature [oC], Humidity [%]"; set ytics auto nomirror
set y2label "Pressure [mbar]"; set y2tics auto
set key top right
do for [NThp=0:NThps-1] {
	plot ResultsFile i IndexDetectorThp+NThp u 2:(C2(IdThpSumTemperature)/C2(IdThpNEvents)) w l lw 3 t "Temperature", \
		ResultsFile i IndexDetectorThp+NThp u 2:(C2(IdThpSumHumidity)/C2(IdThpNEvents)) w l lw 3 lc 4 t "Humidity", \
		ResultsFile i IndexDetectorThp+NThp u 2:(C2(IdThpSumPressure)/C2(IdThpNEvents)) axis x1y2 w l lw 3 t "Pressure"
	}

# HV
set title PlotTitleAdd."HighVoltage parameters time evolution"
set ylabel "Voltage [V]";
set y2label "Current [nA]";
IndexDetectorHv=IndexDetectorHv+NThps-1;
do for [NHv=0:NHvs-1] {
	plot    ResultsFile i IndexDetectorHv u 2:(C2(IdHvSumUIseg)/C2(IdHvNEvents)) w l lw 3 t "URef", \
		ResultsFile i IndexDetectorHv u 2:(C2(IdHvSumUMon)/C2(IdHvNEvents)) w l lw 3 lc 4 t "UMon", \
		ResultsFile i IndexDetectorHv u 2:(C2(IdHvSumIMon)/C2(IdHvNEvents)) axis x1y2 w l lw 3 t "IMon"
	}
# TEST 
#HvInfo="UFw/-0.55 0.55 USw 1.0 IMon 1.0"
#do for [NHv=0:NHvs-1] {
#	plot	ResultsFile i IndexDetectorHv u 2:(C2(IdHvI1)/C2(IdHvN)/word(HvInfo,2)) w l lw 3 lc 4 t word(HvInfo,1), \
#		ResultsFile i IndexDetectorHv u 2:(C2(IdHvI2)/C2(IdHvN)/word(HvInfo,4)) w l lw 3 lc 1 t word(HvInfo,3), \
#		ResultsFile i IndexDetectorHv u 2:(C2(IdHvI3)/C2(IdHvN)/word(HvInfo,6)) axis x1y2 w l lw 3 lc 3 t word(HvInfo,5), \
#	}
# TEST end
unset y2tics; unset y2label; set ytics mirror
set xdata; set format x "%g"; unset grid


## THP version 2
#set title "Environmental parameters time evolution"
#unset xlabel; set xtics auto; set grid xtics
#set xdata time; set format x "%b-%d\n%H:%M"
#unset ylabel; set ytics auto scale 0
#do for [NThp=0:NThps-1] {
#	set multiplot
#	set format y "%g [oC] "
#	set ytics offset graph 0.00,0 right
#	set key top left
#	plot ResultsFile i IndexDetectorThp+NThp u 2:(C2(IdThpSumTemperature)/C2(IdThpNEvents)) w l lw 3 lc 1 t "Temperature"
#	set format y "%g [%%] "
#	set ytics offset graph 0.05,0 left
#	set key top center
#	plot ResultsFile i IndexDetectorThp+NThp u 2:(C2(IdThpSumHumidity)/C2(IdThpNEvents)) w l lw 3 lc 4 t "Humidity"
#	set format y "%g [mbar]"
#	set ytics offset graph 0.99,0 right
#	set key top right
#	plot ResultsFile i IndexDetectorThp+NThp u 2:(C2(IdThpSumPressure)/C2(IdThpNEvents)) w l lw 3 lc 3 t "Pressure"
#	unset multiplot
#	}
#set ytics scale 1,0.5;
#unset y2tics; unset y2label; set ytics mirror
#set xdata; set format x "%g"; unset grid
#

# Adc Hists
if (GainAverage>2) {
	set key out left bottom horizontal opaque maxcol 3
	set title PlotTitleAdd."ADC histograms"
	set xlabel "Signal Charge [adc]"; set xtics auto; set mxtics; set logscale x; #(10,20,50,100,200,500,1000,2000,5000); 
	set ylabel "N.Hits"; 	set ytics auto; set mytics 2 ; set logscale y;
	#plot [20:5000][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTrackCross)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1)
	set xlabel "Signal Charge Scaled [adc]";
	plot [20:5000][0.5:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTrackAdcScaledSum)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1)
	unset logscale x; 
	set title PlotTitleAdd."ADC histograms around Zero (for Events and Tracks)"
	plot [-200:200][:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdAll)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1),\
		for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTrackCross)) w l lw 2 lc NChamber t ""
	unset logscale x; unset logscale y; set mytics;
	set key default; set key vertical;
	}


########################################################################
# Special Detector plots ...
print "Preparing special plots for the Detector ..."

unset xtics; unset xlabel; unset ytics; unset ylabel;
unset colorbox;
set multiplot layout MpY,MpX title PlotTitleAdd."Correlation : Data - Trigger"
do for [NChamber=0:NChambers-1] {
	unset title
	plot [-0.5:NChambers-0.5][-0.5:NChambers-0.5] ResultsFile i IndexDetectorCorrelDT u 1:2:(C2(NChamber)) w image t ""
	}
unset multiplot
set multiplot layout MpY,MpX title PlotTitleAdd."Correlation : Data - Adc"
do for [NChamber=0:NChambers-1] {
	unset title
	plot [-0.5:NChambers-0.5][-0.5:NChambers-0.5] ResultsFile i IndexDetectorCorrelDA u 1:2:(C2(NChamber)) w image t ""
	}
unset multiplot
set xtics; set ytics; set colorbox;

set title PlotTitleAdd."Trigger Coincidence Level"
set xlabel "NTriggers in Event"; set xtics 1;
set ylabel "NEvents"; set ytics auto;
plot [-0.8:NChambers+0.8][0:] ResultsFile i IndexDetectorCoinc u 1:2 w his t "Events", \
	"" i IndexDetectorCoinc u 1:3 w his lc 3 t "Tracks" 


########################################################################

if (ShortVersion==1) { print "Plots stored in ".PdfFileName; set term x11; set out; exit; }

########################################################################


########################################################################
# Preparing plots for the Chambers ...
print "Preparing plots for the Chambers ..."

do for [NChamber=0:NChambers-1] {
	# General
	ChName="Chamber".word(ChamberNameList,NChamber+1)
	print "      Chamber : ".ChName
	set title "Chamber summary info"
	unset xlabel; unset xtics; unset ylabel; unset ytics
	plot [0:5][0:6] for [c=1:4] ResultsFile i NIndexes*NChamber+IndexGeneral u (c):(5.5-$0):(column(c)) with labels t ""
	set xtics auto; set ytics auto;
	
	# HitFreqHists
	set title ChName." - NHits X"
	set xlabel "X coordinate"; set xtics 16; set mxtics 4;
	set ylabel "N.Hits"; set ytics auto;
	set grid xtics
	plot [-1:][] ResultsFile i  NIndexes*NChamber+IndexHitXDep u 1:2 t ""
	set title ChName." - NHits Y"
	set xlabel "Y coordinate";
	plot [-1:][] ResultsFile i  NIndexes*NChamber+IndexHitYDep u 1:2 t ""
	unset grid; set xtics auto; set ytics auto;

	# Cluster Position
	set key top right
	set title ChName." - Clusters X"
	set xlabel "X Position [PadSize]"; set xtics 16; set mxtics 4;
	set ylabel "N.Clusters"; set ytics auto
	plot ResultsFile i NIndexes*NChamber+IndexClusterPosX u 1:(C1(IdCluAllRaw)) w l lc 0 t "Raw", \
		ResultsFile i NIndexes*NChamber+IndexClusterPosX u 1:(C1(IdCluAll)) lc 1 lw 2 t "Smoothed", \
		ResultsFile i NIndexes*NChamber+IndexClusterPosX u 1:(C1(IdCluOk)) lw 2 t "Track"
	set title ChName." - Clusters Y"
	set xlabel "Y Position [PadSize]";
	set ylabel "N.Clusters"; set ytics auto
	plot ResultsFile i NIndexes*NChamber+IndexClusterPosY u 1:(C1(IdCluAllRaw)) w l lc 0 t "Raw", \
		ResultsFile i NIndexes*NChamber+IndexClusterPosY u 1:(C1(IdCluAll)) lc 1 lw 2 t "Smoothed", \
		ResultsFile i NIndexes*NChamber+IndexClusterPosY u 1:(C1(IdCluOk)) lw 2 t "Track"
	
	# PositionDependent Efficiencies
	set cbrange [0.8:1.0]
	set title ChName." - Tracking Efficiency Map (PosDep)"
	set xlabel "X [PadSizeX]"; set xtics 16; set mxtics 4;
	set ylabel "Y [PadSizeY]"; set ytics 16; set mytics 4;
	plot [-1:SizeX][-1:SizeY][0.8:1.01] ResultsFile i NIndexes*NChamber+IndexPositionDep u 1:2:(C2(IdTrackCross)>0?C2(IdClusterOk)/C2(IdTrackCross):0) w image t ""
	set title ChName." - Hit Efficiency Map (PosDep)"
	plot [-1:SizeX][-1:SizeY][0.8:1.01] ResultsFile i NIndexes*NChamber+IndexPositionDep u 1:2:(C2(IdTrackCross)>0?C2(IdHitFound)/C2(IdTrackCross):0) w image t ""
	set title ChName." - Trigger Efficiency Map (PosDep)"
	plot [-1:SizeX][-1:SizeY][0.8:1.01] ResultsFile i NIndexes*NChamber+IndexPositionDep u 1:2:(C2(IdTrackCross)>0?C2(IdTriggerOk)/C2(IdTrackCross):0) w image t ""
	set cbrange [*:*]
	set title ChName." - Gain Map (PosDep)"
	plot [-1:SizeX][-1:SizeY] ResultsFile i NIndexes*NChamber+IndexPositionDep u 1:2:(C2(IdTrackCross)>0?C2(IdTrackAdcScaledSum)/C2(IdTrackCross):0) w image t ""

	# AngleDep
	set title ChName." - Efficiency Map (AngleDep)"
	set xlabel "MX []"; set xtics auto
	set ylabel "MY []"; set ytics auto
	plot [-SlopeMax:SlopeMax][-SlopeMax:SlopeMax][] ResultsFile i NIndexes*NChamber+IndexAngleDep u 1:2:(C2(IdTrackCross)>0?C2(IdClusterOk)/C2(IdTrackCross):0) w image t ""
	
	# AdcDep
	set title ChName." - ADC dependence"
	set xlabel "ADC Value [adc]"; set xtics 1000; set mxtics 5
	set ylabel "N.Hits"
	plot [-500:4800][] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdAll)) lc CAll t "All Events", \
		ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTrackCross)) lc CTrack t "TrackCross", \
		ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdClusterOk)) lc CEffTrack t "ClusterOk", \
		ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTriggerOk)) lc CEffTrig t "TriggerOk"
	set ylabel "Ratio"
	plot [-500:4800][0:1.05] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdClusterOk)/C1(IdTrackCross)) lc CEffTrack t "ClusterOk / TrackCross", \
		ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTriggerOk)/C1(IdTrackCross)) lc CEffTrig t "TriggerOk / TrackCross"
	set xtics 50; set mxtics 5
	plot [-50:500][0:1.05] ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdClusterOk)/C1(IdTrackCross)) lc CEffTrack t "ClusterOk / TrackCross", \
		ResultsFile i NIndexes*NChamber+IndexAdcDep u 1:(C1(IdTriggerOk)/C1(IdTrackCross)) lc CEffTrig t "TriggerOk / TrackCross"

	# TimeDep
	set title ChName." - Time evolution"
	set xdata time; set format x "%b-%d\n%H:%M"
	set xlabel "DateTime"; set xtics auto
	set ylabel "Efficiency"; set ytics auto
	plot [][:] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdClusterOk)/C2(IdTrackCross)) lc CEffTrack t "", \
		ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdClusterOk)/C2(IdTrackCross)):(sqrt(C2(IdTrackCross)-C2(IdClusterOk))/C2(IdTrackCross)) w e ps 0.5 lc CEffTrack t "TrackingEfficiency", \
		ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerOk)/C2(IdTrackCross)) lc CEffTrig t "", \
		ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerOk)/C2(IdTrackCross)):(sqrt(C2(IdTrackCross)-C2(IdTriggerOk))/C2(IdTriggerOk)) w e ps 0.5 lc CEffTrig t "TriggerEfficiency", \
		ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTrackAdcScaledSum)/C2(IdTrackCross)/GainAverage) lc CAdc t "", \
		ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTrackAdcScaledSum)/C2(IdTrackCross)/GainAverage):((C2(IdTrackAdcScaledSum)/(C2(IdTrackCross)**1.5)/GainAverage)) w e ps 0.5 lc CAdc t "Gain (scaled)"
	set xdata; set format x "%g"

	# Resolution
	set title ChName." - Resolution"
	set xlabel "X [PadSize]"; set xtics auto
	set ylabel "Y [PadSize]"; set ytics auto
	plot ResultsFile i NIndexes*NChamber+IndexResolutionMap u 1:2:3 w image t ""
	set multiplot layout 2,2 columnsfirst
	set title ChName." - Resolution X"
	set xlabel "DeltaX [PadSize]"; set xtics 1.0; set mxtics 5
	set ylabel "Entries"; set ytics auto
	plot ResultsFile i NIndexes*NChamber+IndexResolutionX u 1:2 w his t "", ResultsFile i NIndexes*NChamber+IndexResolutionX u 1:($2>0?$2:0.1):(sqrt($2)) w e lc 1 pt 7 ps 0 lw 3 t ""
	set logscal y; replot; unset logscale y
	set title ChName." - Resolution Y"
	set xlabel "DeltaY [PadSize]"; 
	set ylabel "Entries"; set ytics auto
	plot ResultsFile i NIndexes*NChamber+IndexResolutionY u 1:2 w his t "", ResultsFile i NIndexes*NChamber+IndexResolutionY u 1:($2>0?$2:0.1):(sqrt($2)) w e lc 1 pt 7 ps 0 lw 3 t ""
	set logscal y; replot; unset logscale y
	set xtics auto; set mxtics;
	unset multiplot
	
	}

print "Plots stored in ".PdfFileName

set term x11; set out
exit



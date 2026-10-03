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
SlopeMax=SlopeMax*0.98
set title PlotTitleAdd.DetectorName." - Track Counts (AngleDep)"
set xlabel "MX []"; set xtics auto; set xrange [-SlopeMax:SlopeMax]
set ylabel "MY []"; set ytics auto; set yrange [-SlopeMax:SlopeMax]
unset key
plot ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTrackCross)) w image
set cbrange [0.8:1.0]
set title PlotTitleAdd.DetectorName." - Tracking Efficiency Map (AngleDep)"
plot [][][0.1:] ResultsFile i IndexDetectorAngleDep u 1:2:(C2(IdDetTrackingEff)) w image
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
# Tracking Efficiency PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Tracking Efficiency Maps (PosDep)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][PlotMinEff:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdClusterOk)/C2(IdTrackCross):0) w image t ""
	}
unset multiplot
# Trigger Efficiency PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Trigger Efficiency Maps (PosDep)"
do for [NChamber=0:NChambers-1] {
	set title word(ChamberNameList,NChamber+1)
	plot [][][PlotMinEff:] ResultsFile i NIndexes*NChamber+IndexPositionDepBare u 1:2:(C2(IdTrackCross)>0?C2(IdTriggerOk)/C2(IdTrackCross):0) w image t ""
	}
unset multiplot
# Gain PosDep
set multiplot layout MpY,MpX title PlotTitleAdd."Gain Maps (PosDep)"
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

# Reset
set format x "%g";
set xrange [*:*]; set xlabel; set xtics auto
set yrange [*:*]; set ylabel; set ytics auto
set cbrange [*:*]; set cbtics auto; set colorbox; set key

# Get Min-Max ranges
TStep=5; PStep=10; UStep=10; IStep=100; EffMin=0.9;
do for [NChamber=0:NChambers-1] {
	stats ResultsFile i NIndexes*NChamber+IndexTimeDep u (C2(IdClusterOk)/C2(IdTrackCross)) nooutput;
	if (STATS_min<EffMin) { EffMin=STATS_min; }
	stats ResultsFile i NIndexes*NChamber+IndexTimeDep u (C2(IdTriggerOk)/C2(IdTrackCross)) nooutput;
	if (STATS_min<EffMin) { EffMin=STATS_min; }
	}
stats ResultsFile i IndexDetectorThp u (C2(IdThpSumTemperature)/C2(IdThpNEvents)) nooutput; TMin=STATS_min-TStep; TMax=STATS_max+TStep;
stats ResultsFile i IndexDetectorThp u (C2(IdThpSumHumidity)/C2(IdThpNEvents)) nooutput; HMin=STATS_min-TStep; HMax=STATS_max+TStep;
stats ResultsFile i IndexDetectorThp u (C2(IdThpSumPressure)/C2(IdThpNEvents)) nooutput; PMin=STATS_min-PStep; PMax=STATS_max+PStep;
if (TMin>HMin) TMin=HMin; if (TMax<HMax) TMax=HMax;
stats ResultsFile i IndexDetectorHv u (C2(IdHvSumUIseg)/C2(IdHvNEvents)) nooutput;	UMin=STATS_min-UStep; UMax=STATS_max+UStep;
stats ResultsFile i IndexDetectorHv u (C2(IdHvSumUMon)/C2(IdHvNEvents)) nooutput;	U2Min=STATS_min-UStep; U2Max=STATS_max+UStep;
stats ResultsFile i IndexDetectorHv u (C2(IdHvSumIMon)/C2(IdHvNEvents)) nooutput;	IMin=STATS_min-IStep; IMax=STATS_max+IStep;
if (UMin>U2Min) UMin=U2Min; if (UMax<U2Max) UMax=U2Max;



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
plot [][EffMin:1] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdClusterOk)/C2(IdTrackCross)) w l lw 2 lc NChamber t "" ,\
	for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdClusterOk)/C2(IdTrackCross)):(C2(IdTrackCross)/2<C2(IdClusterOk)?sqrt(C2(IdTrackCross)-C2(IdClusterOk))/C2(IdTrackCross):sqrt(C2(IdClusterOk))/C2(IdTrackCross)) w e ps 0.5 lc NChamber t word(ChamberNameList,NChamber+1) \
	, ResultsFile i IndexDetectorTimeDep u 2:(C2(IdDetTrackingEff)) w l lw 4 lc 1 t "Detector"
set title PlotTitleAdd."Trigger Efficiency  time evolution"
plot [][EffMin:1] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerOk)/C2(IdTrackCross)) w l lw 2 lc NChamber t "" ,\
	for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerOk)/C2(IdTrackCross)):(C2(IdTrackCross)/2<C2(IdTriggerOk)?sqrt(C2(IdTrackCross)-C2(IdTriggerOk))/C2(IdTrackCross):sqrt(C2(IdTriggerOk))/C2(IdTrackCross)) w e ps 0.5 lc NChamber t word(ChamberNameList,NChamber+1)
set title PlotTitleAdd."Trigger Rate time evolution"
set ylabel "Trigger Rate [Hz]"
plot [][0:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTriggerRate)) w l lw 2 lc NChamber t word(ChamberNameList,NChamber+1) \
	, ResultsFile i IndexDetectorRate u 2:(C2(IdRateEventRate)) w l lw 4 lc 0 t "Event rate" \
	, ResultsFile i IndexDetectorRate u 2:(C2(IdRateEventRate)):(C2(IdRateEventRate)/sqrt(C2(IdRateNEvents))) w e ps 0 lw 2 lc 0 t "" \
	, ResultsFile i IndexDetectorRate u 2:(C2(IdRateTrackRate)) w l lw 5 lc 1 t "Track rate"
set title PlotTitleAdd."Gain time evolution"
set ylabel "Gain [a.u.]"
plot [][0:] for [NChamber=0:NChambers-1] ResultsFile i NIndexes*NChamber+IndexTimeDep u 2:(C2(IdTrackAdcScaledSum)/C2(IdTrackCross)) w l lw 2 lc NChamber t "",\
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
set yrange [TMin:TMax]; set y2range [PMin:PMax];
plot ResultsFile i IndexDetectorThp u 2:(C2(IdThpSumTemperature)/C2(IdThpNEvents)) w l lw 3 t "Temperature", \
	ResultsFile i IndexDetectorThp u 2:(C2(IdThpSumHumidity)/C2(IdThpNEvents)) w l lw 3 lc 4 t "Humidity", \
	ResultsFile i IndexDetectorThp u 2:(C2(IdThpSumPressure)/C2(IdThpNEvents)) axis x1y2 w l lw 3 t "Pressure"

# HV
set title PlotTitleAdd."HighVoltage parameters time evolution"
set ylabel "Voltage [V]";
set y2label "Current [nA]";
set yrange [UMin:UMax]; set y2range [IMin:IMax];
IndexDetectorHv=IndexDetectorHv+NThps-1;
plot    ResultsFile i IndexDetectorHv u 2:(C2(IdHvSumUIseg)/C2(IdHvNEvents)) w l lw 3 t "URef", \
	ResultsFile i IndexDetectorHv u 2:(C2(IdHvSumUMon)/C2(IdHvNEvents)) w l lw 3 lc 4 t "UMon", \
	ResultsFile i IndexDetectorHv u 2:(C2(IdHvSumIMon)/C2(IdHvNEvents)) axis x1y2 w l lw 3 t "IMon"
unset y2tics; unset y2label; set ytics mirror
set xdata; set format x "%g"; unset grid

print "Plots stored in ".PdfFileName

set term x11; set out
exit



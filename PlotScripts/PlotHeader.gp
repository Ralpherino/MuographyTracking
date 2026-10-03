# Gnuplot Script for MuographyTracking Results visualisation
# Header for specific file is needed
#	version: 2020-03-02
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
#if (NChambers>8) { MpY=3; }
MpX=int((NChambers+0.01)/MpY+0.99)

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






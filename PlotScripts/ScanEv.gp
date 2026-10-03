# Gnuplot Script for Scan Evaluation 
#	version: 2019-10-24
#	written by: G.Hamar

ScanDataFile="ScanEv.sr"
ScanDataFile="Mtl5_HvScan.sr"

NChambers=8

set term pdf size 10,6
#set out "Bhd2_HvScan-202108_Plot20210826.pdf";
set out "Mtl5_HvScan-202209_Plot20220927.pdf";

# General settings
C(x)=column(x+3);
X=1;Y=2;
IdParameter=0;IdRunNumber=1;IdTrackingEff=2;IdTriggerEff=3;IdGain=4;
IdChamber=-2; IdNRun=-1;
Values="RunNumber TrackingEff TriggerEff Gain";

#plot ScanDataFile u (C(IdNRun)):(C(IdParameter)):("Run-".int($2)) w labels
#plot ScanDataFile i 0 u (C(IdParameter)):(C(IdRunNumber)) w p pt 6 ps 3

set title noenhanced
set title "".ScanDataFile
set xlabel "Parameter";
set key right bottom;
set grid x y

set multiplot layout 2,2
do for [v=1:words(Values)] {
	set ylabel word(Values,v)
	plot ScanDataFile i 0 u (C(IdParameter)):(C(v)):1 w lp pt 6 lc var t "", for [c=0:NChambers-1] NaN lw 2 lc c t "Ch-".c
	}
unset multiplot

set term x11; set out;


set xlabel "hejj"

plot ScanDataFile i 0 u (C(IdParameter)):(C(IdGain)):1 w lp pt 6 lc var t "", for [c=0:NChambers-1] NaN lw 2 lc c t "Ch-".c




Params="0 -10 -20 -30"

Params="4 5 6 7 8"

Params="-00.1 +00.0 -09.8 -09.8 -19.8 -05.0 -44.5 -29.9 -59.5 -59.5 +00.0 +09.9 -89.5 +00.0"
Params="1030 1030 1050 1070 1080 1100 1120"
Params="1 2 3 4"
Params="+00.1 -02.2 +03.2 -27.3 -08.0 -90.0"	# Mts40_Inclination
Params="1550 1600 1650 1700 1700 1750"
Params="940 970 1000 1000 1020 1050 1070 1090"
Params="1500 1530 1570 1595 1595 1625 1650"
Params="1500 1550 1600 1650 1700 1750 1800"

set key top right

set title "Flux X"
plot for [p=1:words(Params)] ScanDataFile i 2 u ($2==0?$1:NaN):(C(p-1)) w lp t word(Params,p)

set title "Flux Y"
plot for [p=1:words(Params)] ScanDataFile i 2 u ($1==0?$2:NaN):(C(p-1)) w lp t word(Params,p)



plot ScanDataFile i 0 u 2:1:5:xtic(3) w image


set xlabel "NChamber"; set ylabel "Parameter"; 
splot ScanDataFile u 1:(C(IdParameter)):(C(IdTrackingEff)) w pm3d
set xlabel "NRun"; set ylabel "NChamber"
plot ScanDataFile u 2:1:(C(IdTrackingEff)) w image

# If the Params are inclinations in degrees
at(x)=atan(x)/pi*180.
plot for [p=1:words(Params)] ScanDataFile i 2 u ($1==0?at($2)-int(word(Params,p)):NaN):(C(p-1)) w lp ps 2 pt 6 lc (p-1) t word(Params,p)



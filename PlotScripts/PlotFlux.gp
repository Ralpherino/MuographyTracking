# Gnuplot Script for Flux visualisation
# Input parameters are needed, see below !
#	version: 2022-08-15
#	written by: G.Hamar

# Required parameters
print "".fn
FileNaming=fn

# General settings
RunName=system(sprintf("basename %s",FileNaming))
FluxFileName=FileNaming.".flux"
#PngFileName=FileNaming."_flux.png"
PdfFileName=FileNaming."_flux.pdf"
set style data his
set view map;
set print "-"
set term pdf solid color noenhanced size 6,5 lw 0
set out PdfFileName
unset key
set size ratio 1


# FluxFile info
ColSlopeX=1; ColSlopeY=2;
ColCounts=4;
ColFluxSmooth=10;
ColZenith=12; ColAzimuth=13;

# Settings, cuts
NCountsMin=2;

# Get stats
SideScale=0.01; FluxScale=0.02;
stats FluxFileName u 1 nooutput
SlopeXMin=STATS_min-(STATS_max-STATS_min)*SideScale; 
SlopeXMax=STATS_max-(STATS_max-STATS_min)*SideScale; 
stats FluxFileName u 2 nooutput
SlopeYMin=STATS_min-(STATS_max-STATS_min)*SideScale; 
SlopeYMax=STATS_max-(STATS_max-STATS_min)*SideScale; 
stats FluxFileName u ($4>NCountsMin?$10:NaN) nooutput
FluxMax=STATS_max*(1.0-FluxScale);
FluxMin=STATS_min*(1.0+FluxScale);
# Plot the smoothed flux wrt Slopes
set title RunName." - Flux (Det+Smooth)"
set xlabel "SlopeX"; set ylabel "SlopeY";
set grid x y
set cblabel "Flux colourcode"
set cbrange [0:FluxMax]
plot [SlopeXMin:SlopeXMax][SlopeYMin:SlopeYMax] FluxFileName u 1:2:($4>NCountsMin?$10:0) w image t ""
#plot [SlopeXMin:SlopeXMax][SlopeYMin:SlopeYMax] FluxFileName u 1:2:($8/1.e6) w image t ""
#plot FluxFileName u 1:2:($4>10?$10:0) w image t ""



# Plot flux with rotations included in GeoCoordSystem
#for GeoCoords, Y==North, leftwise
set title RunName." - Flux (Geo)"
set angles deg; ZRange=92;
set xrange [-ZRange:ZRange]; set yrange [-ZRange:ZRange];
set xtics 30; set mxtics 3; set ytics 30; set mytics 3;
unset xlabel; unset ylabel; 
set colorbox; set cblabel "Flux colourcode" offset 1
set label 1 "N" at 0,+85 center front tc rgb "green"; set label 2 "S" at 0,-85 center front tc rgb "green"; 
set label 3 "W" at -85,0 center front tc rgb "green"; set label 4 "E" at +85,0 center front tc rgb "green"; 
#set obj ellipse at 0,0 size 180,180
do for [n=1:9] { set obj n ellipse at 0,0 size (n*20),(n*20) front fs border lt 5 empty dt 3; } 
do for [n=3:9:3] { set obj n front fs border lt 0 dt 1; }

splot FluxFileName u (column(ColZenith)*sin(column(ColAzimuth))):(column(ColZenith)*cos(column(ColAzimuth))):(column(ColCounts)>2?column(ColFluxSmooth):0) w pm3d










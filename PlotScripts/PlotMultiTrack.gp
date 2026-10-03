	# Gnuplot Script for MultiTrack Analysis maps
# Input parameters are needed, see below !
#	version: 2019-02-25
#	written by: G.Hamar

# Files
MapFile=Filenaming.".mm"
PdfFile=Filenaming."_MultiMap.pdf"

# General settings
C(x)=column(x+3);
X=1;Y=2;
NTracks=0;NScat=1;SumScat=2;C10=3;C01=4;
IndexMap=0; IndexScatterDistributionTotal=1;
IndexScatterDistribution=2; IndexRegionContour=3;

RegionPos=system(sprintf("grep -A 5 coordinates %s | head -n 5 | tail -n 1",MapFile));
do for [r=1:words(Region)] { set label r word(Region,r) at word(RegionPos,r*2+1),word(RegionPos,r*2+2)+10 front tc "white" font ",12"; };

# Scatter map
set title "Scatter Map (NScat)"
set view map;
set xlabel "X [mm]"; set xtics 100; set mxtics 2;
set ylabel "Y [mm]"; set ytics 100; set mytics 2;
set key out left bottom horizontal opaque
plot MapFile i IndexMap u X:Y:(C(NTracks)>20?C(NScat)/C(NTracks):0) w image t "", \
	for [r=1:words(Region)] "" i IndexRegionContour u (column(1+2*r)):(column(2+2*r)):(1) w l t "".word(Region,r)

# Scatter distribution
set title "Scatter Distribution"
set xlabel "Scatter Value"; set xtics 0.2; set mxtics 2;
set ylabel "Probability [au]"; set ytics auto
set key in top right vertical
plot [0:1.0][1e-5:0.5] for [r=1:words(Region)] MapFile i IndexScatterDistribution u X:(column(r+1)) w l t "".word(Region,r)



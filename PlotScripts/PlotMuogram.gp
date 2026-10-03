
# Muogram plots
#MuogramFile=m.".mgm"
MuogramFile=m
print "Plot ".MuogramFile;

set term pdf size 5,4.5; 
set out MuogramFile.".pdf"


MinEffA=1.0e4; MinN=50; MinRlC=1;

Ids="IdZenith,IdAzimuth,IdOmega,IdEffArea,IdN,IdFluxM,IdFluxMErr,IdDlM,IdDlMErr,IdRlM,IdRlC,IdDlC,IdFluxC,IdNC,IdSurface,IdTunnels,IdTemp,IdTemp2,NMaps"
IdZenith=0;IdAzimuth=1;IdOmega=2;IdEffArea=3;IdN=4;IdFluxM=5;IdFluxMErr=6;IdDlM=7;IdDlMErr=8;IdRlM=9;IdRlC=10;IdDlC=11;IdFluxC=12;IdNC=13;IdSurface=14;IdTunnels=15;IdTemp=16;IdTemp2=17;NMaps=18;

C(n)=($3>=0?column(n+3):NaN)
PI=acos(0)*2.0; rd=180/PI;
set angles deg

# Get info from Muogramfile
Density=word(system(sprintf("head -n 20 %s \| grep Density",MuogramFile)),2)+0.0
NTracks=word(system(sprintf("head -n 20 %s \| grep NTracks",MuogramFile)),2)+0.0


r=92; set xrange [-r:r]; set yrange [-r:r];
set xtics 30; set mxtics 3; set ytics 30; set mytics 3;
set style data image
unset key
unset xtics; unset ytics; unset border;
LegendGeoColour="yellow"; LegendGeoColourDark="black"; 
GeoLabelPos=85; GeoLabelPos2=70;
set label 11 "N" at 0,+85 center front tc rgb LegendGeoColour;
set label 12 "S" at 0,-85 center front tc rgb LegendGeoColour; 
set label 13 "W" at -85,0 center front tc rgb LegendGeoColour;
set label 14 "E" at +85,0 center front tc rgb LegendGeoColour; 
set label 15 "NE" at +GeoLabelPos2,+GeoLabelPos2 center front tc rgb LegendGeoColourDark;
set label 16 "SE" at +GeoLabelPos2,-GeoLabelPos2 center front tc rgb LegendGeoColourDark;
set label 17 "SW" at -GeoLabelPos2,-GeoLabelPos2 center front tc rgb LegendGeoColourDark;
set label 18 "NW" at -GeoLabelPos2,+GeoLabelPos2 center front tc rgb LegendGeoColourDark;
do for [n=3:9:3] { set label n+0 "".n*10 at n*2,n*10 center front tc rgb LegendGeoColour; }
do for [n=3:9:3] { set label n+1 "".n*10 at -n*2,-n*10+2 center front tc rgb LegendGeoColourDark; }
#Circles
do for [n=1:9]   { set obj 100+n ellipse at 0,0 size (n*20),(n*20) front fs border lt 5 empty dt 1; } 
do for [n=1:9]   { set obj 110+n ellipse at 0,0 size (n*20),(n*20) front fs border lt 0 empty dt (2,2); } 
do for [n=3:9:3] { set obj 110+n front dt (4,4); }

#set obj 111 ellipse at 0,0 size 90,90 front fs border lt 0 empty dashtype 1
#set obj 112 ellipse at 0,0 size 90,90 front fs border lt 5 empty dashtype 2


#set title "Azimuth [deg]"
#plot MuogramFile u 1:2:(C(IdAzimuth))
#set title "Zenith [deg]"
#plot MuogramFile u 1:2:(C(IdZenith))
#set title "Omega/bin [deg2]"
#plot MuogramFile u 1:2:(C(IdOmega)*rd*rd)
#set title "EffArea [dm2]"
#plot MuogramFile u 1:2:(C(IdEffArea)*1.0e-4)
set title "N.Tracks"
plot MuogramFile i 1 u 1:2:(C(IdEffArea)>MinEffA? C(IdN) : NaN)
set title "Flux Measured [1/m2/sr/s]"
plot MuogramFile i 1 u 1:2:(C(IdEffArea)>MinEffA? C(IdFluxM) : NaN)
#set title "DensityLength Measured [kg/m2] "
#plot MuogramFile i 1 u 1:2:(C(IdEffArea)>MinEffA? C(IdDlM) : NaN)
set title "DensityHeight (Dl*cos) Measured [kg/m2] "
plot MuogramFile i 1 u 1:2:(C(IdEffArea)>MinEffA && C(IdN)>MinN && C(IdDlM)>0 ? C(IdDlM)*cos(C(IdZenith)) : NaN)
set title "RockHeight (Rl*cos) Measured [m] (d:2.6)"
plot MuogramFile i 1 u 1:2:(C(IdEffArea)>MinEffA && C(IdN)>MinN && C(IdDlM)>0 ? C(IdDlM)*cos(C(IdZenith))/2.6 : NaN)
#set title "RockLength Calc [m] "
#plot MuogramFile i 1 u 1:2:(C(IdRlC)>MinRlC? C(IdRlC) : NaN)
set title "RockHeight (Rl*cos) Calc [m] "
plot MuogramFile i 1 u 1:2:(C(IdRlC)>MinRlC? C(IdRlC)*cos(C(IdZenith)) : NaN)
#set title "DensityLength Calc [kg/m2] "
#plot MuogramFile i 1 u 1:2:(C(IdRlC)>MinRlC? C(IdDlC) : NaN)
#set title "DensityHeight (Dl*cos) Calc [kg/m2] "
#plot MuogramFile i 1 u 1:2:(C(IdRlC)>MinRlC? C(IdDlC)*cos(C(IdZenith)) : NaN)


set title "SurfaceHeight (Rl_s*cos) Calc [m] "
plot MuogramFile i 1 u 1:2:(C(IdSurface)>MinRlC? C(IdSurface)*cos(C(IdZenith)) : NaN)
set title "TunnelsHeight (Rl_t*cos) Calc [m] "
plot MuogramFile i 1 u 1:2:(C(IdTunnels)>MinRlC? C(IdTunnels)*cos(C(IdZenith)) : NaN)
#set title "Tunnels (Rl_t) Calc [m] "
#plot MuogramFile i 1 u 1:2:(C(IdTunnels)>MinRlC? C(IdTunnels) : NaN)



set title "Average Density [kg/m3] "
set cbrange [2:3]
plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdDlM)/C(IdRlC) : NaN)
set cbrange [*:*]
set title "MissingRock [m] "
plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdRlC)-C(IdDlM)/Density : NaN)
set title "MissingRock [m] " ;
set cbrange [-10:10]
plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdRlC)-C(IdDlM)/Density : NaN)
set cbrange [*:*]

# Multiplot maps
set multiplot layout 2,2
stats [*:*][*:*] MuogramFile i 1 u (C(IdEffArea)>MinEffA && C(IdN)>MinN? C(IdDlM)*cos(C(IdZenith)) : NaN) name "DhM" nooutput
stats [*:*][*:*] MuogramFile i 1 u (C(IdRlC)>MinRlC? C(IdDlC)*cos(C(IdZenith)) : NaN) name "DhC" nooutput
DhMin=int((DhM_min+DhC_min)/2.); DhMax=int((DhM_max+DhC_max)/2.); 
set title "DensityHeight (Dl*cos) Measured [kg/m2] "
set cbrange [DhMin:DhMax+0.1]
plot MuogramFile i 1 u 1:2:(C(IdEffArea)>MinEffA && C(IdN)>MinN? C(IdDlM)*cos(C(IdZenith)) : NaN)
set title "DensityHeight (Dl*cos) Calc [kg/m2] "
plot MuogramFile i 1 u 1:2:(C(IdRlC)>MinRlC? C(IdDlC)*cos(C(IdZenith)) : NaN)
set cbrange [*:*]
set title "Average Density [kg/m3] "
plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdDlM)/C(IdRlC) : NaN)
set title "MissingRock [m] "
set cbrange [-10:10]
plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdRlC)-C(IdDlM)/Density : NaN)
set cbrange [*:*]
unset multiplot



## Multiplot maps test of tunnels
#set multiplot layout 2,2
#stats [*:*][*:*] MuogramFile i 1 u (C(IdEffArea)>MinEffA && C(IdN)>MinN? C(IdDlM)*cos(C(IdZenith)) : NaN) name "DhM" nooutput
#stats [*:*][*:*] MuogramFile i 1 u (C(IdRlC)>MinRlC? C(IdDlC)*cos(C(IdZenith)) : NaN) name "DhC" nooutput
#DhMin=int((DhM_min+DhC_min)/2.); DhMax=int((DhM_max+DhC_max)/2.); 
#set cbrange [-10:10]
#set title "MissingRock from Surface[m] "
#plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdSurface)-C(IdDlM)/Density : NaN)
#set title "Tunnels (Rl_t) Calc [m] "
#plot MuogramFile i 1 u 1:2:(C(IdTunnels)>MinRlC? C(IdTunnels) : NaN)
#set title "MissingRock with Tunnels [m] "
#plot MuogramFile i 1 u 1:2:(C(IdN)>MinN && C(IdRlC)>MinRlC ? C(IdRlC)-C(IdTunnels)-C(IdDlM)/Density : NaN)
#set cbrange [*:*]
#unset multiplot
#




# Clear MapPlot settings
do for [n=1:900] { unset obj n; unset label n; }
set xtics; set ytics; set border; 
# Density
set title "Density scatter plot"
set xtics auto; set mxtics; set ytics auto; set mytics; set xrange [*:*]; set yrange [*:*];  set grid x y;
set bars small; set key top left;
set xlabel "RockLength Calc [m]"; 
set ylabel "DensityLength Meas [kg/m2]"
plot [0:][] MuogramFile i 1 u (C(IdRlC)>MinRlC && C(IdN)>0 ? C(IdRlC) :NaN):(C(IdDlM)):(C(IdDlMErr)) w e lc 0 t "", \
	MuogramFile i 1 u (C(IdRlC)>MinRlC && C(IdN)>MinN ? C(IdRlC) :NaN):(C(IdDlM)):(C(IdDlMErr)) w e lc rgb "red" t "", \
	Density*x lc rgb "blue" t sprintf("Density= %4.2f",Density)


# Histograms
stats MuogramFile i 2 u 2 nooutput
if (STATS_mean < 1) exit;

set title "Density distribution"
set xlabel "DlM/RlC [kg/m3]"; set ylabel "NBins"
G(x)=exp(-(x-Gm)*(x-Gm)/(2.*Gs*Gs)) * Gn/(Gs*sqrt(2.*pi))
Gp(x,m,s,n)=exp(-(x-m)*(x-m)/(2.*s*s)) * n/(s*sqrt(2.*pi))
Gm=Density; Gs=0.3; Gn=1000;
set fit nolog quiet
set fit maxiter 15
fit G(x) MuogramFile i 2 u 1:2 via Gm,Gs,Gn
plot [:] MuogramFile i 2 u 1:2 w his lc rgb "red" lw 2 t "", G(x) lc rgb "blue" t sprintf("\nGauss fit:\n %4.2f , %4.2f",Gm,Gs)

set title "DensityLength Difference Distribution"
set xlabel "(DlM-DlC)/DlMErr []"
Gm=0.1; Gs=1.1;
fit G(x) MuogramFile i 3 u 1:2 via Gm,Gs,Gn
plot [:] MuogramFile i 3 u 1:2 w his lc rgb "red" lw 2 t "", G(x) lc rgb "blue" t sprintf("\nGauss fit:\n %4.2f , %4.2f",Gm,Gs), Gp(x,0,1,Gn) lc rgb "black" t ""



## Tests



set term x11; set out;

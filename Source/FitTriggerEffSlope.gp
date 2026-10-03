# FitTriggerEffSlope.gp
#	Gnuplot script for fitting the Slope-dependent TriggerEfficiency of a Chamber
#	it uses the TriggerEffSlope hist as input
#	and an approximation for the Landau distribution

# Approximated function of the integrated dE Landau distribution
f(q)=1.0/(1.0+q**2) + 0.27/(q**2+0.30/q**2)
# Efficiency calculated from the above function
e(m)=f(Q/sqrt(1+m*m))
# it has only one parameter, the perpendicular dE (noted as Q)
#TesFile="Temp/Tes-0.temp"; # this shall be given

# Estimation for Q if the zero-efficiency is known as Eff0
#Eff0=0.74;
#q=0.01; while (e(0)>Eff0) {q=q+0.01;} ; print q,d,e(0),Eff;
# This is not used now...

set fit quiet
NCut=20;
Q=0.8;

fit [0.2:] e(x) TesFile u ($3>NCut?$1:NaN):4:(sqrt($2-$3+1)/$2) yerror via Q
#plot TesFile u ($3>NCut?$1:NaN):4:(sqrt($2-$3+1)/$2) w e, e(x) lc 3
#pause 0.5
system sprintf("echo %g > %s",Q,TesFile);


////////////////////////////////////////////////////////////////////////
//  FitLineClass - implementation
//  	for fitting a line
//	verion : 2015
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "FitLineClass.h"

FitLineClass::FitLineClass()
	{
	Null();
	};

FitLineClass::~FitLineClass()
	{}

void FitLineClass::Null()
	{
	Sx=0.; Sy=0.; Sxx=0.; Sxy=0.; Syy=0.; N=0.;
	m=0.; b=0.;
	fitted=false;
	};

void FitLineClass::Add(double InX, double InY, double InWeight)
	{
	N+=InWeight;
	Sx+=InX*InWeight; Sy+=InY*InWeight;
	Sxx+=InX*InX*InWeight; Sxy+=InX*InY*InWeight; Syy+=InY*InY*InWeight;
	fitted=false;
	}

bool FitLineClass::Fit()
	{
	if ( (N==0) || ((N*Sxx-Sx*Sx)==0) ) return false;
	m=(N*Sxy-Sx*Sy)/(N*Sxx-Sx*Sx);
	b=(Sxx*Sy-Sxy*Sx)/(N*Sxx-Sx*Sx);
	err2=Syy-m*Sxy-b*Sy;
	err=sqrt(err2);
	fitted=true;
	return true;
	}

bool FitLineClass::IsFitted()
	{ return fitted; }

double FitLineClass::GetSlope()
	{ if (!fitted) Fit(); return m; }

double FitLineClass::GetB()
	{ if (!fitted) Fit(); return b; }

double FitLineClass::GetError()
	{ if (!fitted) Fit(); return err; }
	
double FitLineClass::GetError2()
	{ if (!fitted) Fit(); return err2; }

double FitLineClass::GetEntries()
	{ return N; }

void FitLineClass::PutParams(double *pInM, double *pInB)
	{ if (!fitted) Fit(); *pInM = m; *pInB=b; }

double FitLineClass::ComputeY(double InX)
	{ if (!fitted) Fit(); return (m*InX+b); }

void FitLineClass::Print()
	{
	printf("Fitted line of %g points : ",N);
	if (!fitted) { printf("not fitted.\n"); return; }
	printf("slope=%10f, shift=%10f, err=%10f\n", m,b,err);
	}



////////////////////////////////////////////////////////////////////////////////
// Flux to DensityLength Conversion Class
//      version : 2023-04-28
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include "FluxDlConvClass.h"


// ! TODO !
// atterni mindenhol Radian -> Degree : ahol: Zenith,Azimuth van
//	valamint akkor mar a nevben is "Z" theta helyett.. :(

// Flux-Bin conversion

const double FluxBinMax=100.0;
const double FluxBinResolution=100.0;
double FluxToBin(double InFlux)
	{ return (-FluxBinResolution*log(InFlux/FluxBinMax)); }
double BinToFlux(double InBin)
	{ return exp(-InBin/FluxBinResolution)*FluxBinMax; }

// OTLET :
// Flux(dl)  ~=  100/ ( 1 + x**2/250)
// Flux(dl)/dl ~= 100/ ( 1 + x**3/250)  
// Flux(dl)*dl ~= 100 * x / ( 1 + x**3/250)  
// Flux(dl)*dl ~= 500 * exp(-x/200)



// FluxDlConversionClass

FluxDlConversionClass::FluxDlConversionClass() : MultiMapClass(3)
	{
	}

bool FluxDlConversionClass::ReadFromFileName(char* InFileName)
	{
	FILE * InFile = fopen(InFileName,"r");
	if (InFile==NULL) return false;
	bool Ret=ReadFromFile(InFile);
	fclose(InFile);
	return Ret;
	}

bool FluxDlConversionClass::ReadFromFile(FILE * InFile)
	{
	MultiMapClass::ReadFromFile(InFile);
	ZenithMin=XStartValue; ZenithMax=XEndValue; ZenithBinSize=XBinSize;
	FluxMax=ZAt(IdFlux,0,0); FluxMin=ZAt(IdFlux,0,MaxNYBins-1);
	DlMin=ZAt(IdDl,0,0); DlMax=ZAt(IdDl,0,MaxNYBins-1);
	Resolution=YBinSize;
	return CheckFluxBinFunction();
	}

bool FluxDlConversionClass::CheckFluxBinFunction()
	{
	double Flux1,Flux2;
	int	NWrongBinning=0;
	for (int YBin=0; YBin<MaxNYBins; YBin++)
		{
		Flux1=ZAt(IdFlux,0,YBin);
		Flux2=BinToFlux(YAt(YBin));
		if ( Flux1>0 && Flux2>0 && ((Flux1-Flux2)/(Flux1+Flux2)*2 > 1e-4) ) NWrongBinning++;
		//printf("NBin:%d : FluxBin:%g , %g ?=? %g , refdiff: %g\n",YBin,YAt(YBin),Flux1,Flux2,(Flux1-Flux2)/(Flux1+Flux2)*2);
		}
	if (NWrongBinning>0) { printf("Warning! FluxBin mismatch at %d cases!\n",NWrongBinning); return false; }
	return true;
	}

double FluxDlConversionClass::GetDlFromZF(double InZenith, double InFlux)
	{
	double Bin=FluxToBin(InFlux);
	if (InZenith<0.001) return Z(IdDl,0,Bin);
	else return ZWeighted(IdDl,InZenith,Bin);
	}

double FluxDlConversionClass::GetDlFromZdegF(double InZenithDeg, double InFlux)
	{
	double Bin=FluxToBin(InFlux);
	return ZWeighted(IdDl,InZenithDeg*M_PI/180.,Bin);
	}

double FluxDlConversionClass::GetDlRelErrFromZF(double InZenith, double InFlux)
	{
	double Bin=FluxToBin(InFlux);
	return ZWeighted(IdDlRelErr,InZenith,Bin);
	}


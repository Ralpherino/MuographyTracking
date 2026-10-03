////////////////////////////////////////////////////////////////////////
// Multiple Histogram with Function defined BinSizes - implementation
//	version : 2020-03-13
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiHistFuncBinClass.h"

MultiHistFuncBinClass::MultiHistFuncBinClass(int InMaxNHists, double InStartValue, double InEndValue, double InBinSize0, double (*InBsFunction)(double) , char* InName)
	{
	MaxNHists=InMaxNHists;
	BinSize=InBinSize0;
	StartValue=InStartValue;
	EndValue=InEndValue;
	BinSize=InBinSize0;
	sprintf(TheName,"%s",InName);
	BsFunction=InBsFunction;
	
	BinStart = (double*) calloc(AMaxNBins_MultiHistFuncBinClass,sizeof(double));
	BinSizeA = (double*) calloc(AMaxNBins_MultiHistFuncBinClass,sizeof(double));
	int NBin=0;
	BinStart[0]=StartValue;
	while ( (BinStart[NBin]<EndValue) && (NBin<AMaxNBins_MultiHistFuncBinClass-1) )
		{
		BinSizeA[NBin]=BinSize*BsFunction(BinStart[NBin]);
		BinStart[NBin+1]=BinStart[NBin]+BinSizeA[NBin];
		NBin++;
		}
	BinSizeA[NBin-1]=BinSize*BsFunction(BinStart[NBin-2]); // set the last as the one before
	MaxNBins=NBin;
	BinStart = (double*) realloc(BinStart,MaxNBins*sizeof(double));
	BinSizeA = (double*) realloc(BinSizeA,MaxNBins*sizeof(double));
	AllocateMemory();
	
	// Set Names
	for (int NHist=0; NHist<MaxNHists; NHist++) sprintf(Name[NHist],"Hist%d",NHist);
	sprintf(Name[MaxNHists],"Overflow");

	}


MultiHistFuncBinClass::~MultiHistFuncBinClass()
	{
	if (Hist==NULL) return;
	for (int NHist=0; NHist<MaxNHists+1; NHist++) { free(Hist[NHist]); free(Name[NHist]); }
	free(Hist); free(Name);
	free(BinStart);
	free(BinSizeA);
	}

// General

void MultiHistFuncBinClass::AllocateMemory()
	{
	Hist = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Hist[NHist] = (double*) calloc(MaxNBins,sizeof(double));
	Name = (char**) calloc(MaxNHists+1,sizeof(char*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Name[NHist] = (char*) calloc(100,sizeof(char));

	}


// Bin operations

int MultiHistFuncBinClass::CheckNHist(int InNHist)
	{
	if (InNHist<0 || InNHist>MaxNHists) return MaxNHists;
	return InNHist;
	}

int MultiHistFuncBinClass::CheckNBin(int InNBin)
	{
	if (InNBin<0) return 0;
	if (InNBin>MaxNBins-1) return MaxNBins-1;
	return InNBin;
	}

int MultiHistFuncBinClass::GetBin(double InX)
	{
	int NBin=0;
	while ( (BinStart[NBin]<InX) && (NBin<MaxNBins) ) NBin++;
	NBin--; if (NBin<0) NBin=0;
	return NBin;
	}

void MultiHistFuncBinClass::Add(int NHist, double InX, double InWeight)
	{
	Hist[CheckNHist(NHist)][GetBin(InX)]+=InWeight;
	}

void MultiHistFuncBinClass::Set(int NHist, double InX, double InValue)
	{
	Hist[CheckNHist(NHist)][GetBin(InX)]=InValue;
	}

double MultiHistFuncBinClass::XAt(int InNBin)
	{
	int NBin=CheckNBin(InNBin);
	return (BinStart[NBin]+BinSizeA[NBin]/2.0);
	}

double MultiHistFuncBinClass::YAt(int InNHist, int InNBin)
	{
	return Hist[CheckNHist(InNHist)][CheckNBin(InNBin)];
	}

double MultiHistFuncBinClass::Y(int InNHist, double InX)
	{
	return Hist[CheckNHist(InNHist)][GetBin(InX)];
	}

// Hist operations

double MultiHistFuncBinClass::Integral(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	double IntegralPart=0.;
	for (int NBin=0; NBin<MaxNBins; NBin++) IntegralPart+=Hist[NHist][NBin];
	return IntegralPart;
	}

void MultiHistFuncBinClass::NormIntegral(int InNHist, double NewIntegral)
	{
	int NHist=CheckNHist(InNHist);
	double Scale=NewIntegral/Integral(NHist);
	for (int NBin=0; NBin<MaxNBins; NBin++) Hist[NHist][NBin]*=Scale;
	}

// File Operations

void MultiHistFuncBinClass::WriteToFile(FILE * Outfile)
	{
	int NHist, NBin; 
	fprintf(Outfile,"#%s NHists %d Start %g End %g BinSize %g NBins %d\n#X\tSize",TheName,MaxNHists,StartValue,EndValue,BinSize,MaxNBins);
	for (NHist=0; NHist<MaxNHists; NHist++) fprintf(Outfile,"\t%s",Name[NHist]);
	fprintf(Outfile,"\n"); 
	for (NBin=0; NBin<MaxNBins; NBin++) 
		{
		fprintf(Outfile,"%g\t%g",XAt(NBin),BinSizeA[NBin]);
		for (NHist=0; NHist<MaxNHists; NHist++) fprintf(Outfile,"\t%g",Hist[NHist][NBin]); 
		fprintf(Outfile,"\n"); 
		}
	fprintf(Outfile,"\n\n"); 
	}




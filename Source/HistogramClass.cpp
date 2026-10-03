////////////////////////////////////////////
// HistrogramClass - implementation
//	written by : Gergo" Hamar
// 	version: 2020-06-05
////////////////////////////////////////////

#include "HistogramClass.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ExtraFunctions.h"

HistogramClass::HistogramClass(double InStartValue, double InEndValue, double InBinSize, char * InName)
	{
	Hist=NULL;
	strcpy(Name,InName);
	StartValue=InStartValue;
	EndValue=InEndValue;
	BinSize=InBinSize;
	MaxNBins=(int)((EndValue-StartValue)/BinSize+0.1)+1;
	AllocateMemory();
	Clear();
	}
	
HistogramClass::~HistogramClass()
	{
	FreeMemory();
	}

void HistogramClass::AllocateMemory()
	{
	if (Hist!=NULL) { printf("Warning: Histogram (%s) memory reallocation issue\n",Name); FreeMemory(); }
	Hist=(double*)calloc(MaxNBins,sizeof(double));
	}

void HistogramClass::FreeMemory()
	{
	if (Hist==NULL) return;
	free(Hist);
	Hist=NULL;
	}
	
// Histogram operations ////////////////////////////////////////////////

void HistogramClass::Clear()
	{
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NBin]=0.;
	NData=0;
	}

void HistogramClass::Add(double InX, double InWeight)
	{
	Hist[GetBin(InX)]+=InWeight;
	NData++;
	}

bool HistogramClass::Clone(HistogramClass* HistToClone)
	{
	StartValue	= HistToClone->StartValue;
	EndValue	= HistToClone->EndValue;
	BinSize	= HistToClone->BinSize;
	MaxNBins	= HistToClone->MaxNBins;
	NData		= HistToClone->NData;
	AllocateMemory();
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NBin]=HistToClone->Hist[NBin];
	// cross-checks
	if (MaxNBins != (int)((EndValue-StartValue)/BinSize)+1) return false;
	return true;
	}

bool HistogramClass::SameBinning(HistogramClass* IsSameBinningHist)
	{
	if (	( StartValue == IsSameBinningHist->StartValue ) &&
		( EndValue	== IsSameBinningHist->EndValue ) &&
		( BinSize	== IsSameBinningHist->BinSize ) &&
		( MaxNBins	== IsSameBinningHist->MaxNBins) ) return true;
	return false;
	}

bool HistogramClass::IsSameAs(HistogramClass* OtherHist)
	{
	if (!SameBinning(OtherHist)) return false;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		if (Hist[NBin]!=OtherHist->Hist[NBin]) return false;
	return true;
	}

bool HistogramClass::AddHist(HistogramClass* AddedHist)
	{
	if (!(SameBinning(AddedHist))) return false;

	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NBin]+=AddedHist->Hist[NBin];
	return true;
	}

bool HistogramClass::MultiplyBy(HistogramClass* MultiplicatorHist)
	{
	if (!(SameBinning(MultiplicatorHist))) return false;

	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NBin]*=MultiplicatorHist->Hist[NBin];
	return true;
	}

bool HistogramClass::DivideBy(HistogramClass* DividerHist)
	{
	if (!(SameBinning(DividerHist))) return false;

	for (int NBin=0; NBin<MaxNBins; NBin++)
		if (DividerHist->Hist[NBin]!=0) Hist[NBin]/=DividerHist->Hist[NBin];
		else Hist[NBin]=0;
	return true;
	}

void HistogramClass::Fill(double FillValue)
	{
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NBin]=FillValue; 
	NData=0;
	}

void HistogramClass::NormIntegral(double NewIntegral)
	{
	double NormFactor=NewIntegral/Integral();
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NBin]*=NormFactor;
	}

void HistogramClass::Sink(double SinkLevel)
	{
	for (int NBin=0; NBin<MaxNBins; NBin++) if (YAt(NBin)>SinkLevel) Hist[NBin]-=SinkLevel; else Hist[NBin]=0.;
	}

void HistogramClass::SinkByAverage(double SinkMultiplicator)
	{
	double AverageHeight=Integral()/(double)MaxNBins;
	Sink(AverageHeight*SinkMultiplicator);
	}

void HistogramClass::Crop(double XMin, double XMax)
	{
	for (int NBin=0; NBin<GetBin(XMin); NBin++)
		Hist[NBin]=0.;
	for (int NBin=GetBin(XMax); NBin<MaxNBins; NBin++)
		Hist[NBin]=0.;
	}

void HistogramClass::Cut(double XMin, double XMax)
	{
	for (int NBin=GetBin(XMin); NBin<GetBin(XMax); NBin++)
		Hist[NBin]=0.;
	}

void HistogramClass::Smooth(int NIterations)
	{
	int NBin;
	double SmoothedHist[MaxNBins];
	SmoothedHist[0]=0.75*Hist[0]+0.25*Hist[1];
	for (NBin=1; NBin<MaxNBins-2; NBin++)
		SmoothedHist[NBin]=0.25*Hist[NBin-1]+0.5*Hist[NBin]+0.25*Hist[NBin+1];
	SmoothedHist[MaxNBins-1]=0.75*Hist[MaxNBins-1]+0.25*Hist[MaxNBins-2];
	for (NBin=1; NBin<MaxNBins-2; NBin++)
		Hist[NBin]=SmoothedHist[NBin];
	if (NIterations>1) Smooth(NIterations-1);
	}

void HistogramClass::SmoothStep(int HalfWidth)
	{
	double SmoothedHist[MaxNBins];
	int n, bin;
	double sum=0;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		{
		sum=0;
		for (n=-HalfWidth; n<HalfWidth+1; n++) 
			{
			bin = NBin+n;
			if (bin<0) bin=0;
			if (bin>MaxNBins-1) bin=MaxNBins-1;
			sum+=Hist[bin];
			}
		SmoothedHist[NBin]=sum/(double)(2*HalfWidth+1);
		}
	for (int NBin=0; NBin<MaxNBins-1; NBin++) Hist[NBin]=SmoothedHist[NBin];
	}
	
void HistogramClass::SmoothStepDouble(double HalfWidth)
	{
	SmoothStep(int(HalfWidth/BinSize));
	}

void HistogramClass::CutEdges()
	{
	Hist[0]=0.;
	Hist[MaxNBins-1]=0.;
	}

void HistogramClass::ShiftXBin(int ShiftBin)
	{
	double ShiftedHist[MaxNBins]; int SNBin;
	for (int NBin=0; NBin<MaxNBins; NBin++) ShiftedHist[NBin]=0;
	for (int NBin=0; NBin<MaxNBins; NBin++) 
		{
		SNBin=NBin+ShiftBin;
		if (SNBin<0) SNBin=0;
		if (SNBin>MaxNBins-1) SNBin=MaxNBins-1;
		ShiftedHist[SNBin]=Hist[NBin];
		}
	for (int NBin=0; NBin<MaxNBins; NBin++) 
		Hist[NBin]=ShiftedHist[NBin];
	}

void HistogramClass::ShiftX(double Shift)
	{
	ShiftXBin(int(Shift/BinSize));
	}


// Getting info ////////////////////////////////////////////////////////

double HistogramClass::Mean()
	{
	double Sum=0., TheIntegral=Integral();
	for (int NBin=0; NBin<MaxNBins; NBin++) Sum+=XAt(NBin)*YAt(NBin);
	if (TheIntegral==0) return 0;
	else return (Sum/TheIntegral);
	}

double HistogramClass::Sigma()
	{
	double TheMean=Mean();
	double Error2=0.;
	for (int NBin=0; NBin<MaxNBins; NBin++) Error2+=(XAt(NBin)-TheMean)*(XAt(NBin)-TheMean)*YAt(NBin);
	return sqrt(Error2/Integral());
	}

double HistogramClass::Median()
	{
	if (Integral()<1e-300) return 0;
	int NBin;
	double IntegralUntilX=0., LowerDistance, UpperDistance, TheIntegral=Integral();
	for (NBin=0; NBin<MaxNBins; NBin++)
		{
		IntegralUntilX+=YAt(NBin);
		if (IntegralUntilX>(TheIntegral/2.0)) break;
		}
	if (NBin==0) return 0.;
	UpperDistance=IntegralUntilX/TheIntegral-0.5;
	LowerDistance=0.5-(IntegralUntilX-YAt(NBin))/TheIntegral;
	return ((UpperDistance*XAt(NBin-1) + LowerDistance*XAt(NBin))/(UpperDistance+LowerDistance));
	}

double HistogramClass::TruncatedMean(double TruncMin, double TruncMax)
	{
	double Sum=0,N=0;
	double TheIntegral=Integral(), IntegralUntilX=0;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		{
		IntegralUntilX+=YAt(NBin);
		if ( (IntegralUntilX<TheIntegral*TruncMin) || (IntegralUntilX>TheIntegral*TruncMax) ) continue;
		N+=YAt(NBin);
		Sum+=XAt(NBin)*YAt(NBin);
		}
	if (N==0) return -1;
	else return Sum/N;
	}

double HistogramClass::PeakPosition()
	{
	int	PeakBin=0;
	for (int NBin=0; NBin<MaxNBins; NBin++) if (YAt(NBin)>YAt(PeakBin)) PeakBin=NBin;
	return XAt(PeakBin);
	}

double HistogramClass::PeakHight()
	{
	int	PeakBin=0;
	for (int NBin=0; NBin<MaxNBins; NBin++) if (YAt(NBin)>YAt(PeakBin)) PeakBin=NBin;
	return YAt(PeakBin);
	}

double HistogramClass::PeakSigma(int NInterations, double SigmaCut, double * pThisPeakPosition)
	{
	int	NBinMin, NBinMax, NBin;
	double NewIntegral, NewSum, NewMean=0, NewError2, NewSigma=0;
	NBinMin=GetBin(PeakPosition()-SigmaCut*Sigma()); NBinMax=GetBin(PeakPosition()+SigmaCut*Sigma()+BinSize);
	
	for (int i=0; i<NInterations; i++) 
		{
		NewIntegral=0.; NewSum=0.; NewError2=0.;
		for (NBin=NBinMin; NBin<NBinMax; NBin++) NewIntegral+=YAt(NBin);
		for (NBin=NBinMin; NBin<NBinMax; NBin++) NewSum+=XAt(NBin)*YAt(NBin);
		NewMean=NewSum/NewIntegral;
		for (NBin=NBinMin; NBin<NBinMax; NBin++) NewError2+=(XAt(NBin)-NewMean)*(XAt(NBin)-NewMean)*YAt(NBin); 
		NewSigma=sqrt(NewError2/NewIntegral);
		//printf("%f   %f ,  %f  %f, %f\n", NewMean, NewSigma, X(NBinMin), X(NBinMax), PeakPosition() );
		NBinMin=GetBin(NewMean-SigmaCut*NewSigma); NBinMax=GetBin(NewMean+SigmaCut*NewSigma+BinSize);
		}
	if (pThisPeakPosition!=NULL)  *pThisPeakPosition=NewMean;
	return NewSigma;
	}

double HistogramClass::PeakSmooth(int NInterations, double SigmaCut, double * pThisSigma)
	{
	double ThisMean;
	double ThisSigma=PeakSigma(NInterations,SigmaCut,&ThisMean);
	if (pThisSigma!=NULL) *pThisSigma=ThisSigma;
	return ThisMean;
	}

double HistogramClass::PeakFwhm()
	{
	int HmR=GetBin(PeakPosition()), HmL=GetBin(PeakPosition());
	double HalfHeight=PeakHight()/2.0;
	while ( (Hist[HmR]>HalfHeight) && (HmR<MaxNBins-1) )	HmR++;
	while ( (Hist[HmL]>HalfHeight) && (HmL>1) )		HmL--;
	double HmLReal = Interpolate(Hist[HmL],HmL,Hist[HmL+1],HmL+1,HalfHeight);
	double HmRReal = Interpolate(Hist[HmR],HmR,Hist[HmR-1],HmR-1,HalfHeight);
	if (HmL==GetBin(PeakPosition())) HmLReal=XAt(HmL);
	if (HmR==GetBin(PeakPosition())) HmRReal=XAt(HmR);
	return (XAtDouble(HmRReal)-XAtDouble(HmLReal));
	}

double HistogramClass::LocalPeakPosition(double XMin, double XMax)
	{
	int	LocalPeakBin;
	int	XMinBin=GetBin(XMin), XMaxBin=GetBin(XMax);
	LocalPeakBin=XMinBin;
	for (int NBin=XMinBin; NBin<XMaxBin; NBin++) if (YAt(NBin)>YAt(LocalPeakBin)) LocalPeakBin=NBin;
	return XAt(LocalPeakBin);
	}

double HistogramClass::LocalPeakFwhm(double XMin, double XMax)
	{
	int LocalPeakBin=GetBin(LocalPeakPosition(XMin,XMax));
	int HmR=LocalPeakBin, HmL=LocalPeakBin;
	double HalfHeight=Hist[LocalPeakBin]/2.0;
	while ( (Hist[HmR]>HalfHeight) && (HmR<GetBin(XMax)) )	HmR++;
	while ( (Hist[HmL]>HalfHeight) && (HmL>GetBin(XMin)) )	HmL--;
	double HmLReal = Interpolate(Hist[HmL],HmL,Hist[HmL+1],HmL+1,HalfHeight);
	double HmRReal = Interpolate(Hist[HmR],HmR,Hist[HmR-1],HmR-1,HalfHeight);
	if (HmL==LocalPeakBin) HmLReal=XAt(HmL);
	if (HmR==LocalPeakBin) HmRReal=XAt(HmR);
	return (XAtDouble(HmRReal)-XAtDouble(HmLReal));
	}

double HistogramClass::LocalValleyPosition(double XMin, double XMax)
	{
	int	LocalValleyBin;
	int	XMinBin=GetBin(XMin), XMaxBin=GetBin(XMax);
	LocalValleyBin=XMinBin;
	for (int NBin=XMinBin; NBin<XMaxBin; NBin++) if (YAt(NBin)<YAt(LocalValleyBin)) LocalValleyBin=NBin;
	return XAt(LocalValleyBin);
	}

double HistogramClass::Integral()
	{
	double IntegralPart=0.;
	for (int NBin=0; NBin<=MaxNBins; NBin++) IntegralPart+=YAt(NBin);
	if (IntegralPart<1e-300) return 0;
	return IntegralPart;
	}

double HistogramClass::IntegralFromTo(double InFrom, double InTo)
	{
	double IntegralPart=0.;
	for (int NBin=GetBin(InFrom); NBin<=GetBin(InTo); NBin++) IntegralPart+=YAt(NBin);
	return IntegralPart;
	}
	
double HistogramClass::IntegralFrom(double InFrom)
	{
	double IntegralPart=0.;
	for (int NBin=GetBin(InFrom); NBin<MaxNBins; NBin++) IntegralPart+=YAt(NBin);
	return IntegralPart;
	}

double HistogramClass::IntegralTo(double InTo)
	{
	double IntegralPart=0.;
	for (int NBin=0; NBin<GetBin(InTo); NBin++) IntegralPart+=YAt(NBin);
	return IntegralPart;
	}

double HistogramClass::FirstZeroFromRight(double InFrom)
	{
	for (int NBin=GetBin(InFrom); NBin<MaxNBins; NBin++) if (YAt(NBin)==0) return XAt(NBin);
	return XAt(MaxNBins-1);
	}

double HistogramClass::FirstZeroFromLeft(double InFrom)
	{
	for (int NBin=GetBin(InFrom); NBin>0; NBin--) if (YAt(NBin)==0) return XAt(NBin);
	return XAt(0);
	}

// Bin info and operations /////////////////////////////////////////////

int HistogramClass::GetBin(double InX)
	{
	int NBin=(int)((InX-StartValue)/BinSize+0.5);
	if (NBin < 0) NBin=0;
	if (NBin >= MaxNBins) NBin=MaxNBins-1;
	return NBin;
	}
	
double HistogramClass::XAt(int InNBin)
	{ return (StartValue+BinSize*(double)InNBin); }

double HistogramClass::XMinAt(int InNBin)
	{ return (XAt(InNBin)-0.5*BinSize); }

double HistogramClass::XMaxAt(int InNBin)
	{ return (XAt(InNBin)+0.5*BinSize); }

double HistogramClass::YAt(int InNBin)
	{ return Hist[InNBin]; }

double HistogramClass::YScaledAt(int InNBin)
	{ return (Hist[InNBin]/BinSize); }

double HistogramClass::Y(double InX)
	{ return Hist[GetBin(InX)]; }

double HistogramClass::XAtDouble(double InNBinDouble)
	{ return (StartValue+BinSize*InNBinDouble); }

double HistogramClass::GetBinDouble(double InX)
	{ return ((InX-StartValue)/BinSize); }

double HistogramClass::GetWeightedYAtDouble(double InNBinDouble)
	{
	if ((InNBinDouble<=-1) || (InNBinDouble>=MaxNBins)) return 0.;
	if (InNBinDouble == ((int)InNBinDouble)) return YAt((int)InNBinDouble);
	int NBinLeft=(int)InNBinDouble;
	int NBinRight=(int)InNBinDouble+1;
	double WeightLeft=NBinRight-InNBinDouble;
	double WeightRight=InNBinDouble-NBinLeft;
	double ValueLeft, ValueRight;
	if (NBinLeft==-1) ValueLeft=0.; else ValueLeft=YAt(NBinLeft);
	if (NBinLeft==MaxNBins) ValueRight=0.; else ValueRight=YAt(NBinRight);
	return (ValueLeft*WeightLeft+ValueRight*WeightRight);
	}

double HistogramClass::GetWeightedY(double InX)
	{ return GetWeightedYAtDouble(GetBinDouble(InX)); }


// Fit line ////////////////////////////////////////////////////////////

bool HistogramClass::FitLine(FitLineClass * line, double InXMin, double InXMax)
	{
	line->Null();
	for (int NBin=GetBin(InXMin); NBin<GetBin(InXMax); NBin++)
		line->Add(XAt(NBin),YAt(NBin));
	line->Fit();
	return true;
	}

bool HistogramClass::FitLog(FitLineClass * logline, double InXMin, double InXMax)
	{
	logline->Null();
	for (int NBin=GetBin(InXMin); NBin<GetBin(InXMax); NBin++)
		if (YAt(NBin)>0) logline->Add(XAt(NBin),log(YAt(NBin)));
	logline->Fit();
	return true;
	}

// Write to... /////////////////////////////////////////////////////////


void HistogramClass::WriteParametersToScreen()
	{
	printf("%s :: Intergral: %g, Mean: %g,  Sigma: %g,  Median: %g\n",Name,Integral(),Mean(),Sigma(),Median());
	}

void HistogramClass::WriteToScreen()
	{
	WriteParametersToScreen();
	for (int NBin=0; NBin<MaxNBins; NBin++) printf("%g\t%g\n",XAt(NBin),YAt(NBin)); 
	}

void HistogramClass::WriteToFile(FILE * Outfile)
	{
	if (Outfile==NULL) { printf("Warning: Non valid file has been called\n"); return; }
	fprintf(Outfile,"#%s Start %g End %g BinSize %g NBins %d\n",Name,StartValue,EndValue,BinSize,MaxNBins);
	for (int NBin=0; NBin<MaxNBins; NBin++) fprintf(Outfile,"%g\t%g\n",XAt(NBin),YAt(NBin));
	fprintf(Outfile,"\n\n");
	}

void HistogramClass::WriteToFilename(char* Filename, char* OpenType)
	{
	FILE * OutFile;
	if ( (OutFile=fopen(Filename,OpenType)) == NULL ) 
		{printf("\nERROR : file: %s could not be opened\n\n",Filename); return;}
	WriteToFile(OutFile);
	fclose(OutFile);
	}

void HistogramClass::WriteToNewFile(char* NewFilename)
	{
	FILE * NewFile;
	if ( (NewFile=fopen(NewFilename,"w")) == NULL ) 
		{printf("\nERROR : file: %s could not be opened\n\n",NewFilename); return;}
	WriteToFile(NewFile);
	fclose(NewFile);
	}

bool HistogramClass::ReadFromFile(FILE* InFile)
	{
	int	NConversions=-1;
	char	TempName[50];
	NConversions=fscanf(InFile,"%s %*s %lf %*s %lf %*s %lf %*s %d",TempName,&StartValue,&EndValue,&BinSize,&MaxNBins);
	sprintf(Name,"%s",&(TempName[1]));
	if (NConversions!=5) { printf("Warning : Histogram read error\n"); return false; }
	int CalcMaxNBins = (int)((EndValue-StartValue)/BinSize+0.1)+1;
	if (CalcMaxNBins!=MaxNBins) { printf("Inconsystency in MaxNBins\n"); return false; }
	AllocateMemory();
	Clear();
	double InX, DiffMax=0.001*BinSize;
	for (int NBin=0; NBin<MaxNBins; NBin++) 
		{
		fscanf(InFile,"%lf",&InX);
		if (fabs(InX-XAt(NBin))>DiffMax) { printf("Inconsystency: XAt(%d)=%lf != %lf\n",NBin,XAt(NBin),InX); return false; }
		fscanf(InFile,"%lf",&(Hist[NBin]));
		}
	return true;
	}

bool HistogramClass::ReadFromGeneralFile(FILE* InFile, int MaxNReadBins)
	{
	if (InFile==NULL) { printf("Warning: Non valid file has been called\n"); return false; }
	bool	ContinueReadFile=true;
	char	Line[202];
	double	ReadX[MaxNReadBins],ReadY[MaxNReadBins];
	int	NLine=0, NReadBins=0, NConversions=-1;
	while (ContinueReadFile && !feof(InFile))
		{
		fgets(Line,200,InFile); NLine++;
		if (Line[0]=='#') { sscanf(&(Line[1]),"%s",Name); continue; }
		NConversions=sscanf(Line,"%lf %lf",&(ReadX[NReadBins]),&(ReadY[NReadBins]));
		if (NConversions<2 && NReadBins>0) ContinueReadFile=false;
		if (NConversions==2) NReadBins++;
		if (NReadBins>MaxNReadBins) { printf("Histogram size underestimated\n"); ContinueReadFile=false; }
		}
	if (NReadBins<2) { printf("No histogram has been found\n");  return false; }
	free(Hist);
	StartValue=ReadX[0]; EndValue=ReadX[NReadBins-1]; BinSize=ReadX[1]-ReadX[0];
	MaxNBins=(int)((EndValue-StartValue)/BinSize)+1;
	if (MaxNBins!=NReadBins) {printf("Inconsystency in data: %d read instead of %d\n",NReadBins,MaxNBins); return false;}
	AllocateMemory();
	Clear();
	for (int NBin=0; NBin<MaxNBins; NBin++) Hist[NBin]=ReadY[NBin];
	return true;
	}


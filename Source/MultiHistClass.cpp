////////////////////////////////////////////////////////////////////////
// Multiple Histograms Class - implementation
//	version : 2019-08-24
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiHistClass.h"
#include "ExtraFunctions.h"

MultiHistClass::MultiHistClass()
	{
	Hist=NULL;
	MaxNHists=0;
	}

MultiHistClass::MultiHistClass(int InMaxNHists, double InStartValue, double InEndValue, double InBinSize, char* InName)
	{
	MaxNHists = InMaxNHists;
	StartValue = InStartValue;
	EndValue = InEndValue;
	BinSize = InBinSize;
	if (StartValue>EndValue) { StartValue=InEndValue; EndValue=InStartValue; }
	MaxNBins = (int)((EndValue-StartValue)/BinSize+0.1)+1;
	AllocateMemory();
	ClearAll();
	sprintf(TheName,"%s",InName);
	char	NameString[100];
	for (int NHist=0; NHist<MaxNHists; NHist++)
		{ sprintf(NameString,"Hist%d",NHist); SetName(NHist,NameString); }
	SetName(MaxNHists,(char*)"Overflow");
	}

MultiHistClass::MultiHistClass(int InMaxNHists, MultiHistClass* SimilarMultiHist)
	: MultiHistClass(InMaxNHists, SimilarMultiHist->StartValue, SimilarMultiHist->EndValue, SimilarMultiHist->BinSize)
	{ }

void MultiHistClass::AllocateMemory()
	{
	Hist = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Hist[NHist] = (double*) calloc(MaxNBins,sizeof(double));
	}

MultiHistClass::~MultiHistClass()
	{
	if (Hist==NULL) return;
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Hist[NHist]);
	free(Hist);
	}

void MultiHistClass::ClearAll()
	{
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<MaxNBins; NBin++)
			Hist[NHist][NBin]=0.;
	}

int MultiHistClass::CheckNHist(int InNHist)
	{
	if (InNHist<0 || InNHist>MaxNHists) return MaxNHists;
	return InNHist;
	}

void MultiHistClass::SetName(int InNHist, char* InName)
	{
	strcpy(Name[CheckNHist(InNHist)],InName);
	}

void MultiHistClass::Clear(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]=0.;
	}


// Bin operations //////////////////////////////////////////////////////

void MultiHistClass::Add(int NHist, double InX, double InWeight)
	{
	Hist[CheckNHist(NHist)][GetBin(InX)]+=InWeight;
	}

void MultiHistClass::Set(int NHist, double InX, double InValue)
	{
	Hist[CheckNHist(NHist)][GetBin(InX)]=InValue;
	}

int MultiHistClass::GetBin(double InX)
	{
	int NBin=(int)((InX-StartValue)/BinSize+0.5);
	if (NBin < 0) NBin=0;
	if (NBin >= MaxNBins) NBin=MaxNBins-1;
	return NBin;
	}

double MultiHistClass::XAt(int InNBin)
	{ return (StartValue+BinSize*(double)InNBin); };

double MultiHistClass::XAtDouble(double InNBinDouble)
	{ return (StartValue+BinSize*InNBinDouble); };
	
double MultiHistClass::YAt(int NHist, int InNBin)
	{ return Hist[CheckNHist(NHist)][InNBin]; };

double MultiHistClass::Y(int NHist, double InX)
	{ return Hist[CheckNHist(NHist)][GetBin(InX)]; };


// Getting info ////////////////////////////////////////////////////////

double MultiHistClass::Integral(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	double IntegralPart=0.;
	for (int NBin=0; NBin<=MaxNBins; NBin++) IntegralPart+=YAt(NHist,NBin);
	return IntegralPart;
	}

double MultiHistClass::IntegralFromTo(int InNHist, double InFrom, double InTo)
	{
	int NHist=CheckNHist(InNHist);
	double IntegralPart=0.;
	for (int NBin=GetBin(InFrom); NBin<=GetBin(InTo); NBin++) IntegralPart+=YAt(NHist,NBin);
	return IntegralPart;
	}
	
double MultiHistClass::IntegralFrom(int InNHist, double InFrom)
	{
	int NHist=CheckNHist(InNHist);
	double IntegralPart=0.;
	for (int NBin=GetBin(InFrom); NBin<MaxNBins; NBin++) IntegralPart+=YAt(NHist,NBin);
	return IntegralPart;
	}

double MultiHistClass::IntegralTo(int InNHist, double InTo)
	{
	int NHist=CheckNHist(InNHist);
	double IntegralPart=0.;
	for (int NBin=0; NBin<GetBin(InTo); NBin++) IntegralPart+=YAt(NHist,NBin);
	return IntegralPart;
	}

double MultiHistClass::Mean(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	double Sum=0., TheIntegral=Integral(NHist);
	for (int NBin=0; NBin<MaxNBins; NBin++) Sum+=XAt(NBin)*YAt(NHist,NBin);
	if (TheIntegral==0) return 0;
	else return (Sum/TheIntegral);
	}

double MultiHistClass::PeakPosition(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	int PeakBin=0;
	for (int NBin=0; NBin<MaxNBins; NBin++) if (YAt(NHist,NBin)>YAt(NHist,PeakBin)) PeakBin=NBin;
	return XAt(PeakBin);
	}

double MultiHistClass::PeakPositionInRange(int InNHist, double XMin, double XMax)
	{
	int NHist=CheckNHist(InNHist);
	int PeakBin=0;
	for (int NBin=GetBin(XMin); NBin<GetBin(XMax+BinSize); NBin++) if (YAt(NHist,NBin)>YAt(NHist,PeakBin)) PeakBin=NBin;
	return XAt(PeakBin);
	}

double MultiHistClass::YAverage(int InNHist, bool IncludeZeros)
	{
	int NHist=CheckNHist(InNHist);
	double Sum=0, YAvg, NValuableBins=0;
	for (int NBin=0; NBin<MaxNBins; NBin++) Sum+=YAt(NHist,NBin);
	YAvg=Sum/(double)MaxNBins;
	if (IncludeZeros) return YAvg;
	Sum=0;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		if (fabs(YAt(NHist,NBin))>YAvg*1e-6) { Sum+=YAt(NHist,NBin); NValuableBins++; }
	if (NValuableBins==0) return 0;
	return (Sum/NValuableBins);
	}

double MultiHistClass::YMin(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	double Min=YAt(NHist,0);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		if (YAt(NHist,NBin)<Min) Min=YAt(NHist,NBin);
	return Min;
	}

double MultiHistClass::YMax(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	double Max=YAt(NHist,0);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		if (YAt(NHist,NBin)>Max) Max=YAt(NHist,NBin);
	return Max;
	}


bool MultiHistClass::IsSameBinningAsHist(HistogramClass* OtherHist)
	{
	if (	( StartValue	== OtherHist->StartValue ) &&
		( EndValue	== OtherHist->EndValue ) &&
		( BinSize	== OtherHist->BinSize ) &&
		( MaxNBins	== OtherHist->MaxNBins) 		) 
		return true;
	else return false;
	}

bool MultiHistClass::IsSameBinningAsHist(MultiHistClass* OtherMHist)
	{
	if (	( StartValue	== OtherMHist->StartValue ) &&
		( EndValue	== OtherMHist->EndValue ) &&
		( BinSize	== OtherMHist->BinSize ) &&
		( MaxNBins	== OtherMHist->MaxNBins) 		) 
		return true;
	else return false;
	}


bool MultiHistClass::IsSimilarAs(MultiHistClass* SomeMultiHist)
	{
	if (	( MaxNHists	== SomeMultiHist->MaxNHists ) &&
		( StartValue	== SomeMultiHist->StartValue ) &&
		( EndValue	== SomeMultiHist->EndValue ) &&
		( BinSize	== SomeMultiHist->BinSize ) &&
		( MaxNBins	== SomeMultiHist->MaxNBins) 	) 
		return true;
	else return false;
	}


// MultiHist operations ////////////////////////////////////////////////

void MultiHistClass::Fill(int InNHist, double FillValue)
	{
	int NHist=CheckNHist(InNHist);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]=FillValue;
	}

void MultiHistClass::NormIntegral(int InNHist, double NewIntegral)
	{
	int NHist=CheckNHist(InNHist);
	double NormFactor=NewIntegral/Integral(NHist);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]*=NormFactor;
	}

void MultiHistClass::CutEdges(int NHist)
	{
	Hist[CheckNHist(NHist)][0]=0.;
	Hist[CheckNHist(NHist)][MaxNBins-1]=0.;
	}

void MultiHistClass::Crop(int InNHist, double XMin, double XMax)
	{
	int NHist=CheckNHist(InNHist);
	for (int NBin=0; NBin<GetBin(XMin); NBin++)
		Hist[NHist][NBin]=0.;
	for (int NBin=GetBin(XMax); NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]=0.;
	}

void MultiHistClass::Cut(int InNHist, double XMin, double XMax)
	{
	int NHist=CheckNHist(InNHist);
	for (int NBin=GetBin(XMin); NBin<GetBin(XMax); NBin++)
		Hist[NHist][NBin]=0.;
	}

void MultiHistClass::Export(int InNHist, HistogramClass* SomeHist)
	{
	int NHist=CheckNHist(InNHist);
	strcpy(SomeHist->Name,Name[NHist]);
	SomeHist->StartValue	= StartValue;
	SomeHist->EndValue	= EndValue;
	SomeHist->BinSize	= BinSize;
	SomeHist->MaxNBins	= MaxNBins;
	SomeHist->AllocateMemory();
	SomeHist->Clear();
	for (int NBin=0; NBin<MaxNBins; NBin++)
		SomeHist->Hist[NBin] = Hist[NHist][NBin];
	}

bool MultiHistClass::Import(int InNHist, HistogramClass* SomeHist)
	{
	int NHist=CheckNHist(InNHist);
	sprintf(Name[NHist],"%s",SomeHist->Name);
	if (!(IsSameBinningAsHist(SomeHist))) return false;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin] = SomeHist->Hist[NBin];
	return true;
	}

bool MultiHistClass::Import(int InNHist, MultiHistClass* OtherMultiHist, int InOtherHistIdToImport, bool CopyName)
	{
	int NHist=CheckNHist(InNHist);
	int NOtherHist=OtherMultiHist->CheckNHist(InOtherHistIdToImport);
	if (!(IsSameBinningAsHist(OtherMultiHist))) return false;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin] = OtherMultiHist->Hist[NOtherHist][NBin];
	if (CopyName) sprintf(Name[NHist],"%s",OtherMultiHist->Name[NOtherHist]);
	return true;
	}

void MultiHistClass::AddHist(int InNHist, HistogramClass* SomeHist)
	{
	int NHist=CheckNHist(InNHist);
	for (int NBin=0; NBin<SomeHist->MaxNBins; NBin++)
		Add(NHist,SomeHist->XAt(NBin),SomeHist->YAt(NBin));
	}

void MultiHistClass::AddHist(int InNHist, MultiHistClass* SomeMultiHist, int InSomeNHist)
	{
	int NHist=CheckNHist(InNHist);
	int SomeNHist=SomeMultiHist->CheckNHist(InSomeNHist);
	for (int NBin=0; NBin<SomeMultiHist->MaxNBins; NBin++)
		Add(NHist,SomeMultiHist->XAt(NBin),SomeMultiHist->YAt(SomeNHist,NBin));
	}

void MultiHistClass::MultiplyWithNumber(int InNHist, double Multiplicator)
	{
	int NBin, NHist=CheckNHist(InNHist);
	for (NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]*=Multiplicator;
	}

void MultiHistClass::CopyFromTo(int InNHistFrom, int InNHistTo)
	{
	int NHistTo=CheckNHist(InNHistTo);
	int NHistFrom=CheckNHist(InNHistFrom);
	if (NHistTo==NHistFrom) return;
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHistTo][NBin]=Hist[NHistFrom][NBin];
	}

void  MultiHistClass::ModifyHist(int InNHist, double (*Function) (double))
	{
	int NBin, NHist=CheckNHist(InNHist);
	for (NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]=Function(Hist[NHist][NBin]);
	}

void  MultiHistClass::ModifyHistWith(int NHistToModify, int NHistToFunction, double (*Function) (double,double))
	{
	int NHist1=CheckNHist(NHistToModify), NHist2=CheckNHist(NHistToFunction);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		Hist[NHist1][NBin]=Function(Hist[NHist1][NBin],Hist[NHist2][NBin]);
	}

void MultiHistClass::PutSumOfThese(int InNHistResult, int NHistsToUse, int* HistNArray)
	{
	if (NHistsToUse<1) return;
	int NHistResult=CheckNHist(InNHistResult);
	int NUsedHists=NHistsToUse;
	int HistId[NUsedHists];
	for (int NUsed=0; NUsed<NUsedHists; NUsed++)
		HistId[NUsed]=CheckNHist(HistNArray[NUsed]);
	Clear(NHistResult);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		for (int NUsed=0; NUsed<NUsedHists; NUsed++)
			Hist[NHistResult][NBin]+=Hist[HistId[NUsed]][NBin];
	}

void MultiHistClass::PutProductOfThese(int InNHistResult, int NHistsToUse, int* HistNArray)
	{
	if (NHistsToUse<1) return;
	int NHistResult=CheckNHist(InNHistResult);
	int NUsedHists=NHistsToUse;
	int HistId[NUsedHists];
	for (int NUsed=0; NUsed<NUsedHists; NUsed++)
		HistId[NUsed]=CheckNHist(HistNArray[NUsed]);
	Fill(NHistResult,1.0);
	for (int NBin=0; NBin<MaxNBins; NBin++)
		for (int NUsed=0; NUsed<NUsedHists; NUsed++)
			Hist[NHistResult][NBin]*=Hist[HistId[NUsed]][NBin];
	}

void MultiHistClass::ExchangeToIntegral(int InNHist)
	{
	int NHist=CheckNHist(InNHist);
	for (int NBin=1; NBin<MaxNBins; NBin++)
		Hist[NHist][NBin]+=Hist[NHist][NBin-1];
	}


bool MultiHistClass::AddSimilarMultiHist(MultiHistClass* SomeMultiHist)
	{
	if (!IsSimilarAs(SomeMultiHist)) { printf("Warning: MultiHists are not similar! (operation not done)\n"); return false;}
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<MaxNBins; NBin++)
			Hist[NHist][NBin]+=SomeMultiHist->Hist[NHist][NBin];
	return true;
	}

bool MultiHistClass::AddSimilarMultiHistSelection(MultiHistClass* SomeMultiHist, int NHistsToAdd, int* pNHistToAdd)
	{
	if (!IsSimilarAs(SomeMultiHist)) { printf("Warning: MultiHists are not similar! (operation not done)\n"); return false;}
	for (int NHistToAdd=0; NHistToAdd<NHistsToAdd; NHistToAdd++) 
		{
		int NHist=CheckNHist(pNHistToAdd[NHistToAdd]);
		for (int NBin=0; NBin<MaxNBins; NBin++)
			Hist[NHist][NBin]+=SomeMultiHist->Hist[NHist][NBin];
		}
	return true;
	}

bool MultiHistClass::AddMultiHist(MultiHistClass* SomeMultiHist)
	{
	if (MaxNHists!=SomeMultiHist->MaxNHists) { printf("Warning: MultiHist addition requires same number of hists (operation not done)\n"); return false; }
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<SomeMultiHist->MaxNBins; NBin++)
			Add(NHist,SomeMultiHist->XAt(NBin),SomeMultiHist->YAt(NHist,NBin));
	return true;
	}

bool MultiHistClass::AddMultiHistSelection(MultiHistClass* SomeMultiHist, int NHistsToAdd, int* pNHistToAdd)
	{
	if (MaxNHists!=SomeMultiHist->MaxNHists) { printf("Warning: MultiHist addition requires same number of hists (operation not done)\n"); return false; }
	for (int NHistToAdd=0; NHistToAdd<NHistsToAdd; NHistToAdd++) 
		{
		int NHist=CheckNHist(pNHistToAdd[NHistToAdd]);
		for (int NBin=0; NBin<SomeMultiHist->MaxNBins; NBin++)
			Add(NHist,SomeMultiHist->XAt(NBin),SomeMultiHist->YAt(NHist,NBin));
		}
	return true;
	}



// File operations /////////////////////////////////////////////////////
void MultiHistClass::WriteParametersToScreen()
	{
	printf("MultiHist: %s\n",TheName);
	for (int NHist=0; NHist<MaxNHists; NHist++)
		{
		printf("\t%s - Integral: %g\n",Name[NHist],Integral(NHist));
		}
	}

void MultiHistClass::WriteToFile(FILE * Outfile)
	{
	int NHist, NBin; 
	fprintf(Outfile,"#%s NHists %d Start %g End %g BinSize %g NBins %d\nX ",TheName,MaxNHists,StartValue,EndValue,BinSize,MaxNBins);
	for (NHist=0; NHist<MaxNHists; NHist++) fprintf(Outfile,"\t%s",Name[NHist]);
	fprintf(Outfile,"\n"); 
	for (NBin=0; NBin<MaxNBins; NBin++) 
		{
		fprintf(Outfile,"%g",XAt(NBin));
		for (NHist=0; NHist<MaxNHists; NHist++) fprintf(Outfile,"\t%g",YAt(NHist,NBin)); 
		fprintf(Outfile,"\n"); 
		}
	fprintf(Outfile,"\n\n"); 
	}

void MultiHistClass::WriteToFileZs(FILE * Outfile)
	{
	int NHist, NBin, NBinMin=0, NBinMax=MaxNBins-1; 
	bool Cont;
	fprintf(Outfile,"#%s NHists %d Start %g End %g BinSize %g NBins %d\n#Hists ",TheName,MaxNHists,StartValue,EndValue,BinSize,MaxNBins);
	for (NHist=0; NHist<MaxNHists; NHist++) fprintf(Outfile,"\t%s",Name[NHist]);
	fprintf(Outfile,"\n"); 
	// Get Support
	Cont=true;
	while ( Cont && NBinMin<MaxNBins)
		{
		for (NHist=0; NHist<MaxNHists; NHist++) Cont&=(YAt(NHist,NBinMin)==0);
		NBinMin++;
		}
	if (NBinMin>0) NBinMin--;
	Cont=true;
	while ( Cont && NBinMax>0)
		{
		for (NHist=0; NHist<MaxNHists; NHist++) Cont&=(YAt(NHist,NBinMax)==0);
		NBinMax--;
		}
	if (NBinMax<MaxNBins-1) NBinMax++;
	if (NBinMin>NBinMax) { NBinMin=0; NBinMax=1; }
	//printf("SuppMin: %d , SuppMax : %d \n",NBinMin,NBinMax);
	for (NBin=NBinMin; NBin<NBinMax+1; NBin++) 
		{
		fprintf(Outfile,"%g",XAt(NBin));
		for (NHist=0; NHist<MaxNHists; NHist++) fprintf(Outfile,"\t%g",YAt(NHist,NBin)); 
		fprintf(Outfile,"\n"); 
		}
	fprintf(Outfile,"\n\n"); 
	}

void MultiHistClass::WriteToNewFile(char* NewFilename)
	{
	FILE * NewFile;
	if ( (NewFile=fopen(NewFilename,"w")) == NULL ) 
		{printf("\nERROR : file: %s could not be opened\n\n",NewFilename); return;}
	WriteToFile(NewFile);
	fclose(NewFile);
	}

bool MultiHistClass::ReadFromFile(FILE* InFile)
	{
	if (InFile==NULL) { printf("Warning: Non valid file has been called\n"); return false; }
	int	NConversions=-1; char TempName[50];
	NConversions=fscanf(InFile,"%s %*s %d %*s %lf %*s %lf %*s %lf %*s %d %*s",TempName,&MaxNHists,&StartValue,&EndValue,&BinSize,&MaxNBins);
	sprintf(TheName,"%s",&(TempName[1]));
	if (NConversions!=6) { printf("%s: MultiHist read error\n",TheName); return false; }
	int CalcMaxNBins = (int)((EndValue-StartValue)/BinSize+0.1)+1;
	if (CalcMaxNBins!=MaxNBins) { printf("%s: Inconsystency in MaxNBins\n",TheName); return false; }
	// Reallocate memory
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Hist[NHist]);
	free(Hist);
	AllocateMemory();
	ClearAll();
	// Read data
	for (int NHist=0; NHist<MaxNHists; NHist++) fscanf(InFile,"%s",Name[NHist]);
	SetName(MaxNHists,(char*)"Overflow");
	double	InX, DiffMax=0.001*BinSize;
	for (int NBin=0; NBin<MaxNBins; NBin++) 
		{
		fscanf(InFile,"%lf",&InX);
		if (fabs(InX-XAt(NBin))>DiffMax) { printf("%s: Inconsystency: XAt(%d)=%lf != %lf\n",TheName,NBin,XAt(NBin),InX); return false; }
		for (int NHist=0; NHist<MaxNHists; NHist++) fscanf(InFile,"%lf",&(Hist[NHist][NBin]));
		}
	return true;
	}

bool MultiHistClass::AddFromFile(FILE* InFile)
	{
	if (InFile==NULL) { printf("Warning: Non valid file has been called\n"); return false; }
	int	NConversions=-1;
	int	ReadMaxNHists,ReadMaxNBins;
	double	ReadStartValue,ReadEndValue,ReadBinSize;
	NConversions=fscanf(InFile,"%*s %*s %d %*s %lf %*s %lf %*s %lf %*s %d %*s",&ReadMaxNHists,&ReadStartValue,&ReadEndValue,&ReadBinSize,&ReadMaxNBins);
	if (NConversions!=5) { printf("MultiHist read error\n"); return false; }
	int CalcMaxNBins = (int)((ReadEndValue-ReadStartValue)/ReadBinSize+0.1)+1;
	if (CalcMaxNBins!=ReadMaxNBins) { printf("Inconsystency in MaxNBins\n"); return false; }
	// Check consistency
	if ( (ReadMaxNHists!=MaxNHists) || (ReadMaxNBins!=MaxNBins) || (fabs(ReadStartValue-StartValue)>0.1*BinSize) || (fabs(ReadBinSize-BinSize)>0.1*BinSize) || (fabs(ReadEndValue-EndValue)>0.1*BinSize) )
		{ printf("Warning! Read MultiMap does not match with %s, will be skipped\n",TheName); return false; }
	// Read and Add data
	for (int NHist=0; NHist<MaxNHists; NHist++) fscanf(InFile,"%*s");
	double	ReadX, ReadY;
	for (int NBin=0; NBin<MaxNBins; NBin++) 
		{
		fscanf(InFile,"%lf",&ReadX);
		for (int NHist=0; NHist<MaxNHists; NHist++) { fscanf(InFile,"%lf",&ReadY); Hist[NHist][NBin]+=ReadY; }
		}
	return true;
	}



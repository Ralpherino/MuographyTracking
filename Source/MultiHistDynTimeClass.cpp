////////////////////////////////////////////////////////////////////////
// Multiple Histogram Dynamic Time Class - implementation
//	version : 2019-03-19
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiHistDynTimeClass.h"

MultiHistDynTimeClass::MultiHistDynTimeClass(int InMaxNHists, double InBinSize, char* InName)
	{
	MaxNHists=InMaxNHists;
	BinSize=InBinSize;
	HistType=true;
	sprintf(TheName,"%s",InName);
	MaxNBins=0;
	Hist = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		Hist[NHist] = (double*) calloc(MaxNBins,sizeof(double)); // zero
	char	NameString[100];
	for (int NHist=0; NHist<MaxNHists; NHist++)
		{ sprintf(NameString,"Hist%d",NHist); SetName(NHist,NameString); }
	SetName(MaxNHists,(char*)"Overflow");
	//ThisDateTime = new struct tm;
	//SetTimeZero((char*)"2018-01-01_00:00:00");
	LastX=-1; TimeSec=-1;
	AutoScale=false;
	}

MultiHistDynTimeClass::MultiHistDynTimeClass(int InMaxNHists, MultiHistDynTimeClass* SimilarMhdt)
	{
	MaxNHists=InMaxNHists;
	StartValue=SimilarMhdt->StartValue;
	EndValue=SimilarMhdt->EndValue;
	BinSize=SimilarMhdt->BinSize;
	MaxNBins=SimilarMhdt->MaxNBins;
	sprintf(TheName,"%s",SimilarMhdt->TheName);
	HistType=true;
	Hist = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		Hist[NHist] = (double*) calloc(MaxNBins,sizeof(double));
	char	NameString[100];
	for (int NHist=0; NHist<MaxNHists; NHist++)
		{ sprintf(NameString,"Hist%d",NHist); SetName(NHist,NameString); }
	SetName(MaxNHists,(char*)"Overflow");
	TimeZero=SimilarMhdt->TimeZero;
	LastX=SimilarMhdt->LastX;
	TimeSec=SimilarMhdt->TimeSec;
	AutoScale=SimilarMhdt->AutoScale;
	AutoScaleLimitNBins=SimilarMhdt->AutoScaleLimitNBins;
	}

MultiHistDynTimeClass::~MultiHistDynTimeClass()
	{
	// The destructor of the MultiHistClass is enough
	}

// General

void MultiHistDynTimeClass::SetTimeZero(char* InDateTimeString, bool Exact)
	{
	ThisDateTime = (tm*) calloc(1,sizeof(tm));
	sscanf(InDateTimeString,"%4d-%2d-%2d_%2d:%2d:%2d",&DTYear,&DTMonth,&(ThisDateTime->tm_mday),
		&(ThisDateTime->tm_hour),&(ThisDateTime->tm_min),&(ThisDateTime->tm_sec));
	ThisDateTime->tm_year=DTYear-1900; ThisDateTime->tm_mon=DTMonth-1; ThisDateTime->tm_isdst=-1;
	if (!Exact) ThisDateTime->tm_sec+=BinSize/2;
	TimeZero=mktime(ThisDateTime);
	free(ThisDateTime);
	}

void MultiHistDynTimeClass::AllocateMemory(int NewBin)
	{
	//printf("AllocateMemory : NewBin=%d\n",NewBin);
	// Calculate parameters
	int NewMaxNBins, BinShift;
	if (NewBin>MaxNBins-1)	{ NewMaxNBins=NewBin+1; BinShift=0; }
	else if (NewBin<0)	{ NewMaxNBins=MaxNBins-NewBin; BinShift=-NewBin; }
	else 			{ printf("Non valid extension: %d (Max:%d)\nExit!\n",NewBin,MaxNBins); exit(2); }
	// Clone the present Hists
	double	**Temp;
	Temp = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Temp[NHist] = (double*) calloc(MaxNBins,sizeof(double));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<MaxNBins; NBin++)
			Temp[NHist][NBin] = Hist[NHist][NBin];
	// Free the memory
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Hist[NHist]);
	// Reallocate memory
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Hist[NHist] = (double*) calloc(NewMaxNBins,sizeof(double));
	// Copy old data to new place
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<MaxNBins; NBin++)
			Hist[NHist][NBin+BinShift] = Temp[NHist][NBin];
	// Redefine parameters
	StartValue-=BinShift*BinSize;
	MaxNBins=NewMaxNBins;
	EndValue=StartValue+(MaxNBins-1)*BinSize;
	// Free Temp
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Temp[NHist]);
	free(Temp);
	}

void MultiHistDynTimeClass::ClearAll()
	{
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Hist[NHist]);
	MaxNBins=0; BinSize=1;
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		Hist[NHist] = (double*) calloc(MaxNBins,sizeof(double)); // zero
	LastX=-1; TimeSec=-1;
	}

char* MultiHistDynTimeClass::PrintDt(time_t * DtTimet)
	{
	ThisDateTime = localtime(DtTimet);
	mktime(ThisDateTime);
	sprintf(DateTimeStringTemp,"%04d-%02d-%02d_%02d:%02d:%02d",ThisDateTime->tm_year+1900,ThisDateTime->tm_mon+1,ThisDateTime->tm_mday,ThisDateTime->tm_hour,ThisDateTime->tm_min,ThisDateTime->tm_sec);
	return DateTimeStringTemp;
	}

// Bin level operations

int MultiHistDynTimeClass::GetBinDyn(double InX)
	{
	int NBin=(int)floor((InX-StartValue)/BinSize+0.5);
	return NBin;
	}

char* MultiHistDynTimeClass::TAt(int NBin)
	{
	ThisDateTime = localtime(&TimeZero);
	ThisDateTime->tm_sec+=XAt(NBin);
	mktime(ThisDateTime);
	sprintf(TAtString,"%04d-%02d-%02d_%02d:%02d:%02d",ThisDateTime->tm_year+1900,ThisDateTime->tm_mon+1,ThisDateTime->tm_mday,ThisDateTime->tm_hour,ThisDateTime->tm_min,ThisDateTime->tm_sec);
	return TAtString;
	}

void MultiHistDynTimeClass::Add(int InNHist, char* InDateTimeString, double InWeight)
	{
	if (MaxNBins==0) SetTimeZero(InDateTimeString);
	while (AutoScale && MaxNBins>AutoScaleLimitNBins) ScaleBinSize();
	ThisDateTime = localtime(&TimeZero);
	sscanf(InDateTimeString,"%4d-%2d-%2d_%2d:%2d:%2d",&DTYear,&DTMonth,&(ThisDateTime->tm_mday),
		&(ThisDateTime->tm_hour),&(ThisDateTime->tm_min),&(ThisDateTime->tm_sec));
	ThisDateTime->tm_year=DTYear-1900; ThisDateTime->tm_mon=DTMonth-1;
	// Calculate Diff in Seconds
	TimeSec=difftime(mktime(ThisDateTime),TimeZero);
	AddX(InNHist,TimeSec,InWeight);
	}

void MultiHistDynTimeClass::Set(int InNHist, char* InDateTimeString, double InValue)
	{
	if (MaxNBins==0) SetTimeZero(InDateTimeString);
	if (AutoScale && MaxNBins>AutoScaleLimitNBins) ScaleBinSize();
	ThisDateTime = localtime(&TimeZero);
	sscanf(InDateTimeString,"%4d-%2d-%2d_%2d:%2d:%2d",&DTYear,&DTMonth,&(ThisDateTime->tm_mday),
		&(ThisDateTime->tm_hour),&(ThisDateTime->tm_min),&(ThisDateTime->tm_sec));
	ThisDateTime->tm_year=DTYear-1900; ThisDateTime->tm_mon=DTMonth-1;
	// Calculate Diff in Seconds
	TimeSec=difftime(mktime(ThisDateTime),TimeZero);
	SetX(InNHist,TimeSec,InValue);
	}

void MultiHistDynTimeClass::AddX(int NHist, double InX, double InWeight)
	{
	if (MaxNBins==0) { StartValue = double(int(InX/BinSize))*BinSize; AllocateMemory(0); }
	if (GetBinDyn(InX)<0 || GetBinDyn(InX)>MaxNBins-1) 
		AllocateMemory(GetBinDyn(InX));
	Hist[CheckNHist(NHist)][GetBin(InX)]+=InWeight;
	LastX=InX;
	}

void MultiHistDynTimeClass::SetX(int NHist, double InX, double InValue)
	{
	//printf("Add : Hist=%d, X=%g\t",NHist,InX);
	if (MaxNBins==0) { StartValue = double(int(InX/BinSize))*BinSize; AllocateMemory(0); }
	if (GetBinDyn(InX)<0 || GetBinDyn(InX)>MaxNBins-1) 
		AllocateMemory(GetBinDyn(InX));
	Hist[CheckNHist(NHist)][GetBin(InX)]=InValue;
	//printf("Bin: %d\n",GetBin(InX));
	}

// Hist level operations

void MultiHistDynTimeClass::ScaleBinSize(int Scale)
	{
	// Check parameters
	if (Scale<1) return;
	// Clone the present Hists
	double	**Temp;
	Temp = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Temp[NHist] = (double*) calloc(MaxNBins,sizeof(double));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<MaxNBins; NBin++)
			Temp[NHist][NBin] = Hist[NHist][NBin];
	// Free the memory
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Hist[NHist]);
	// Reallocate memory
	int NewMaxNBins=ceil(double(MaxNBins)/double(Scale));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Hist[NHist] = (double*) calloc(NewMaxNBins,sizeof(double));
	// Copy old data to new place
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		for (int NBin=0; NBin<MaxNBins; NBin++)
			if (HistType) Hist[NHist][NBin/Scale] += Temp[NHist][NBin];
			else Hist[NHist][NBin/Scale] = Temp[NHist][NBin];
	// Free Temp
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Temp[NHist]);
	free(Temp);
	// Redefine parameters
	StartValue+=(double(Scale)-1.0)*BinSize/2.0;
	BinSize*=(double)Scale;
	MaxNBins=NewMaxNBins;
	EndValue=StartValue+(MaxNBins-1)*BinSize;
	}

void MultiHistDynTimeClass::SetAutoScale(int InAutoScaleLimitNBins)
	{
	if (InAutoScaleLimitNBins<1) { AutoScale=false; return; }
	AutoScaleLimitNBins=InAutoScaleLimitNBins;
	AutoScale=true;
	}

void MultiHistDynTimeClass::ConvertToRate(int InToRate)
	{
	RateFromTo(InToRate,InToRate);
	}

void MultiHistDynTimeClass::RateFromTo(int InFrom, int InTo)
	{
	int IdRate=CheckNHist(InTo);
	CopyFromTo(InFrom,IdRate);
	MultiplyWithNumber(IdRate,1.0/BinSize);
	// The last bin contains less data, thus needs special care
	Hist[IdRate][MaxNBins-1]/= (LastX-XAt(MaxNBins-1)+BinSize/2.0) / BinSize;
	}

void MultiHistDynTimeClass::AddTimeHist(int InNHist, MultiHistDynTimeClass* OtherMultiTimeHist, int InOtherNHist)
	{
	int NHist=CheckNHist(InNHist);
	int OtherNHist=OtherMultiTimeHist->CheckNHist(InOtherNHist);
	for (int NBin=0; NBin<OtherMultiTimeHist->MaxNBins; NBin++)
		Add(NHist,OtherMultiTimeHist->TAt(NBin),OtherMultiTimeHist->YAt(OtherNHist,NBin));
	}

bool MultiHistDynTimeClass::AddMultiTimeHist(MultiHistDynTimeClass* OtherMultiTimeHist)
	{
	if (MaxNHists!=OtherMultiTimeHist->MaxNHists) { printf("Warning: MultiTimeHists do not have same NHists! (operation not done)\n"); return false; }
	for (int NBin=0; NBin<OtherMultiTimeHist->MaxNBins; NBin++)
		for (int NHist=0; NHist<MaxNHists; NHist++)
			Add(NHist,OtherMultiTimeHist->TAt(NBin),OtherMultiTimeHist->YAt(NHist,NBin));
	return true;
	}


// File operations

void MultiHistDynTimeClass::WriteToFile(FILE* OutFile)
	{
	int NHist, NBin;
	ThisDateTime = localtime(&TimeZero);
	mktime(ThisDateTime);
	fprintf(OutFile,"#%s NHists %d Start %g End %.10g BinSize %g NBins %d",TheName,MaxNHists,StartValue,EndValue,BinSize,MaxNBins);
	fprintf(OutFile," TimeZero %04d-%02d-%02d_%02d:%02d:%02d",ThisDateTime->tm_year+1900,ThisDateTime->tm_mon+1,ThisDateTime->tm_mday,ThisDateTime->tm_hour,ThisDateTime->tm_min,ThisDateTime->tm_sec);
	fprintf(OutFile,"\n#Hists");
	for (NHist=0; NHist<MaxNHists; NHist++) fprintf(OutFile,"\t%s",Name[NHist]);
	fprintf(OutFile,"\n"); 
	for (NBin=0; NBin<MaxNBins; NBin++) 
		{
		fprintf(OutFile,"%g\t%s",XAt(NBin),TAt(NBin));
		for (NHist=0; NHist<MaxNHists; NHist++) fprintf(OutFile,"\t%g",YAt(NHist,NBin)); 
		fprintf(OutFile,"\n"); 
		}
	fprintf(OutFile,"\n\n"); 
	}

bool MultiHistDynTimeClass::ReadFromFile(FILE* InFile)
	{
	if (InFile==NULL) { printf("Warning: Non valid file has been called\n"); return false; }
	int	NConversions=-1;
	char TempName[50],DtString[50];
	// Free the memory
	for (int NHist=0; NHist<MaxNHists+1; NHist++) free(Hist[NHist]);
	free(Hist);
	// Read header
	NConversions=fscanf(InFile,"%s %*s %d %*s %lf %*s %lf %*s %lf %*s %d %*s %s %*s",TempName,&MaxNHists,&StartValue,&EndValue,&BinSize,&MaxNBins,DtString);
	// Allocate memory
	Hist = (double**) calloc(MaxNHists+1,sizeof(double*));
	for (int NHist=0; NHist<MaxNHists+1; NHist++)
		 Hist[NHist] = (double*) calloc(MaxNBins,sizeof(double));
	sprintf(TheName,"%s",&(TempName[1]));
	SetTimeZero(DtString,true);
	// Cross-check of values
	if (NConversions!=7) { printf("MultiHistDynTime read error (NConv:%d,Name:%s,DT:%s)\n",NConversions,TempName,DtString); return false; }
	int CalcMaxNBins = (int)((EndValue-StartValue)/BinSize)+1;
	if (abs(CalcMaxNBins-MaxNBins)>1) { printf("Inconsystency in MaxNBins (Calc:%d != Data:%d)\n",CalcMaxNBins,MaxNBins); return false; }
	// ClearAll(); maybe something similar is needed ?
	for (int NHist=0; NHist<MaxNHists; NHist++) fscanf(InFile,"%s",Name[NHist]);
	double	InX,InWeight;
	for (int NBin=0; NBin<MaxNBins; NBin++) 
		{
		fscanf(InFile,"%lf %*s",&InX);
		for (int NHist=0; NHist<MaxNHists; NHist++) 
			{ fscanf(InFile,"%lf",&InWeight); AddX(NHist,InX,InWeight); }
		}
	return true;
	}




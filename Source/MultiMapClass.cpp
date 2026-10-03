////////////////////////////////////////////////////////////////////////
// Multiple Maps Class - implemetation
//	version : 2021-02-18
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiMapClass.h"

MultiMapClass::MultiMapClass(int InMaxNMaps, double InXStartValue, double InXEndValue, double InXBinSize, double InYStartValue, double InYEndValue, double InYBinSize, char* InName)
	{
	MaxNMaps=InMaxNMaps;
	XStartValue=InXStartValue;
	XEndValue=InXEndValue;
	XBinSize=InXBinSize;
	if (XStartValue>XEndValue) { XStartValue=InXEndValue; XEndValue=InXStartValue; }
	MaxNXBins=(int)((XEndValue-XStartValue)/XBinSize+0.1)+1;
	YStartValue=InYStartValue;
	YEndValue=InYEndValue;
	YBinSize=InYBinSize;
	if (YStartValue>YEndValue) { YStartValue=InYEndValue; YEndValue=InYStartValue; }
	MaxNYBins=(int)((YEndValue-YStartValue)/YBinSize+0.1)+1;
	// Initialization
	AllocateMemory();
	ClearAll();
	sprintf(TheName,"%s",InName);
	char	NameString[100];
	for (int NMap=0; NMap<MaxNMaps; NMap++)
		{ sprintf(NameString,"%s%d",TheName,NMap); SetName(NMap,NameString); }
	SetName(MaxNMaps,(char*)"Overflow");
	}

MultiMapClass::MultiMapClass(int InMaxNMaps, MultiMapClass * SimilarMap, char* InName)
	: MultiMapClass(InMaxNMaps,SimilarMap->XStartValue,SimilarMap->XEndValue,SimilarMap->XBinSize,SimilarMap->YStartValue,SimilarMap->YEndValue,SimilarMap->YBinSize,InName)
	{ }

MultiMapClass::~MultiMapClass()
	{
	FreeMemory();
	}


// Basic functions

void MultiMapClass::AllocateMemory()
	{
	int	NMap, NXBin;
	Map = (double***) calloc(MaxNMaps+1,sizeof(double**));
	for (NMap=0; NMap<MaxNMaps+1; NMap++)
		 Map[NMap] = (double**) calloc(MaxNXBins,sizeof(double*));
	for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++)
		 Map[NMap][NXBin] = (double*) calloc (MaxNYBins,sizeof(double)); 
	
	}

void MultiMapClass::FreeMemory()
	{
	int NMap,NXBin;
	for (NMap=0; NMap<MaxNMaps+1; NMap++) 
		for (NXBin=0; NXBin<MaxNXBins; NXBin++) 
			free(Map[NMap][NXBin]);
	for (NMap=0; NMap<MaxNMaps+1; NMap++) free(Map[NMap]);
	free(Map);
	}


void MultiMapClass::ClearAll()
	{
	int	NMap, NXBin, NYBin;
	for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=0.;
	}

void MultiMapClass::Clear(int InNMap)
	{
	int NMap=CheckNMap(InNMap);
	int NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=0.;
	}

int MultiMapClass::CheckNMap(int InNMap)
	{
	if (InNMap<0 || InNMap>MaxNMaps) return MaxNMaps;
	return InNMap;
	}

void MultiMapClass::SetTheName(char* InTheName)
	{
	sprintf(TheName,"%s",InTheName);
	}

void MultiMapClass::SetName(int InNMap, char* InName)
	{
	sprintf(Name[CheckNMap(InNMap)],"%s",InName);
	}

void MultiMapClass::ResetToSize(double InXStartValue, double InXEndValue, double InYStartValue, double InYEndValue)
	{
	FreeMemory();
	XStartValue=InXStartValue;
	XEndValue=InXEndValue;
	MaxNXBins=(int)((XEndValue-XStartValue)/XBinSize+0.1)+1;
	YStartValue=InYStartValue;
	YEndValue=InYEndValue;
	MaxNYBins=(int)((YEndValue-YStartValue)/YBinSize+0.1)+1;
	AllocateMemory();
	ClearAll();
	}


// Bin operations

int MultiMapClass::GetXBin(double InX)
	{
	int NXBin=(int)floor((InX-XStartValue)/XBinSize+0.5);
	if (NXBin < 0) NXBin=0;
	if (NXBin >= MaxNXBins) NXBin=MaxNXBins-1;
	return NXBin; 
	}

int MultiMapClass::GetYBin(double InY)
	{
	int NYBin=(int)floor((InY-YStartValue)/YBinSize+0.5);
	if (NYBin < 0) NYBin=0;
	if (NYBin >= MaxNYBins) NYBin=MaxNYBins-1;
	return NYBin;
	}

void MultiMapClass::Add(int InNMap, double InX, double InY, double InWeight)
	{
	Map[CheckNMap(InNMap)][GetXBin(InX)][GetYBin(InY)]+=InWeight;
	}

void MultiMapClass::Set(int InNMap, double InX, double InY, double InValue)
	{
	Map[CheckNMap(InNMap)][GetXBin(InX)][GetYBin(InY)]=InValue;
	}

void MultiMapClass::SetAt(int InNMap, int InNXBin, int InNYBin, double InValue)
	{
	Map[CheckNMap(InNMap)][InNXBin][InNYBin]=InValue;
	}

void MultiMapClass::SetLineX(int InNMap, double InX, double InValue)
	{
	int NMap=CheckNMap(InNMap), NXBin=GetXBin(InX);
	for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=InValue;
	}

void MultiMapClass::SetLineY(int InNMap, double InY, double InValue)
	{
	int NMap=CheckNMap(InNMap), NYBin=GetYBin(InY);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++)
		Map[NMap][NXBin][NYBin]=InValue;
	}

void MultiMapClass::SetLineXWidth(int InNMap, double InX, double InValue, double InWidth)
	{
	int NMap=CheckNMap(InNMap);
	int NXBinMin=GetXBin(InX-InWidth*0.5+XBinSize*0.01), NXBinMax=GetXBin(InX+InWidth*0.5-XBinSize*0.01);
	for (int NXBin=NXBinMin; NXBin<=NXBinMax; NXBin++)
		for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
			Map[NMap][NXBin][NYBin]=InValue;
	}

void MultiMapClass::SetLineYWidth(int InNMap, double InY, double InValue, double InWidth)
	{
	int NMap=CheckNMap(InNMap);
	int NYBinMin=GetYBin(InY-InWidth*0.5+YBinSize*0.01), NYBinMax=GetYBin(InY+InWidth*0.5-YBinSize*0.01);
	for (int NYBin=NYBinMin; NYBin<=NYBinMax; NYBin++)
		for (int NXBin=0; NXBin<MaxNXBins; NXBin++)
			Map[NMap][NXBin][NYBin]=InValue;
	}

double MultiMapClass::XAt(int InNXBin)
	{ return (XStartValue+XBinSize*(double)InNXBin); };
double MultiMapClass::XMinAt(int InNXBin)
	{ return (XStartValue+XBinSize*(double)InNXBin-0.5*XBinSize); };
double MultiMapClass::XMaxAt(int InNXBin)
	{ return (XStartValue+XBinSize*(double)InNXBin+0.5*XBinSize); };

double MultiMapClass::YAt(int InNYBin)
	{ return (YStartValue+YBinSize*(double)InNYBin); };
double MultiMapClass::YMinAt(int InNYBin)
	{ return (YStartValue+YBinSize*(double)InNYBin-0.5*YBinSize); };
double MultiMapClass::YMaxAt(int InNYBin)
	{ return (YStartValue+YBinSize*(double)InNYBin+0.5*YBinSize); };

double MultiMapClass::ZAt(int InNMap, int InNXBin, int InNYBin)
	{ return Map[InNMap][InNXBin][InNYBin]; };

double MultiMapClass::Z(int InNMap, double InX, double InY)
	{ return Map[CheckNMap(InNMap)][GetXBin(InX)][GetYBin(InY)]; }

double MultiMapClass::ZWeighted(int InNMap, double InX, double InY)
	{
	int	NMap=CheckNMap(InNMap);
	if ( (InX<=XAt(0)) || (InX>=XAt(MaxNXBins-1)) ) return 0.;
	if ( (InY<=YAt(0)) || (InY>=YAt(MaxNYBins-1)) ) return 0.;
	if ( (InX == XAt(GetXBin(InX))) && (InY == YAt(GetYBin(InY))) ) return Z(InNMap,InX,InY);
	double	NX=(InX-XStartValue)/XBinSize, NY=(InY-YStartValue)/YBinSize;
	int	NXBin=int(NX), NYBin=int(NY);
	double	WX0=double(NXBin+1)-NX, WX1=NX-double(NXBin), WY0=double(NYBin+1)-NY, WY1=NY-double(NYBin);
	return ( ZAt(NMap,NXBin,NYBin)*WX0*WY0+ZAt(NMap,NXBin+1,NYBin)*WX1*WY0+ZAt(NMap,NXBin,NYBin+1)*WX0*WY1+ZAt(NMap,NXBin+1,NYBin+1)*WX1*WY1 );
	}


// Get info

bool MultiMapClass::SameBinningAsHist2D(Histogram2DClass * AnyHist2D)
	{
	if ( (XStartValue==(AnyHist2D->XStartValue)) && (XEndValue==(AnyHist2D->XEndValue)) &&  (XBinSize==(AnyHist2D->XBinSize)) &&
	     (YStartValue==(AnyHist2D->YStartValue)) && (YEndValue==(AnyHist2D->YEndValue)) &&  (YBinSize==(AnyHist2D->YBinSize)) ) return true;
	else return false;
	}

bool MultiMapClass::SameBinningAsMultiMap(MultiMapClass * MMap)
	{
	if ( (XStartValue==(MMap->XStartValue)) && (XEndValue==(MMap->XEndValue)) &&  (XBinSize==(MMap->XBinSize)) &&
	     (YStartValue==(MMap->YStartValue)) && (YEndValue==(MMap->YEndValue)) &&  (YBinSize==(MMap->YBinSize)) ) return true;
	else return false;
	}

bool MultiMapClass::IsSimilarAs(MultiMapClass* SomeMultiMap)
	{
	if ( (MaxNMaps==SomeMultiMap->MaxNMaps) && SameBinningAsMultiMap(SomeMultiMap) ) return true;
	else return false;
	}

double MultiMapClass::GetMeanX(int InNMap)
	{
	double Sum=0., Integral=0.;
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{ Integral+=ZAt(NMap,NXBin,NYBin); Sum+=XAt(NXBin)*ZAt(NMap,NXBin,NYBin); }
	return (Sum/Integral);
	}

double MultiMapClass::GetMeanY(int InNMap)
	{
	double Sum=0., Integral=0.;
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{ Integral+=ZAt(NMap,NXBin,NYBin); Sum+=YAt(NYBin)*ZAt(NMap,NXBin,NYBin); }
	return (Sum/Integral);
	}

double MultiMapClass::GetZMin(int InNMap)
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	double	Min=ZAt(NMap,0,0);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (ZAt(NMap,NXBin,NYBin)<Min) Min=ZAt(NMap,NXBin,NYBin);
	return Min;
	}

double MultiMapClass::GetZMax(int InNMap)
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	double	Max=ZAt(NMap,0,0);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (ZAt(NMap,NXBin,NYBin)>Max) Max=ZAt(NMap,NXBin,NYBin);
	return Max;
	}

double MultiMapClass::Integral(int InNMap)
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	double Sum=0;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Sum+=Map[NMap][NXBin][NYBin];
	return Sum;
	}

double MultiMapClass::IntegralInRange(int InNMap, double XMin, double XMax, double YMin, double YMax)
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	double Sum=0;
	for (NXBin=GetXBin(XMin); NXBin<GetXBin(XMax); NXBin++) 
		for (NYBin=GetYBin(YMin); NYBin<GetYBin(YMax); NYBin++)
			Sum+=Map[NMap][NXBin][NYBin];
	return Sum;
	}

double MultiMapClass::AverageInRange(int InNMap, double XMin, double XMax, double YMin, double YMax)
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	double Sum=0, SumDiv=0;
	for (NXBin=GetXBin(XMin); NXBin<GetXBin(XMax); NXBin++) 
		for (NYBin=GetYBin(YMin); NYBin<GetYBin(YMax); NYBin++)
			{ Sum+=Map[NMap][NXBin][NYBin]; SumDiv++; }
	if (SumDiv==0) return 0;
	return Sum/SumDiv;
	}

double MultiMapClass::Peak(int InNMap, double* pPeakPosX, double* pPeakPosY)
	{
	int NMap=CheckNMap(InNMap);
	double PeakValue=ZAt(NMap,0,0);
	int PeakPosXBin=0,PeakPosYBin=0;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (ZAt(NMap,NXBin,NYBin)>PeakValue)
			{
			PeakPosXBin=NXBin;
			PeakPosYBin=NYBin;
			PeakValue=ZAt(NMap,NXBin,NYBin);
			}
	if (pPeakPosX!=NULL) (*pPeakPosX)=XAt(PeakPosXBin);
	if (pPeakPosY!=NULL) (*pPeakPosY)=YAt(PeakPosYBin);
	return PeakValue;
	}

double MultiMapClass::PeakPositionOnLineX(int InNMap, double InX)
	{
	int NXBin=GetXBin(InX),NYBin,NMap=CheckNMap(InNMap);
	int NYBinPeak=0;
	for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (ZAt(NMap,NXBin,NYBin)>ZAt(NMap,NXBin,NYBinPeak)) NYBinPeak=NYBin;
	return YAt(NYBinPeak); 
	}

double MultiMapClass::PeakPositionOnLineY(int InNMap, double InY)
	{
	int NXBin, NYBin=GetYBin(InY),NMap=CheckNMap(InNMap);
	int NXBinPeak=0;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++)
		if (ZAt(NMap,NXBin,NYBin)>ZAt(NMap,NXBinPeak,NYBin)) NXBinPeak=NXBin;
	return XAt(NXBinPeak); 
	}

bool MultiMapClass::GetSliceXIntegral(int InNMap, double YMin, double YMax, MultiHistClass* MHist, int InNHist)
	{
	if (MHist==NULL) return false;
	int	NHist=MHist->CheckNHist(InNHist), NMap=CheckNMap(InNMap);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) 
		for (int NYBin=GetYBin(YMin); NYBin<GetYBin(YMax); NYBin++)
			MHist->Add(NHist,XAt(NXBin),ZAt(NMap,NXBin,NYBin));
	return true;
	}

bool MultiMapClass::GetSliceYIntegral(int InNMap, double XMin, double XMax, MultiHistClass* MHist, int InNHist)
	{
	if (MHist==NULL) return false;
	int	NHist=MHist->CheckNHist(InNHist), NMap=CheckNMap(InNMap);
	for (int NYBin=0; NYBin<MaxNYBins; NYBin++) 
		for (int NXBin=GetXBin(XMin); NXBin<GetXBin(XMax); NXBin++)
			MHist->Add(NHist,XAt(NXBin),ZAt(NMap,NXBin,NYBin));
	return true;
	}


double MultiMapClass::AreaAbove(int InNMap, double Min, double* MidX, double* MidY)
	{
	int NMap=CheckNMap(InNMap);
	int MinX=MaxNXBins,MaxX=0,MinY=MaxNYBins,MaxY=0;
	int AreaCount=0;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (ZAt(NMap,NXBin,NYBin)>Min)
		{
		AreaCount++;
		if (NXBin<MinX) MinX=NXBin; if (NXBin>MaxX) MaxX=NXBin;
		if (NYBin<MinY) MinY=NYBin; if (NYBin>MaxY) MaxY=NYBin;
		}
	//printf("XY: %d-%d , %d-%d\n",MinX,MaxX,MinY,MaxY);
	if (MidX!=NULL) *MidX=(XAt(MaxX)+XAt(MinX))/2.0;
	if (MidY!=NULL) *MidY=(YAt(MaxY)+YAt(MinY))/2.0;
	return (double(AreaCount)*XBinSize*YBinSize);
	}



// Map operations

void MultiMapClass::Fill(int InNMap, double FillValue)
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=FillValue;
	}

void MultiMapClass::FillFunction(int InNMap, double (*Function) (double,double))
	{
	int	NXBin, NYBin, NMap=CheckNMap(InNMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=Function(XAt(NXBin),YAt(NYBin));
	}

void MultiMapClass::ClearBorders(int InNMap)
	{
	if (InNMap==-1) 
		{ for (int NMap=0; NMap<MaxNMaps; NMap++) ClearBorders(NMap); return; }
	int NMap=CheckNMap(InNMap);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) { Map[NMap][NXBin][0]=0; Map[NMap][NXBin][MaxNYBins-1]=0; }
	for (int NYBin=0; NYBin<MaxNYBins; NYBin++) { Map[NMap][0][NYBin]=0; Map[NMap][MaxNXBins-1][NYBin]=0; }
	}

void MultiMapClass::Export(int InNMap, Histogram2DClass* OutHist2D)
	{
	int NMap=CheckNMap(InNMap);
	OutHist2D->FreeMemory();
	sprintf(OutHist2D->Name,"%s",Name[NMap]);
	OutHist2D->XStartValue	= XStartValue;
	OutHist2D->XEndValue	= XEndValue;
	OutHist2D->XBinSize	= XBinSize;
	OutHist2D->MaxNXBins	= MaxNXBins;
	OutHist2D->YStartValue	= YStartValue;
	OutHist2D->YEndValue	= YEndValue;
	OutHist2D->YBinSize	= YBinSize;
	OutHist2D->MaxNYBins	= MaxNYBins;
	OutHist2D->AllocateMemory();
	OutHist2D->Clear();
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		OutHist2D->Hist[NXBin][NYBin] = Map[NMap][NXBin][NYBin];
	}

bool MultiMapClass::Import(int InNMap, Histogram2DClass* InMap)
	{
	if (!(SameBinningAsHist2D(InMap))) return false;
	int NMap=CheckNMap(InNMap);
	sprintf(Name[NMap],"%s",InMap->Name);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin] = InMap->Hist[NXBin][NYBin];
	return true;
	}

bool MultiMapClass::Import(int InNMap, MultiMapClass* OtherMap, int InOtherMapIdToImport, bool InCopyName)
	{
	if (!(SameBinningAsMultiMap(OtherMap))) return false;
	int NMap=CheckNMap(InNMap);
	int OtherMapIdToImport=OtherMap->CheckNMap(InOtherMapIdToImport);
	if (InCopyName) sprintf(Name[NMap],"%s",OtherMap->Name[OtherMapIdToImport]);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin] = OtherMap->Map[OtherMapIdToImport][NXBin][NYBin];
	return true;
	
	}

void MultiMapClass::AddMap(int InNMap, Histogram2DClass* SomeMap)
	{
	int NMap=CheckNMap(InNMap);
	for (int NXBin=0; NXBin<SomeMap->MaxNXBins; NXBin++) for (int NYBin=0; NYBin<SomeMap->MaxNYBins; NYBin++)
		Add(NMap,SomeMap->XAt(NXBin),SomeMap->YAt(NYBin),SomeMap->ZAt(NXBin,NYBin));
	}

void MultiMapClass::AddMap(int InNMap, MultiMapClass* OtherMultiMap, int InOtherNMapToAdd)
	{
	int NMap=CheckNMap(InNMap);
	int OtherNMapToAdd=OtherMultiMap->CheckNMap(InOtherNMapToAdd);
	for (int NXBin=0; NXBin<OtherMultiMap->MaxNXBins; NXBin++) for (int NYBin=0; NYBin<OtherMultiMap->MaxNYBins; NYBin++)
		Add(NMap,OtherMultiMap->XAt(NXBin),OtherMultiMap->YAt(NYBin),OtherMultiMap->ZAt(OtherNMapToAdd,NXBin,NYBin));
	}

void MultiMapClass::AddVolumeShifted(int InNMap, MultiMapClass* OtherMultiMap, int InOtherNMapToAdd, double ShiftX, double ShiftY)
	{
	int NMap=CheckNMap(InNMap);
	int OtherNMapToAdd=OtherMultiMap->CheckNMap(InOtherNMapToAdd);
	int LocalNXMin,LocalNXMax,LocalNYMin,LocalNYMax;
	double OverlapX,OverlapY;
	
	for (int OtherNXBin=0; OtherNXBin<OtherMultiMap->MaxNXBins; OtherNXBin++) 
		{
		LocalNXMin=GetXBin(OtherMultiMap->XMinAt(OtherNXBin)+ShiftX);
		LocalNXMax=GetXBin(OtherMultiMap->XMaxAt(OtherNXBin)+ShiftX);
		for (int LocalNX=LocalNXMin; LocalNX<=LocalNXMax; LocalNX++)
			{
			if (LocalNXMin==LocalNXMax) OverlapX=OtherMultiMap->XBinSize;
			else if (LocalNX==LocalNXMin) OverlapX = (XMaxAt(LocalNX) - OtherMultiMap->XMinAt(OtherNXBin)-ShiftX);
			else if (LocalNX==LocalNXMax) OverlapX = (OtherMultiMap->XMaxAt(OtherNXBin)+ShiftX - XMinAt(LocalNX));
			else OverlapX = XBinSize;
			for (int OtherNYBin=0; OtherNYBin<OtherMultiMap->MaxNYBins; OtherNYBin++)
				{
				LocalNYMin=GetYBin(OtherMultiMap->YMinAt(OtherNYBin)+ShiftY);
				LocalNYMax=GetYBin(OtherMultiMap->YMaxAt(OtherNYBin)+ShiftY);
				for (int LocalNY=LocalNYMin; LocalNY<=LocalNYMax; LocalNY++)
					{
					if (LocalNYMin==LocalNYMax) OverlapY=OtherMultiMap->YBinSize;
					else if (LocalNY==LocalNYMin) OverlapY = (YMaxAt(LocalNY) - OtherMultiMap->YMinAt(OtherNYBin)-ShiftY);
					else if (LocalNY==LocalNYMax) OverlapY = (OtherMultiMap->YMaxAt(OtherNYBin)+ShiftY - YMinAt(LocalNY));
					else OverlapY = YBinSize;
					Map[NMap][LocalNX][LocalNY]+=OtherMultiMap->ZAt(OtherNMapToAdd,OtherNXBin,OtherNYBin)*OverlapX*OverlapY/XBinSize/YBinSize;
					}
				}
			}
		}
	
	}

void MultiMapClass::CopyFromTo(int InNMapFrom, int InNMapTo)
	{
	int NMapFrom=CheckNMap(InNMapFrom);
	int NMapTo=CheckNMap(InNMapTo);
	if (NMapTo==NMapFrom) return;
	int NXBin,NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMapTo][NXBin][NYBin]=Map[NMapFrom][NXBin][NYBin];
	}

void MultiMapClass::MultiplyMap(int InNMap, double InMultiplicator)
	{
	int NXBin, NYBin, NMap=CheckNMap(InNMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]*=InMultiplicator;
	}

void MultiMapClass::ModifyMap(int InNMap, double (*Function) (double))
	{
	int NXBin, NYBin, NMap=CheckNMap(InNMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=Function(Map[NMap][NXBin][NYBin]);
	}

void MultiMapClass::ModifyMapWith(int InNMap1, int InNMap2, double (*Function) (double,double))
	{
	int	NXBin, NYBin, NMap1=CheckNMap(InNMap1), NMap2=CheckNMap(InNMap2);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap1][NXBin][NYBin]=Function(Map[NMap1][NXBin][NYBin],Map[NMap2][NXBin][NYBin]);
	}

void MultiMapClass::DivideMapWithCutElse(int InNMap1, int InNMap2, double InCut, double InElse)
	{
	int NXBin, NYBin, NMap1=CheckNMap(InNMap1), NMap2=CheckNMap(InNMap2);
	if (InCut<=0) { printf("Warning : MultiMapClass::DivideMapWithCutElse : Cut=%g <= 0",InCut); return; }
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (Map[NMap2][NXBin][NYBin]>InCut) Map[NMap1][NXBin][NYBin]/=Map[NMap2][NXBin][NYBin];
		else Map[NMap1][NXBin][NYBin]=InElse;
	}

void MultiMapClass::DivideMapWithAvgCutElse(int InNMap1, int InNMap2, double AvgRadius, double InCut, double InElse, double InElseCut)
	{
	double ElseCut=InElseCut; if (ElseCut<0) ElseCut=InCut;
	int NXBin, NYBin, NMap1=CheckNMap(InNMap1), NMap2=CheckNMap(InNMap2), RMax=int(AvgRadius/XBinSize+0.5),AvgR;
	double DivMap[MaxNXBins][MaxNYBins], Sum1=0,Sum2=0;
	if (InCut<=0) { printf("Warning : MultiMapClass::DivideMapWithCutElse : Cut=%g <= 0",InCut); return; }
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		AvgR=0; Sum2=0;
		while (AvgR<=RMax && Sum2<=InCut)
			{
			Sum1=0; Sum2=0;
			for (int nx=-AvgR;nx<AvgR+1;nx++) for (int ny=-AvgR;ny<AvgR+1;ny++)
				{
				Sum1+=Map[NMap1][GetXBin(XAt(NXBin+nx))][GetYBin(YAt(NYBin+ny))];
				Sum2+=Map[NMap2][GetXBin(XAt(NXBin+nx))][GetYBin(YAt(NYBin+ny))];
				}
			AvgR++;
			}
		if (Sum2>ElseCut) DivMap[NXBin][NYBin]=Sum1/Sum2;
		else DivMap[NXBin][NYBin]=InElse;
		}
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap1][NXBin][NYBin]=DivMap[NXBin][NYBin];
	}

void MultiMapClass::PutSumOfThese(int InNMap, int NMapsToUse, int * MapIdArray)
	{
	if (NMapsToUse<1) return;
	int NMap=CheckNMap(InNMap), NXBin, NYBin;
	int NUsedMaps=NMapsToUse;
	int MapId[NUsedMaps];
	for (int NUsed=0; NUsed<NUsedMaps; NUsed++)
		MapId[NUsed]=CheckNMap(MapIdArray[NUsed]);
	Clear(NMap);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		for (int NUsed=0; NUsed<NUsedMaps; NUsed++)
			Map[NMap][NXBin][NYBin]+=Map[MapId[NUsed]][NXBin][NYBin];
	}

void MultiMapClass::PutProductOfThese(int InNMap, int NMapsToUse, int * MapIdArray)
	{
	if (NMapsToUse<1) return;
	int NMap=CheckNMap(InNMap), NXBin, NYBin;
	int NUsedMaps=NMapsToUse;
	int MapId[NUsedMaps];
	for (int NUsed=0; NUsed<NUsedMaps; NUsed++)
		MapId[NUsed]=CheckNMap(MapIdArray[NUsed]);
	Clear(NMap);
	Fill(NMap,1.0);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		for (int NUsed=0; NUsed<NUsedMaps; NUsed++)
			Map[NMap][NXBin][NYBin]*=Map[MapId[NUsed]][NXBin][NYBin];
	}

bool MultiMapClass::AddSimilarMultiMap(MultiMapClass* SomeMultiMap)
	{
	if (!IsSimilarAs(SomeMultiMap)) { printf("Warning: MultiMaps[%s] are not similar! (operation not done)\n",TheName); return false; }
	int NMap, NXBin, NYBin;
	for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]+=SomeMultiMap->Map[NMap][NXBin][NYBin];
	return true;
	}

bool MultiMapClass::AddSimilarMultiMapSelection(MultiMapClass* SomeMultiMap, int NMapsToAdd, int* pSelection)
	{
	if (!IsSimilarAs(SomeMultiMap)) { printf("Warning: MultiMaps are not similar! (operation not done)\n"); return false; }
	int NMap, NXBin, NYBin;
	for (int NMapToAdd=0; NMapToAdd<NMapsToAdd; NMapToAdd++) for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		NMap=CheckNMap(pSelection[NMapToAdd]);
		Map[NMap][NXBin][NYBin]+=SomeMultiMap->Map[NMap][NXBin][NYBin];
		}
	return true;
	}


bool MultiMapClass::FillGapsRangeCutTemp(int InNMap, int RMax, double Cut, int InNTempMap)
	{
	int NMap=CheckNMap(InNMap), TempMap=CheckNMap(InNTempMap);
	CopyFromTo(NMap,TempMap);
	int DirXM,DirXP,DirYM,DirYP;
	double	AvgDirX,AvgDirY,Avg;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (ZAt(NMap,NXBin,NYBin)<Cut)
		{
		DirXP=NXBin; DirXM=NXBin; DirYP=NYBin; DirYM=NYBin;
		while (DirXP<MaxNXBins-1 && (DirXP-NXBin)<RMax && ZAt(TempMap,DirXP,NYBin)<=Cut) DirXP++;
		while (DirXM>0 && (NXBin-DirXM)<RMax && ZAt(TempMap,DirXM,NYBin)<=Cut) DirXM--;
		while (DirYP<MaxNYBins-1 && (DirYP-NYBin)<RMax && ZAt(TempMap,NXBin,DirYP)<=Cut) DirYP++;
		while (DirYM>0 && (NYBin-DirYM)<RMax && ZAt(TempMap,NXBin,DirYM)<=Cut) DirYM--;
		if (ZAt(TempMap,DirXP,NYBin)>Cut && ZAt(TempMap,DirXM,NYBin)>Cut) AvgDirX=(ZAt(TempMap,DirXP,NYBin)+ZAt(TempMap,DirXM,NYBin))/2; else AvgDirX=Cut;
		if (ZAt(TempMap,NXBin,DirYP)>Cut && ZAt(TempMap,NXBin,DirYM)>Cut) AvgDirY=(ZAt(TempMap,NXBin,DirYP)+ZAt(TempMap,NXBin,DirYM))/2; else AvgDirY=Cut;
		// Set Average based in X and Y averages
		if (AvgDirX>Cut && AvgDirY<=Cut) Avg=AvgDirX;
		else if (AvgDirX<=Cut && AvgDirY>Cut) Avg=AvgDirY;
		else if (AvgDirX<=Cut && AvgDirY<=Cut) Avg=Cut;
		else if (AvgDirX>Cut && AvgDirY>Cut) 
			if (DirXP-DirXM > DirYP-DirYM+1) Avg=AvgDirX;
			else if (DirYP-DirYM > DirXP-DirXM+1) Avg=AvgDirY;
			else Avg=(AvgDirX+AvgDirY)/2;
		else Avg=Cut;
		if (Avg>Cut) SetAt(NMap,NXBin,NYBin,Avg);
		}
	Clear(TempMap);
	return true;
	}


// File operations

void MultiMapClass::WriteToFile(FILE * OutFile)
	{
	int	NMap, NXBin, NYBin;
	fprintf(OutFile,"#%s   NMaps %d   XStart %g XEnd %g XBinSize %g XNBins %d   YStart %g YEnd %g YBinSize %g YNBins %d\n",TheName,MaxNMaps,XStartValue,XEndValue,XBinSize,MaxNXBins,YStartValue,YEndValue,YBinSize,MaxNYBins);
	fprintf(OutFile,"#X\tY ");
	for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%s",Name[NMap]);
	fprintf(OutFile,"\n"); 
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) 
		{ 
		for (NYBin=0; NYBin<MaxNYBins; NYBin++)
			{
			fprintf(OutFile,"%g\t%g",XAt(NXBin),YAt(NYBin));
			for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",ZAt(NMap,NXBin,NYBin)); 
			fprintf(OutFile,"\n"); 
			}
		fprintf(OutFile,"\n"); 
		}
	fprintf(OutFile,"\n\n");
	}

void MultiMapClass::WriteToNewFile(char* NewFileName)
	{
	FILE * NewFile;
	if ( (NewFile=fopen(NewFileName,"w")) == NULL ) 
		{printf("\nERROR : File: %s could not be opened\n\n",NewFileName); return;}
	WriteToFile(NewFile);
	fclose(NewFile);
	}

void MultiMapClass::WriteToFileGpBars(FILE * OutFile)
	{
	int	NMap, NXBin, NYBin;
	fprintf(OutFile,"#%s X Y ",TheName);
	for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%s",Name[NMap]);
	fprintf(OutFile,"\n"); 
	for (NXBin=0; NXBin<MaxNXBins-1; NXBin++) for (int x=0; x<2; x++) 
		{ 
		for (NYBin=0; NYBin<MaxNYBins-1; NYBin++)
			if (x==0) 
				{
				fprintf(OutFile,"%g\t%g",XAt(NXBin)-XBinSize/2.,YAt(NYBin)-YBinSize/2.);
				for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",ZAt(NMap,NXBin,NYBin)); 
				fprintf(OutFile,"\n"); 
				fprintf(OutFile,"%g\t%g",XAt(NXBin)-XBinSize/2.,YAt(NYBin)+YBinSize/2.);
				for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",ZAt(NMap,NXBin,NYBin)); 
				fprintf(OutFile,"\n"); 
				}
			else
				{
				fprintf(OutFile,"%g\t%g",XAt(NXBin)+XBinSize/2.,YAt(NYBin)-YBinSize/2.);
				for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",ZAt(NMap,NXBin,NYBin)); 
				fprintf(OutFile,"\n"); 
				fprintf(OutFile,"%g\t%g",XAt(NXBin)+XBinSize/2.,YAt(NYBin)+YBinSize/2.);
				for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",ZAt(NMap,NXBin,NYBin)); 
				fprintf(OutFile,"\n"); 
				}
		
		
		fprintf(OutFile,"\n"); 
		}
	fprintf(OutFile,"\n\n");
	}

void MultiMapClass::WriteToFileNameGpBars(char* FileName)
	{
	FILE * OutFile;
	if ( (OutFile=fopen(FileName,"w")) == NULL )
		{printf("\nERROR : File: %s could not be opened\n\n",FileName); return;}
	WriteToFileGpBars(OutFile);
	fclose(OutFile);
	}

void MultiMapClass::WriteToFileProjections(int InNMap, FILE* OutFile)
	{
	int	NMap=CheckNMap(InNMap), NXBin, NYBin;
	double	SumValue;
	fprintf(OutFile,"#%s:%s - ProjectionToX\n",TheName,Name[NMap]);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) 
		{ 
		SumValue=0.;
		for (NYBin=0; NYBin<MaxNYBins; NYBin++) SumValue+=ZAt(NMap,NXBin,NYBin);
		fprintf(OutFile,"%g\t%g\n",XAt(NXBin),SumValue); 
		}
	fprintf(OutFile,"\n\n");
	fprintf(OutFile,"#%s:%s - ProjectionToY\n",TheName,Name[NMap]);
	for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{ 
		SumValue=0.;
		for (NXBin=0; NXBin<MaxNXBins; NXBin++)  SumValue+=ZAt(NMap,NXBin,NYBin);
		fprintf(OutFile,"%g\t%g\n",YAt(NYBin),SumValue); 
		}
	fprintf(OutFile,"\n\n");
	}

bool MultiMapClass::ReadFromFile(FILE* InFile)
	{
	if (InFile==NULL) { printf("Warning: Non valid file has been called\n"); return false; }
	int	NConversions=-1,ReadMaxNMaps,ReadMaxNXBins,ReadMaxNYBins; char ReadName[50];
	double	ReadXStartValue,ReadXEndValue,ReadXBinSize,ReadYStartValue,ReadYEndValue,ReadYBinSize;
	NConversions=fscanf(InFile,"%s %*s %d %*s %lf %*s %lf %*s %lf %*s %d %*s %lf %*s %lf %*s %lf %*s %d",ReadName,&ReadMaxNMaps,&ReadXStartValue,&ReadXEndValue,&ReadXBinSize,&ReadMaxNXBins,&ReadYStartValue,&ReadYEndValue,&ReadYBinSize,&ReadMaxNYBins);
	if ( (NConversions!=10) || (ReadName[0]!='#') ) { printf("MultiMap read error : Info line inconsystency\n"); return false; }
	if ( ReadMaxNXBins != ((int)((ReadXEndValue-ReadXStartValue)/ReadXBinSize+0.1)+1) ) { printf("Inconsystency in MaxNXBins\n"); return false; }
	if ( ReadMaxNYBins != ((int)((ReadYEndValue-ReadYStartValue)/ReadYBinSize+0.1)+1) ) { printf("Inconsystency in MaxNYBins\n"); return false; }
	// Free memory
	FreeMemory();
	int	NMap, NXBin, NYBin;
	// Set the read values and allocate memory
	sprintf(TheName,"%s",&(ReadName[1]));
	MaxNMaps=ReadMaxNMaps;
	XStartValue=ReadXStartValue; XEndValue=ReadXEndValue; XBinSize=ReadXBinSize; MaxNXBins=ReadMaxNXBins;
	YStartValue=ReadYStartValue; YEndValue=ReadYEndValue; YBinSize=ReadYBinSize; MaxNYBins=ReadMaxNYBins;
	AllocateMemory();
	ClearAll();
	fscanf(InFile,"%*s %*s");
	for (NMap=0; NMap<MaxNMaps; NMap++) fscanf(InFile,"%s",Name[NMap]);
	SetName(MaxNMaps,(char*)"Overflow");
	double	InX,InY;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		fscanf(InFile,"%lf %lf",&InX,&InY);
		if ( (fabs(InX-XAt(NXBin))>0.1*XBinSize) || (fabs(InY-YAt(NYBin))>0.1*YBinSize) ) { printf("Inconsystency: (X,Y)At(%d,%d)=(%lf,%lf) != (%lf,%lf)\n",NXBin,NYBin,XAt(NXBin),YAt(NYBin),InX,InY); return false; }
		for (NMap=0; NMap<MaxNMaps; NMap++)  fscanf(InFile,"%lf",&(Map[NMap][NXBin][NYBin]));
		}
	return true;
	}

bool MultiMapClass::ReadFromFileName(char* InFileName)
	{
	FILE * InFile = fopen(InFileName,"r");
	if (InFile==NULL) return false;
	bool Ret=ReadFromFile(InFile);
	fclose(InFile);
	return Ret;
	}

// User and debug

void MultiMapClass::ShowInfo()
	{
	printf("MultiMap : %s\n",TheName);
	printf("\tRange:[%g:%g/%g][%g:%g/%g]\n",XStartValue,XEndValue,XBinSize,YStartValue,YEndValue,YBinSize);
	for (int NMap=0; NMap<MaxNMaps; NMap++)
		printf("\t%s :: Integral:%g, Mean:(%g,%g)\n",Name[NMap],Integral(NMap),GetMeanX(NMap),GetMeanY(NMap));
	}






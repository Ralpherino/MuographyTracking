////////////////////////////////////////////////////////////////////////
// Histrogram2Dim Class  - header
//	version : 2019-01-10
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "Histogram2DClass.h"

Histogram2DClass::Histogram2DClass(double InXStartValue, double InXEndValue, double InXBinSize, 
				double InYStartValue, double InYEndValue, double InYBinSize, char * InName)
	{
	Hist=NULL;
	strcpy(Name,InName);
	XStartValue=InXStartValue;
	XEndValue=InXEndValue;
	XBinSize=InXBinSize;
	MaxNXBins=(int)((XEndValue-XStartValue)/XBinSize+0.1)+1;
	YStartValue=InYStartValue;
	YEndValue=InYEndValue;
	YBinSize=InYBinSize;
	MaxNYBins=(int)((YEndValue-YStartValue)/YBinSize+0.1)+1;
	AllocateMemory();
	Clear();
	}

Histogram2DClass::~Histogram2DClass()
	{
	FreeMemory();
	}

void Histogram2DClass::AllocateMemory()
	{
	if (Hist!=NULL) { printf("Warning: Histogram2D (%s) memory reallocation issue\n",Name); FreeMemory(); }
	int	NXBin;
	Hist=(double**) calloc(MaxNXBins, sizeof(double *));
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) Hist[NXBin]=(double*) calloc(MaxNYBins, sizeof(double));
	}

void Histogram2DClass::FreeMemory()
	{
	if (Hist==NULL) return;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) free(Hist[NXBin]);
	free(Hist);
	Hist=NULL;
	}

// Histogram operations /////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Histogram2DClass::Clear()
	{
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin]=0.;
	NData=0;
	}

void Histogram2DClass::Add(double InX, double InY, double InWeight)
	{
	Hist[GetXBin(InX)][GetYBin(InY)]+=InWeight;
	}

bool Histogram2DClass::Clone(Histogram2DClass *OriginalHist2D)
	{
	strcpy(Name,OriginalHist2D->Name);
	XStartValue=OriginalHist2D->XStartValue;
	XEndValue=OriginalHist2D->XEndValue;
	XBinSize=OriginalHist2D->XBinSize;
	MaxNXBins=OriginalHist2D->MaxNXBins;
	YStartValue=OriginalHist2D->YStartValue;
	YEndValue=OriginalHist2D->YEndValue;
	YBinSize=OriginalHist2D->YBinSize;
	MaxNYBins=OriginalHist2D->MaxNYBins;
	AllocateMemory();
	Clear();
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin] = OriginalHist2D->Hist[NXBin][NYBin];
	return true;
	}

bool Histogram2DClass::SameBinningAs(Histogram2DClass * AnyHist2D)
	{
	if ( (XStartValue==(AnyHist2D->XStartValue)) && (XEndValue==(AnyHist2D->XEndValue)) &&  (XBinSize==(AnyHist2D->XBinSize)) &&
	     (YStartValue==(AnyHist2D->YStartValue)) && (YEndValue==(AnyHist2D->YEndValue)) &&  (YBinSize==(AnyHist2D->YBinSize)) ) return true;
	else return false;
	}

bool Histogram2DClass::Add2DHist(Histogram2DClass * MapToAdd)
	{
	if (!(SameBinningAs(MapToAdd))) return false;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin]+=(MapToAdd->Hist[NXBin][NYBin]);
	return true;
	}

bool Histogram2DClass::MultiplyBy(Histogram2DClass * MultiplicatorHist2D)
	{
	if (!(SameBinningAs(MultiplicatorHist2D))) return false;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin]*=(MultiplicatorHist2D->Hist[NXBin][NYBin]);
	return true;
	}

bool Histogram2DClass::DivideBy(Histogram2DClass * DivisorHist2D)
	{
	if (!(SameBinningAs(DivisorHist2D))) return false;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		if (DivisorHist2D->Hist[NXBin][NYBin] > 0.) Hist[NXBin][NYBin]/=(DivisorHist2D->Hist[NXBin][NYBin]);
		else Hist[NXBin][NYBin]=0.;
		}
	return true;
	}

bool Histogram2DClass::BuildFromTwoHists(HistogramClass* HistX, HistogramClass* HistY)
	{
	XStartValue	= HistX->StartValue;
	XEndValue	= HistX->EndValue;
	XBinSize	= HistX->BinSize;
	MaxNXBins	= HistX->MaxNBins;
	YStartValue	= HistY->StartValue;
	YEndValue	= HistY->EndValue;
	YBinSize	= HistY->BinSize;
	MaxNYBins	= HistY->MaxNBins;
	
	AllocateMemory();
	Clear();

	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin]=(HistX->Hist[NXBin]) * (HistY->Hist[NYBin]);
	return true;
	}

void Histogram2DClass::Fill(double FillValue)
	{
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin]=FillValue;
	}

void Histogram2DClass::NormIntegral(double NewIntegral)
	{
	double NormFactor=NewIntegral/Integral();
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Hist[NXBin][NYBin]*=NormFactor;
	}

void Histogram2DClass::Sink(double SinkLevel)
	{
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (Hist[NXBin][NYBin]>SinkLevel) Hist[NXBin][NYBin]-=SinkLevel;
		else Hist[NXBin][NYBin]=0.;
	}

void Histogram2DClass::SinkByAverage(double SinkMultiplicator)
	{
	Sink(GetZAverage()*SinkMultiplicator);
	}

void Histogram2DClass::Crop(double XMin, double XMax, double YMin, double YMax)
	{
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if ( (XAt(NXBin)<XMin) || (XAt(NXBin)>XMax) || (YAt(NYBin)<YMin) || (YAt(NYBin)>YMax) )
			Hist[NXBin][NYBin]=0.;
	}

void Histogram2DClass::Cut(double XMin, double XMax, double YMin, double YMax)
	{
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if ( (XAt(NXBin)>=XMin) && (XAt(NXBin)<=XMax) && (YAt(NYBin)>=YMin) && (YAt(NYBin)>=YMax) )
			Hist[NXBin][NYBin]=0.;
	}

void Histogram2DClass::Smooth(int NIterations)
	{
	double SmoothedHist[MaxNXBins][MaxNYBins];
	int	NXBin, NYBin;
	
	for (NXBin=1; NXBin<MaxNXBins-2; NXBin++) for (NYBin=1; NYBin<MaxNYBins-2; NYBin++)
		SmoothedHist[NXBin][NYBin]=	0.0625*Hist[NXBin-1][NYBin-1] + 0.1250*Hist[NXBin][NYBin-1] + 0.0625*Hist[NXBin+1][NYBin-1]+
						0.1250*Hist[NXBin-1][NYBin]   + 0.2500*Hist[NXBin][NYBin]   + 0.1250*Hist[NXBin+1][NYBin]  +
						0.0625*Hist[NXBin-1][NYBin+1] + 0.1250*Hist[NXBin][NYBin+1] + 0.0625*Hist[NXBin+1][NYBin+1];
	// For the borders
	for (NXBin=1; NXBin<MaxNXBins-2; NXBin++)
		SmoothedHist[NXBin][0]=		0.1875*Hist[NXBin-1][0] + 0.3750*Hist[NXBin][0] + 0.1875*Hist[NXBin+1][0]  +
						0.0625*Hist[NXBin-1][1] + 0.1250*Hist[NXBin][1] + 0.0625*Hist[NXBin+1][1];
	for (NXBin=1; NXBin<MaxNXBins-2; NXBin++)
		SmoothedHist[NXBin][MaxNYBins-1]=0.1875*Hist[NXBin-1][MaxNYBins-1] + 0.3750*Hist[NXBin][MaxNYBins-1] + 0.1875*Hist[NXBin+1][MaxNYBins-1] +
						 0.0625*Hist[NXBin-1][MaxNYBins-2] + 0.1250*Hist[NXBin][MaxNYBins-2] + 0.0625*Hist[NXBin+1][MaxNYBins-2];
	for (NYBin=1; NYBin<MaxNYBins-2; NYBin++)
		SmoothedHist[0][NYBin]=		0.1875*Hist[0][NYBin-1] + 0.3750*Hist[0][NYBin] + 0.1875*Hist[0][NYBin+1] +
						0.0625*Hist[1][NYBin-1] + 0.1250*Hist[1][NYBin] + 0.0625*Hist[1][NYBin+1];
	for (NYBin=1; NYBin<MaxNYBins-2; NYBin++)
		SmoothedHist[MaxNXBins-1][NYBin]=0.1875*Hist[MaxNXBins-1][NYBin-1] + 0.3750*Hist[MaxNXBins-1][NYBin] + 0.1875*Hist[MaxNXBins-1][NYBin+1] +
						 0.0625*Hist[MaxNXBins-2][NYBin-1] + 0.1250*Hist[MaxNXBins-2][NYBin] + 0.0625*Hist[MaxNXBins-2][NYBin+1];
	// For the corners
	SmoothedHist[0][0]=0.75*0.75*Hist[0][0]+0.25*0.75*Hist[0][1]+0.75*0.25*Hist[1][0]+0.25*0.25*Hist[1][1];
	SmoothedHist[MaxNXBins-1][0]=0.75*0.75*Hist[MaxNXBins-1][0]+0.25*0.75*Hist[MaxNXBins-1][1]+0.75*0.25*Hist[MaxNXBins-2][0]+0.25*0.25*Hist[MaxNXBins-2][1];
	SmoothedHist[0][MaxNYBins-1]=0.75*0.75*Hist[0][MaxNYBins-1]+0.25*0.75*Hist[0][MaxNYBins-2]+0.75*0.25*Hist[1][MaxNYBins-1]+0.25*0.25*Hist[1][MaxNYBins-2];
	SmoothedHist[MaxNXBins-1][MaxNYBins-1]=0.75*0.75*Hist[MaxNXBins-1][MaxNYBins-1]+0.25*0.75*Hist[MaxNXBins-1][MaxNYBins-2]+0.75*0.25*Hist[MaxNXBins-2][MaxNYBins-1]+0.25*0.25*Hist[MaxNXBins-2][MaxNYBins-2];

	// Reassign the original Hist
	for (NXBin=1; NXBin<MaxNXBins-2; NXBin++) for (NYBin=1; NYBin<MaxNYBins-2; NYBin++)
		Hist[NXBin][NYBin]=SmoothedHist[NXBin][NYBin];
	
	if (NIterations>1) Smooth(NIterations-1);
	}

void Histogram2DClass::SmearSquare(double InRadius)
	{
	double SmearedMap[MaxNXBins][MaxNYBins];
	int	NXBin, NYBin;
	int	nx,ny;
	double	NSourceData;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++) SmearedMap[NXBin][NYBin]=0;

	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		NSourceData=0;
		for (nx=GetXBin(XAt(NXBin)-InRadius); nx<GetXBin(XAt(NXBin)+InRadius+XBinSize/2.); nx++) for (ny=GetYBin(YAt(NYBin)-InRadius); ny<GetYBin(YAt(NYBin)+InRadius+YBinSize/2.); ny++)
			{
			SmearedMap[NXBin][NYBin]+=Hist[nx][ny];
			NSourceData++;
			}
		if (NSourceData>0) SmearedMap[NXBin][NYBin]/=NSourceData; else SmearedMap[NXBin][NYBin]=0;
		}
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++) Hist[NXBin][NYBin]=SmearedMap[NXBin][NYBin];
	}
	
void Histogram2DClass::SmearRound(double InRadius)
	{
	double SmearedMap[MaxNXBins][MaxNYBins];
	int	NXBin, NYBin;
	int	nx,ny;
	double	NSourceData;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++) SmearedMap[NXBin][NYBin]=0;

	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		NSourceData=0;
		for (nx=GetXBin(XAt(NXBin)-InRadius); nx<GetXBin(XAt(NXBin)+InRadius+XBinSize/2.); nx++) for (ny=GetYBin(YAt(NYBin)-InRadius); ny<GetYBin(YAt(NYBin)+InRadius+YBinSize/2.); ny++)
			if ( ( (XAt(nx)-XAt(NXBin))*(XAt(nx)-XAt(NXBin)) + (YAt(ny)-YAt(NYBin))*(YAt(ny)-YAt(NYBin)) ) < (InRadius*InRadius) )
				{
				SmearedMap[NXBin][NYBin]+=Hist[nx][ny];
				NSourceData++;
				}
		if (NSourceData>0) SmearedMap[NXBin][NYBin]/=NSourceData; else SmearedMap[NXBin][NYBin]=0;
		}
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++) Hist[NXBin][NYBin]=SmearedMap[NXBin][NYBin];
	}

void Histogram2DClass::CutEdges()
	{
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++)
		{ Hist[NXBin][0]=0.; Hist[NXBin][MaxNYBins-1]=0.; }
	for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{ Hist[0][NYBin]=0.; Hist[MaxNXBins-1][NYBin]=0.; }
	}

void Histogram2DClass::AddXHistAtY(HistogramClass* HistToAdd, double InY)
	{
	for (int NBin=0; NBin<HistToAdd->MaxNBins; NBin++)
		Add(HistToAdd->XAt(NBin),InY,HistToAdd->YAt(NBin));
	}

void Histogram2DClass::AddYHistAtX(HistogramClass* HistToAdd, double InX)
	{
	for (int NBin=0; NBin<HistToAdd->MaxNBins; NBin++)
		Add(InX,HistToAdd->XAt(NBin),HistToAdd->YAt(NBin));
	}

double Histogram2DClass::Integral()
	{
	double TheIntegral=0.;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		TheIntegral+=Hist[NXBin][NYBin];
	return TheIntegral;
	}

double Histogram2DClass::MeanX()
	{
	double Sum=0.;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Sum+=XAt(NXBin)*ZAt(NXBin,NYBin);
	return (Sum/Integral());
	}

double Histogram2DClass::MeanY()
	{
	double Sum=0.;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Sum+=YAt(NYBin)*ZAt(NXBin,NYBin);
	return (Sum/Integral());
	}

double Histogram2DClass::SigmaX()
	{
	// projected to the X plane
	double meanx=MeanX();
	double error2=0.;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		error2+=(XAt(NXBin)-meanx)*(XAt(NXBin)-meanx)*ZAt(NXBin,NYBin);
	return sqrt(error2/Integral());
	}

double Histogram2DClass::SigmaY()
	{
	// projected to the Y plane
	double meany=MeanY();
	double error2=0.;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		error2+=(YAt(NYBin)-meany)*(YAt(NYBin)-meany)*ZAt(NXBin,NYBin);
	return sqrt(error2/Integral());
	}

double Histogram2DClass::Sigma()
	{
	double meanx=MeanX();
	double meany=MeanY();
	double error2=0.;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		error2+=((XAt(NXBin)-meanx)*(XAt(NXBin)-meanx) + (YAt(NYBin)-meany)*(YAt(NYBin)-meany)) * ZAt(NXBin,NYBin);
	return sqrt(error2/Integral());
	}

double Histogram2DClass::Peak(double * pPeakX, double * pPeakY)
	{
	double	PeakHeight=ZAt(0,0);
	int	PeakXBin=0, PeakYBin=0;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		if (ZAt(NXBin,NYBin)>PeakHeight)
			{ PeakXBin=NXBin; PeakYBin=NYBin; PeakHeight=ZAt(NXBin,NYBin); };
	if (pPeakX != NULL) *pPeakX=XAt(PeakXBin); 
	if (pPeakY != NULL) *pPeakY=YAt(PeakYBin);
	return PeakHeight;
	}

void Histogram2DClass::GetZDistribution(HistogramClass* ZHist)
	{
	int	NXBin, NYBin;
	ZHist->Clear();
	for (NXBin=0; NXBin<MaxNXBins; NXBin++)  
		for (NYBin=0; NYBin<MaxNYBins; NYBin++)
			ZHist->Add(ZAt(NXBin,NYBin));
	}

double Histogram2DClass::GetZMin()
	{
	int	NXBin, NYBin;
	double ZMin=ZAt(0,0);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++)  
		for (NYBin=0; NYBin<MaxNYBins; NYBin++)
			if (ZAt(NXBin,NYBin)<ZMin) ZMin=ZAt(NXBin,NYBin);
	return ZMin;
	}

double Histogram2DClass::GetZMax()
	{
	int	NXBin, NYBin;
	double ZMax=ZAt(0,0);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++)  
		for (NYBin=0; NYBin<MaxNYBins; NYBin++)
			if (ZAt(NXBin,NYBin)>ZMax) ZMax=ZAt(NXBin,NYBin);
	return ZMax;
	}

double Histogram2DClass::GetZAverage()
	{
	return (Integral()/(double)(MaxNXBins*MaxNYBins));
	}


// Bin info and operations ///////////////////////////////////////////////////////////////////////////////////////////////


int Histogram2DClass::GetXBin(double InX)
	{
	int NXBin=(int)((InX-XStartValue)/XBinSize+0.5);
	if (NXBin < 0) NXBin=0;
	if (NXBin >= MaxNXBins) NXBin=MaxNXBins-1;
	return NXBin; 
	}

int Histogram2DClass::GetYBin(double InY)
	{
	int NYBin=(int)((InY-YStartValue)/YBinSize+0.5);
	if (NYBin < 0) NYBin=0;
	if (NYBin >= MaxNYBins) NYBin=MaxNYBins-1;
	return NYBin;
	}

double Histogram2DClass::XAt(int InNXBin)
	{ return (XStartValue+XBinSize*(double)InNXBin); }

double Histogram2DClass::XMinAt(int InNXBin)
	{ return (XStartValue+XBinSize*(double)InNXBin-0.5*XBinSize); }

double Histogram2DClass::XMaxAt(int InNXBin)
	{ return (XStartValue+XBinSize*(double)InNXBin+0.5*XBinSize); }

double Histogram2DClass::YAt(int InNYBin)
	{ return (YStartValue+YBinSize*(double)InNYBin); }

double Histogram2DClass::YMinAt(int InNYBin)
	{ return (YStartValue+YBinSize*(double)InNYBin-0.5*YBinSize); }

double Histogram2DClass::YMaxAt(int InNYBin)
	{ return (YStartValue+YBinSize*(double)InNYBin+0.5*YBinSize); }

double Histogram2DClass::ZAt(int InNXBin, int InNYBin)
	{ return Hist[InNXBin][InNYBin]; }

double Histogram2DClass::ZScaledAt(int InNXBin, int InNYBin)
	{ return (Hist[InNXBin][InNYBin]/XBinSize/YBinSize); }

double Histogram2DClass::Z(double InX, double InY)
	{ return ZAt(GetXBin(InX), GetYBin(InY)); }

double Histogram2DClass::GetXBinDouble(double InX)
	{ return ((InX-XStartValue)/XBinSize); } 
	
double Histogram2DClass::GetYBinDouble(double InY)
	{ return ((InY-YStartValue)/YBinSize); }

double Histogram2DClass::GetWeightedZAtBinDouble(double InNXBinDouble, double InNYBinDouble)
	{
	if ((InNXBinDouble<=-1) || (InNXBinDouble>=MaxNXBins)) return 0.;
	if ((InNYBinDouble<=-1) || (InNYBinDouble>=MaxNYBins)) return 0.;
	if ( (InNXBinDouble == ((int)InNXBinDouble)) && (InNYBinDouble == ((int)InNYBinDouble)) ) return ZAt((int)InNXBinDouble,(int)InNYBinDouble);
	int NBinLeft=(int)InNXBinDouble, NBinRight=(int)InNXBinDouble+1;
	int NBinDown=(int)InNYBinDouble, NBinUp=(int)InNYBinDouble+1;
	
	double WeightLeft=NBinRight-InNXBinDouble;
	double WeightRight=InNXBinDouble-NBinLeft;
	double WeightDown=NBinUp-InNYBinDouble;
	double WeightUp=InNYBinDouble-NBinDown;
	double ValueLeftDown, ValueRightDown, ValueLeftUp, ValueRightUp;
	if ( (NBinLeft==-1) || (NBinDown==-1) ) 	ValueLeftDown=0.;	else ValueLeftDown=ZAt(NBinLeft,NBinDown);
	if ( (NBinLeft==-1) || (NBinUp==MaxNYBins) )	ValueLeftUp=0.;		else ValueLeftUp=ZAt(NBinLeft,NBinUp);
	if ( (NBinRight==MaxNXBins) || (NBinDown==-1) ) ValueRightDown=0.;	else ValueRightDown=ZAt(NBinRight,NBinDown);
	if ( (NBinRight==MaxNXBins) || (NBinUp==MaxNYBins) ) ValueRightUp=0.;	else ValueRightUp=ZAt(NBinRight,NBinUp);
	return ( ValueLeftDown*WeightLeft*WeightDown + ValueLeftUp*WeightLeft*WeightUp + ValueRightDown*WeightRight*WeightDown + ValueRightUp*WeightRight*WeightUp );
	}

double Histogram2DClass::GetWeightedZInXY(double InX, double InY)
	{ return GetWeightedZAtBinDouble(GetXBinDouble(InX),GetYBinDouble(InY)); }


// Projections, slices //////////////////////////////////////////////////////////////////////////////////////////////////


bool Histogram2DClass::GetXSliceAtYBin(int InNYBin, HistogramClass * HistSlice)
	{
	if (InNYBin < 0 || InNYBin >= MaxNYBins || HistSlice==NULL) return false;
	HistSlice->FreeMemory();
	HistSlice->StartValue	= XStartValue;
	HistSlice->EndValue	= XEndValue;
	HistSlice->BinSize	= XBinSize;
	HistSlice->MaxNBins	= MaxNXBins;
	HistSlice->AllocateMemory();
	HistSlice->Clear();
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++)
		HistSlice->Hist[NXBin] = Hist[NXBin][InNYBin];
	return true;
	}

bool Histogram2DClass::GetXSliceAtY(double InY, HistogramClass * HistSlice)
	{ return GetXSliceAtYBin(GetYBin(InY), HistSlice); }

bool Histogram2DClass::GetYSliceAtXBin(int InNXBin, HistogramClass * HistSlice)
	{
	if (InNXBin < 0 || InNXBin >= MaxNXBins || HistSlice==NULL) return false;
	HistSlice->FreeMemory();
	HistSlice->StartValue	= YStartValue;
	HistSlice->EndValue	= YEndValue;
	HistSlice->BinSize	= YBinSize;
	HistSlice->MaxNBins	= MaxNYBins;
	HistSlice->AllocateMemory();
	HistSlice->Clear();
	for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		HistSlice->Hist[NYBin] = Hist[InNXBin][NYBin];
	return true;
	}

bool Histogram2DClass::GetYSliceAtX(double InX, HistogramClass * HistSlice)
	{ return GetYSliceAtXBin(GetXBin(InX), HistSlice); }

void Histogram2DClass::GetProjectionToX(HistogramClass* HistProjection)
	{
	HistProjection->FreeMemory();
	HistProjection->StartValue	= XStartValue;
	HistProjection->EndValue	= XEndValue;
	HistProjection->BinSize		= XBinSize;
	HistProjection->MaxNBins	= MaxNXBins;
	HistProjection->NData		= NData;
	HistProjection->AllocateMemory();
	HistProjection->Clear();
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		HistProjection->Hist[NXBin]+=Hist[NXBin][NYBin];
	}

void Histogram2DClass::GetProjectionToY(HistogramClass* HistProjection)
	{
	HistProjection->FreeMemory();
	HistProjection->StartValue	= YStartValue;
	HistProjection->EndValue	= YEndValue;
	HistProjection->BinSize	= YBinSize;
	HistProjection->MaxNBins	= MaxNYBins;
	HistProjection->NData		= NData;
	HistProjection->AllocateMemory();
	HistProjection->Clear();
	int	NXBin, NYBin;
	for (NYBin=0; NYBin<MaxNYBins; NYBin++) for (NXBin=0; NXBin<MaxNXBins; NXBin++)
		HistProjection->Hist[NYBin]+=Hist[NXBin][NYBin];
	}


// Write to... /////////////////////////////////////////////////////////////////////////////////////////


void Histogram2DClass::WriteParametersToScreen()
	{
	printf("%s :: Integral: %g,  MeanX: %g +/- %g,  MeanY: %g +/- %g, 2d Sigma: %g\n", Name,Integral(),MeanX(),SigmaX(),MeanY(),SigmaY(),Sigma());
	}

void Histogram2DClass::WriteToFile(FILE * OutFile)
	{
	fprintf(OutFile,"#%s  XStart %g XEnd %g XBinSize %g XNBins %d   YStart %g YEnd %g YBinSize %g YNBins %d\n",Name,XStartValue,XEndValue,XBinSize,MaxNXBins,YStartValue,YEndValue,YBinSize,MaxNYBins);
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) 
		{ 
		for (NYBin=0; NYBin<MaxNYBins; NYBin++)
			fprintf(OutFile,"%g\t%g\t%g\n",XAt(NXBin),YAt(NYBin),ZAt(NXBin,NYBin)); 
		fprintf(OutFile,"\n"); 
		}
	fprintf(OutFile,"\n\n");
	}

bool Histogram2DClass::WriteToNewFile(char* NewFileName)
	{
	FILE * NewFile;
	if ( (NewFile=fopen(NewFileName,"w")) == NULL ) 
		{printf("\nERROR : File: %s could not be opened\n\n",NewFileName); return false;}
	WriteToFile(NewFile);
	fclose(NewFile);
	return true;
	}

void Histogram2DClass::WriteToFileGpBars(FILE * OutFile)
	{
	fprintf(OutFile,"#%s GnuplotBars\n",Name);
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins-1; NXBin++) for (int x=0; x<2; x++) 
		{ 
		for (NYBin=0; NYBin<MaxNYBins-1; NYBin++)
			if (x==0) fprintf(OutFile,"%g\t%g\t%g\n%g\t%g\t%g\n", XMinAt(NXBin),YMinAt(NYBin),ZAt(NXBin,NYBin),XMinAt(NXBin),YMaxAt(NYBin),ZAt(NXBin,NYBin)); 
			else	  fprintf(OutFile,"%g\t%g\t%g\n%g\t%g\t%g\n", XMaxAt(NXBin),YMinAt(NYBin),ZAt(NXBin,NYBin),XMaxAt(NXBin),YMaxAt(NYBin),ZAt(NXBin,NYBin)); 
		fprintf(OutFile,"\n"); 
		}
	fprintf(OutFile,"\n\n");
	}

void Histogram2DClass::WriteToFileGpRebin(FILE * OutFile, double DensityMult)
	{
	fprintf(OutFile,"#%s with RebinDensity %f\n",Name,DensityMult);
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (double x=0; x<DensityMult; x++) 
		{ 
		for (NYBin=0; NYBin<MaxNYBins; NYBin++) for (double y=0; y<DensityMult; y++)
				fprintf(OutFile,"%f\t%f\t%f\n",XAt(NXBin)+x*XBinSize/DensityMult,YAt(NYBin)+y*YBinSize/DensityMult,ZAt(NXBin,NYBin)); 
		fprintf(OutFile,"\n"); 
		}
	fprintf(OutFile,"\n\n");
	}

void Histogram2DClass::WriteToFileProjectionToX(FILE * OutFile)
	{
	fprintf(OutFile,"#%s Projection to X\n",Name);
	double SumValue;
	int	NXBin, NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) 
		{ 
		SumValue=0.;
		for (NYBin=0; NYBin<MaxNYBins; NYBin++) SumValue+=ZAt(NXBin,NYBin);
		fprintf(OutFile,"%g\t%g\n",XAt(NXBin),SumValue); 
		}
	fprintf(OutFile,"\n\n");
	}
	
void Histogram2DClass::WriteToFileProjectionToY(FILE * OutFile)
	{
	fprintf(OutFile,"#%s Projection to Y\n",Name);
	double SumValue;
	int	NXBin, NYBin;
	for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{ 
		SumValue=0.;
		for (NXBin=0; NXBin<MaxNXBins; NXBin++)  SumValue+=ZAt(NXBin,NYBin);
		fprintf(OutFile,"%g\t%g\n",YAt(NYBin),SumValue); 
		}
	fprintf(OutFile,"\n\n");
	}

bool Histogram2DClass::ReadFromFile(FILE * InFile)
	{
	if (InFile==NULL) { printf("Warning: Non valid file has been called\n"); return false; }
	int	NConversions=-1,ReadMaxNXBins,ReadMaxNYBins; char ReadName[50];
	double	ReadXStartValue,ReadXEndValue,ReadXBinSize,ReadYStartValue,ReadYEndValue,ReadYBinSize;
	NConversions=fscanf(InFile,"%s %*s %lf %*s %lf %*s %lf %*s %d %*s %lf %*s %lf %*s %lf %*s %d",ReadName,&ReadXStartValue,&ReadXEndValue,&ReadXBinSize,&ReadMaxNXBins,&ReadYStartValue,&ReadYEndValue,&ReadYBinSize,&ReadMaxNYBins);
	if ( (NConversions!=9) || (ReadName[0]!='#') ) { printf("Histogram2D read error : Info line inconsystency : (ReadName:%s)\n",ReadName); return false; }
	if ( ReadMaxNXBins != ((int)((ReadXEndValue-ReadXStartValue)/ReadXBinSize+0.1)+1) ) { printf("Inconsystency in MaxNXBins\n"); return false; }
	if ( ReadMaxNYBins != ((int)((ReadYEndValue-ReadYStartValue)/ReadYBinSize+0.1)+1) ) { printf("Inconsystency in MaxNYBins\n"); return false; }
	// Free memory
	FreeMemory();
	int	NXBin,NYBin;
	// Set the read values and allocate memory
	sprintf(Name,"%s",&(ReadName[1]));
	XStartValue=ReadXStartValue; XEndValue=ReadXEndValue; XBinSize=ReadXBinSize; MaxNXBins=ReadMaxNXBins;
	YStartValue=ReadYStartValue; YEndValue=ReadYEndValue; YBinSize=ReadYBinSize; MaxNYBins=ReadMaxNYBins;
	AllocateMemory();
	Clear();
	double	InX,InY;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		fscanf(InFile,"%lf %lf",&InX,&InY);
		if ( (fabs(InX-XAt(NXBin))>0.1*XBinSize) || (fabs(InY-YAt(NYBin))>0.1*YBinSize) ) { printf("Inconsystency: (X,Y)At(%d,%d)=(%lf,%lf) != (%lf,%lf)\n",NXBin,NYBin,XAt(NXBin),YAt(NYBin),InX,InY); return false; }
		fscanf(InFile,"%lf",&(Hist[NXBin][NYBin]));
		}
	return true;
	}



////////////////////////////////////////////////////////////////////////
// Position Smoother Class - implementation
//      version : 2021-10-12
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "PositionSmootherClass.h"


////////////////////////////////////////////////////////////////////////
// PositionSmootherClass

PositionSmootherClass::PositionSmootherClass()
	{
	PeriodLengthX=0; PeriodLength=0;
	IsSet=false;
	Period=NULL; NPeriods=0;
	ThePeriod=NULL; ThePeriodF=NULL;
	UseRatio=0.50;
	UseFlip=false;
	IsFlipped=NULL;
	PeriodGroupSizeX=32;
	sprintf(Name,"PositionSmoother_Noname");
	}

PositionSmootherClass::~PositionSmootherClass()
	{
	if (Period!=NULL) { for (int p=0; p<NPeriods; p++) delete Period[p]; free(Period); }
	delete ThePeriod; 
	if (UseFlip) delete ThePeriodF;
	if (IsFlipped!=NULL) free(IsFlipped);
	}

void PositionSmootherClass::Set(MultiHistClass * InMultiHist, int InHistId, double InPeriodLengthX, int NFlips, double *FlipPosition)
	{
	if (InMultiHist==NULL || !(InMultiHist->Integral(InHistId)>1e-3) ) 
		{ printf("Warning! Smoother: Hist: %s:%d (Intergral=%g)\n",InMultiHist->TheName,InHistId,InMultiHist->Integral(InHistId)); return; }
	StartValue=InMultiHist->StartValue;
	BinSize=InMultiHist->BinSize;
	int NBins=InMultiHist->MaxNBins;
	// Check PeriodLength in Bin units
	PeriodLengthX=InPeriodLengthX;
	double PeriodLengthDouble=PeriodLengthX/BinSize;
	PeriodLength=int(PeriodLengthDouble);
	PeriodGroupSize=int(PeriodGroupSizeX/BinSize+0.1);
	PeriodGroupMultiplicity=int(PeriodGroupSizeX/PeriodLengthX+0.1);
	if (fabs(double(PeriodLength)-PeriodLengthDouble)>0.1) printf("Warning : Inconsystency in BinSize and PeriodLength\n");
	// Allocate memory
	NPeriods=(NBins+PeriodLength/4)/PeriodLength;
	Period = (SmootherPeriodClass**) calloc(NPeriods,sizeof(SmootherPeriodClass*));
	for (int p=0; p<NPeriods; p++) Period[p] = new SmootherPeriodClass(PeriodLength);
	ThePeriod = new SmootherPeriodClass(PeriodLength);
	if (NPeriods<6) UseRatio=1.0; // Note: NUsed = (NPeriods-2)*UseRatio ; this shall be reasonable in small detectors as well
	// Check Flips and set if required
	if (NFlips>0)
		{
		UseFlip=true;
		ThePeriodF = new SmootherPeriodClass(PeriodLength);
		IsFlipped = (bool*) calloc(NPeriods,sizeof(bool));
		for (int p=0; p<NPeriods; p++) IsFlipped[p]=false;
		int FlipPeriodsFrom;
		for (int f=0; f<NFlips; f++)
			{
			FlipPeriodsFrom = InMultiHist->GetBin(FlipPosition[f])/PeriodLength;
			for (int p=FlipPeriodsFrom; p<NPeriods; p++) IsFlipped[p]=!IsFlipped[p];
			}
		}
	// Copy the Histogram data to the Periods
	int ThisP,ThisL;
	for (int n=0; n<InMultiHist->MaxNBins; n++)
		{
		GetPeriodCoordinates(n,&ThisP,&ThisL);
		if (ThisP>=0 && ThisL>=0) Period[ThisP]->DataRaw[ThisL]=InMultiHist->YAt(InHistId,n);
		}

//	for (int p=0; p<NPeriods; p++) for (int l=0; l<PeriodLength; l++)
//		if (p*PeriodLength+l < NBins) Period[p]->DataRaw[l]=InMultiHist->YAt(InHistId,p*PeriodLength+l);
	for (int p=0; p<NPeriods; p++) Period[p]->CalculateAverage();
	// Equalize multiplicities, and combine them
	for (int p=1; p<NPeriods-1; p++) Period[p]->Equalize(Period[p-1],Period[p+1]);
	for (int p=1; p<NPeriods-1; p++)
		if (UseFlip && IsFlipped[p]) ThePeriodF->AddEq(Period[p]);
		else ThePeriod->AddEq(Period[p]);
	// Check Flipped shift
	double FlipDiffThis,FlipDiffBest,FlipShiftBest=0;
	if (UseFlip)
		{
		ThePeriodF->Flip();
		FlipDiffBest=ThePeriodF->CompareTo(ThePeriod);
		for (int s=0; s<PeriodLength; s++)
			{
			FlipDiffThis=ThePeriodF->CompareTo(ThePeriod);
			if (FlipDiffThis<FlipDiffBest) {FlipShiftBest=s;FlipDiffBest=FlipDiffThis;}
			ThePeriodF->Shift();
			}
		ThePeriodF->Shift(FlipShiftBest);
		for (int p=0; p<NPeriods; p++) if (IsFlipped[p]) 
			{
			Period[p]->Flip();
			Period[p]->Shift(FlipShiftBest);
			ThePeriod->AddEq(Period[p]);
			}
		}
	// Check usability
	double PeriodDiff[NPeriods], PeriodDiffLimit=0;
	for (int p=0; p<NPeriods; p++) PeriodDiff[p]=Period[p]->CompareTo(ThePeriod);
	SortArrayDescending(PeriodDiff,NPeriods);
	PeriodDiffLimit=PeriodDiff[int((1.0-UseRatio)*double(NPeriods-2))]; // -2 : do not count the sides
	ThePeriod->ClearEq();
	for (int p=1; p<NPeriods-1; p++) 
		if ( (Period[p]->ComparedDiff<PeriodDiffLimit) && (Period[p]->ComparedDiff>=0) ) ThePeriod->AddEq(Period[p]);
	ThePeriod->CalculateAverage();
	// Prepare the real smoother
	ThePeriod->PrepareSmoother();
	IsSet=true;
	// Make ThePeriodF from ThePeriod
	if (UseFlip)
		{
		ThePeriodF->ClearEq();
		ThePeriodF->AddEq(ThePeriod);
		ThePeriodF->Flip();
		ThePeriodF->Shift(FlipShiftBest);
		ThePeriodF->PrepareSmoother();
		}
	// Free memory
	for (int p=0; p<NPeriods; p++) delete Period[p]; free(Period); Period=NULL;
	}

void PositionSmootherClass::GetPeriodCoordinates(int Bin, int* pCoordPeriod, int* pCoordLocal)
	{
	// eg.  PGS=32,  P=3, b=34=32+2->1*10+0,2
	int GroupN= int(Bin/PeriodGroupSize); // 1 
	int CoordPeriodInGroup=(Bin-GroupN*PeriodGroupSize)/PeriodLength; // 0
	if (CoordPeriodInGroup>=PeriodGroupMultiplicity) {*pCoordPeriod=-1;*pCoordLocal=-1;return;}
	*pCoordPeriod=GroupN*PeriodGroupMultiplicity+CoordPeriodInGroup; // 1*10+0
	*pCoordLocal=(Bin-GroupN*PeriodGroupSize)%PeriodLength; // 2
	return;
	}

double PositionSmootherClass::Smooth(double InX)
	{
	if (!IsSet) return InX;
	int	n=int((InX-StartValue)/BinSize+0.5); int p,l;
	GetPeriodCoordinates(n,&p,&l);
	//printf("%5.1lf : %d , %d:%d\n",InX,n,p,l);
	if (p<0 || l<0) return InX;
	if (UseFlip && IsFlipped[p]) return (InX+BinSize*(ThePeriodF->Smooth(l)-double(l)));
	return (InX+BinSize*(ThePeriod->Smooth(l)-double(l)));
	}


////////////////////////////////////////////////////////////////////////
// SmootherPeriodClass

SmootherPeriodClass::SmootherPeriodClass(int InLength)
	{
	if (InLength<1) { printf("Warning! PeriodLength in BinUnits cannot be smaller than 1\n"); return; }
	Length=InLength;
	DataRaw = (double*) calloc(Length,sizeof(double));
	DataEq = (double*) calloc(Length,sizeof(double));
	Start = (double*) calloc(Length,sizeof(double));
	Mult = (double*) calloc(Length,sizeof(double));
	AverageRaw=-1; AverageEq=-1;
	Good=true;
	ComparedDiff=100;
	// Defaul for no smoothing
	for (int l=0;l<Length;l++) Start[l]=-0.5+double(l);
	}

SmootherPeriodClass::~SmootherPeriodClass()
	{
	free(DataRaw);
	free(DataEq);
	free(Start);
	free(Mult);
	}

void SmootherPeriodClass::CalculateAverage()
	{
	AverageRaw=0;
	for (int l=0; l<Length; l++) AverageRaw+=DataRaw[l];
	AverageRaw/=double(Length);
	AverageEq=0;
	for (int l=0; l<Length; l++) AverageEq+=DataEq[l];
	AverageEq/=double(Length);
	}

void SmootherPeriodClass::Equalize(SmootherPeriodClass * PeriodBefore, SmootherPeriodClass * PeriodAfter)
	{
	if (PeriodBefore->AverageRaw==0 && PeriodAfter->AverageRaw==0)
		{ for (int l=0;l<Length;l++) DataEq[l] = 0; CalculateAverage(); return; }
	double HalfLength=double(Length)/2.0;
	for (int l=0;l<Length;l++)
		DataEq[l] = DataRaw[l] * AverageRaw / Interpolate(0-HalfLength,PeriodBefore->AverageRaw,3*HalfLength,PeriodAfter->AverageRaw,double(l));
	CalculateAverage();
	}

void SmootherPeriodClass::ClearEq()
	{
	for (int l=0;l<Length;l++) DataEq[l]=0;
	CalculateAverage();
	}

void SmootherPeriodClass::Flip()
	{
	double DataTemp[Length];
	for (int l=0;l<Length;l++) DataTemp[l]=DataRaw[l];
	for (int l=0;l<Length;l++) DataRaw[l]=DataTemp[Length-1-l];
	for (int l=0;l<Length;l++) DataTemp[l]=DataEq[l];
	for (int l=0;l<Length;l++) DataEq[l]=DataTemp[Length-1-l];
	}

void SmootherPeriodClass::Shift(int InNShift)
	{
	if (InNShift<1) return;
	double DataTemp;
	DataTemp=DataRaw[0];
	for (int l=0;l<Length-1;l++) DataRaw[l]=DataRaw[l+1];
	DataRaw[Length-1]=DataTemp;
	DataTemp=DataEq[0];
	for (int l=0;l<Length-1;l++) DataEq[l]=DataEq[l+1];
	DataEq[Length-1]=DataTemp;
	Shift(InNShift-1);
	return;
	}

void SmootherPeriodClass::AddEq(SmootherPeriodClass * InPeriod)
	{
	for (int l=0;l<Length;l++) DataEq[l]+=InPeriod->DataEq[l];
	CalculateAverage();
	}

double SmootherPeriodClass::CompareTo(SmootherPeriodClass * InPeriod)
	{
	if (AverageEq<0.01) return ComparedDiff;
	ComparedDiff=0;
	for (int l=0;l<Length;l++) ComparedDiff+= fabs( DataEq[l]/AverageEq - InPeriod->DataEq[l]/InPeriod->AverageEq );
	return ComparedDiff;
	}

void SmootherPeriodClass::PrepareSmoother()
	{
	if (AverageEq<0.01) return;
	for (int l=0;l<Length;l++) Mult[l]=DataEq[l]/AverageEq;
	Start[0]=-0.5;
	for (int l=1;l<Length;l++) Start[l]=Start[l-1]+Mult[l-1];
	// Check if it is ok
	double EndAt=Start[Length-1]+Mult[Length-1]-Start[0];
	if (fabs(EndAt-double(Length))>0.1) printf("Inconsistency in Smoothing preparation\n");
	}

double SmootherPeriodClass::Smooth(int X)
	{
	return (Start[X]+drand48()*Mult[X]);
	}






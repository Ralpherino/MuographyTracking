////////////////////////////////////////////////////////////////////////
// Chamber Class - implementation
//      version : 2021-08-26
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "ChamberClass.h"
#include "TrackClass.h"

ChamberClass::ChamberClass(int InSizeX, int InSizeY)
	{
	sprintf(Name,"Chamber");
	FileContents="General HitXDep HitYDep ClusterPosX ClusterPosY ClusterSizePosX ClusterSizePosY ClusterSizeX ClusterSizeY PositionDepBare PositionDep AngleDepBare AngleDep AdcDepBare AdcDep PosAngXDep PosAngYDep TimeDep ResolutionMap ResolutionX ResolutionY PositionTune TriggerEffSlope";
	// Default values
	PositionX=0; PositionY=0; PositionZ=0;
	PadSizeX=1.0; PadSizeY=1.0;
	// Set MultiHist names
	NIds=13; // +2
	IdName = (char**) calloc(NIds+2,sizeof(char*));
	for (int NId=0; NId<NIds+2; NId++) IdName[NId] = (char*) calloc(30,sizeof(char));
	sprintf(IdName[IdAll],"All"); sprintf(IdName[IdTrackCross],"TrackCross"); 
	sprintf(IdName[IdTriggerOk],"TriggerOk"); sprintf(IdName[IdClusterOk],"ClusterOk"); sprintf(IdName[IdHitFound],"HitFound"); 
	sprintf(IdName[IdTrackingEff],"TrackingEff"); sprintf(IdName[IdTriggerEff],"TriggerEff");
	sprintf(IdName[IdTrackAdcSum],"TrackAdcSum"); sprintf(IdName[IdTrackAdcScaledSum],"TrackAdcScaledSum"); 
	sprintf(IdName[IdShiftXSum],"ShiftXSum"); sprintf(IdName[IdShiftYSum],"ShiftYSum");
	sprintf(IdName[IdFiducial],"Fiducial"); sprintf(IdName[IdFiducialNew],"FiducialNew");
	// the extras for TimeEvolution
	sprintf(IdName[IdNTriggers],"NTriggers"); sprintf(IdName[IdTriggerRate],"TriggerRate");
	NCluIds=4;
	CluIdName = (char**) calloc(NCluIds,sizeof(char*));
	for (int NCluId=0; NCluId<NCluIds; NCluId++) CluIdName[NCluId] = (char*) calloc(20,sizeof(char));
	sprintf(CluIdName[IdCluAllRaw],"CluAllRaw"); sprintf(CluIdName[IdCluAll],"CluAll"); 
	sprintf(CluIdName[IdCluOkRaw],"CluOkRaw"); sprintf(CluIdName[IdCluOk],"CluOk"); 
	// Hit multihist names
	NHitIds=4;
	HitIdName = (char**) calloc(NHitIds,sizeof(char*));
	for (int NId=0; NId<NHitIds; NId++) HitIdName[NId] = (char*) calloc(20,sizeof(char));
	sprintf(HitIdName[IdHitAll],"HitAll"); sprintf(HitIdName[IdHitTrack],"HitTrack");
	sprintf(HitIdName[IdHitNoise],"HitNoise"); sprintf(HitIdName[IdHitNoiseBase],"HitNoiseBase");
	// Set size and allocate memory, if defined!
	SizeX=InSizeX; SizeY=InSizeY;
	if (SizeX>0 && SizeY>0) AllocateMemory();
	// Defaults for the ADCs
	AdcPedestal=0; AdcScale=1;
	// SideGrow
	SideGrowX=3; SideGrowY=3; // rough default value, shall be overwritten if used
	// Noise
	NoiseClearance=5.5;
	// MisPosition
	UseMisPositionCorrection=false;
	MisPositionXMin=0; MisPositionXMax=0; MisPositionXValue=0;
	MisPositionYMin=0; MisPositionYMax=0; MisPositionYValue=0;
	// Broken channels
	UseBrokenChannels=false;
	NBrokenChannelsX=0; BrokenChannelX=NULL;
	NBrokenChannelsY=0; BrokenChannelY=NULL;
	// Reset gneral scalars
	EfficiencyTracking=0; EfficiencyTrigger=0;
	GainAdc=0; EffSize=0; EffGood=0;
	TesQ=0;
	}

void ChamberClass::SetAnalysisParameters(AnalysisParametersClass* InAParameters)
	{
	AParameters=InAParameters;
	if (AParameters==NULL) 
		{
		AParameters = new AnalysisParametersClass();
		printf("Warning! No AnalysisParameters were given, using defaults\n");
		}
	}

void ChamberClass::SetSize(int InSizeX, int InSizeY)
	{
	SizeX=InSizeX; SizeY=InSizeY;
	if (SizeX>0 && SizeY>-1) AllocateMemory();
	else printf("Warning: incorrect size for chamber : %d x %d\n",SizeX,SizeY);
	}

void ChamberClass::AllocateMemory()
	{
	if (SizeX<1 || SizeY<0) { printf("Size shall be defined in ChamberBasics\nExit\n\n"); exit(50); }
	HitPositionX = (int*) calloc(SizeX,sizeof(int));
	HitPositionY = (int*) calloc(SizeY,sizeof(int));
	// Hit level classes
	HitXDep = new MultiHistClass(4,0.0,SizeX-1,1,(char*)"HitXDep");
	HitYDep = new MultiHistClass(4,0.0,SizeY-1,1,(char*)"HitYDep");
	for (int NId=0; NId<NHitIds; NId++) HitXDep->SetName(NId,HitIdName[NId]);
	for (int NId=0; NId<NHitIds; NId++) HitYDep->SetName(NId,HitIdName[NId]);
	PositionDep = new MultiMapClass(NIds,-0.5+AParameters->PositionDepBinSize/2.0,SizeX-1,AParameters->PositionDepBinSize,-0.5+AParameters->PositionDepBinSize/2.0,SizeY-1,AParameters->PositionDepBinSize,(char*)"PositionDep");
	for (int NId=0; NId<NIds; NId++) PositionDep->SetName(NId,IdName[NId]);
	AngleDep = new MultiMapClass(NIds,-(AParameters->AngleMax),AParameters->AngleMax,AParameters->AngleDepBinSize,-(AParameters->AngleMax),AParameters->AngleMax,AParameters->AngleDepBinSize,(char*)"AngleDep");
	for (int NId=0; NId<NIds; NId++) AngleDep->SetName(NId,IdName[NId]);
	AdcDep = new MultiHistClass(NIds,AParameters->AdcMin,AParameters->AdcMax,AParameters->AdcBinSize,(char*)"AdcDep");
	for (int NId=0; NId<NIds; NId++) AdcDep->SetName(NId,IdName[NId]);
	PosAngXDep = new MultiMapClass(NIds,-0.5+AParameters->PositionDepBinSize/2.0,SizeX-1,AParameters->PositionDepBinSize,-(AParameters->AngleMax),AParameters->AngleMax,AParameters->AngleDepBinSize,(char*)"PosAngXDep");
	for (int NId=0; NId<NIds; NId++) PosAngXDep->SetName(NId,IdName[NId]);
	PosAngYDep = new MultiMapClass(NIds,-0.5+AParameters->PositionDepBinSize/2.0,SizeY-1,AParameters->PositionDepBinSize,-(AParameters->AngleMax),AParameters->AngleMax,AParameters->AngleDepBinSize,(char*)"PosAngYDep");
	for (int NId=0; NId<NIds; NId++) PosAngYDep->SetName(NId,IdName[NId]);
	TimeDep = new MultiHistDynTimeClass(NIds+2,AParameters->TimeDepBinSizeSeconds,(char*)"TimeDep");
	for (int NId=0; NId<NIds+2; NId++) TimeDep->SetName(NId,IdName[NId]);
	TimeDep->SetAutoScale(AParameters->MaxNTimeBins);
	Resolution = new Histogram2DClass(-(AParameters->DistMax),AParameters->DistMax,AParameters->ResolutionBinSize,-(AParameters->DistMax),AParameters->DistMax,AParameters->ResolutionBinSize,(char*)"Resolution");
	PositionTune = new MultiHistClass(4,-(AParameters->AngleMax),AParameters->AngleMax,AParameters->AngleDepBinSize,(char*)"PositionTune");
	PositionTune->SetName(IdDeltaXN,(char*)"DeltaXN"); PositionTune->SetName(IdDeltaXSum,(char*)"DeltaXSum");
	PositionTune->SetName(IdDeltaYN,(char*)"DeltaYN"); PositionTune->SetName(IdDeltaYSum,(char*)"DeltaYSum");
	TriggerEffSlope = new MultiHistClass(3,0,AParameters->DistMax,AParameters->AngleDepBinSize,(char*)"TriggerEffSlope");
	TriggerEffSlope->SetName(IdTesN,(char*)"N"); TriggerEffSlope->SetName(IdTesOk,(char*)"Ok"); TriggerEffSlope->SetName(IdTesEff,(char*)"Eff");
	FiducialX = new HistogramClass(PositionDep->XStartValue,PositionDep->XEndValue,PositionDep->XBinSize,(char*)"FiducialX");
	FiducialY = new HistogramClass(PositionDep->YStartValue,PositionDep->YEndValue,PositionDep->YBinSize,(char*)"FiducialY");
	// Clusters
	MaxNClustersX=SizeX/2+1;
	ClusterX = (ClusterClass**) calloc(MaxNClustersX,sizeof(ClusterClass*));
	for (int NClusterX=0; NClusterX<MaxNClustersX; NClusterX++) ClusterX[NClusterX] = new ClusterClass();
	MaxNClustersX=0;
	MaxNClustersY=SizeY/2+1;
	ClusterY = (ClusterClass**) calloc(MaxNClustersY,sizeof(ClusterClass*));
	for (int NClusterY=0; NClusterY<MaxNClustersY; NClusterY++) ClusterY[NClusterY] = new ClusterClass();
	MaxNClustersY=0;
	// Histos for clusters
	ClusterSizeXHist = new MultiHistClass(NCluIds,0,AParameters->ClusterSizeLimit*2,1,(char*)"ClusterSizeXHist");
	for (int NCluId=0; NCluId<NCluIds; NCluId++) ClusterSizeXHist->SetName(NCluId,CluIdName[NCluId]);
	ClusterSizeYHist = new MultiHistClass(NCluIds,0,AParameters->ClusterSizeLimit*2,1,(char*)"ClusterSizeYHist");;
	for (int NCluId=0; NCluId<NCluIds; NCluId++) ClusterSizeYHist->SetName(NCluId,CluIdName[NCluId]);
	ClusterPositionXHist = new MultiHistClass(NCluIds,0,SizeX-1,AParameters->ClusterPositionBinSize,(char*)"ClusterPositionXHist");
	for (int NCluId=0; NCluId<NCluIds; NCluId++) ClusterPositionXHist->SetName(NCluId,CluIdName[NCluId]);
	ClusterPositionYHist = new MultiHistClass(NCluIds,0,SizeY-1,AParameters->ClusterPositionBinSize,(char*)"ClusterPositionYHist");
	for (int NCluId=0; NCluId<NCluIds; NCluId++) ClusterPositionYHist->SetName(NCluId,CluIdName[NCluId]);
	NPositionSmoothers = AParameters->ClusterSizeLimit+2;
	PositionSmootherX = (PositionSmootherClass**) calloc(NPositionSmoothers,sizeof(PositionSmootherClass*));
	for (int CluSize=0; CluSize<NPositionSmoothers; CluSize++)	{ PositionSmootherX[CluSize] = new PositionSmootherClass(); sprintf(PositionSmootherX[CluSize]->Name,"PosSmoothX-%s-CluSize-%d",Name,CluSize); }
	PositionSmootherY = (PositionSmootherClass**) calloc(NPositionSmoothers,sizeof(PositionSmootherClass*));
	for (int CluSize=0; CluSize<NPositionSmoothers; CluSize++)	{ PositionSmootherY[CluSize] = new PositionSmootherClass(); sprintf(PositionSmootherY[CluSize]->Name,"PosSmoothY-%s-CluSize-%d",Name,CluSize); }
	ClusterSizePositionXHist = new MultiHistClass(NPositionSmoothers,0,SizeX-1,AParameters->ClusterPositionBinSize,(char*)"ClusterSizePositionXHist");
	ClusterSizePositionYHist = new MultiHistClass(NPositionSmoothers,0,SizeY-1,AParameters->ClusterPositionBinSize,(char*)"ClusterSizePositionYHist");
	// Set defaults
	PositionDep->Fill(IdFiducial,1);
	double BorderSize=2;
	PositionDep->SetLineXWidth(IdFiducial,0,0,BorderSize); PositionDep->SetLineXWidth(IdFiducial,SizeX-1,0,BorderSize);
	PositionDep->SetLineYWidth(IdFiducial,0,0,BorderSize); PositionDep->SetLineYWidth(IdFiducial,SizeY-1,0,BorderSize);
	// TEST TEMP not bad, but some initial info shall be nicer..
	if (SizeX==3*32) { PositionDep->SetLineXWidth(IdFiducial,31.5,0,BorderSize); PositionDep->SetLineXWidth(IdFiducial,63.5,0,BorderSize); }
	if (SizeX==2*32) PositionDep->SetLineXWidth(IdFiducial,(SizeX-1)/2.0,0,BorderSize);
	if (SizeY==2*32) PositionDep->SetLineYWidth(IdFiducial,(SizeY-1)/2.0,0,BorderSize);
	if (SizeX==8*16) PositionDep->SetLineXWidth(IdFiducial,(SizeX-1)/2.0,0,BorderSize);
	if (SizeY==8*16) PositionDep->SetLineYWidth(IdFiducial,(SizeY-1)/2.0,0,BorderSize);
	if (SizeX==1*16) PositionDep->SetLineXWidth(IdFiducial,(SizeX-1)*1.0,0.0,1.0);
	FiducialConvertMapToXY();
	}

ChamberClass::~ChamberClass()
	{
	for (int NId=0; NId<NIds; NId++) free(IdName[NId]); free(IdName);
	for (int NCluId=0; NCluId<NCluIds; NCluId++) free(CluIdName[NCluId]); free(CluIdName);
	if (HitPositionX!=NULL) free(HitPositionX);
	if (HitPositionY!=NULL) free(HitPositionY);
	delete PositionDep;
	delete AngleDep;
	delete AdcDep;
	delete PosAngXDep; delete PosAngYDep;
	delete Resolution;
	delete PositionTune;
	delete TriggerEffSlope;
	// Clusters
	MaxNClustersX=SizeX/2+1; MaxNClustersY=SizeY/2+1;
	for (int NClusterX=0; NClusterX<MaxNClustersX; NClusterX++) delete ClusterX[NClusterX];
	for (int NClusterY=0; NClusterY<MaxNClustersY; NClusterY++) delete ClusterY[NClusterY];
	free(ClusterX); free(ClusterY);
	delete ClusterSizeXHist; delete ClusterSizeYHist;
	delete ClusterPositionXHist; delete ClusterPositionYHist;
	}

void ChamberClass::ClearStatistics()
	{
	HitXDep->ClearAll();
	HitYDep->ClearAll();
	ClusterPositionXHist->ClearAll();
	ClusterPositionYHist->ClearAll();
	ClusterSizePositionXHist->ClearAll();
	ClusterSizePositionYHist->ClearAll();
	for (int NId=0; NId<NIds; NId++) if ((NId!=IdTriggerEff)&&(NId!=IdTrackingEff)&&(NId!=IdFiducial))
		{
		PositionDep->Clear(NId);
		AngleDep->Clear(NId);
		AdcDep->Clear(NId);
		PosAngXDep->Clear(NId);
		PosAngYDep->Clear(NId);
		}
	TimeDep->ClearAll();
	Resolution->Clear();
	PositionTune->ClearAll();
	TriggerEffSlope->ClearAll();
	// ReSet AParams
	TimeDep->BinSize=AParameters->TimeDepBinSizeSeconds;
	TimeDep->SetAutoScale(AParameters->MaxNTimeBins);
	
	}

void ChamberClass::FiducialConvertMapToXY()
	{
	double	Sum=0;
	int	NX,NY;
	for (NX=0; NX<FiducialX->MaxNBins; NX++)
		{
		Sum=0;
		for (NY=0;NY<FiducialY->MaxNBins;NY++) Sum+=PositionDep->ZAt(IdFiducial,NX,NY);
		if (Sum>0.5) FiducialX->Hist[NX]=1; else FiducialX->Hist[NX]=0;
		}
	for (NY=0; NY<FiducialY->MaxNBins; NY++)
		{
		Sum=0;
		for (NX=0;NX<FiducialX->MaxNBins;NX++) Sum+=PositionDep->ZAt(IdFiducial,NX,NY);
		if (Sum>0.5) FiducialY->Hist[NY]=1; else FiducialY->Hist[NY]=0;
		}
	}

void ChamberClass::GoodEvent()
	{
	if (UseMisPositionCorrection) MisPositionCorrection();
	if (UseBrokenChannels) BrokenChannelsCorrection();
	//printf("%d:%d, %d:%d\n",NHitsX,MaxNClustersX,NHitsY,MaxNClustersY);
	for (int NHitX=0; NHitX<NHitsX; NHitX++) HitXDep->Add(IdHitAll,HitPositionX[NHitX]);
	for (int NHitY=0; NHitY<NHitsY; NHitY++) HitYDep->Add(IdHitAll,HitPositionY[NHitY]);
	// Fill histos
	Adc=((double)AdcRaw-AdcPedestal)*AdcScale;
	if (AdcRaw==0) Adc=0;
	AdcScaled=Adc;
	AdcDep->Add(IdAll,Adc);
	TimeDep->Add(IdAll,DateTimeString);
	Triggered = (TriggerBit==1);
	if (Triggered) TimeDep->Add(IdNTriggers,DateTimeString);
	TrackCrossed=false;
	}

void ChamberClass::MisPositionCorrection()
	{
	for (int NHitX=0; NHitX<NHitsX; NHitX++)
		if ( (HitPositionX[NHitX]>=MisPositionXMin) && (HitPositionX[NHitX]<MisPositionXMax) )
			{
			HitPositionX[NHitX]+=MisPositionXValue;
			if (HitPositionX[NHitX]<0) HitPositionX[NHitX]=0;
			if (HitPositionX[NHitX]>SizeX-1) HitPositionX[NHitX]=SizeX-1;
			}
	for (int NHitY=0; NHitY<NHitsY; NHitY++)
		if ( (HitPositionY[NHitY]>=MisPositionYMin) && (HitPositionY[NHitY]<MisPositionYMax) )
			{
			HitPositionY[NHitY]+=MisPositionYValue;
			if (HitPositionY[NHitY]<0) HitPositionY[NHitY]=0;
			if (HitPositionY[NHitY]>SizeY-1) HitPositionY[NHitY]=SizeY-1;
			}
	}

void ChamberClass::BrokenChannelsCorrection()
	{
	for (int NBrokenCh=0; NBrokenCh<NBrokenChannelsX; NBrokenCh++) 
		for (int NHitX=0; NHitX<NHitsX; NHitX++)
			if ( HitPositionX[NHitX] == BrokenChannelX[NBrokenCh] )
				{
				for (int n=NHitX; n<NHitsX-1; n++) HitPositionX[NHitX]=HitPositionX[NHitX+1];
				NHitsX--;
				}
	for (int NBrokenCh=0; NBrokenCh<NBrokenChannelsY; NBrokenCh++) 
		for (int NHitY=0; NHitY<NHitsY; NHitY++)
			if ( HitPositionY[NHitY] == BrokenChannelY[NBrokenCh] )
				{
				for (int n=NHitY; n<NHitsY-1; n++) HitPositionY[NHitY]=HitPositionY[NHitY+1];
				NHitsY--;
				}
	}

void ChamberClass::Clusterize()
	{
	// X
	MaxNClustersX=0;
	ClusterX[MaxNClustersX]->Start(HitPositionX[0]);
	for (int NHitX=1; NHitX<NHitsX; NHitX++)
		{
		if ( (HitPositionX[NHitX]-HitPositionX[NHitX-1]) == 1 ) 
			{ ClusterX[MaxNClustersX]->ExtendOne(); }
		else if ( (HitPositionX[NHitX]-HitPositionX[NHitX-1]) == 2 ) 
			{ ClusterX[MaxNClustersX]->ExtendOne(); ClusterX[MaxNClustersX]->ExtendOne(); }
		else 
			{ MaxNClustersX++; ClusterX[MaxNClustersX]->Start(HitPositionX[NHitX]); }
		}
	if (NHitsX>0) MaxNClustersX++;
	// Y
	MaxNClustersY=0;
	ClusterY[MaxNClustersY]->Start(HitPositionY[0]);
	for (int NHitY=1; NHitY<NHitsY; NHitY++)
		{
		if ( (HitPositionY[NHitY]-HitPositionY[NHitY-1]) == 1 ) 
			{ ClusterY[MaxNClustersY]->ExtendOne(); }
		else if ( (HitPositionY[NHitY]-HitPositionY[NHitY-1]) == 2 ) 
			{ ClusterY[MaxNClustersY]->ExtendOne(); ClusterY[MaxNClustersY]->ExtendOne(); }
		else 
			{ MaxNClustersY++; ClusterY[MaxNClustersY]->Start(HitPositionY[NHitY]); }
		}
	if (NHitsY>0) MaxNClustersY++;
	// Fill raw cluster histos
	for (int NClusterX=0; NClusterX<MaxNClustersX; NClusterX++)
		{
		ClusterPositionXHist->Add(IdCluAllRaw,ClusterX[NClusterX]->Center);
		ClusterSizeXHist->Add(IdCluAllRaw,ClusterX[NClusterX]->Size);
		}
	for (int NClusterY=0; NClusterY<MaxNClustersY; NClusterY++)
		{
		ClusterPositionYHist->Add(IdCluAllRaw,ClusterY[NClusterY]->Center);
		ClusterSizeYHist->Add(IdCluAllRaw,ClusterY[NClusterY]->Size);
		}
	// Purge or Break Clusters with large size ( ClusterBreak added 2021-Feb )
	int MaxNClustersXRaw=MaxNClustersX, MaxNClustersYRaw=MaxNClustersY;
	for (int NClusterX=0; NClusterX<MaxNClustersXRaw; NClusterX++) if ( ClusterX[NClusterX]->Size > AParameters->ClusterSizeLimit )
		{
		if (AParameters->ClusterBreak==0)
			{
			ClusterTemp=ClusterX[NClusterX]; ClusterX[NClusterX]=ClusterX[MaxNClustersX-1]; ClusterX[MaxNClustersX-1]=ClusterTemp;
			MaxNClustersX--; NClusterX--; MaxNClustersXRaw--;
			}
		else if (AParameters->ClusterBreak==1)
			{
			ClusterX[NClusterX]->Size = AParameters->ClusterSizeLimit;
			}
		else if (AParameters->ClusterBreak==2)
			{
			double Shift=ClusterX[NClusterX]->Size/2.0-1, NewSize=AParameters->ClusterSizeLimit;
			ClusterX[MaxNClustersX]->Center = ClusterX[NClusterX]->Center+Shift;
			ClusterX[MaxNClustersX]->Size = NewSize;
			ClusterX[NClusterX]->Center = ClusterX[NClusterX]->Center-Shift;
			ClusterX[NClusterX]->Size = NewSize;
			MaxNClustersX++;
			}
		else if (AParameters->ClusterBreak==3)
			{
			double Shift=ClusterX[NClusterX]->Size/2.0-1, NewSize=AParameters->ClusterSizeLimit;
			ClusterX[MaxNClustersX]->Center = ClusterX[NClusterX]->Center-Shift;
			ClusterX[MaxNClustersX]->Size = NewSize;
			ClusterX[MaxNClustersX+1]->Center = ClusterX[NClusterX]->Center+Shift;
			ClusterX[MaxNClustersX+1]->Size = NewSize;
			ClusterX[NClusterX]->Size = NewSize;
			MaxNClustersX+=2;
			}
		else { printf("Error : Incorrect ClusterBreak : %d (Can be: 0,1,2,3) \nExit\n\n",AParameters->ClusterBreak); exit(5); }
		}
	for (int NClusterY=0; NClusterY<MaxNClustersYRaw; NClusterY++) if ( ClusterY[NClusterY]->Size > AParameters->ClusterSizeLimit )
		{
		if (AParameters->ClusterBreak==0)
			{
			ClusterTemp=ClusterY[NClusterY]; ClusterY[NClusterY]=ClusterY[MaxNClustersY-1]; ClusterY[MaxNClustersY-1]=ClusterTemp;
			MaxNClustersY--; NClusterY--; MaxNClustersYRaw--;
			}
		else if (AParameters->ClusterBreak==1)
			{
			ClusterY[NClusterY]->Size = AParameters->ClusterSizeLimit;
			}
		else if (AParameters->ClusterBreak==2)
			{
			double Shift=ClusterY[NClusterY]->Size/2.0-1, NewSize=AParameters->ClusterSizeLimit;
			ClusterY[MaxNClustersY]->Center = ClusterY[NClusterY]->Center+Shift;
			ClusterY[MaxNClustersY]->Size = NewSize;
			ClusterY[NClusterY]->Center = ClusterY[NClusterY]->Center-Shift;
			ClusterY[NClusterY]->Size = NewSize;
			MaxNClustersY++;
			}
		else if (AParameters->ClusterBreak==3)
			{
			double Shift=ClusterY[NClusterY]->Size/2.0-1, NewSize=AParameters->ClusterSizeLimit;
			ClusterY[MaxNClustersY]->Center = ClusterY[NClusterY]->Center-Shift;
			ClusterY[MaxNClustersY]->Size = NewSize;
			ClusterY[MaxNClustersY+1]->Center = ClusterY[NClusterY]->Center+Shift;
			ClusterY[MaxNClustersY+1]->Size = NewSize;
			ClusterY[NClusterY]->Size = NewSize;
			MaxNClustersY+=2;
			}
		else { printf("Error : Incorrect ClusterBreak : %d (Can be: 0,1,2,3) \nExit\n\n",AParameters->ClusterBreak); exit(5); }
		}
	// Cluster purge for position is not needed nor recommended,
	//	as it distorts the efficiency calculation as well.
	//	It shall be put into the FitFiducial
	// Position Smoother + Set GlobalPos
	for (int NClusterX=0; NClusterX<MaxNClustersX; NClusterX++)
		{
		ClusterX[NClusterX]->Smooth(PositionSmootherX[int(ClusterX[NClusterX]->Size)]);
		ClusterX[NClusterX]->GlobalPos=ConvertLocalToGlobalX(ClusterX[NClusterX]->Center);
		ClusterPositionXHist->Add(IdCluAll,ClusterX[NClusterX]->Center);
		ClusterSizeXHist->Add(IdCluAll,ClusterX[NClusterX]->Size);
		}
	for (int NClusterY=0; NClusterY<MaxNClustersY; NClusterY++)
		{
		ClusterY[NClusterY]->Smooth(PositionSmootherY[int(ClusterY[NClusterY]->Size)]);
		ClusterY[NClusterY]->GlobalPos=ConvertLocalToGlobalY(ClusterY[NClusterY]->Center);
		ClusterPositionYHist->Add(IdCluAll,ClusterY[NClusterY]->Center);
		ClusterSizeYHist->Add(IdCluAll,ClusterY[NClusterY]->Size);
		}
	
	if ( MaxNClustersX > AParameters->NClustersLimit ) MaxNClustersX=0;
	if ( MaxNClustersY > AParameters->NClustersLimit ) MaxNClustersY=0;
	}

double ChamberClass::ConvertLocalToGlobalX(double LocalX)
	{
	return (LocalX+0.5)*PadSizeX+PositionX;
	}

double ChamberClass::ConvertLocalToGlobalY(double LocalY)
	{
	return (LocalY+0.5)*PadSizeY+PositionY;
	}

double ChamberClass::ConvertGlobalToLocalX(double GlobalX)
	{
	return (GlobalX-PositionX)/PadSizeX-0.5;
	}
	
double ChamberClass::ConvertGlobalToLocalY(double GlobalY)
	{
	return (GlobalY-PositionY)/PadSizeY-0.5;
	}

bool ChamberClass::IsTrackCrossed(TrackClass* Track)
	{
	double ThisTrackAtX = ConvertGlobalToLocalX(Track->GetXAtZ(PositionZ));
	double ThisTrackAtY = ConvertGlobalToLocalY(Track->GetYAtZ(PositionZ));
	// IDEA ! check with larger halo ? eg 0.5 ?
	bool ThisTrackCrossed = ( ThisTrackAtX>0 && ThisTrackAtX<(SizeX-1) && ThisTrackAtY>0 && ThisTrackAtY<(SizeY-1) );
	//printf("Chamber %s, track crossed at local : %6.2lf,%6.2lf\n",Name,TrackAtX,TrackAtY);
	return ThisTrackCrossed;
	}

void ChamberClass::FindClusterAtTrack(TrackClass* Track)
	{
	ClusterOk=false; TrackClusterX=-1; TrackClusterY=-1;
	IsTrackCrossed(Track);
	if (!TrackCrossed) return;
	// Check closest Cluster in X and Y
	DistX=AParameters->DistMax*2; DistY=AParameters->DistMax*2;
	for (int NClusterX=0; NClusterX<MaxNClustersX; NClusterX++)
		{
		Dist = ClusterX[NClusterX]->Center - TrackAtX;
		if (fabs(Dist)<fabs(DistX)) { DistX=Dist; TrackClusterX=NClusterX; }
		}
	for (int NClusterY=0; NClusterY<MaxNClustersY; NClusterY++)
		{
		Dist = ClusterY[NClusterY]->Center - TrackAtY;
		if (fabs(Dist)<fabs(DistY)) { DistY=Dist; TrackClusterY=NClusterY; }
		}
	if (fabs(DistX)>AParameters->DistMax) { TrackClusterX=-1; ClusterXOk=false; } else ClusterXOk=true;
	if (fabs(DistY)>AParameters->DistMax) { TrackClusterY=-1; ClusterYOk=false; } else ClusterYOk=true;
	ClusterOk = ( ClusterXOk && ClusterYOk );
	}

void ChamberClass::GoodTrack(TrackClass* Track)
	{
	// Reset connected variables
	ClusterOk=false; HitFound=false;
	// Compute basic values
	TrackAtX = ConvertGlobalToLocalX(Track->GetXAtZ(PositionZ));
	TrackAtY = ConvertGlobalToLocalY(Track->GetYAtZ(PositionZ));
	TrackSlopeX = Track->GetSlopeX();
	TrackSlopeY = Track->GetSlopeY();
	AdcScaled=Adc/Track->GetLength();
	TrackCrossed=IsTrackCrossed(Track);
	// Noise check
	for (int NHitX=0; NHitX<NHitsX; NHitX++) if (fabs(HitPositionX[NHitX]-TrackAtX)<AParameters->DistMax) HitXDep->Add(IdHitTrack,HitPositionX[NHitX]);
	for (int NHitX=0; NHitX<NHitsX; NHitX++) if (fabs(HitPositionX[NHitX]-TrackAtX)>NoiseClearance) HitXDep->Add(IdHitNoise,HitPositionX[NHitX]);
	for (int NPos=0; NPos<(TrackAtX-NoiseClearance); NPos++) HitXDep->Add(IdHitNoiseBase,NPos);
	for (int NPos=int(TrackAtX+NoiseClearance+0.99); NPos<SizeX; NPos++) HitXDep->Add(IdHitNoiseBase,NPos);
	for (int NHitY=0; NHitY<NHitsY; NHitY++) if (fabs(HitPositionY[NHitY]-TrackAtY)<AParameters->DistMax) HitYDep->Add(IdHitTrack,HitPositionY[NHitY]);
	for (int NHitY=0; NHitY<NHitsY; NHitY++) if (fabs(HitPositionY[NHitY]-TrackAtY)>NoiseClearance) HitYDep->Add(IdHitNoise,HitPositionY[NHitY]);
	for (int NPos=0; NPos<(TrackAtY-NoiseClearance); NPos++) HitYDep->Add(IdHitNoiseBase,NPos);
	for (int NPos=int(TrackAtX+NoiseClearance+0.99); NPos<SizeY; NPos++) HitYDep->Add(IdHitNoiseBase,NPos);
	// Continoue only if Track crossed
	if (!TrackCrossed) { ClusterOk=false; return; }
	// Check closest Cluster in X and Y
	FindClusterAtTrack(Track);
	if (ClusterXOk)
		{
		ClusterPositionXHist->Add(IdCluOk,ClusterX[TrackClusterX]->Center);
		ClusterPositionXHist->Add(IdCluOkRaw,ClusterX[TrackClusterX]->CenterRaw);
		ClusterSizePositionXHist->Add(ClusterX[TrackClusterX]->Size,ClusterX[TrackClusterX]->CenterRaw);
		//ClusterSizePositionXHist->Add(ClusterX[TrackClusterX]->Size,ClusterX[TrackClusterX]->Center); // TEST
		}
	if (ClusterYOk)
		{
		ClusterPositionYHist->Add(IdCluOk,ClusterY[TrackClusterY]->Center);
		ClusterPositionYHist->Add(IdCluOkRaw,ClusterY[TrackClusterY]->CenterRaw);
		ClusterSizePositionYHist->Add(ClusterY[TrackClusterY]->Size,ClusterY[TrackClusterY]->CenterRaw);
		//ClusterSizePositionYHist->Add(ClusterY[TrackClusterY]->Size,ClusterY[TrackClusterY]->Center); // TEST
		}
	// Check if Hit can be found at good position
	HitFoundX=false; HitFoundY=false;
	for (int NHitX=0; NHitX<NHitsX; NHitX++) if (fabs(HitPositionX[NHitX]-TrackAtX)<AParameters->DistMax) HitFoundX=true;
	for (int NHitY=0; NHitY<NHitsY; NHitY++) if (fabs(HitPositionY[NHitY]-TrackAtY)<AParameters->DistMax) HitFoundY=true;
	HitFound = HitFoundX && HitFoundY;
	// Fill histograms
	PositionDep->Add(IdTrackCross,TrackAtX,TrackAtY);
	AngleDep->Add(IdTrackCross,TrackSlopeX,TrackSlopeY);
	AdcDep->Add(IdTrackCross,Adc);
	AdcDep->Add(IdTrackAdcScaledSum,AdcScaled);
	PosAngXDep->Add(IdTrackCross,TrackAtX,TrackSlopeX);
	PosAngYDep->Add(IdTrackCross,TrackAtY,TrackSlopeY);
	TimeDep->Add(IdTrackCross,DateTimeString);
	if (PositionDep->Z(IdFiducial,TrackAtX,TrackAtY)>0.5) TriggerEffSlope->Add(IdTesN,sqrt(TrackSlopeX*TrackSlopeX+TrackSlopeY*TrackSlopeY));
	// Fill AdcSums
	PositionDep->Add(IdTrackAdcSum,TrackAtX,TrackAtY,Adc);
	PositionDep->Add(IdTrackAdcScaledSum,TrackAtX,TrackAtY,AdcScaled);
	AngleDep->Add(IdTrackAdcSum,TrackSlopeX,TrackSlopeY,Adc);
	AngleDep->Add(IdTrackAdcScaledSum,TrackSlopeX,TrackSlopeY,AdcScaled);
	PosAngXDep->Add(IdTrackAdcSum,TrackAtX,TrackSlopeX,Adc);
	PosAngXDep->Add(IdTrackAdcScaledSum,TrackAtX,TrackSlopeX,AdcScaled);
	PosAngYDep->Add(IdTrackAdcSum,TrackAtY,TrackSlopeY,Adc);
	PosAngYDep->Add(IdTrackAdcScaledSum,TrackAtY,TrackSlopeY,AdcScaled);
	TimeDep->Add(IdTrackAdcSum,DateTimeString,Adc);
	TimeDep->Add(IdTrackAdcScaledSum,DateTimeString,AdcScaled);
	// Fill conditionals
	if (ClusterOk)
		{
		PositionDep->Add(IdClusterOk,TrackAtX,TrackAtY);
		AngleDep->Add(IdClusterOk,TrackSlopeX,TrackSlopeY);
		AdcDep->Add(IdClusterOk,Adc);
		PosAngXDep->Add(IdClusterOk,TrackAtX,TrackSlopeX);
		PosAngYDep->Add(IdClusterOk,TrackAtY,TrackSlopeY);
		TimeDep->Add(IdClusterOk,DateTimeString);
		Resolution->Add(DistX,DistY);
		PositionDep->Add(IdShiftXSum,TrackAtX,TrackAtY,DistX);
		PositionDep->Add(IdShiftYSum,TrackAtX,TrackAtY,DistY);
		PositionTune->Add(IdDeltaXN,TrackSlopeX); PositionTune->Add(IdDeltaXSum,TrackSlopeX,DistX); 
		PositionTune->Add(IdDeltaYN,TrackSlopeY); PositionTune->Add(IdDeltaYSum,TrackSlopeY,DistY);
		AngleDep->Add(IdShiftXSum,TrackSlopeX,TrackSlopeY,DistX);
		AngleDep->Add(IdShiftYSum,TrackSlopeX,TrackSlopeY,DistY);
		PosAngXDep->Add(IdShiftXSum,TrackAtX,TrackSlopeX,DistX);
		PosAngYDep->Add(IdShiftYSum,TrackAtY,TrackSlopeY,DistY);
		}
	if (Triggered)
		{
		PositionDep->Add(IdTriggerOk,TrackAtX,TrackAtY);
		AngleDep->Add(IdTriggerOk,TrackSlopeX,TrackSlopeY);
		AdcDep->Add(IdTriggerOk,Adc);
		TimeDep->Add(IdTriggerOk,DateTimeString);
		if (PositionDep->Z(IdFiducial,TrackAtX,TrackAtY)>0.5) TriggerEffSlope->Add(IdTesOk,sqrt(TrackSlopeX*TrackSlopeX+TrackSlopeY*TrackSlopeY));
		}
	if (HitFound)
		{
		PositionDep->Add(IdHitFound,TrackAtX,TrackAtY);
		AngleDep->Add(IdHitFound,TrackSlopeX,TrackSlopeY);
		AdcDep->Add(IdHitFound,Adc);
		TimeDep->Add(IdHitFound,DateTimeString);
		}
	}

void ChamberClass::Print()
	{
	printf("Chamber: %s\n",Name);
	printf("\tPositionXYZ = %g %g %g\n",PositionX,PositionY,PositionZ);
	printf("\tPositionShiftXYZ: %6.2lf, %6.2lf, %6.2lf\n",PositionShiftX,PositionShiftY,PositionShiftZ);
	printf("\tTrackingEfficiency = %5.2lf%%, (%g/%g)\n",EfficiencyTracking*100.,PositionDep->Integral(IdClusterOk),PositionDep->Integral(IdTrackCross));
	printf("\tTriggerEfficiency  = %5.2lf%%, (%g/%g)\n",EfficiencyTrigger*100.,PositionDep->Integral(IdTriggerOk),PositionDep->Integral(IdTrackCross));
	printf("\tEffGood  = %5.2lf%% \t EffSize = %5.1lf (%2.0lf%%)\n",EffGood*100.,EffSize,EffSize*EffSize/double(SizeX*SizeY)*100.);
	printf("\tResolution [PadUnits] : X: %lf , Y:% lf\n",ResolutionXSigma,ResolutionYSigma);
	printf("\tPadSize [mm] : X: %g , Y: %g\n",PadSizeX,PadSizeY);
	// TEST
	//HitXDep->YAverage(IdHitNoise),HitXDep->YAverage(0),HitXDep->YAverage(IdHitNoiseBase)
	printf("\tNoisiness : %g,%g,%g  ,  %g,%g,%g\n", HitXDep->YAverage(IdHitNoise),HitXDep->YAverage(0),HitXDep->YAverage(IdHitNoiseBase), HitYDep->YAverage(IdHitNoise),HitYDep->YAverage(0),HitYDep->YAverage(IdHitNoiseBase));
	}

void ChamberClass::ComputeResults()
	{
	// Efficiency on the good parts
	double SumEff=0, SumN=0;
	for (int x=0; x<PositionDep->MaxNXBins; x++) for (int y=0; y<PositionDep->MaxNYBins; y++)
		if (PositionDep->ZAt(IdFiducial,x,y)>0.5) { SumEff+=PositionDep->ZAt(IdClusterOk,x,y); SumN+=PositionDep->ZAt(IdTrackCross,x,y); }
	if (SumN>1) EffGood = SumEff/SumN; else EffGood=0;
	// Differencian Efficiencies
	AngleDep->CopyFromTo(IdClusterOk,IdTrackingEff);
	AngleDep->DivideMapWithAvgCutElse(IdTrackingEff,IdTrackCross,3,100,EffGood,10);
	PositionDep->CopyFromTo(IdClusterOk,IdTrackingEff);
	PositionDep->DivideMapWithAvgCutElse(IdTrackingEff,IdTrackCross,1,100,EffGood,10);
	TimeDep->CopyFromTo(IdClusterOk,IdTrackingEff);
	TimeDep->ModifyHistWith(IdTrackingEff,IdTrackCross,FunctionDivide);
	// Compute scalar efficiencies
	EfficiencyTracking= PositionDep->Integral(IdClusterOk) / PositionDep->Integral(IdTrackCross);
	EfficiencyTrigger = PositionDep->Integral(IdTriggerOk) / PositionDep->Integral(IdTrackCross);
	EffSize = sqrt(PositionDep->Integral(IdTrackingEff)*PositionDep->XBinSize*PositionDep->YBinSize);
	// Other dependencies	
	TimeDep->RateFromTo(IdNTriggers,IdTriggerRate);
	GainAdc=AdcDep->Mean(IdTrackAdcScaledSum);
	TriggerEffSlope->CopyFromTo(IdTesOk,IdTesEff);
	TriggerEffSlope->ModifyHistWith(IdTesEff,IdTesN,FunctionDivide);
	FitTriggerEffSlope();
	// Resolution and Shifts
	ComputePositionShifts();
	// Fiducial
	SuggestFiducial();
	}

double TesF(double q) { double qq=q*q; return (1.0/(1.0+qq) + 0.27/(qq+0.30/qq)); }
double TesE(double Q, double m) { return TesF(Q/sqrt(1.0+m*m)); }

void ChamberClass::FitTriggerEffSlope()
	{
	if (TriggerEffSlope->Integral(IdTesOk)<1000) 
		{ AngleDep->Fill(IdTriggerEff,EfficiencyTrigger); return; }
	char	TempFileName[50];
	int	RandomNumber=rand(); // Random generation is seeded by Tracking:IdNumber 
	sprintf(TempFileName,"Temp/Tes-%d.temp",RandomNumber);
	TriggerEffSlope->WriteToNewFile(TempFileName);
	char Command[200];
	sprintf(Command,"gnuplot -e \"TesFile='%s'\" Source/FitTriggerEffSlope.gp",TempFileName);
	system(Command);
	FILE * TempFile = fopen(TempFileName,"r");
	fscanf(TempFile,"%lf",&TesQ);
	for (int NXBin=0; NXBin<AngleDep->MaxNXBins; NXBin++) for (int NYBin=0; NYBin<AngleDep->MaxNYBins; NYBin++)
		AngleDep->Map[IdTriggerEff][NXBin][NYBin]= TesF( TesQ/sqrt(1.0+AngleDep->XAt(NXBin)*AngleDep->XAt(NXBin)+AngleDep->YAt(NYBin)*AngleDep->YAt(NYBin)) );
	sprintf(Command,"rm %s",TempFileName);
	system(Command);
	}

void ChamberClass::ComputePositionShifts()
	{
	if (Resolution->Integral()<1000) return;
	HistogramClass	* TempHist = new HistogramClass();
	Resolution->GetProjectionToX(TempHist);
	ResolutionXSigma=TempHist->PeakSigma();
	PositionShiftX=TempHist->Median()*PadSizeX;
	Resolution->GetProjectionToY(TempHist);
	ResolutionYSigma=TempHist->PeakSigma();
	PositionShiftY=TempHist->Median()*PadSizeY;
	delete TempHist;
	// Get Z position finetuning
	FitLineClass * ZTuneFit = new FitLineClass();
	for (int NBin=0; NBin<PositionTune->MaxNBins; NBin++)
		if (PositionTune->YAt(IdDeltaXN,NBin)>10)
			ZTuneFit->Add(PositionTune->XAt(NBin),PositionTune->YAt(IdDeltaXSum,NBin)/PositionTune->YAt(IdDeltaXN,NBin));
	ZTuneFit->Fit();
	PositionShiftZ=ZTuneFit->GetSlope()*PadSizeX;
	ZTuneFit->Null();
	for (int NBin=0; NBin<PositionTune->MaxNBins; NBin++)
		if (PositionTune->YAt(IdDeltaYN,NBin)>10)
			ZTuneFit->Add(PositionTune->XAt(NBin),PositionTune->YAt(IdDeltaYSum,NBin)/PositionTune->YAt(IdDeltaYN,NBin));
	ZTuneFit->Fit();
	PositionShiftZ+=ZTuneFit->GetSlope()*PadSizeY;
	PositionShiftZ/=2.0;
	delete ZTuneFit;
	}

void ChamberClass::SuggestFiducial()
	{
	// Suggest Fiducial section
	PositionDep->Fill(IdFiducialNew,1);
	// Suggest Fiducial - cut the borders
	double	FiducialBorderWidth=1.0;
	PositionDep->SetLineXWidth(IdFiducialNew,0,0,FiducialBorderWidth*2);
	PositionDep->SetLineXWidth(IdFiducialNew,SizeX-1,0,FiducialBorderWidth*2);
	PositionDep->SetLineYWidth(IdFiducialNew,0,0,FiducialBorderWidth*2);
	PositionDep->SetLineYWidth(IdFiducialNew,SizeY-1,0,FiducialBorderWidth*2);
	// Suggest Fiducial - from PositionDep Efficiency
	double FiducialsRelativeEfficiencyCut=0.7;
	double FiducialCutWidthFromEfficiency=1.0;
	Histogram2DClass	* TempMap = new Histogram2DClass();
	HistogramClass		* TempXHist = new HistogramClass();
	HistogramClass		* TempYHist = new HistogramClass();
	PositionDep->Export(IdTrackingEff,TempMap);
	TempMap->GetProjectionToX(TempXHist);
	TempXHist->NormIntegral(TempMap->MaxNXBins);
	for (int NX=0; NX<TempXHist->MaxNBins; NX++)
		if (TempXHist->YAt(NX)<FiducialsRelativeEfficiencyCut) PositionDep->SetLineXWidth(IdFiducialNew,TempXHist->XAt(NX),0,FiducialCutWidthFromEfficiency);
	TempMap->GetProjectionToY(TempYHist);
	TempYHist->NormIntegral(TempMap->MaxNYBins);
	for (int NY=0; NY<TempYHist->MaxNBins; NY++)
		if (TempYHist->YAt(NY)<FiducialsRelativeEfficiencyCut) PositionDep->SetLineYWidth(IdFiducialNew,TempYHist->XAt(NY),0,FiducialCutWidthFromEfficiency);
	delete TempMap;	delete TempXHist; delete TempYHist;
	// Suggest Fiducial - from HitPosition Distribution
	double	HitScaleOk=2.0;
	double	FiducialCutWidthFromBadChannel=2;
	for (int NBin=1; NBin<HitXDep->MaxNBins-1; NBin++)
		if ( (HitXDep->YAt(IdHitAll,NBin)<HitXDep->YAt(IdHitAll,NBin-1)/HitScaleOk) || (HitXDep->YAt(IdHitAll,NBin)>HitXDep->YAt(IdHitAll,NBin-1)*HitScaleOk)
			|| (HitXDep->YAt(IdHitAll,NBin)<HitXDep->YAt(IdHitAll,NBin+1)/HitScaleOk) || (HitXDep->YAt(IdHitAll,NBin)>HitXDep->YAt(IdHitAll,NBin+1)*HitScaleOk) )
			PositionDep->SetLineXWidth(IdFiducialNew,HitXDep->XAt(NBin),0,HitXDep->BinSize*FiducialCutWidthFromBadChannel);
	for (int NBin=1; NBin<HitYDep->MaxNBins-1; NBin++)
		if ( (HitYDep->YAt(IdHitAll,NBin)<HitYDep->YAt(IdHitAll,NBin-1)/HitScaleOk) || (HitYDep->YAt(IdHitAll,NBin)>HitYDep->YAt(IdHitAll,NBin-1)*HitScaleOk)
			|| (HitYDep->YAt(IdHitAll,NBin)<HitYDep->YAt(IdHitAll,NBin+1)/HitScaleOk) || (HitYDep->YAt(IdHitAll,NBin)>HitYDep->YAt(IdHitAll,NBin+1)*HitScaleOk) )
			PositionDep->SetLineYWidth(IdFiducialNew,HitYDep->XAt(NBin),0,HitYDep->BinSize*FiducialCutWidthFromBadChannel);
	
	}

// File operations

bool ChamberClass::ReadBasicsFromFile(FILE* InFile)
	{
	char	ChamberMark[20],Type[20];
	fscanf(InFile,"%s",ChamberMark);
	if (strcmp(ChamberMark,"ChamberBasics")!=0) { printf("Incorrect Mark : %s \n",ChamberMark); exit(1); }
	fscanf(InFile,"%s",Type);
	while ( strcmp(Type,"End")!=0 )
		{
		if (strcmp(Type,"Name")==0)
			fscanf(InFile,"%s",Name);
		else if (strcmp(Type,"Size")==0)		fscanf(InFile,"%d %d",&SizeX,&SizeY);
		else if (strcmp(Type,"PosZ")==0)		fscanf(InFile,"%lf ",&PositionZ);
		else if (strcmp(Type,"PosX")==0)		fscanf(InFile,"%lf ",&PositionX);
		else if (strcmp(Type,"PosY")==0)		fscanf(InFile,"%lf ",&PositionY);
		else if (strcmp(Type,"PadSizeX")==0)		fscanf(InFile,"%lf ",&PadSizeX);
		else if (strcmp(Type,"PadSizeY")==0)		fscanf(InFile,"%lf ",&PadSizeY);
		else if (strcmp(Type,"PadSizeXY")==0)		fscanf(InFile,"%lf %lf ",&PadSizeX,&PadSizeY);
		else if (strcmp(Type,"AdcPedestal")==0)		fscanf(InFile,"%lf ",&AdcPedestal);
		else if (strcmp(Type,"AdcScale")==0)		fscanf(InFile,"%lf ",&AdcScale);
		else if (strcmp(Type,"SideGrow")==0)		fscanf(InFile,"%lf %lf ",&SideGrowX,&SideGrowY);
		else if (strcmp(Type,"Flips")==0)		
			{ fscanf(InFile,"%d",&NFlipsX); FlipPositionX=(double*)calloc(NFlipsX,sizeof(double)); for (int f=0;f<NFlipsX;f++) fscanf(InFile,"%lf",&(FlipPositionX[f]));
			fscanf(InFile,"%d",&NFlipsY); FlipPositionY=(double*)calloc(NFlipsY,sizeof(double)); for (int f=0;f<NFlipsY;f++) fscanf(InFile,"%lf",&(FlipPositionY[f])); }
		else if (strcmp(Type,"MisPositionX")==0)	{ fscanf(InFile,"%d %d %d ",&MisPositionXMin,&MisPositionXMax,&MisPositionXValue); UseMisPositionCorrection=true; }
		else if (strcmp(Type,"MisPositionY")==0)	{ fscanf(InFile,"%d %d %d ",&MisPositionYMin,&MisPositionYMax,&MisPositionYValue); UseMisPositionCorrection=true; }
		else if (strcmp(Type,"BrokenChannelsX")==0)	{ fscanf(InFile,"%d",&NBrokenChannelsX); UseBrokenChannels=true; BrokenChannelX=(int*)calloc(NBrokenChannelsX,sizeof(int)); for (int n=0;n<NBrokenChannelsX;n++) fscanf(InFile,"%d",&(BrokenChannelX[n])); }
		else if (strcmp(Type,"BrokenChannelsY")==0)	{ fscanf(InFile,"%d",&NBrokenChannelsY); UseBrokenChannels=true; BrokenChannelY=(int*)calloc(NBrokenChannelsY,sizeof(int)); for (int n=0;n<NBrokenChannelsY;n++) fscanf(InFile,"%d",&(BrokenChannelY[n])); }
		else { printf("Unrecognized ChamberBasics Type: %s\nExit\n\n",Type); exit(50); }
		fscanf(InFile,"%s",Type);
		}
	// Check if Size is ok
	if (SizeX<1 || SizeY<0) { printf("Size shall be defined in ChamberBasics\nExit\n\n"); exit(50); }
	AllocateMemory();
	// TODO :: Fiducial set : DeadChannels, NoBorders ?
	return true;
	}

void ChamberClass::WriteGeneralInfoToFile(FILE* OutFile)
	{
	fprintf(OutFile,"Name\t%s\n",Name);
	fprintf(OutFile,"SizeXY\t%d %d\n",SizeX,SizeY);
	fprintf(OutFile,"PositionXYZ\t%g %g %g\n",PositionX,PositionY,PositionZ);
	fprintf(OutFile,"Efficiency\t%g\tTrigger %g\n",EfficiencyTracking,EfficiencyTrigger);
	fprintf(OutFile,"AdcPedestal %g\tAdcScale %g\n",AdcPedestal,AdcScale);
	fprintf(OutFile,"Gain %g\n",GainAdc);
	fprintf(OutFile,"TesQ %g\n",TesQ);
	fprintf(OutFile,"End\n\n\n"); // Separator
	}

void ChamberClass::ReadGeneralInfoFromFile(FILE* InFile)
	{
	bool DoRead=true;
	char VarName[50];
	while (DoRead)
		{
		fscanf(InFile,"%s",VarName);
		if (strcmp(VarName,"Name")==0) 			fscanf(InFile,"%s",Name);
		else if (strcmp(VarName,"SizeXY")==0)		fscanf(InFile,"%d %d",&SizeX,&SizeY);
		else if (strcmp(VarName,"PositionXYZ")==0)	fscanf(InFile,"%lf %lf %lf",&PositionX,&PositionY,&PositionZ);
		else if (strcmp(VarName,"Efficiency")==0)	fscanf(InFile,"%lf",&EfficiencyTracking);
		else if (strcmp(VarName,"Trigger")==0)		fscanf(InFile,"%lf",&EfficiencyTrigger);
		else if (strcmp(VarName,"AdcPedestal")==0)	fscanf(InFile,"%lf",&AdcPedestal);
		else if (strcmp(VarName,"AdcScale")==0) 	fscanf(InFile,"%lf",&AdcScale);
		else if (strcmp(VarName,"Gain")==0) 		fscanf(InFile,"%lf",&GainAdc);
		else if (strcmp(VarName,"TesQ")==0) 		fscanf(InFile,"%lf",&TesQ);
		else if (strcmp(VarName,"End")==0)		DoRead=false;
		else printf("WARNING : Unknown  Chamber General Info  type : %s\n",VarName);
		}
	}

int ChamberClass::WriteGnuplotInfo(FILE* OutFile)
	{
	std::string FileContentItem;
	int PosBegin=0,PosEnd=1, Index=0;
	while ( PosEnd > 0)
		{
		PosEnd = FileContents.find_first_of(' ',PosBegin);
		FileContentItem=FileContents.substr(PosBegin,PosEnd-PosBegin);
		fprintf(OutFile,"Index%s=%d\n",FileContentItem.data(),Index);
		PosBegin=PosEnd+1;
		Index++;
		}
	return Index;
	}

void ChamberClass::WriteResultsToFile(FILE* OutFile)
	{
	std::string FileContentItem;
	int PosBegin=0,PosEnd=1;
	fprintf(OutFile,"!Chamber\n");
	fprintf(OutFile,"FileContents %d %s\n",CountWordsInString(FileContents),FileContents.data());
	while ( PosEnd > 0)
		{
		PosEnd = FileContents.find_first_of(' ',PosBegin);
		FileContentItem=FileContents.substr(PosBegin,PosEnd-PosBegin);
		PosBegin=PosEnd+1;
		if (FileContentItem.compare("General")==0) 		WriteGeneralInfoToFile(OutFile);
		else if (FileContentItem.compare("HitXDep")==0) 	HitXDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("HitYDep")==0) 	HitYDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("ClusterPosX")==0) 	ClusterPositionXHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("ClusterPosY")==0) 	ClusterPositionYHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("ClusterSizePosX")==0) ClusterSizePositionXHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("ClusterSizePosY")==0) ClusterSizePositionYHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("ClusterSizeX")==0) 	ClusterSizeXHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("ClusterSizeY")==0) 	ClusterSizeYHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("PositionDepBare")==0) PositionDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("PositionDep")==0) 	PositionDep->WriteToFileGpBars(OutFile);
		else if (FileContentItem.compare("AngleDepBare")==0)	AngleDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("AngleDep")==0) 	AngleDep->WriteToFileGpBars(OutFile);
		else if (FileContentItem.compare("AdcDepBare")==0) 	AdcDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("AdcDep")==0) 		AdcDep->WriteToFileZs(OutFile);
		else if (FileContentItem.compare("PosAngXDep")==0)	PosAngXDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("PosAngYDep")==0)	PosAngYDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("TimeDep")==0)		TimeDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("ResolutionMap")==0)	Resolution->WriteToFile(OutFile);
		else if (FileContentItem.compare("ResolutionX")==0)	Resolution->WriteToFileProjectionToX(OutFile);
		else if (FileContentItem.compare("ResolutionY")==0)	Resolution->WriteToFileProjectionToY(OutFile);
		else if (FileContentItem.compare("PositionTune")==0)	PositionTune->WriteToFile(OutFile);
		else if (FileContentItem.compare("TriggerEffSlope")==0)	TriggerEffSlope->WriteToFile(OutFile);
		else printf("WARNING : Unknown Chamber-FileContent-item : \"%s\"\n",FileContentItem.data());
		}
	}

bool ChamberClass::ReadInfoFromFile(FILE* InFile, bool DoClearStat, bool DoComputeTuning)
	{
	// Read the Detector level info : General AngleDep TrackKhi2 Thp
	char	ChamberMark[20];
	fscanf(InFile,"%s",ChamberMark);
	if (strcmp(ChamberMark,"!Chamber")!=0) { printf("ERROR : Incorrect ChamberMark : %s \nExit\n\n",ChamberMark); exit(1); }
	fscanf(InFile,"%s",ChamberMark);
	if (strcmp(ChamberMark,"FileContents")!=0) { printf("ERROR : Incorrect FileContents mark : %s \nExit\n\n",ChamberMark); exit(1); }
	int NFileContentsRead=0; 
	std::string FileContentsRead;
	char TempString[50];
	fscanf(InFile,"%d",&NFileContentsRead);
	for (int n=0; n<NFileContentsRead; n++)
		{
		fscanf(InFile,"%s",TempString);
		if (FileContentsRead.length()>2) FileContentsRead+=" ";
		FileContentsRead+=TempString;
		}
	std::string FileContentItem;
	int PosBegin=0,PosEnd=1;
	while ( PosEnd > 0)
		{
		PosEnd = FileContentsRead.find_first_of(' ',PosBegin);
		FileContentItem=FileContentsRead.substr(PosBegin,PosEnd-PosBegin);
		//printf("This: %s\n",FileContentItem.c_str());	
		PosBegin=PosEnd+1;
		if (FileContentItem.compare("General")==0) 		ReadGeneralInfoFromFile(InFile);
		else if (FileContentItem.compare("HitXDep")==0)		HitXDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("HitYDep")==0)		HitYDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("ClusterPosX")==0)	ClusterPositionXHist->ReadFromFile(InFile);
		else if (FileContentItem.compare("ClusterPosY")==0)	ClusterPositionYHist->ReadFromFile(InFile);
		else if (FileContentItem.compare("ClusterSizePosX")==0)	ClusterSizePositionXHist->ReadFromFile(InFile);
		else if (FileContentItem.compare("ClusterSizePosY")==0)	ClusterSizePositionYHist->ReadFromFile(InFile);
		else if (FileContentItem.compare("ClusterSizeX")==0)	ClusterSizeXHist->ReadFromFile(InFile);
		else if (FileContentItem.compare("ClusterSizeY")==0)	ClusterSizeYHist->ReadFromFile(InFile);
		else if (FileContentItem.compare("PositionDepBare")==0)	PositionDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("PositionDep")==0)	SkipBlockInFile(InFile); // PositionDep-GpBars
		else if (FileContentItem.compare("AngleDepBare")==0)	AngleDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("AngleDep")==0)	SkipBlockInFile(InFile); // AngleDep-GpBars
		else if (FileContentItem.compare("AdcDepBare")==0)	AdcDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("AdcDep")==0)		SkipBlockInFile(InFile); // AdcDep-Zs
		else if (FileContentItem.compare("PosAngXDep")==0)	PosAngXDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("PosAngYDep")==0)	PosAngYDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("TimeDep")==0)		TimeDep->ReadFromFile(InFile);
		else if (FileContentItem.compare("ResolutionMap")==0)	Resolution->ReadFromFile(InFile);
		else if (FileContentItem.compare("ResolutionX")==0)	SkipBlockInFile(InFile); // Resolution-ProjectionX
		else if (FileContentItem.compare("ResolutionY")==0)	SkipBlockInFile(InFile); // Resolution-ProjectionY
		else if (FileContentItem.compare("PositionTune")==0)	PositionTune->ReadFromFile(InFile);
		else if (FileContentItem.compare("TriggerEffSlope")==0)	TriggerEffSlope->ReadFromFile(InFile);
		else printf("WARNING : Unknown Chamber-FileContent-item : %s\n",FileContentItem.data());
		}
	// Do accrding to the input flags
	if (DoComputeTuning) ComputeTuning();
	if (DoClearStat) ClearStatistics();
	return true;
	}

void ChamberClass::ComputeTuning()
	{
	if (Verbose>2) printf("Chamber:%s : ComputeTuning\n",Name);
	double	SmootherIntegralLimit = AParameters->SmoothingPeriodLength/AParameters->ClusterPositionBinSize*100;
	double	IntegralThis,IntegralM2,IntegralP2;
	int	ChoosenSize=-1;

	// TEST : TEMP : for the MTS40 setup the normal Smoothing was not ok
	// 	TODO : this shall be checked in details !
	//	meanwhile this dummy sum works ...
	if (strncmp(Name,"Mt4040",6)==0)
		{
		ClusterSizePositionXHist->PutSumOfThese(0,NPositionSmoothers-1,&(CountArray[1]));
		for (int CluSize=1; CluSize<NPositionSmoothers; CluSize++) ClusterSizePositionXHist->CopyFromTo(0,CluSize);
		ClusterSizePositionYHist->PutSumOfThese(0,NPositionSmoothers-1,&(CountArray[1]));
		for (int CluSize=1; CluSize<NPositionSmoothers; CluSize++) ClusterSizePositionYHist->CopyFromTo(0,CluSize);
		}
	if (Verbose>3) printf("Smoothers by size:");
	for (int CluSize=1; CluSize<NPositionSmoothers; CluSize++)
		{
		// X
		IntegralThis=ClusterSizePositionXHist->Integral(CluSize);
		IntegralM2=ClusterSizePositionXHist->Integral(CluSize-2);
		IntegralP2=ClusterSizePositionXHist->Integral(CluSize+2);
		if (IntegralThis > SmootherIntegralLimit) ChoosenSize=CluSize;
		else if (IntegralM2>=IntegralP2 && IntegralM2>SmootherIntegralLimit) ChoosenSize=CluSize-2;
		else if (IntegralP2>=IntegralM2 && IntegralP2>SmootherIntegralLimit) ChoosenSize=CluSize+2;
		else ChoosenSize=-1;
		if (Verbose>3) printf("X:%d->%d, ",CluSize,ChoosenSize);
		if (ChoosenSize>0) PositionSmootherX[CluSize]->Set(ClusterSizePositionXHist,ChoosenSize,AParameters->SmoothingPeriodLength,NFlipsX,FlipPositionX);
		// Y
		IntegralThis=ClusterSizePositionYHist->Integral(CluSize);
		IntegralM2=ClusterSizePositionYHist->Integral(CluSize-2);
		IntegralP2=ClusterSizePositionYHist->Integral(CluSize+2);
		if (IntegralThis > SmootherIntegralLimit) ChoosenSize=CluSize;
		else if (IntegralM2>=IntegralP2 && IntegralM2>SmootherIntegralLimit) ChoosenSize=CluSize-2;
		else if (IntegralP2>=IntegralM2 && IntegralP2>SmootherIntegralLimit) ChoosenSize=CluSize+2;
		else ChoosenSize=-1;
		if (Verbose>3) printf("Y:%d->%d, ",CluSize,ChoosenSize);
		if (ChoosenSize>0) PositionSmootherY[CluSize]->Set(ClusterSizePositionYHist,ChoosenSize,AParameters->SmoothingPeriodLength,NFlipsY,FlipPositionY);
		}
	if (Verbose>3) printf("\n");
	ComputePositionShifts();
	PositionX-=PositionShiftX;
	PositionY-=PositionShiftY;
	PositionZ+=PositionShiftZ;
	//printf("PositionShiftXYZ: %4.1lf %4.1lf %4.1lf\n",PositionShiftX,PositionShiftY,PositionShiftZ);
	// Fiducial
	PositionDep->CopyFromTo(IdFiducialNew,IdFiducial);
	FiducialConvertMapToXY();
	}

bool ChamberClass::AddInfoFromChamber(ChamberClass* OtherChamber, bool DoComputeResults)
	{
	if (SizeX!=OtherChamber->SizeX || SizeY!=OtherChamber->SizeY) return false;
	// TODO : Here I assume that the "*Similar*" functions are usable, it has to be rethought
	HitXDep->AddSimilarMultiHist(OtherChamber->HitXDep);
	HitYDep->AddSimilarMultiHist(OtherChamber->HitYDep);
	ClusterPositionXHist->AddSimilarMultiHist(OtherChamber->ClusterPositionXHist);
	ClusterPositionYHist->AddSimilarMultiHist(OtherChamber->ClusterPositionYHist);
	ClusterSizePositionXHist->AddSimilarMultiHist(OtherChamber->ClusterSizePositionXHist);
	ClusterSizePositionYHist->AddSimilarMultiHist(OtherChamber->ClusterSizePositionYHist);
	PositionDep->AddSimilarMultiMap(OtherChamber->PositionDep);
	AngleDep->AddSimilarMultiMap(OtherChamber->AngleDep);
	AdcDep->AddSimilarMultiHist(OtherChamber->AdcDep);
	PosAngXDep->AddSimilarMultiMap(OtherChamber->PosAngXDep);
	PosAngYDep->AddSimilarMultiMap(OtherChamber->PosAngYDep);
	TimeDep->AddMultiTimeHist(OtherChamber->TimeDep);
	Resolution->Add2DHist(OtherChamber->Resolution);
	PositionTune->AddSimilarMultiHist(OtherChamber->PositionTune);
	TriggerEffSlope->AddSimilarMultiHist(OtherChamber->TriggerEffSlope);
	if (DoComputeResults) ComputeResults();
	return true;
	}

bool ChamberClass::CheckReadWriteConsystency()
	{
	FILE*	TempFile;
	TempFile=fopen("TempFile.temp","w");
	WriteResultsToFile(TempFile);
	fclose(TempFile);
	TempFile=fopen("TempFile.temp","r");
	ChamberClass * ClonedChamber = new ChamberClass(SizeX,SizeY);
	ClonedChamber->ReadInfoFromFile(TempFile);
	fclose(TempFile);
	// TODO : the real comparision shall come here
	delete TempFile;
	delete ClonedChamber;
	return true;
	}

void ChamberClass::PrintEvent(TrackClass* TrackShow)
	{
	int PadX[SizeX],PadY[SizeY];
	char VisualBit[9]=".x-y*X*Y";
	for (int NPad=0; NPad<SizeX; NPad++) PadX[NPad]=0;
	for (int NHit=0; NHit<NHitsX; NHit++) PadX[HitPositionX[NHit]]=1;
	for (int NPad=0; NPad<SizeY; NPad++) PadY[NPad]=2;
	for (int NHit=0; NHit<NHitsY; NHit++) PadY[HitPositionY[NHit]]=3;
	// Visualize TrackShow
	if (TrackShow->Good)
		{
		TrackAtX = ConvertGlobalToLocalX(TrackShow->GetXAtZ(PositionZ));
		TrackAtY = ConvertGlobalToLocalY(TrackShow->GetYAtZ(PositionZ));
		if (TrackAtX>=0 && TrackAtX<SizeX && TrackAtY>=0 && TrackAtY<SizeY)
			{ PadX[int(TrackAtX+0.49)]+=4; PadY[int(TrackAtY+0.49)]+=4; }
		}
	// Print the AsciiArt and info
	for (int NPad=0; NPad<SizeX; NPad++) printf("%c",VisualBit[PadX[NPad]]);
	printf("   ");
	for (int NPad=0; NPad<SizeY; NPad++) printf("%c",VisualBit[PadY[NPad]]);
	printf("\tCluX:%d, CluY:%d, Trig:%d, Adc:%4.0lf (%4d)\n",MaxNClustersX,MaxNClustersY,TriggerBit,Adc,AdcRaw);
	}


// ClusterClass functions

ClusterClass::ClusterClass()
	{
	Size=0;
	}

void ClusterClass::Start(double InCenter)
	{
	Center=InCenter;
	Size=1;
	}

void ClusterClass::ExtendOne()
	{
	Center+=0.5;
	Size++;
	}

void ClusterClass::Smooth(PositionSmootherClass * PosSmoother)
	{
	CenterRaw=Center;
	Center=PosSmoother->Smooth(CenterRaw);
	}


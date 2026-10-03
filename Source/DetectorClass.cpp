////////////////////////////////////////////////////////////////////////
// Detector Class - implementation
//      version : 2022-12-15
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "DetectorClass.h"
#include "ZonedMapsClass.h"
#include "TrackClass.h"

DetectorClass::DetectorClass()
	{
	ResetVariables();
	DateTimeString=&NullChar;
	pEventTimeDiffUs=&NullLong;
	FileContents="General AngleDep TimeDep Rate Coinc DiffTime CorrelDT CorrelDA TimeDepSum Thp Hv";
	}

DetectorClass::DetectorClass(int InNChambers, ChamberClass** InChamber, char* InName) : DetectorClass()
	{
	// First calls the default constructor via delegation
	SetChambers(InNChambers,InChamber);
	SetName(InName);
	}

void DetectorClass::ResetVariables()
	{
	sprintf(Name,"Detector");
	Verbose=0;
	NChambers=0;
	AngleDep=NULL;
	AParameters=NULL;
	Chamber=NULL;
	Thp=NULL;
	HvInfo=NULL;
	Rates=NULL;
	TimeDep=NULL;
	TimeDepSum=NULL;
	CoincLevelHist=NULL;
	DiffTimeHist=NULL;
	CorrelMapChambersDataTrigger=NULL;
	CorrelMapChambersDataAdc=NULL;
	// Reset Statistics info
	TotalTimeSec=0; TotalTimeLiveSec=0; TimeCorrectionSec=0;
	DeadTimePerEventUs=0; DeadTimeZipSec=0;
	NEventsPerFile=1;
	DeadTimePerEventUsEstimation=0; DeadTimeZipPerEventUsEstimation=0; DeadTimeZipPerEventUsApprox=0;
	EventRate=0; TrackRate=0;
	NEvents=0; NTracks=0; NFiles=0;
	FluxMid=0; MaxNThps=0;
	AngleViewX=0; AngleViewY=0;
	TriggerCoinc=1;
	sprintf(ResFilenaming,"ResFilenaming");
	}

void DetectorClass::AllocateMemory()
	{
	AngleDep = new MultiMapClass(6,Chamber[0]->AngleDep);
	AngleDep->SetTheName((char*)"AngleDep");
	AngleDep->SetName(IdTrackCross,(char*)"TrackCross");	AngleDep->SetName(IdTriggerEff,(char*)"TriggerEff");
	AngleDep->SetName(IdTrackingEff,(char*)"TrackingEff");	AngleDep->SetName(IdAcceptance,(char*)"Acceptance");
	AngleDep->SetName(IdEffArea,(char*)"EffArea");		AngleDep->SetName(IdFlux,(char*)"Flux");
	Rates = new MultiHistDynTimeClass(4,AParameters->TimeDepBinSizeSeconds,(char*)"Rates");
	Rates->SetAutoScale(AParameters->MaxNTimeBins);
	TimeDep = new MultiHistDynTimeClass(6,AParameters->TimeDepBinSizeSeconds,(char*)"TimeDep");
	TimeDep->SetName(IdTrackCross,(char*)"TrackCross");	TimeDep->SetName(IdTriggerEff,(char*)"TriggerEff");
	TimeDep->SetName(IdTrackingEff,(char*)"TrackingEff");	TimeDep->SetName(IdAcceptance,(char*)"None");
	TimeDep->SetName(IdEffArea,(char*)"None");		TimeDep->SetName(IdFlux,(char*)"Flux");
	TimeDep->SetAutoScale(AParameters->MaxNTimeBins);
	TdsCh=3+2+2+1;
	TimeDepSum = new MultiHistDynTimeClass(TdsCh+NChambers,AParameters->TimeDepBinSizeSeconds,(char*)"TimeDepSum");
	TimeDepSum->SetName(IdTdsT,(char*)"T"); TimeDepSum->SetName(IdTdsH,(char*)"H"); TimeDepSum->SetName(IdTdsP,(char*)"P");
	TimeDepSum->SetName(IdTdsRateEvent,(char*)"RateEvent"); TimeDepSum->SetName(IdTdsRateTrack,(char*)"RateTrack");
	TimeDepSum->SetName(IdTdsUMon,(char*)"UMon"); TimeDepSum->SetName(IdTdsIMon,(char*)"IMon");
	TimeDepSum->SetName(IdTdsNSum,(char*)"NSum");
	char NameTemp[20]="";
	for (int NChamber=0; NChamber<NChambers; NChamber++) { sprintf(NameTemp,"GainCh%d",NChamber); TimeDepSum->SetName(TdsCh+NChamber,NameTemp); }
	CoincLevelHist = new MultiHistClass(2,0,NChambers,1,(char*)"CoincLevelHist");
	CoincLevelHist->SetName(IdClEvent,(char*)"Event"); CoincLevelHist->SetName(IdClEvent,(char*)"Track");
	DiffTimeHist = new MultiHistClass(2,0,10000,10,(char*)"DiffTimeUsHist");
	DiffTimeHist->SetName(IdClEvent,(char*)"Event"); DiffTimeHist->SetName(IdClEvent,(char*)"Track");
	CorrelMapChambersDataTrigger = new MultiMapClass(NChambers+1,0,NChambers,1,0,NChambers,1,(char*)"CorrelMapChDataTrigger");
	CorrelMapChambersDataAdc     = new MultiMapClass(NChambers+1,0,NChambers,1,0,NChambers,1,(char*)"CorrelMapChDataAdc");
	CoincArea = new MultiMapClass(NChambers+1,AngleDep);
	// idea: MultiMap for expected coincidence level distribution: Sx,Sy;NCh : EffArea (if eff=1.0? to simplify computation)
	}

DetectorClass::~DetectorClass()
	{
	if (AngleDep!=NULL) delete AngleDep;
	if (Rates!=NULL) delete Rates;
	if (TimeDep!=NULL) delete TimeDep;
	if (TimeDepSum!=NULL) delete TimeDepSum;
	if (Thp!=NULL) { for (int NThp=0; NThp<MaxNThps; NThp++) delete Thp[NThp]; free(Thp); }
	if (HvInfo!=NULL) delete HvInfo;
	if (CoincLevelHist!=NULL) delete CoincLevelHist;
	if (CorrelMapChambersDataTrigger!=NULL) delete CorrelMapChambersDataTrigger;
	if (CorrelMapChambersDataAdc!=NULL) delete CorrelMapChambersDataAdc;
	if (CoincArea!=NULL) delete CoincArea;
	}

void DetectorClass::SetName(char* InName)
	{ sprintf(Name,"%s",InName); }

void DetectorClass::ReadBasics(char* InDetectorBasicsFileName)
	{
	if (strcmp(InDetectorBasicsFileName,"")==0) { printf("Error ! No valid file name was gives for DetectorBasics\nExit\n\n"); exit(1); }
	printf("Reading DetectorBasics from : %s\n",InDetectorBasicsFileName);
	if (AParameters==NULL) AParameters = new AnalysisParametersClass();
	// Open file
	FILE * DetectorBasicsFile=fopen(InDetectorBasicsFileName,"r");
	if (DetectorBasicsFile==NULL) { printf("Error ! Cannot read DetectorBasics file : %s\nExit\n",InDetectorBasicsFileName); exit(1); }
	// Read info
	char	ParName[50]; int NTemp;
	while (!feof(DetectorBasicsFile))
		{
		sprintf(ParName,"-"); // reset ParName 
		fscanf(DetectorBasicsFile,"%s",ParName);
		if (strlen(ParName)<3) continue;
		if (ParName[0]=='#') {SkipLine(DetectorBasicsFile); continue;}
		if (strcmp(ParName,"DetectorName")==0)
			fscanf(DetectorBasicsFile,"%s",Name);
		else if (strcmp(ParName,"NChambers")==0)
			{
			fscanf(DetectorBasicsFile,"%d",&NChambers);
			CreateChambers(NChambers);
			}
		else if (strcmp(ParName,"NThps")==0)
			{
			fscanf(DetectorBasicsFile,"%d",&MaxNThps);
			Thp = (ThpClass**) calloc(MaxNThps,sizeof(ThpClass*));
			for (int NThp=0; NThp<MaxNThps; NThp++) 
				Thp[NThp] = new ThpClass(AParameters->TimeDepBinSizeSeconds,AParameters->MaxNTimeBins);
			}
		else if (strcmp(ParName,"HvControl")==0)
			{
			int HvControlInt=0;
			fscanf(DetectorBasicsFile,"%d",&HvControlInt);
			if (HvControlInt==1) HvInfo = new HvClass(AParameters->TimeDepBinSizeSeconds,AParameters->MaxNTimeBins);
			}
		else if ( (strcmp(ParName,"NAdcs")==0) ||  (strcmp(ParName,"Trigger")==0))
			{ fscanf(DetectorBasicsFile,"%d %*s",&NTemp); for (int n=0;n<NTemp;n++) fscanf(DetectorBasicsFile,"%*d");	}
		else if (strcmp(ParName,"NChecks")==0)
			fscanf(DetectorBasicsFile,"%*d");
		else if (strcmp(ParName,"PadSizeMm")==0)
			{ fscanf(DetectorBasicsFile,"%lf",&PadSizeMm);
			for (int NChamber=0; NChamber<NChambers; NChamber++) { Chamber[NChamber]->PadSizeX=PadSizeMm; Chamber[NChamber]->PadSizeY=PadSizeMm; } }
		else if (strcmp(ParName,"ChamberBasicsInfo")==0)
			for (int NChamber=0; NChamber<NChambers; NChamber++)
				Chamber[NChamber]->ReadBasicsFromFile(DetectorBasicsFile);
		else if (strcmp(ParName,"TriggerCoinc")==0)
			fscanf(DetectorBasicsFile,"%d",&TriggerCoinc);
		else if (strcmp(ParName,"EventFormat")==0) fscanf(DetectorBasicsFile,"%*s ");
		else if (strcmp(ParName,"DeadTime")==0) fscanf(DetectorBasicsFile,"%lf %lf ",&DeadTimePerEventUs,&DeadTimeZipPerEventUsApprox);
		else 
			{ printf("Error ! DetectorBasics file : %s, unrecognized Parameter: %s\n",InDetectorBasicsFileName,ParName); exit(1); }
		}
	fclose(DetectorBasicsFile);
	AllocateMemory();
	FillAcceptanceMap();	
	}

void DetectorClass::SetChambers(int InNChambers, ChamberClass** InChamber)
	{
	if (InNChambers<1) { printf("Warning ! : Detector::NChambers shall be positive\n"); return; }
	NChambers=InNChambers;
	Chamber=InChamber;
	if (AParameters==NULL) SetAnalysisParameters(Chamber[0]->AParameters);
	AllocateMemory();
	//FillAcceptanceMap();
	}

void DetectorClass::SetAnalysisParameters(AnalysisParametersClass * InAParameters)
	{
	AParameters=InAParameters;
	TrackingNHits=AParameters->TrackingNHits;
	}
	
void DetectorClass::SetThp(int InMaxNThps, ThpClass ** InThp)
	{
	MaxNThps=InMaxNThps;
	Thp=InThp;
	}

void DetectorClass::SetHvInfo(HvClass * InHvInfo)
	{
	HvInfo=InHvInfo;
	}

void DetectorClass::CreateChambers(int InNChambers)
	{
	if (AParameters==NULL) AParameters = new AnalysisParametersClass();
	if (InNChambers<1) { printf("Error ! : Detector::NChambers shall be positive\nExit\n\n"); exit(1); }
	NChambers=InNChambers;
	AParameters->SetNChambers(NChambers);
	Chamber = (ChamberClass**) calloc(NChambers,sizeof(ChamberClass*));
	for (int NChamber=0; NChamber<NChambers; NChamber++) Chamber[NChamber] = new ChamberClass();
	for (int NChamber=0; NChamber<NChambers; NChamber++) Chamber[NChamber]->SetAnalysisParameters(AParameters);
	}

void DetectorClass::ClearStatistics()
	{
	// Chamber level
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		Chamber[NChamber]->ClearStatistics();
	// Detector level
	AngleDep->Clear(IdTrackCross);
	for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->ClearAll();
	if (HvInfo!=NULL) HvInfo->TimeDev->ClearAll();
	Rates->ClearAll();
	TimeDep->ClearAll();
	TimeDepSum->ClearAll();
	// Single variables
	TotalTimeSec=0; TotalTimeLiveSec=0;
	NEvents=0; NTracks=0; NFiles=0;
	FluxMid=0;
	// ReSet AParams
	for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->BinSize=AParameters->TimeDepBinSizeSeconds;
	if (HvInfo!=NULL) HvInfo->TimeDev->BinSize=AParameters->TimeDepBinSizeSeconds;
	Rates->BinSize=AParameters->TimeDepBinSizeSeconds;
	TimeDep->BinSize=AParameters->TimeDepBinSizeSeconds;
	
	}

void DetectorClass::SetNTimeBins(int NewNTimeBins)
	{
	for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->SetAutoScale(NewNTimeBins);
	if (HvInfo!=NULL) HvInfo->TimeDev->SetAutoScale(NewNTimeBins);
	Rates->SetAutoScale(NewNTimeBins);
	TimeDep->SetAutoScale(NewNTimeBins);
	for (int NChamber=0; NChamber<NChambers; NChamber++) Chamber[NChamber]->TimeDep->SetAutoScale(NewNTimeBins);
	}

void DetectorClass::AddInfoFromDetector(DetectorClass* SimilarDetector)
	{
	// TODO : Check for similarity !!
	if (NChambers!=SimilarDetector->NChambers) {printf("Warning : Detectors are non similar, therefore not additive\nExit\n\n"); exit(1); }
	// Chamber level
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		Chamber[NChamber]->AddInfoFromChamber(SimilarDetector->Chamber[NChamber],false);
	// Detector level
	AngleDep->AddSimilarMultiMap(SimilarDetector->AngleDep);
	Rates->AddMultiTimeHist(SimilarDetector->Rates);
	TimeDep->AddMultiTimeHist(SimilarDetector->TimeDep);
	if (Thp!=NULL) for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->AddMultiTimeHist(SimilarDetector->Thp[NThp]->TimeDev);
	if (HvInfo!=NULL) HvInfo->TimeDev->AddMultiTimeHist(SimilarDetector->HvInfo->TimeDev);
	CoincLevelHist->AddSimilarMultiHist(SimilarDetector->CoincLevelHist);
	CorrelMapChambersDataAdc->AddSimilarMultiMap(SimilarDetector->CorrelMapChambersDataAdc);
	CorrelMapChambersDataTrigger->AddSimilarMultiMap(SimilarDetector->CorrelMapChambersDataTrigger);
	// Single parameters
	TotalTimeSec+=SimilarDetector->TotalTimeSec;
	TotalTimeLiveSec+=SimilarDetector->TotalTimeLiveSec;
	NEvents+=SimilarDetector->NEvents;
	NTracks+=SimilarDetector->NTracks;
	NFiles+=SimilarDetector->NFiles;
	}

double DetectorClass::CalculateAcceptance(double SlopeX, double SlopeY)
	{
	return ( GetMax(ChamberSizeX*Chamber[0]->PadSizeX-ZEff*fabs(SlopeX),0) * GetMax(ChamberSizeY*Chamber[0]->PadSizeY-ZEff*fabs(SlopeY),0) ) / sqrt(1.0+SlopeX*SlopeX+SlopeY*SlopeY);
	}

void DetectorClass::FillAcceptanceMap()
	{
	// Chamber Size (if all are the same)
	ChamberSizeX=Chamber[0]->SizeX*Chamber[0]->PadSizeX;
	ChamberSizeY=Chamber[0]->SizeY*Chamber[0]->PadSizeY;
	// Max Z with (N-1)-Tracks
	TrackingNHits=AParameters->TrackingNHits;
	ZEff = Chamber[TrackingNHits-1]->PositionZ - Chamber[NChambers-TrackingNHits]->PositionZ;
	// Than the Acceptance is:
	for (int NX=0; NX<AngleDep->MaxNXBins; NX++) for (int NY=0; NY<AngleDep->MaxNYBins; NY++)
		AngleDep->Map[IdAcceptance][NX][NY]=CalculateAcceptance(AngleDep->XAt(NX),AngleDep->YAt(NY));
	}

void DetectorClass::GoodEvent()
	{
	NEvents++;
	Rates->Add(IdRateNEvents,DateTimeString);
	TimeDep->Add(IdTrackCross,DateTimeString,0);
	int NCoincTriggers=0;
	for (int NChamber=0; NChamber<NChambers; NChamber++) if (Chamber[NChamber]->Triggered) NCoincTriggers++;
	CoincLevelHist->Add(IdClEvent,NCoincTriggers);
	DiffTimeHist->Add(IdClEvent, *pEventTimeDiffUs);
	}

void DetectorClass::GoodTrack(TrackClass* Track)
	{
	NTracks++;
	Rates->Add(IdRateNTracks,DateTimeString);
	TimeDep->Add(IdTrackCross,DateTimeString);
	AngleDep->Add(IdTrackCross,Track->GetSlopeX(),Track->GetSlopeY());
	int NCoincTriggers=0;
	for (int NChamber=0; NChamber<NChambers; NChamber++) if (Chamber[NChamber]->Triggered) NCoincTriggers++;
	CoincLevelHist->Add(IdClTrack,NCoincTriggers);
	DiffTimeHist->Add(IdClTrack,Track->TrackTimeDiff);
	// Chamber correlations
	double AdcCut=100; int NTriggerMissing=NChambers-Track->NTriggers, NAdcSmall=0;
	if (Track->NCrossed==NChambers)	for (int NChData=0; NChData<NChambers; NChData++) if (Chamber[NChData]->NHitsX + Chamber[NChData]->NHitsY < 2)
		{
		for (int NChTrigg=0; NChTrigg<NChambers; NChTrigg++)
			if (!(Chamber[NChTrigg]->Triggered)) CorrelMapChambersDataTrigger->Add(NTriggerMissing,NChData,NChTrigg);
		for (int NChAdc=0; NChAdc<NChambers; NChAdc++) if (Chamber[NChAdc]->Adc<AdcCut) NAdcSmall++;
		for (int NChAdc=0; NChAdc<NChambers; NChAdc++)
			if (Chamber[NChAdc]->Adc<AdcCut) CorrelMapChambersDataAdc->Add(NAdcSmall,NChData,NChAdc);
		}
	}

char* DetectorClass::GetInfoForTrack(TrackClass* Track)
	{
	TrackEffArea=AngleDep->ZWeighted(IdEffArea,Track->GetSlopeX(),Track->GetSlopeY());
	TrackEfficiency=AngleDep->ZWeighted(IdTrackingEff,Track->GetSlopeX(),Track->GetSlopeY());	
	sprintf(InfoForTrack,"%7.1lf",TrackEffArea);
	return InfoForTrack;
	}

void DetectorClass::ComputeResults(bool DoForChambers)
	{
	if (Verbose>0) printf("Detector:%s : Compute Results\n",Name);
	FillAcceptanceMap();
	if (DoForChambers) for (int NChamber=0; NChamber<NChambers; NChamber++) Chamber[NChamber]->ComputeResults();
	AngleDep->Clear(IdEffArea);
	AngleDep->Clear(IdFlux);
	Rates->RateFromTo(IdRateNEvents,IdRateEventRate);
	Rates->RateFromTo(IdRateNTracks,IdRateTrackRate);
	EventRate=Rates->YAverage(IdRateEventRate,false);
	TrackRate=Rates->YAverage(IdRateTrackRate,false);
	// Do the calculations
	CalculateSingleValues();
	CalculateAngleOfView();
	CalculateAngDepSimpleEfficiencies();
	CalculateAngDepEffectiveArea();
	// CalculateAngDepCoincArea(); // TEST
	CalculateTimeDep();
	CalculateFlux();
	CheckCorrelationMaps();
	}

void DetectorClass::CalculateSingleValues()
	{
	if (Verbose>1) printf("Detector:%s : Compute Results : Calculate Single Values\n",Name);
	// Compure simple Efficiencies
	double	EffArray[NChambers];
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		EffArray[NChamber]=Chamber[NChamber]->EfficiencyTracking;
	Efficiency=CalculateCombinedEfficiency(NChambers,TrackingNHits,EffArray);
	// Compute average Gain
	double GainSum=0,GainN=0;
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		if (Chamber[NChamber]->GainAdc>1) {GainSum+=Chamber[NChamber]->GainAdc; GainN++;}
	if (GainN>0) GainAverage=GainSum/GainN; else GainAverage=1;
	// Get total time
	if (Rates->LastX>0) TotalTimeSec=Rates->LastX; else TotalTimeSec=Rates->XAt(Rates->MaxNBins-1)-Rates->XAt(0);
	TotalTimeSec+=TimeCorrectionSec;
	double TotalDeadTimeSec = DeadTimePerEventUs*double(NEvents)*1e-6 + DeadTimeZipSec;
	TotalTimeLiveSec=TotalTimeSec-TotalDeadTimeSec;
	// DeadTime estimation
	if ( (NFiles>1) && (NEventsPerFile>1) ) 
		DeadTimeZipPerEventUsEstimation = DeadTimeZipSec/double(NEventsPerFile)*1e6/double(NFiles-1);
	else DeadTimeZipPerEventUsEstimation=0;
	double DtBase=DiffTimeHist->PeakPositionInRange(0,10,1500)*3.0;
	int EventOrTrack=-1;
	double DataValidityIntegral=50;
	if (DiffTimeHist->IntegralFromTo(IdClEvent,DtBase*1,DtBase*2)>DataValidityIntegral) EventOrTrack=IdClEvent;
	if (DiffTimeHist->IntegralFromTo(IdClTrack,DtBase*1,DtBase*2)>DataValidityIntegral) EventOrTrack=IdClTrack;
	if (EventOrTrack>-1)
		{
		double Q0=DiffTimeHist->IntegralFromTo(EventOrTrack,DtBase*0,DtBase*1), Q1=DiffTimeHist->IntegralFromTo(EventOrTrack,DtBase*1,DtBase*2), Q2=DiffTimeHist->IntegralFromTo(EventOrTrack,DtBase*2,DtBase*3);
		double k=Q2/Q1, Q0e=Q1/k;
		DeadTimePerEventUsEstimation=log(Q0/Q0e*(1.0-k)+k)/log(k)*DtBase - 0.5*DiffTimeHist->BinSize;
		}
	else DeadTimePerEventUsEstimation=0;
	if (DeadTimePerEventUs<1) // no initial value, but estimation is not bad
		{
		printf("Warning: No DeadTimePerEvent[us] value is given.\n");
		double EstimationFromBasics=(Chamber[0]->SizeX+Chamber[0]->SizeY)*1.5;
		if (NChambers>10) EstimationFromBasics*=2;
		printf("\t Estimation from Detector basics it is around %4.0f\n",EstimationFromBasics);
		if (DeadTimePerEventUsEstimation>1) printf("\t Estimation from this very data is %4.0lf\n",DeadTimePerEventUsEstimation);
		}
	else if ( (DeadTimePerEventUsEstimation>1) && (fabs(DeadTimePerEventUs-DeadTimePerEventUsEstimation)/DeadTimePerEventUs > 0.4) )
		{
		printf("Warning: the given and the data-estimated DeadTimePerEvent[us] values are far away:\n");
		printf("\t Given value from DetectorBasics is %4.0lf\n",DeadTimePerEventUs);
		printf("\t Estimation from Detector basics it is around %4.0f\n", (Chamber[0]->SizeX+Chamber[0]->SizeY)*1.5 );
		if (DeadTimePerEventUsEstimation>1) printf("\t Estimation from this very data is %4.0lf\n",DeadTimePerEventUsEstimation);
		}
	if ( (DeadTimeZipPerEventUsEstimation>0) && (DeadTimeZipPerEventUsApprox>0) && (fabs(DeadTimeZipPerEventUsApprox-DeadTimeZipPerEventUsEstimation)/DeadTimeZipPerEventUsApprox > 0.4) )
		printf("Warning: DeadTimeZipPerEvent[us] time approx and this runs calculated are far away : %4.0lf , %4.0lf\n",DeadTimeZipPerEventUsApprox,DeadTimeZipPerEventUsEstimation);
	}

void DetectorClass::CalculateAngleOfView()
	{
	// Calculate the Angle of View
	ZMiddle=(Chamber[0]->PositionZ+Chamber[NChambers-1]->PositionZ)/2.0;
	double	AngleViewXMinThis, AngleViewXMaxThis, AngleViewYMinThis, AngleViewYMaxThis, AovTemp;
	AngleViewX=0; AngleViewXMin=0; AngleViewXMax=0; AngleViewY=0; AngleViewYMin=0; AngleViewYMax=0;
	for (int ArrayStart=0; ArrayStart<(NChambers-TrackingNHits+1); ArrayStart++)
		{
		int ArrayEnd=ArrayStart+TrackingNHits-1;
		AngleViewXMaxThis = (Chamber[ArrayEnd]->SizeX*Chamber[ArrayEnd]->PadSizeX+Chamber[ArrayEnd]->PositionX - Chamber[ArrayStart]->PositionX) / (Chamber[ArrayEnd]->PositionZ - Chamber[ArrayStart]->PositionZ);
		AngleViewXMinThis = (Chamber[ArrayEnd]->PositionX - (Chamber[ArrayStart]->SizeX*Chamber[ArrayStart]->PadSizeX+Chamber[ArrayStart]->PositionX)) / (Chamber[ArrayEnd]->PositionZ - Chamber[ArrayStart]->PositionZ);
		if (AngleViewXMaxThis<AngleViewXMinThis) { AovTemp=AngleViewXMinThis; AngleViewXMinThis=AngleViewXMaxThis; AngleViewXMaxThis=AovTemp; }
		if (AngleViewXMax<AngleViewXMaxThis) AngleViewXMax=AngleViewXMaxThis;
		if (AngleViewXMin>AngleViewXMinThis) AngleViewXMin=AngleViewXMinThis;

		AngleViewYMaxThis = (Chamber[ArrayEnd]->SizeY*Chamber[ArrayEnd]->PadSizeY+Chamber[ArrayEnd]->PositionY - Chamber[ArrayStart]->PositionY) / (Chamber[ArrayEnd]->PositionZ - Chamber[ArrayStart]->PositionZ);
		AngleViewYMinThis = (Chamber[ArrayEnd]->PositionY - (Chamber[ArrayStart]->SizeY*Chamber[ArrayStart]->PadSizeY+Chamber[ArrayStart]->PositionY)) / (Chamber[ArrayEnd]->PositionZ - Chamber[ArrayStart]->PositionZ);
		if (AngleViewYMaxThis<AngleViewYMinThis) { AovTemp=AngleViewYMinThis; AngleViewYMinThis=AngleViewYMaxThis; AngleViewYMaxThis=AovTemp; }
		if (AngleViewYMax<AngleViewYMaxThis) AngleViewYMax=AngleViewYMaxThis;
		if (AngleViewYMin>AngleViewYMinThis) AngleViewYMin=AngleViewYMinThis;
		}
	AngleViewX=AngleViewXMax-AngleViewXMin;
	AngleViewY=AngleViewYMax-AngleViewYMin;
	printf("AngleView : X:[%g,%g], Y:[%g,%g] (Tracking: %d of %d)\n",AngleViewXMin,AngleViewXMax,AngleViewYMin,AngleViewYMax,TrackingNHits,NChambers);
	}

void DetectorClass::CalculateAngDepSimpleEfficiencies()
	{
	if (Verbose>1) printf("Detector:%s : Compute Results : Calculate Angular Dependent Simple Efficiencies\n",Name);
	// Calculate angular dependent Tracking Efficiency
	MultiMapClass* TempMap = new MultiMapClass(NChambers+1,AngleDep);
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		TempMap->Import(NChamber,Chamber[NChamber]->AngleDep,Chamber[NChamber]->IdTrackingEff);
	CombinedEfficiencyMapClass * EffCombiner = new CombinedEfficiencyMapClass();
	EffCombiner->SetCombination(NChambers,TrackingNHits);
	EffCombiner->SetMap(TempMap);
	EffCombiner->SaveResultsTo(NChambers);
	int ChamberId[NChambers]; for (int NChamber=0; NChamber<NChambers; NChamber++) ChamberId[NChamber]=NChamber;
	EffCombiner->SetEfficiencyIdArray(NChambers,ChamberId);
	EffCombiner->Calculate();
	AngleDep->Import(IdTrackingEff,TempMap,NChambers,false);
	// Calculate angular dependent Trigger Efficiency
	TempMap->ClearAll();
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		TempMap->Import(NChamber,Chamber[NChamber]->AngleDep,Chamber[NChamber]->IdTriggerEff);
	EffCombiner->SetCombination(NChambers,TriggerCoinc);
	EffCombiner->SetMap(TempMap);
	EffCombiner->SaveResultsTo(NChambers);
	EffCombiner->SetEfficiencyIdArray(NChambers,ChamberId);
	EffCombiner->Calculate();
	AngleDep->Import(IdTriggerEff,TempMap,NChambers,false);
	delete EffCombiner;
	delete TempMap;
	}

void DetectorClass::CalculateAngDepEffectiveArea()
	{
	if (Verbose>1) printf("Detector:%s : Compute Results : Calculate Angular Dependent Effective Area\n",Name);
	// Calculate AngularEff via the ZonedMaps of the Chamber level PositionDepEffMaps
	// Let's assume the chambers are ordered in Z
	ZonedMapsClass	*EffZMaps;
	ZonedMapsClass	*EffZMap[NChambers];
	const int		NZLevels=2;
	double	ZLevel[NZLevels]={0.50,0.90};
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		{
		EffZMap[NChamber] = new ZonedMapsClass(Chamber[NChamber]->PositionDep,Chamber[NChamber]->IdTrackingEff,ZLevel,NZLevels);
		EffZMap[NChamber]->Scale(Chamber[NChamber]->PadSizeX,Chamber[NChamber]->PadSizeY);
		EffZMap[NChamber]->Shift(Chamber[NChamber]->PositionX,Chamber[NChamber]->PositionY);
		}
	int ChamberId[NChambers]; for (int NChamber=0; NChamber<NChambers; NChamber++) ChamberId[NChamber]=NChamber;
	CombinedEfficiencyMapClass * EffPosCombiner = new CombinedEfficiencyMapClass();
	EffPosCombiner->SetCombination(NChambers,TrackingNHits);
	EffPosCombiner->SaveResultsTo(NChambers);
	EffPosCombiner->SetEfficiencyIdArray(NChambers,ChamberId);
	// SideGrow init
	double	ChamberSideXMin[NChambers],ChamberSideXMax[NChambers],ChamberSideYMin[NChambers],ChamberSideYMax[NChambers];
	if (AParameters->DoSideGrowInt==1) for (int NChamber=0; NChamber<NChambers; NChamber++)
		{
		ChamberSideXMin[NChamber]=EffZMap[NChamber]->XMarker[0]; ChamberSideXMax[NChamber]=EffZMap[NChamber]->XMarker[EffZMap[NChamber]->MaxNXBins];
		ChamberSideYMin[NChamber]=EffZMap[NChamber]->YMarker[0]; ChamberSideYMax[NChamber]=EffZMap[NChamber]->YMarker[EffZMap[NChamber]->MaxNYBins];
		}
	// Loop for the Angles
	double	SlopeX,SlopeY;
	for (int nx=0; nx<AngleDep->MaxNXBins; nx++) for (int ny=0; ny<AngleDep->MaxNYBins; ny++)
		{
		SlopeX=AngleDep->XAt(nx); SlopeY=AngleDep->YAt(ny);
		if ( SlopeX>AngleViewXMax || SlopeX<AngleViewXMin || SlopeY>AngleViewYMax || SlopeY<AngleViewYMin ) continue;
		EffZMaps = new ZonedMapsClass(NChambers+1);
		// Compute SideGrow
		if (AParameters->DoSideGrowInt==1) for (int NChamber=0; NChamber<NChambers; NChamber++)	
			{
			EffZMap[NChamber]->XMarker[0]=ChamberSideXMin[NChamber]-Chamber[NChamber]->SideGrowX*fabs(SlopeX);
			EffZMap[NChamber]->XMarker[EffZMap[NChamber]->MaxNXBins]=ChamberSideXMax[NChamber]+Chamber[NChamber]->SideGrowX*fabs(SlopeX);
			EffZMap[NChamber]->YMarker[0]=ChamberSideYMin[NChamber]-Chamber[NChamber]->SideGrowY*fabs(SlopeY);
			EffZMap[NChamber]->YMarker[EffZMap[NChamber]->MaxNYBins]=ChamberSideYMax[NChamber]+Chamber[NChamber]->SideGrowY*fabs(SlopeY);
			}
		for (int NChamber=0; NChamber<NChambers; NChamber++)
			EffZMaps->AddMapShifted(NChamber,EffZMap[NChamber],Chamber[NChamber]->PositionX+(ZMiddle-Chamber[NChamber]->PositionZ)*SlopeX,Chamber[NChamber]->PositionY+(ZMiddle-Chamber[NChamber]->PositionZ)*SlopeY);
		EffZMaps->CreateDummy();
		EffPosCombiner->SetMap(EffZMaps->Dummy);
		EffPosCombiner->Calculate();
		EffZMaps->GetFromDummy(NChambers);
		AngleDep->Add(IdEffArea,SlopeX,SlopeY,EffZMaps->Integral(NChambers)/sqrt(1.0+SlopeX*SlopeX+SlopeY*SlopeY)); // AreaProjection perpedicular = cos = (1+xx+yy)^(-0.5)
		delete EffZMaps;
		}
	delete EffPosCombiner;
	for (int NChamber=0; NChamber<NChambers; NChamber++) delete EffZMap[NChamber];
	// Include the Trigger efficiency
	AngleDep->ModifyMapWith(IdEffArea,IdTriggerEff,FunctionMultiply);	// Somehow this is not enough if efficiency is extreme low.. 
	}

void DetectorClass::CalculateAngDepCoincArea()
	{
	if (Verbose>1) printf("Detector:%s : Compute Coincidence Area expectations\n",Name);
	// Let's assume the chambers are ordered in Z
	ZonedMapsClass	*EffZMaps;
	ZonedMapsClass	*EffZMap[NChambers];
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		{
		EffZMap[NChamber] = new ZonedMapsClass(Chamber[NChamber]->PositionDep,Chamber[NChamber]->IdTriggerEff);
		EffZMap[NChamber]->Map[0][0][0]=1.0;
		EffZMap[NChamber]->Scale(Chamber[NChamber]->PadSizeX,Chamber[NChamber]->PadSizeY);
		EffZMap[NChamber]->Shift(Chamber[NChamber]->PositionX,Chamber[NChamber]->PositionY);
		}
	CombinedEfficiencyMapClass * EffPosCombinerCoinc[NChambers+1];
	for (int Nc=1; Nc<=NChambers; Nc++)
		{
		EffPosCombinerCoinc[Nc] = new CombinedEfficiencyMapClass();
		EffPosCombinerCoinc[Nc]->SetCombination(NChambers,Nc);
		EffPosCombinerCoinc[Nc]->SaveResultsTo(NChambers);
		EffPosCombinerCoinc[Nc]->SetEfficiencyIdArray(NChambers,CountArray);
		}
	// Loop for the Angles
	double	SlopeX,SlopeY;
	for (int nx=0; nx<AngleDep->MaxNXBins; nx++) for (int ny=0; ny<AngleDep->MaxNYBins; ny++)
		{
		SlopeX=AngleDep->XAt(nx); SlopeY=AngleDep->YAt(ny);
		//if ( SlopeX>AngleViewXMax || SlopeX<AngleViewXMin || SlopeY>AngleViewYMax || SlopeY<AngleViewYMin ) continue;
		EffZMaps = new ZonedMapsClass(NChambers+1);
		// Compute SideGrow
		for (int NChamber=0; NChamber<NChambers; NChamber++)
			EffZMaps->AddMapShifted(NChamber,EffZMap[NChamber],Chamber[NChamber]->PositionX+(ZMiddle-Chamber[NChamber]->PositionZ)*SlopeX,Chamber[NChamber]->PositionY+(ZMiddle-Chamber[NChamber]->PositionZ)*SlopeY);
		EffZMaps->CreateDummy();		
		for (int Nc=1; Nc<=NChambers; Nc++)
			{
			EffPosCombinerCoinc[Nc]->SetMap(EffZMaps->Dummy);
			EffPosCombinerCoinc[Nc]->Calculate();
			EffZMaps->GetFromDummy(NChambers);
			CoincArea->Set(Nc,SlopeX,SlopeY,EffZMaps->Integral(NChambers)/sqrt(1.0+SlopeX*SlopeX+SlopeY*SlopeY));
			}
		delete EffZMaps;
		}
	for (int Nc=1; Nc<=NChambers; Nc++) delete EffPosCombinerCoinc[Nc];
	for (int NChamber=0; NChamber<NChambers; NChamber++) delete EffZMap[NChamber];
	//CoincArea->WriteToNewFile((char*)"Results/aa");	// TEMP
	}

void DetectorClass::CalculateTimeDep()
	{
	if (Verbose>1) printf("Detector:%s : Compute Results : Calculate Time Dependence\n",Name);
	MultiHistDynTimeClass* EffHists = new MultiHistDynTimeClass(NChambers+1,Chamber[0]->TimeDep);
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		EffHists->Import(NChamber,Chamber[NChamber]->TimeDep,Chamber[NChamber]->IdTrackingEff);
	CombinedEfficiencyHistClass * EffCombiner = new CombinedEfficiencyHistClass();
	EffCombiner->SetCombination(NChambers,TrackingNHits);
	EffCombiner->SetHist(EffHists);
	EffCombiner->SaveResultsTo(NChambers);
	int ChamberId[NChambers]; for (int NChamber=0; NChamber<NChambers; NChamber++) ChamberId[NChamber]=NChamber;
	EffCombiner->SetEfficiencyIdArray(NChambers,ChamberId);
	EffCombiner->Calculate();
	bool ImportOk=TimeDep->Import(IdTrackingEff,EffHists,NChambers,false);
	if (!ImportOk) printf("Warning : Detector:TimeDep:CombinedEff  calculation was not done\n");
	// Gather info for TimeDepThpGains
	TimeDepSum->ClearAll();
	TimeDepSum->TimeZero=TimeDep->TimeZero;	TimeDepSum->StartValue=TimeDep->StartValue;
	TimeDepSum->BinSize=TimeDep->BinSize;	TimeDepSum->EndValue=TimeDep->EndValue;
	TimeDepSum->AllocateMemory(TimeDep->MaxNBins-1);
	TimeDepSum->LastX=TimeDep->LastX;
	int TempId=IdTdsRateEvent;
	if (Thp!=NULL)
		{
		int NThpId=0; if (MaxNThps>2) NThpId=1;
		TimeDepSum->AddTimeHist(TempId,Thp[NThpId]->TimeDev,0);
		TimeDepSum->AddTimeHist(IdTdsT,Thp[NThpId]->TimeDev,1); TimeDepSum->ModifyHistWith(IdTdsT,TempId,FunctionDivide);
		TimeDepSum->AddTimeHist(IdTdsH,Thp[NThpId]->TimeDev,2); TimeDepSum->ModifyHistWith(IdTdsH,TempId,FunctionDivide);
		TimeDepSum->AddTimeHist(IdTdsP,Thp[NThpId]->TimeDev,3); TimeDepSum->ModifyHistWith(IdTdsP,TempId,FunctionDivide);
		TimeDepSum->Clear(TempId);
		}
	if (HvInfo!=NULL)
		{
		TimeDepSum->AddTimeHist(TempId,HvInfo->TimeDev,0);
		TimeDepSum->AddTimeHist(IdTdsUMon,HvInfo->TimeDev,2); TimeDepSum->ModifyHistWith(IdTdsUMon,TempId,FunctionDivide);
		TimeDepSum->AddTimeHist(IdTdsIMon,HvInfo->TimeDev,3); TimeDepSum->ModifyHistWith(IdTdsIMon,TempId,FunctionDivide);
		TimeDepSum->Clear(TempId);
		}
	// NSum from Chamber0:TimeDep:IdAll
	TimeDepSum->AddTimeHist(IdTdsNSum,Chamber[0]->TimeDep,Chamber[0]->IdAll);
	TimeDepSum->ExchangeToIntegral(IdTdsNSum);
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		{
		TimeDepSum->AddTimeHist(TempId,Chamber[NChamber]->TimeDep,Chamber[NChamber]->IdTrackCross);
		TimeDepSum->AddTimeHist(TdsCh+NChamber,Chamber[NChamber]->TimeDep,Chamber[NChamber]->IdTrackAdcScaledSum);
		TimeDepSum->ModifyHistWith(TdsCh+NChamber,TempId,FunctionDivide);
		TimeDepSum->Clear(TempId);
		}
	TimeDepSum->AddTimeHist(IdTdsRateEvent,Rates,IdRateEventRate);
	TimeDepSum->AddTimeHist(IdTdsRateTrack,Rates,IdRateTrackRate);
	}

void DetectorClass::CalculateFlux()
	{
	if (Verbose>1) printf("Detector:%s : Compute Results : Calculate Flux\n",Name);
	// Calculate flux
	AngleDep->CopyFromTo(IdTrackCross,IdFlux);
	AngleDep->ModifyMapWith(IdFlux,IdEffArea,FunctionDivide);	// This includes the PadSize in mm
	AngleDep->FillFunction(IdAcceptance,FunctionJacobiMFlux);
	AngleDep->ModifyMapWith(IdFlux,IdAcceptance,FunctionDivide);
	//AngleDep->ModifyMapWith(IdFlux,IdTrackingEff,FunctionDivide);	// Is this needed? it is PARTIALLY included into the EffArea.. ?
	if (TotalTimeLiveSec==0) TotalTimeLiveSec=TotalTimeSec;
	AngleDep->MultiplyMap(IdFlux,(1.0e6/TotalTimeLiveSec/(AngleDep->XBinSize*AngleDep->YBinSize))); // normalize with time, and multiply for BinSize of AngleDep
	AngleDep->ClearBorders(IdFlux);
	// FluxMid
	double FluxMid=0;
	for (int NX=AngleDep->GetXBin(0)-1; NX<AngleDep->GetXBin(0)+2; NX++) for (int NY=AngleDep->GetYBin(0)-1; NY<AngleDep->GetYBin(0)+2; NY++) FluxMid+=AngleDep->ZAt(IdFlux,NX,NY)/9.0;
	}

void DetectorClass::CheckCorrelationMaps()
	{
	if (Verbose>1) printf("Detector:%s : Check Correlation Maps\n",Name);
	bool SuspiciousDT=false, SuspiciousDA=false;
	// Check Correlation maps, if connections are ok
	int NTriggerMissing=0;
	while ( (NTriggerMissing<NChambers) && (CorrelMapChambersDataTrigger->Integral(NTriggerMissing)<1) ) NTriggerMissing++;	
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		if (CorrelMapChambersDataTrigger->PeakPositionOnLineX(NTriggerMissing,NChamber) != NChamber) SuspiciousDT=true;
	int NAdcSmall=0;
	while ( (NAdcSmall<NChambers) && (CorrelMapChambersDataAdc->Integral(NAdcSmall)<1) ) NAdcSmall++;	
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		if (CorrelMapChambersDataAdc->PeakPositionOnLineX(NAdcSmall,NChamber) != NChamber) SuspiciousDA=true;
	if (SuspiciousDT) printf("Warning : Suspicious Data-Trigger correlation (or not all used).\n");
	if (SuspiciousDA) printf("Warning : Suspicious Data-Adc correlation (or not all used).\n");
	}

void DetectorClass::Print(bool DoForChambers)
	{
	printf("Detector: %s",Name);
	printf("\tTrackingEfficiency = %5.2f%%,",Efficiency*100.);
	printf("\tTotalTime = %s\n",NiceDurationSec(TotalTimeSec));
	printf("\tEventRate: %5.2lf Hz , TrackRate: %5.2lf Hz\n",EventRate,TrackRate);
	double FluxMid=0;
	for (int NX=AngleDep->GetXBin(0)-1; NX<AngleDep->GetXBin(0)+2; NX++) for (int NY=AngleDep->GetYBin(0)-1; NY<AngleDep->GetYBin(0)+2; NY++) FluxMid+=AngleDep->ZAt(IdFlux,NX,NY)/9.0;
	printf("\tFlux at MidView : %5.1lf /m2/sr/s\n",FluxMid);
	if (DoForChambers)
		for (int NChamber=0; NChamber<NChambers; NChamber++) Chamber[NChamber]->Print();
	}

// File operations

void DetectorClass::WriteGeneralInfoToFile(FILE* OutFile)
	{
	fprintf(OutFile,"Name\t%s\n",Name);
	fprintf(OutFile,"Efficiency\t%f\n",Efficiency);
	fprintf(OutFile,"TotalTime\t%s\n",NiceDurationSec(TotalTimeSec));
	fprintf(OutFile,"NEvents\t%ld\tNTracks\t%ld\n",NEvents,NTracks);
	fprintf(OutFile,"ResultsFilenaming\t%s\n",ResFilenaming);
	fprintf(OutFile,"EventRate\t%g\tTrackRate\t%g\n",EventRate,TrackRate);
	fprintf(OutFile,"FluxMid\t%g\n",FluxMid);
	fprintf(OutFile,"TotalTimeMin\t%.0lf\t%.0lf\n",TotalTimeSec/60.,TotalTimeLiveSec/60.);
	fprintf(OutFile,"DeadTimeEstimation\t%g\t%g\n",DeadTimePerEventUsEstimation,DeadTimeZipPerEventUsEstimation);
	fprintf(OutFile,"End\n");
	fprintf(OutFile,"\n\n"); // Separator
	}
	
void DetectorClass::ReadGeneralInfoFromFile(FILE* DetectorInfoFile)
	{	
	bool DoRead=true;
	char VarName[50];
	while (DoRead)
		{
		fscanf(DetectorInfoFile,"%s",VarName);
		if (strcmp(VarName,"Name")==0) fscanf(DetectorInfoFile,"%s",Name);
		else if (strcmp(VarName,"Efficiency")==0)	fscanf(DetectorInfoFile,"%lf",&Efficiency);
		else if (strcmp(VarName,"TotalTimeMin")==0)	{fscanf(DetectorInfoFile,"%lf %lf",&TotalTimeSec,&TotalTimeLiveSec); TotalTimeSec*=60.; TotalTimeLiveSec*=60; }
		else if (strcmp(VarName,"TotalTime")==0)	fscanf(DetectorInfoFile,"%*s");
		else if (strcmp(VarName,"NEvents")==0)		fscanf(DetectorInfoFile,"%ld",&NEvents);
		else if (strcmp(VarName,"NTracks")==0)		fscanf(DetectorInfoFile,"%ld",&NTracks);
		else if (strcmp(VarName,"EventRate")==0)	fscanf(DetectorInfoFile,"%lf",&EventRate);
		else if (strcmp(VarName,"TrackRate")==0)	fscanf(DetectorInfoFile,"%lf",&TrackRate);
		else if (strcmp(VarName,"FluxMid")==0) 		fscanf(DetectorInfoFile,"%lf",&FluxMid);
		else if (strcmp(VarName,"DeadTimeEstimation")==0) 	fscanf(DetectorInfoFile,"%lf %lf",&DeadTimePerEventUsEstimation,&DeadTimeZipPerEventUsEstimation);
		else if (strcmp(VarName,"ResultsFilenaming")==0) fscanf(DetectorInfoFile,"%s",ResFilenaming);
		else if (strcmp(VarName,"End")==0)		DoRead=false;
		else printf("WARNING : Unknown  Detector General Info  type : %s\n",VarName);
		}
	}

int DetectorClass::WriteGnuplotInfo(FILE* OutFile, int StartIndex)
	{
	std::string FileContentItem;
	int PosBegin=0,PosEnd=1, Index=StartIndex;
	while ( PosEnd > 0)
		{
		PosEnd = FileContents.find_first_of(' ',PosBegin);
		FileContentItem=FileContents.substr(PosBegin,PosEnd-PosBegin);
		fprintf(OutFile,"IndexDetector%s=%d\n",FileContentItem.data(),Index);
		PosBegin=PosEnd+1;
		Index++;
		}
	return Index;
	}

void DetectorClass::WriteResultsToFile(FILE* OutFile)
	{
	// FileContents looks like this
	//	FileContents="General AngleDep TimeDep Rate Coinc Thp Hv";
	std::string FileContentItem;
	int PosBegin=0,PosEnd=1;
	fprintf(OutFile,"!Detector\n");
	fprintf(OutFile,"FileContents %d %s\n",CountWordsInString(FileContents),FileContents.data());
	while ( PosEnd > 0)
		{
		PosEnd = FileContents.find_first_of(' ',PosBegin);
		FileContentItem=FileContents.substr(PosBegin,PosEnd-PosBegin);
		PosBegin=PosEnd+1;
		if (FileContentItem.compare("General")==0) WriteGeneralInfoToFile(OutFile);
		else if (FileContentItem.compare("AngleDep")==0) AngleDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("TimeDep")==0) TimeDep->WriteToFile(OutFile);
		else if (FileContentItem.compare("Rate")==0) Rates->WriteToFile(OutFile);
		else if (FileContentItem.compare("TimeDepSum")==0) TimeDepSum->WriteToFile(OutFile);
		else if (FileContentItem.compare("Coinc")==0) CoincLevelHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("DiffTime")==0) DiffTimeHist->WriteToFile(OutFile);
		else if (FileContentItem.compare("CorrelDT")==0) CorrelMapChambersDataTrigger->WriteToFile(OutFile);
		else if (FileContentItem.compare("CorrelDA")==0) CorrelMapChambersDataAdc->WriteToFile(OutFile);
		else if (FileContentItem.compare("Thp")==0) { if (Thp!=NULL) for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->WriteToFile(OutFile); }
		else if (FileContentItem.compare("Hv")==0) { if (HvInfo!=NULL) HvInfo->TimeDev->WriteToFile(OutFile); }
		else printf("WARNING : Unknown Detector-FileContent-item : \"%s\"\n",FileContentItem.data());
		}
	}

void DetectorClass::WriteResultsToFileName(char* OutResultsFileName)
	{
	// Open file
	FILE * DetectorResultsFile = fopen(OutResultsFileName,"w");
	if (DetectorResultsFile==NULL) { printf("Warning : Cannot write DetectorInfoFile: %s\n",OutResultsFileName); return; }
	WriteResultsToFile(DetectorResultsFile);
	fclose(DetectorResultsFile);
	}

void DetectorClass::ReadInfoFromFile(char* InDetectorInfoFileName, bool DoClearStat, bool DoTuneChambers)
	{
	// Open file
	FILE * DetectorInfoFile=fopen(InDetectorInfoFileName,"r");
	if (DetectorInfoFile==NULL) { printf("Cannot read DetectorInfoFile: %s\nExit\n",InDetectorInfoFileName); exit(1); }
	// Read Chamber info from the file
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		Chamber[NChamber]->ReadInfoFromFile(DetectorInfoFile,DoClearStat,DoTuneChambers);
	// Read the Detector level info : General AngleDep Thp
	char	DetectorMark[20];
	fscanf(DetectorInfoFile,"%s",DetectorMark);
	if (strcmp(DetectorMark,"!Detector")!=0) { printf("ERROR : Incorrect DetectorMark : %s \nExit\n\n",DetectorMark); exit(1); }
	fscanf(DetectorInfoFile,"%s",DetectorMark);
	if (strcmp(DetectorMark,"FileContents")!=0) { printf("ERROR : Incorrect FileContents mark : %s \nExit\n\n",DetectorMark); exit(1); }
	int NFileContentsRead=0; 
	std::string FileContentsRead;
	char TempString[50];
	fscanf(DetectorInfoFile,"%d",&NFileContentsRead);
	for (int n=0; n<NFileContentsRead; n++)
		{
		fscanf(DetectorInfoFile,"%s",TempString);
		if (FileContentsRead.length()>2) FileContentsRead+=" ";
		FileContentsRead+=TempString;
		}
	std::string FileContentItem;
	int PosBegin=0,PosEnd=1;
	while ( PosEnd > 0)
		{
		PosEnd = FileContentsRead.find_first_of(' ',PosBegin);
		FileContentItem=FileContentsRead.substr(PosBegin,PosEnd-PosBegin);
		PosBegin=PosEnd+1;
		if (FileContentItem.compare("General")==0) ReadGeneralInfoFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("AngleDep")==0) AngleDep->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("TrackKhi2")==0) SkipBlockInFile(DetectorInfoFile);
		else if (FileContentItem.compare("TimeDep")==0) TimeDep->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("Rate")==0) Rates->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("TimeDepSum")==0) SkipBlockInFile(DetectorInfoFile); //TimeDepSum->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("Coinc")==0) CoincLevelHist->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("DiffTime")==0) DiffTimeHist->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("CorrelDT")==0) CorrelMapChambersDataTrigger->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("CorrelDA")==0) CorrelMapChambersDataAdc->ReadFromFile(DetectorInfoFile);
		else if (FileContentItem.compare("Thp")==0) { if (Thp!=NULL) for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->ReadFromFile(DetectorInfoFile); }
		else if (FileContentItem.compare("Hv")==0) { if (HvInfo!=NULL) HvInfo->TimeDev->ReadFromFile(DetectorInfoFile); }
		else printf("WARNING : Unknown Detector-FileContent-item : %s\n",FileContentItem.data());
		}
	// The rest of the file contains info about the Tracking and Analysis
	fclose(DetectorInfoFile);
	// Clear statistics if requested
	if (DoClearStat) ClearStatistics();

	}


////////////////////////////////////////////////////////////////////////

double FunctionJacobiMFlux(double mx, double my)
	{
	return pow(1.0+mx*mx+my*my,-1.5);	//   (1+mx*mx+my*my) == cos**-2
						//  AngleOfView==Jacobi== cos3  ; // AreaAngle: cos taken care in EffArea
	}


////////////////////////////////////////////////////////////////////////
// Track Class - implementation
//      version : 2020-05-14
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "TrackClass.h"

TrackClass::TrackClass()
	{
	sprintf(Name,"Track");
	Id=-1;
	MaxKhi2OverNdf=5.0;
	GoodKhi2OverNdf=0.0;
	Good=false;
	Detector=NULL;
	Chamber=NULL;
	ChamberId=NULL;
	EventIdDef=0;
	pEventId=&EventIdDef;
	pEventTimeDiff=&EventIdDef;
	ZMin=0; ZMax=0; ZMid=0;
	UseFixPoint=false;
	FixPointZ=0; FixPointX=0; FixPointY=0;
	KonMap=NULL; CrossHitsMap=NULL;
	KonBinSize=0.1; KonMaxMult=3;
	Kon=0; NCrossed=0;
	}

TrackClass::~TrackClass()
	{
	if (Detector==NULL) return;
	free(ChamberId);
	free(Chamber);
	//for (int NFitPoint=2 NFitPoint<=MaxNChambers; NFitPoint++) { printf("NF: %d\n",NFitPoint); delete HitSelection[NFitPoint]; }
	//free(HitSelection);
	//free(NClustersArray);
	delete TrackX0; delete TrackX1;
	delete TrackY0; delete TrackY1;
	delete KonMap; delete CrossHitsMap;
	}

void TrackClass::Set(DetectorClass * InDetector, int InNChambers, int *InChamberId, int InMinNHits)
	{
	// Save the given values
	Detector = InDetector;
	NChambers = InNChambers;
	MinNHits = InMinNHits;
	ChamberId = (int*) calloc(NChambers,sizeof(int));
	for (int NCh=0; NCh<NChambers; NCh++) ChamberId[NCh]=InChamberId[NCh];
	Set();
	}

void TrackClass::Set(DetectorClass * InDetector)
	{
	Detector = InDetector;
	if (ChamberId!=NULL) Set();
	}

void TrackClass::Set()
	{
	Chamber = (ChamberClass**) calloc(NChambers,sizeof(ChamberClass*));
	for (int NCh=0; NCh<NChambers; NCh++) Chamber[NCh]=Detector->Chamber[ChamberId[NCh]];
	// Hit Selections
	NClustersArray = (int*) calloc(NChambers,sizeof(int));
	HitSelection = (SelectionClass**) calloc(NChambers,sizeof(SelectionClass*));
	for (int NFitPoint=1; NFitPoint<=NChambers; NFitPoint++)
		HitSelection[NFitPoint] = new SelectionClass(NChambers,NFitPoint);
	// 1Dim Search Tracks
	TrackX0 = new Track1DimClass();
	TrackX1 = new Track1DimClass();
	TrackY0 = new Track1DimClass();
	TrackY1 = new Track1DimClass();
	TrackX=TrackX0;
	TrackY=TrackY0;
	double ResolutionGeneral=(Chamber[0]->PadSizeX+Chamber[0]->PadSizeY)*0.15;
	TrackX0->Resolution=ResolutionGeneral; TrackX1->Resolution=ResolutionGeneral;
	TrackY0->Resolution=ResolutionGeneral; TrackY1->Resolution=ResolutionGeneral;
	// Get Z info
	ZMin=Chamber[0]->PositionZ; ZMax=Chamber[0]->PositionZ;
	for (int NCh=0; NCh<NChambers; NCh++) if (ZMin>Chamber[NCh]->PositionZ) ZMin=Chamber[NCh]->PositionZ;
	for (int NCh=0; NCh<NChambers; NCh++) if (ZMax<Chamber[NCh]->PositionZ) ZMax=Chamber[NCh]->PositionZ;
	ZMid=(ZMin+ZMax)/2.0;
	// Define Statistics maps
	KonMap = new MultiMapClass(2,0,MaxKhi2OverNdf*KonMaxMult,KonBinSize,0,MaxKhi2OverNdf*KonMaxMult,KonBinSize,(char*)"KonMap");
	KonMap->SetName(IdAll,(char*)"All"); KonMap->SetName(IdGood,(char*)"Good");
	CrossHitsMap = new MultiMapClass(NChambers+1,0,NChambers+1,1,0,NChambers+1,1,(char*)"CrossHitsMap");
	}

void TrackClass::SetId(int InId, char* InName)
	{
	Id=InId;
	sprintf(Name,"%s",InName);
	}


bool TrackClass::ReadSettingsFromFile(FILE * InFile)
	{
	bool	DoRead=true;
	char	Parameter[20];
	while (DoRead)
		{
		fscanf(InFile,"%s",Parameter);
		if (feof(InFile)) DoRead=false;
		if (Parameter[0]=='#') { SkipLine(InFile); continue; }
		if (strcmp(Parameter,"Id")==0)		fscanf(InFile,"%d",&Id);
		else if (strcmp(Parameter,"Name")==0)	fscanf(InFile,"%s",Name);
		else if (strcmp(Parameter,"Hits")==0)	fscanf(InFile,"%d",&MinNHits);
		else if (strcmp(Parameter,"Khi2")==0)	fscanf(InFile,"%lf",&MaxKhi2OverNdf);
		else if (strcmp(Parameter,"Khi2G")==0)	fscanf(InFile,"%lf",&GoodKhi2OverNdf);
		else if (strcmp(Parameter,"KonBs")==0)	fscanf(InFile,"%lf",&KonBinSize);
		else if (strcmp(Parameter,"NCh")==0)
			{ fscanf(InFile,"%d",&NChambers); ChamberId = (int*) calloc(NChambers,sizeof(int)); }
		else if (strcmp(Parameter,"ChId")==0) 	
			for (int NCh=0;NCh<NChambers;NCh++) fscanf(InFile,"%d",&(ChamberId[NCh]));
		else if (strcmp(Parameter,"End")==0)	DoRead=false;
		else if (strcmp(Parameter,"Short")==0)
			{
			fscanf(InFile,"%d %s %d %d",&Id,Name,&MinNHits,&NChambers); 
			ChamberId = (int*) calloc(NChambers,sizeof(int)); 
			for (int NCh=0;NCh<NChambers;NCh++) fscanf(InFile,"%d",&(ChamberId[NCh]));
			DoRead=false;
			}
		else { printf("\nWARNING : Incorrect Track parameter: %s\n\n",Parameter); return false; }
		}
	return true;
	}

bool TrackClass::WriteSettingsToFile(FILE * OutFile)
	{
	if (OutFile==NULL) return false;
	fprintf(OutFile,"Track\n");
	fprintf(OutFile,"\tName\t%s\n",Name);
	fprintf(OutFile,"\tId\t%d\n",Id);
	fprintf(OutFile,"\tNCh\t%d\n",NChambers);
	fprintf(OutFile,"\tHits\t%d\n",MinNHits);
	fprintf(OutFile,"\tChId\t");
	for (int NCh=0;NCh<NChambers;NCh++) fprintf(OutFile," %d ",ChamberId[NCh]);
	fprintf(OutFile,"\n\tEnd\n");
	return true;
	}

// Get info

void TrackClass::Get()
	{
	// Checks the previous Track and update TimeDiff info
	if (Good) TrackTimeDiff=0;
	TrackTimeDiff+=(*pEventTimeDiff);
	// Get this track
	SearchTrackX();
	SearchTrackY();
	//RetuneTrack();
	Kon=(TrackX->Khi2OverNdf + TrackY->Khi2OverNdf)/2.0;
	if ( Kon < MaxKhi2OverNdf ) Good=true; else Good=false;
	// Check info
	CheckTriggerContent();
	NClustersAverage=double(NClustersTotalX+NClustersTotalY)/double(NChambers)/2.0;
	FillStatistics();
	}

void TrackClass::SearchTrackX()
	{
	NClustersTotalX=0;
	for (int NChamber=0; NChamber<NChambers; NChamber++) NClustersArray[NChamber]=Chamber[NChamber]->MaxNClustersX;
	for (int NChamber=0; NChamber<NChambers; NChamber++) NClustersTotalX+=NClustersArray[NChamber];
	NTrackXCandidates=0;
	TrackX=TrackX0; TrackXTest=TrackX1;
	TrackX->Clear();
	int NFitPoint=NChambers;
	while ( NFitPoint>=MinNHits && (TrackX->Khi2OverNdf>GoodKhi2OverNdf))
		{
		HitSelection[NFitPoint]->SetArray(NClustersArray);
		while (HitSelection[NFitPoint]->IsValid())
			{
			TrackXTest->Clear(); 
			if (UseFixPoint) TrackXTest->AddPoint(FixPointZ,FixPointX);
			for (int p=0; p<HitSelection[NFitPoint]->K; p++)
				TrackXTest->AddPointX(Chamber[HitSelection[NFitPoint]->GetChamber(p)],HitSelection[NFitPoint]->GetCluster(p));
			TrackXTest->Fit();
			if ( TrackXTest->Khi2OverNdf < TrackX->Khi2OverNdf )
				{ TrackXTemp=TrackX; TrackX=TrackXTest; TrackXTest=TrackXTemp; }
			NTrackXCandidates++;
			HitSelection[NFitPoint]->Increment();
			}
		NFitPoint--;
		}
	// Retune track
	/*	
	 * this requires the FindClusterOnTrack call before !!
	TrackX->Clear();
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		if ( (Chamber[NChamber]->TrackClusterX>=0) && (Chamber[NChamber]->FiducialX->YAt(Chamber[NChamber]->ClusterX[Chamber[NChamber]->TrackClusterX]->Center)>0.5) )
			TrackX->AddPointX(Chamber[NChamber],Chamber[NChamber]->TrackClusterX);
	if ( TrackX->FittedLine->GetEntries() < 2 )
		{
		TrackX->Clear();
		for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
			if ( Chamber[NChamber]->TrackClusterX >= 0 ) 
				TrackX->AddPointX(Chamber[NChamber],Chamber[NChamber]->TrackClusterX);
		}
	TrackX->Fit();
	*/

	}

void TrackClass::SearchTrackY()
	{
	NClustersTotalY=0;
	for (int NChamber=0; NChamber<NChambers; NChamber++) NClustersArray[NChamber]=Chamber[NChamber]->MaxNClustersY;
	for (int NChamber=0; NChamber<NChambers; NChamber++) NClustersTotalY+=NClustersArray[NChamber];
	NTrackYCandidates=0;
	TrackY=TrackY0; TrackYTest=TrackY1;
	TrackY->Clear();
	int NFitPoint=NChambers;
	while ( NFitPoint>=MinNHits && (TrackY->Khi2OverNdf>GoodKhi2OverNdf) )
		{
		HitSelection[NFitPoint]->SetArray(NClustersArray);
		while (HitSelection[NFitPoint]->IsValid())
			{
			TrackYTest->Clear(); 
			if (UseFixPoint) TrackYTest->AddPoint(FixPointZ,FixPointY);
			for (int p=0; p<HitSelection[NFitPoint]->K; p++)
				TrackYTest->AddPointY(Chamber[HitSelection[NFitPoint]->GetChamber(p)],HitSelection[NFitPoint]->GetCluster(p));
			TrackYTest->Fit();
			if ( TrackYTest->Khi2OverNdf < TrackY->Khi2OverNdf )
				{ TrackYTemp=TrackY; TrackY=TrackYTest; TrackYTest=TrackYTemp; }
			NTrackYCandidates++;
			HitSelection[NFitPoint]->Increment();
			}
		NFitPoint--;
		}
	// Retune track
	/* 
	 * 
	TrackY->Clear();
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		if ( (Chamber[NChamber]->TrackClusterY>=0) && (Chamber[NChamber]->FiducialY->YAt(Chamber[NChamber]->ClusterY[Chamber[NChamber]->TrackClusterY]->Center)>0.5) )
			TrackY->AddPointY(Chamber[NChamber],Chamber[NChamber]->TrackClusterY);
	if ( TrackY->FittedLine->GetEntries() < 2 )
		{
		TrackY->Clear();
		for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
			if ( Chamber[NChamber]->TrackClusterY >= 0 ) 
				TrackX->AddPointY(Chamber[NChamber],Chamber[NChamber]->TrackClusterY);
		}
	TrackY->Fit();
	*/
	}

void TrackClass::CheckTriggerContent()
	{
	NTriggers=0;
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		if (Chamber[NChamber]->Triggered) NTriggers++;
	}

void TrackClass::FillStatistics()
	{
	KonMap->Add(IdAll,TrackX->Khi2OverNdf,TrackY->Khi2OverNdf);
	NCrossed=0;
	if (!Good) return;
	// From this on, only if Good
	KonMap->Add(IdGood,TrackX->Khi2OverNdf,TrackY->Khi2OverNdf);
	for (int NChamber=0; NChamber<NChambers; NChamber++)
		if (Chamber[NChamber]->IsTrackCrossed(this)) NCrossed++;
	CrossHitsMap->Add(NCrossed,TrackX->NPoints,TrackY->NPoints);
	}

void TrackClass::SetFixPoint(double InZ, double InX, double InY)
	{
	UseFixPoint=true;
	FixPointZ=InZ;
	FixPointX=InX;
	FixPointY=InY;
	}

void TrackClass::UnsetFixPoint()
	{
	UseFixPoint=false;
	}


// Get info

void TrackClass::PrintDef()
	{
	printf("Track: %s , Id: %d , ",Name,Id);
	printf("NCh: %d : ",NChambers);
	for (int n=0;n<NChambers;n++) printf("%d ",ChamberId[n]);
	printf(" , Hits: %d , Kon: %g, %g\n",MinNHits,MaxKhi2OverNdf,GoodKhi2OverNdf);
	}

double TrackClass::GetSlopeX()
	{
	return	TrackX->FittedLine->GetSlope();
	}

double TrackClass::GetSlopeY()
	{
	return	TrackY->FittedLine->GetSlope();
	}

double TrackClass::GetXAtZ(double InZ)
	{
	return	TrackX->FittedLine->ComputeY(InZ);
	}

double TrackClass::GetYAtZ(double InZ)
	{
	return	TrackY->FittedLine->ComputeY(InZ);
	}

double TrackClass::GetLength()
	{
	return (sqrt(1+GetSlopeX()*GetSlopeX()+GetSlopeY()*GetSlopeY()));
	}

char * TrackClass::GetInfo()
	{
	//sprintf(Info,"%10.6lf %10.6lf %7.2lf %7.2lf",GetSlopeX(),GetSlopeY(),GetXAtZ(0),GetYAtZ(0));
	//sprintf(Info,"%10.6lf %10.6lf %7.2lf %7.2lf %6.3lf %6.3lf",GetSlopeX(),GetSlopeY(),GetXAtZ(0),GetYAtZ(0),TrackX->Khi2OverNdf,TrackY->Khi2OverNdf);
	sprintf(Info,"%ld\t%ld\t%s %d\t%10.6lf %10.6lf %7.2lf %7.2lf %6.3lf %6.3lf",*pEventId,TrackTimeDiff,Name,Id,GetSlopeX(),GetSlopeY(),GetXAtZ(0),GetYAtZ(0),TrackX->Khi2OverNdf,TrackY->Khi2OverNdf);
	return Info;
	}

void TrackClass::Print()
	{
	printf("Track : %d : %s  (p: min %d/%d)\n",Id,Name,MinNHits,NChambers);
	if (TrackX->FittedLine->IsFitted()) printf("\tTrackX : Points: %2.0lf , Slope: %8.4lf , Position: %7.2lf , Khi2/ndf: %6.3lf\n",TrackX->FittedLine->GetEntries(),GetSlopeX(),GetXAtZ(0),TrackX->Khi2OverNdf);
	if (TrackY->FittedLine->IsFitted()) printf("\tTrackY : Points: %2.0lf , Slope: %8.4lf , Position: %7.2lf , Khi2/ndf: %6.3lf\n",TrackY->FittedLine->GetEntries(),GetSlopeY(),GetYAtZ(0),TrackY->Khi2OverNdf);
	}

void TrackClass::WriteStatisticsToFile(FILE* OutFile)
	{
	KonMap->WriteToFile(OutFile);
	CrossHitsMap->WriteToFile(OutFile);
	}

////////////////////////////////////////////////////////////////////////

double GetTrackSlopeDiff(TrackClass * Track1 , TrackClass * Track2)
	{
	return sqrt( (Track1->GetSlopeX()-Track2->GetSlopeX())*(Track1->GetSlopeX()-Track2->GetSlopeX()) + (Track1->GetSlopeY()-Track2->GetSlopeY())*(Track1->GetSlopeY()-Track2->GetSlopeY()) );
	}


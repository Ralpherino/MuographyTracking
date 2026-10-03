////////////////////////////////////////////////////////////////////////
//  MultiTrack Parameters Class - implementaion
//      version : 2020-03-13
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiTrackAnalysisClass.h"

MultiTrackAnalysisClass::MultiTrackAnalysisClass()
	{
	// These are the defaults as NULL
	NTracks=0;
	Track=NULL;
	Detector=NULL;
	NIntersections=0;
	Intersection=NULL;
	NScatterMaps=0;
	ScatterMap=NULL;
	MultiTrackListFile=NULL;
	DoWriteTracksFile=false;
	Verbose=0;
	EventIdDef=0;pEventId=&EventIdDef;pEventTimeDiff=&EventIdDef;
	SimulationInfo=NULL;
	// Filename defaults
	sprintf(FileNaming,"Results/a");
	sprintf(DirName,"Results");
	}

MultiTrackAnalysisClass::~MultiTrackAnalysisClass()
	{
	for (int NTrack=0; NTrack<NTracks; NTrack++) delete Track[NTrack];
	free(Track);
	for (int NIs=0; NIs<NIntersections; NIs++) delete Intersection[NIs];
	free(Intersection);
	for (int NSm=0; NSm<NScatterMaps; NSm++) delete ScatterMap[NSm];
	free(ScatterMap);
	if (MultiTrackListFile!=NULL) fclose(MultiTrackListFile);
	}

bool MultiTrackAnalysisClass::ReadSettingsFromFile(char* FileName)
	{
	if (Verbose>0) printf("MultiTrack : ReadFromFile\n");
	FILE * MAFile = fopen(FileName,"r");
	if (MAFile==NULL) { printf("Warning ! MultiTrack Parameters File : %s cannot be opened\n",FileName); return false; }
	char	ParName[50],StringAnswer[50];
	int	TrackNum=-1, IntersectionNum=-1, ScatterMapNum=-1;
	while (!feof(MAFile))
		{
		if (feof(MAFile)) break;
		sprintf(ParName,"#");
		fscanf(MAFile,"%s ",ParName);
		if (ParName[0]=='#') { SkipLine(MAFile); continue; }
		else if (strcmp(ParName,"WriteTracksFile")==0)
			{
			fscanf(MAFile,"%s",StringAnswer);
			DoWriteTracksFile=GetYesNoAnswer(StringAnswer);
			}
		else if (strcmp(ParName,"NTracks")==0)
			{
			fscanf(MAFile,"%d",&NTracks);
			Track = (TrackClass**) calloc(NTracks,sizeof(TrackClass*));
			for (int NTrack=0; NTrack<NTracks; NTrack++) Track[NTrack] = new TrackClass();
			}
		else if (strcmp(ParName,"Track")==0)
			{
			TrackNum++;
			if (TrackNum>=NTracks) { printf("WARNING : Too many tracks wrt NTracks !\n"); return false; }
			Track[TrackNum]->ReadSettingsFromFile(MAFile);
			Track[TrackNum]->pEventId=pEventId;
			Track[TrackNum]->pEventTimeDiff=pEventTimeDiff;
			if (Detector!=NULL) Track[TrackNum]->Set(Detector);
			}
		else if (strcmp(ParName,"NIntersections")==0)
			{
			fscanf(MAFile,"%d",&NIntersections);
			Intersection = (TrackIntersectionClass**) calloc(NIntersections,sizeof(TrackIntersectionClass*));
			for (int NIs=0; NIs<NIntersections; NIs++) Intersection[NIs] = new TrackIntersectionClass(this,NIs);
			}
		else if (strcmp(ParName,"Intersection")==0)
			{
			IntersectionNum++;
			if (IntersectionNum>=NIntersections) { printf("WARNING : Too many intersections wrt NIntersections !\n"); return false; }
			Intersection[IntersectionNum]->ReadFromFile(MAFile);
			}
		else if (strcmp(ParName,"NScatterMaps")==0)
			{
			fscanf(MAFile,"%d",&NScatterMaps);
			ScatterMap = (ScatterMapClass**) calloc(NScatterMaps,sizeof(ScatterMapClass*));
			for (int NSm=0; NSm<NScatterMaps; NSm++) ScatterMap[NSm] = new ScatterMapClass(this,NSm);
			}
		else if (strcmp(ParName,"ScatterMap")==0)
			{
			ScatterMapNum++;
			if (ScatterMapNum>=NScatterMaps) { printf("WARNING : Too many ScatterMap:%d wrt NScatterMaps:%d !\n",ScatterMapNum,NScatterMaps); return false; }
			if (ScatterMapNum>0) ScatterMap[ScatterMapNum]->CopyParameters(ScatterMap[ScatterMapNum-1]);
			ScatterMap[ScatterMapNum]->ReadFromFile(MAFile);
			}
		else { printf("\nWARNING : Incorrect MultiTrack parameter : %s\n\n",ParName); return false; }
		}
	fclose(MAFile);
	// Check info consistency
	ScatterMapNum++; TrackNum++;
	if (ScatterMapNum!=NScatterMaps) printf("WARNING : Missing Scatter maps! Found %d, instead of the given %d\n",ScatterMapNum,NScatterMaps);
	NScatterMaps=ScatterMapNum;
	if (TrackNum!=NTracks) printf("WARNING : Missing Track definitions ! Found %d, instead of the given %d\n",TrackNum,NTracks);
	NTracks=TrackNum;
	// Verbose
	if (Verbose>1) printf("\tRead: Tracks:%d , Intersections:%d , ScatterMaps:%d\n",NTracks,NIntersections,NScatterMaps);
	return true;
	}

bool MultiTrackAnalysisClass::WriteSettingsToFile(FILE* OutFile)
	{
	if (OutFile==NULL) return false;
	fprintf(OutFile,"#MultiTrackAnalysisParameters\n");
	fprintf(OutFile,"NTracks\t%d\n",NTracks);
	for (int NTrack=0; NTrack<NTracks; NTrack++) Track[NTrack]->WriteSettingsToFile(OutFile);
	fprintf(OutFile,"\n#End of MultiTrackAnalysisParameters\n\n\n");
	return true;
	}

void MultiTrackAnalysisClass::SetDetector(DetectorClass* InDetector)
	{
	Detector = InDetector;
	for (int NTrack=0; NTrack<NTracks; NTrack++)
		Track[NTrack]->Set(Detector);
	}

void MultiTrackAnalysisClass::SetFileNaming(char* InFileNaming, char* InDirName)
	{
	sprintf(FileNaming,"%s",InFileNaming);
	sprintf(DirName,"%s",InDirName);
	if (DoWriteTracksFile)
		{
		sprintf(MultiTrackListFileName,"%s/%s.ml",DirName,FileNaming);
		MultiTrackListFile = fopen(MultiTrackListFileName,"w");
		if (MultiTrackListFile==NULL) printf("Cannot open for write : %s\n",MultiTrackListFileName);
		fprintf(MultiTrackListFile,"#EvId\tTrackName TrackId\tSlopeX SlopeY PosX PosY KonX KonY\n");
		//fprintf(MultiTrackListFile,"#EvId\tIsId\tX\tY\tZ\tS\tSx\tZx\tSy\tZy\n");
		for (int NIs=0; NIs<NIntersections; NIs++) Intersection[NIs]->OutFile=MultiTrackListFile;
		}
	}

void MultiTrackAnalysisClass::Get()
	{
	for (int NTrack=0; NTrack<NTracks; NTrack++)
		Track[NTrack]->Get();
	for (int NIs=0; NIs<NIntersections; NIs++)
		Intersection[NIs]->Check();
	for (int NSm=0; NSm<NScatterMaps; NSm++)
		ScatterMap[NSm]->Compute();
	if (DoWriteTracksFile) WriteTracksFile();
	
	
	// TEST
/*	double ZTarget=1215, ZAbs1=325, ZAbs2=120;
	double ScatTarget,ScatAbs;

	Track[0]->Get();
	Track[1]->SetFixPoint(ZTarget, Track[0]->GetXAtZ(ZTarget), Track[0]->GetYAtZ(ZTarget) );
	Track[1]->Get();
	if (Track[0]->Good && Track[1]->Good) ScatTarget=GetTrackSlopeDiff(Track[0],Track[1]); else return;
	Track[2]->SetFixPoint(ZAbs1, Track[1]->GetXAtZ(ZAbs1), Track[1]->GetYAtZ(ZAbs1) );
	Track[2]->Get();
	if (Track[2]->Good) ScatAbs= GetTrackSlopeDiff(Track[1],Track[2]); else ScatAbs=-1;

//	if (Track[0]->Good && Track[1]->Good)
//		printf("%g %g\t%g %g\t%d %d\n",Track[0]->GetXAtZ(ZTarget),Track[0]->GetYAtZ(ZTarget),ScatTarget,ScatAbs, Track[3]->Chamber[0]->NHitsX+Track[3]->Chamber[0]->NHitsY, Track[3]->Chamber[1]->NHitsX+Track[3]->Chamber[1]->NHitsY);
	
	
*/	
/*	double Scat[3]={-2,-2,-2};
	if (Track[0]->Good && Track[1]->Good) Scat[0]=GetTrackSlopeDiff(Track[0],Track[1]); else Scat[0]=-2;
	if ( (Track[2]->Chamber[0]->IsTrackCrossed(Track[1])) && (Track[2]->Chamber[0]->MaxNClustersX==1) && (Track[2]->Chamber[0]->MaxNClustersY==1) ) Scat[1]=GetTrackSlopeDiff(Track[1],Track[2]);
	if ( (Track[2]->Chamber[0]->IsTrackCrossed(Track[1])) && (Track[2]->Chamber[0]->MaxNClustersX==0) && (Track[2]->Chamber[0]->MaxNClustersY==0) ) Scat[1]=-1;
	if ( ! (Track[2]->Chamber[0]->IsTrackCrossed(Track[1])) ) Scat[1]=-3;
	if ( (Track[3]->Chamber[0]->IsTrackCrossed(Track[2])) && (Track[3]->Chamber[0]->MaxNClustersX==1) && (Track[3]->Chamber[0]->MaxNClustersY==1) ) Scat[2]=GetTrackSlopeDiff(Track[2],Track[3]);
	if ( (Track[3]->Chamber[0]->IsTrackCrossed(Track[2])) && (Track[3]->Chamber[0]->MaxNClustersX==0) && (Track[3]->Chamber[0]->MaxNClustersY==0) ) Scat[2]=-1;
	if ( ! (Track[3]->Chamber[0]->IsTrackCrossed(Track[2])) ) Scat[2]=-3;
*/	
//	if (Track[0]->Good && Track[1]->Good)
//		printf("%g %g \t %g %g %g\n",Track[0]->GetXAtZ(ZTarget),Track[0]->GetYAtZ(ZTarget),Scat[0],Scat[1],Scat[2]);
	
	
	
	// TEST END
	}

void MultiTrackAnalysisClass::WriteTracksFile()
	{
	for (int NTrack=0; NTrack<NTracks; NTrack++) if (Track[NTrack]->Good)
		{
		if (SimulationInfo==NULL)
			fprintf(MultiTrackListFile,"%s\n",Track[NTrack]->GetInfo());
		else
			fprintf(MultiTrackListFile,"%s %g\n",Track[NTrack]->GetInfo(),*SimulationInfo);
		}
		
	// TEMP : for Bhd-spec-bordereffcalc 
		//{
		//fprintf(MultiTrackListFile,"%s\t",Track[NTrack]->GetInfo());
		//for (int n=0;n<8;n++) fprintf(MultiTrackListFile," %d %d ",Detector->Chamber[n]->NHitsX,Detector->Chamber[n]->NHitsY);
		//fprintf(MultiTrackListFile,"\n");
		//}
	// TEMP : for MTL2HR
	//if (Track[0]->Good && Detector->Chamber[8]->
        //
	//fprintf(MultiTrackListFile,"%s\n",Track[NTrack]->GetInfo());

	}

void MultiTrackAnalysisClass::WriteMapsToFile()
	{
	char	ScatterMapsFileName[200];
	sprintf(ScatterMapsFileName,"%s/%s.mm",DirName,FileNaming);
	ScatterMapsFile = fopen(ScatterMapsFileName,"w");
	if (ScatterMapsFile==NULL) printf("Cannot open for write : %s\n",ScatterMapsFileName);
	// Header and info
	//fprintf(ScatterMapsFile,"# Scatter Maps file\n");
	// Scatter maps
	for (int NSm=0; NSm<NScatterMaps; NSm++) ScatterMap[NSm]->WriteToFile(ScatterMapsFile);
	// Intersections
	for (int NIs=0; NIs<NIntersections; NIs++) Intersection[NIs]->ZDsMap->WriteToFile(ScatterMapsFile);
	fclose(ScatterMapsFile);
	}

void MultiTrackAnalysisClass::Print()
	{
	for (int NTrack=0; NTrack<NTracks; NTrack++)
		Track[NTrack]->Print();
	for (int NIs=0; NIs<NIntersections; NIs++)
		Intersection[NIs]->Print();
	for (int NSm=0; NSm<NScatterMaps; NSm++)
		ScatterMap[NSm]->Print();
	}

TrackClass * MultiTrackAnalysisClass::GetTrackBy(char* Specifier, char* Parameter)
	{
	int	TrackNum=-1;
	if (strcmp(Specifier,"Num")==0)
		{ sscanf(Parameter,"%d",&TrackNum); return Track[TrackNum]; }
	else if (strcmp(Specifier,"Name")==0)
		{ return GetTrackByName(Parameter); }
	else if (strcmp(Specifier,"Id")==0)
		{ sscanf(Parameter,"%d",&TrackNum); return GetTrackById(TrackNum); }
	return NULL;
	}

TrackClass * MultiTrackAnalysisClass::GetTrackByName(char* InName)
	{
	int ThisNTrack=-1;
	for (int NTrack=0; NTrack<NTracks; NTrack++)
		if (strcmp(Track[NTrack]->Name,InName)==0) ThisNTrack=NTrack;
	if (ThisNTrack>-1) return Track[ThisNTrack];
	else return NULL;
	}

TrackClass * MultiTrackAnalysisClass::GetTrackById(int InId)
	{
	int ThisNTrack=-1;
	for (int NTrack=0; NTrack<NTracks; NTrack++)
		if (Track[NTrack]->Id==InId) ThisNTrack=NTrack;
	if (ThisNTrack>-1) return Track[ThisNTrack];
	else return NULL;
	}

////////////////////////////////////////////////////////////////////////

TrackIntersectionClass::TrackIntersectionClass(MultiTrackAnalysisClass	*InMultiTrack, int InId)
	{
	MultiTrack=InMultiTrack;
	Id=InId;
	OutFile=MultiTrack->MultiTrackListFile;
	pEventId=MultiTrack->pEventId;
	ZDsMap=NULL;
	}

TrackIntersectionClass::~TrackIntersectionClass()
	{
	if (ZDsMap!=NULL) delete ZDsMap;
	}

bool TrackIntersectionClass::ReadFromFile(FILE * InFile)
	{
	bool	DoRead=true;
	char	Parameter[20],Parameter2[20];
	while (DoRead)
		{
		fscanf(InFile,"%s",Parameter);
		if (feof(InFile)) DoRead=false;
		if (Parameter[0]=='#') { SkipLine(InFile); continue; }
		if (strcmp(Parameter,"Name")==0)	fscanf(InFile,"%s",Name);
		else if (strcmp(Parameter,"Tracks")==0)
			{
			fscanf(InFile,"%s %s",Parameter,Parameter2);
			Track1 = MultiTrack->GetTrackBy(Parameter,Parameter2);
			fscanf(InFile,"%s %s",Parameter,Parameter2);
			Track2 = MultiTrack->GetTrackBy(Parameter,Parameter2);
			if (Track1==NULL || Track2==NULL) { printf("\nWARNING : Intersection : Incorrect Track identifications!\n"); return false; }
			}
		else if (strcmp(Parameter,"End")==0)	DoRead=false;
		else { printf("\nWARNING : Incorrect Intersection parameter: %s\n\n",Parameter); return false; }
		}
	InitClasses();
	return true;
	}

void TrackIntersectionClass::SetTracks(TrackClass * InTrack1, TrackClass * InTrack2)
	{
	Track1=InTrack1;
	Track2=InTrack2;
	if (Track1==NULL || Track2==NULL) printf("Intersection : tracks are not defined!\n");
	}

void TrackIntersectionClass::InitClasses()
	{
	ZDsMap = new MultiMapClass(1,Track1->ZMid,Track2->ZMid,10,0,1.0,0.010,(char*)"ZDsMap");
	}

bool TrackIntersectionClass::Check()
	{
	if ( (!(Track1->Good)) || (!(Track2->Good)) ) 
		{ Intersect=false; return false; }
	
	double SlopeDiffMin=0.030; //0.020;
	if ( ( fabs(Track1->GetSlopeX()-Track2->GetSlopeX()) < SlopeDiffMin ) || ( fabs(Track1->GetSlopeX()-Track2->GetSlopeX()) < SlopeDiffMin ) )
		{ Intersect=false; return false; }
	
	dSx=Track1->GetSlopeX()-Track2->GetSlopeX();
	Zx=(Track2->GetXAtZ(0)-Track1->GetXAtZ(0))/dSx;
	Xc=Track1->GetXAtZ(Zx);
	
	dSy=Track1->GetSlopeY()-Track2->GetSlopeY();
	Zy=(Track2->GetYAtZ(0)-Track1->GetYAtZ(0))/dSy;
	Yc=Track1->GetYAtZ(Zy);

	dS=sqrt(dSx*dSx+dSy*dSy);
	Z=(Zx+Zy)/2.0;
	//fprintf(OutFile,"%ld\t%d\t%g\t%g\t%g\t%g\t%g %g %g %g\n",*pEventId,Id,Xc,Yc,Z,dS,dSx,Zx,dSy,Zy);
	
	ZDsMap->Add(0,Z,dS);
	
	return true;
	}

void TrackIntersectionClass::Print()
	{
	printf("Intersection :: %s :: dSx: %g , Xc : %g , Zx : %g\n",Name,dSx,Xc,Zx);
	}


////////////////////////////////////////////////////////////////////////

ScatterMapClass::ScatterMapClass(MultiTrackAnalysisClass *InMultiTrack, int InId)
	{
	MultiTrack=InMultiTrack;
	Id=InId;	
	Map=NULL;
	ZLevel=0;
	BinSize=1;
	ShiftX=0;ShiftY=0;
	DMax=30;
	ScatterMin=0.100; ScatterMax=0.500;
	Scatter=0;
	Region=NULL;
	NRegions=0;
	}

ScatterMapClass::~ScatterMapClass()
	{
	if (Map!=NULL) delete Map;
	for (int NR=0; NR<NRegions; NR++) delete Region[NR];
	free(Region);
	}

bool ScatterMapClass::ReadFromFile(FILE * InFile)
	{
	bool	DoRead=true;
	char	Parameter[20],Parameter2[20];
	int	RegionNum=-1;
	while (DoRead)
		{
		fscanf(InFile,"%s",Parameter);
		if (feof(InFile)) DoRead=false;
		if (Parameter[0]=='#') { SkipLine(InFile); continue; }
		if (strcmp(Parameter,"Name")==0)		fscanf(InFile,"%s",Name);
		else if (strcmp(Parameter,"Z")==0)		fscanf(InFile,"%lf",&ZLevel);
		else if (strcmp(Parameter,"Shift")==0)		fscanf(InFile,"%lf %lf",&ShiftX,&ShiftY);
		else if (strcmp(Parameter,"BinSize")==0)	fscanf(InFile,"%lf",&BinSize);
		else if (strcmp(Parameter,"ScatterMin")==0)	fscanf(InFile,"%lf",&ScatterMin);
		else if (strcmp(Parameter,"ScatterMax")==0)	fscanf(InFile,"%lf",&ScatterMax);
		else if (strcmp(Parameter,"DMax")==0)		fscanf(InFile,"%lf",&DMax);
		else if (strcmp(Parameter,"Tracks")==0)
			{
			fscanf(InFile,"%s %s",Parameter,Parameter2);
			Track1 = MultiTrack->GetTrackBy(Parameter,Parameter2);
			fscanf(InFile,"%s %s",Parameter,Parameter2);
			Track2 = MultiTrack->GetTrackBy(Parameter,Parameter2);
			if (Track1==NULL || Track2==NULL) { printf("\nWARNING : ScatterMap : Incorrect Track identifications!\n\n"); return false; }
			}
		else if (strcmp(Parameter,"NRegions")==0)
			{
			fscanf(InFile,"%d",&NRegions);
			Region = (RegionClass**) calloc(NRegions,sizeof(RegionClass*));
			for (int NR=0; NR<NRegions; NR++) Region[NR] = new RegionClass();
			}
		else if (strcmp(Parameter,"Region")==0)
			{
			RegionNum++;
			if (RegionNum>=NRegions) { printf("WARNING : Too many Regions (%d) wrt NRegions (%d) !\n",RegionNum,NRegions); return false; }
			Region[RegionNum]->ReadFromFile(InFile);
			}
		else if (strcmp(Parameter,"End")==0)	DoRead=false;
		else { printf("\nWARNING : Incorrect ScatterMap parameter: %s\n\n",Parameter); return false; }
		}
	InitClasses();
	return true;
	}

bool ScatterMapClass::CopyParameters(ScatterMapClass* OtherSM)
	{
	if (OtherSM==NULL) return false;
	sprintf(Name,"%s",OtherSM->Name);
	ZLevel=OtherSM->ZLevel;
	ShiftX=OtherSM->ShiftX;
	ShiftY=OtherSM->ShiftY;
	BinSize=OtherSM->BinSize;
	ScatterMin=OtherSM->ScatterMin;
	ScatterMax=OtherSM->ScatterMax;
	Track1=OtherSM->Track1;
	Track2=OtherSM->Track2;
	return true;
	}

double ScatterHistBinSizeFunc(double x)
	{
	return exp(x*10);
	}

void ScatterMapClass::InitClasses()
	{
	double MapSizeX=Track2->Chamber[0]->SizeX*Track2->Chamber[0]->PadSizeX;
	double MapSizeY=Track2->Chamber[0]->SizeY*Track2->Chamber[0]->PadSizeY;
	Map = new MultiMapClass(5,0+ShiftX,MapSizeX+ShiftX,BinSize,0+ShiftY,MapSizeY+ShiftY,BinSize,Name);
	Map->SetName(IdN,(char*)"N"); 
	Map->SetName(IdNScatter,(char*)"NScat"); Map->SetName(IdSumScatter,(char*)"SumScat");
	Map->SetName(IdOnlyFirst,(char*)"10"); Map->SetName(IdOnlySecond,(char*)"01");
	//ScatterInRegions = new MultiHistClass(NRegions,0,1.000,0.010,(char*)"ScatterInRegions");
	//ScatterInRegions = new MultiHistClass(NRegions,0,1.000,0.001,(char*)"ScatterInRegions");
	//for (int NR=0; NR<NRegions; NR++) ScatterInRegions->SetName(NR,Region[NR]->Name);
	ScatterInRegions = new MultiHistFuncBinClass(NRegions,0,1.000,0.001,&ScatterHistBinSizeFunc,(char*)"ScatterInRegions");
	}

void ScatterMapClass::Compute()
	{
	// the Track1 extrapolated to Track2 end shall be within the chamber
	Scatter=0;
	double	ExtrapolContour=30;
	if (
		(Track1->GetXAtZ(Track2->Chamber[Track2->NChambers-1]->PositionZ) < Track2->Chamber[Track2->NChambers-1]->PositionX+ExtrapolContour) ||
		(Track1->GetXAtZ(Track2->Chamber[Track2->NChambers-1]->PositionZ) > Track2->Chamber[Track2->NChambers-1]->SizeX*Track2->Chamber[Track2->NChambers-1]->PadSizeX+Track2->Chamber[Track2->NChambers-1]->PositionX-ExtrapolContour) ||
		(Track1->GetYAtZ(Track2->Chamber[Track2->NChambers-1]->PositionZ) < Track2->Chamber[Track2->NChambers-1]->PositionX+ExtrapolContour) ||
		(Track1->GetYAtZ(Track2->Chamber[Track2->NChambers-1]->PositionZ) > Track2->Chamber[Track2->NChambers-1]->SizeY*Track2->Chamber[Track2->NChambers-1]->PadSizeY+Track2->Chamber[Track2->NChambers-1]->PositionY-ExtrapolContour) )
		return;
	// Only one of them are ok
	if ( (Track1->Good) && (Track2->NClustersTotalX==0) && (Track2->NClustersTotalY==0) )
		Map->Add(IdOnlyFirst,Track1->GetXAtZ(ZLevel)+ShiftX,Track1->GetYAtZ(ZLevel)+ShiftY);
	if ( (!(Track1->Good)) && (Track2->Good) )
		Map->Add(IdOnlySecond,Track2->GetXAtZ(ZLevel)+ShiftX,Track2->GetYAtZ(ZLevel)+ShiftY);
	if (!(Track1->Good && Track2->Good)) return;
	// From now on, both Tracks are good
	double	dSx,dSy,DX,DY;
	dSx=Track1->GetSlopeX()-Track2->GetSlopeX();
	dSy=Track1->GetSlopeY()-Track2->GetSlopeY();
	Scatter=sqrt(dSx*dSx+dSy*dSy);
	X=0.5*(Track1->GetXAtZ(ZLevel)+Track2->GetXAtZ(ZLevel))+ShiftX;
	Y=0.5*(Track1->GetYAtZ(ZLevel)+Track2->GetYAtZ(ZLevel))+ShiftY;
	DX=fabs(Track1->GetXAtZ(ZLevel)-Track2->GetXAtZ(ZLevel));
	DY=fabs(Track1->GetYAtZ(ZLevel)-Track2->GetYAtZ(ZLevel));
	if (DX>DMax || DY>DMax) return;
	Map->Add(IdN,X,Y);
	if (Scatter>ScatterMin && Scatter<ScatterMax) Map->Add(IdNScatter,X,Y);
	if (Scatter>ScatterMin && Scatter<ScatterMax) Map->Add(IdSumScatter,X,Y,Scatter);
	for (int NR=0; NR<NRegions; NR++) 
		if (Region[NR]->IsInside(X,Y)) ScatterInRegions->Add(NR,Scatter);
	}

void ScatterMapClass::Print()
	{
	printf("ScatterMap : %s\n",Name);
	}

bool ScatterMapClass::WriteToFile(FILE * OutFile)
	{
	if (OutFile==NULL) return false;
	Map->WriteToFile(OutFile);
	ScatterInRegions->WriteToFile(OutFile);
	for (int NR=0; NR<NRegions; NR++) ScatterInRegions->NormIntegral(NR,ScatterInRegions->Integral(0));
	ScatterInRegions->WriteToFile(OutFile);
	// Region comparision
	fprintf(OutFile,"# Region comparision\n#Id\tName");
	for (int NR=0; NR<NRegions; NR++) fprintf(OutFile,"\t%s",Region[NR]->Name); fprintf(OutFile,"\n");
	for (int NMLev=0; NMLev<NMapLevels; NMLev++)
		{
		fprintf(OutFile,"#%d\t%s",NMLev,Map->Name[NMLev]);
		for (int NR=0; NR<NRegions; NR++) 
			fprintf(OutFile,"\t%7.3lf",Map->AverageInRange(NMLev,Region[NR]->XMin,Region[NR]->XMax,Region[NR]->YMin,Region[NR]->YMax));
		fprintf(OutFile,"\n");
		}
	fprintf(OutFile,"\n\n");
	// Region coordinates
	fprintf(OutFile,"#Region coordinates\n");
	fprintf(OutFile,"XMin YMin"); for (int NR=0; NR<NRegions; NR++) fprintf(OutFile,"\t%g %g",Region[NR]->XMin,Region[NR]->YMin); fprintf(OutFile,"\n");
	fprintf(OutFile,"XMax YMin"); for (int NR=0; NR<NRegions; NR++) fprintf(OutFile,"\t%g %g",Region[NR]->XMax,Region[NR]->YMin); fprintf(OutFile,"\n");
	fprintf(OutFile,"XMax YMax"); for (int NR=0; NR<NRegions; NR++) fprintf(OutFile,"\t%g %g",Region[NR]->XMax,Region[NR]->YMax); fprintf(OutFile,"\n");
	fprintf(OutFile,"XMin YMax"); for (int NR=0; NR<NRegions; NR++) fprintf(OutFile,"\t%g %g",Region[NR]->XMin,Region[NR]->YMax); fprintf(OutFile,"\n");
	fprintf(OutFile,"XMin YMin"); for (int NR=0; NR<NRegions; NR++) fprintf(OutFile,"\t%g %g",Region[NR]->XMin,Region[NR]->YMin); fprintf(OutFile,"\n");
	fprintf(OutFile,"\n\n");
	
	return true;
	}

////////////////////////////////////////////////////////////////////////

RegionClass::RegionClass()
	{
	sprintf(Name,"Region");
	}

bool RegionClass::ReadFromFile(FILE* InFile)
	{
	int NConv=fscanf(InFile,"%s %lf %lf %lf %lf ",Name,&XMin,&XMax,&YMin,&YMax);
	if (NConv!=5) return false;
	return true;
	}

bool RegionClass::IsInside(double X, double Y)
	{
	if (X<XMin || X>XMax || Y<YMin || Y>YMax) return false;
	else return true;
	}


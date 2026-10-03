////////////////////////////////////////////////////////////////////////
// Tracking for a Detector with several 2dim chambers - implementation
//      version : 2022-08-23
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "Tracking.h"

int main(int ArgC, char** ArgV)
{
setbuf(stdout,NULL);
printf("Start : Tracking\n");
ReadArguments(ArgC,ArgV);
InitEbeList();
ReadInfo();
SetResultsFilenaming();
AssignData();

printf("Event loop ...\n");
OpenNextEbeFile();
SkipFirstEvents(SkipFirstNEvent);
while (ReadNextEvent())
	{
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++) Chamber[NChamber]->GoodEvent();
	Detector->GoodEvent();
	ClusterizeAll();
	TrackMain->Get();
	if (TrackMain->Good) Detector->GoodTrack(TrackMain);
	if (ShallComputeResults) for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		PerformanceTrack[NChamber]->Get();
		//if ( (PerformanceTrack[NChamber]->Good) && (PerformanceTrack[NChamber]->NTriggers >= Detector->TriggerCoinc) && (PerformanceTrack[NChamber]->NClustersAverage < 1.1) )
		if (PerformanceTrack[NChamber]->Good)
			Chamber[NChamber]->GoodTrack(PerformanceTrack[NChamber]);
		}
	if (ShallWriteTracksFile) WriteTracksFile();
	if (DoMultiTrack) MultiTrack->Get();
	ShowEvent();	
	}
printf("Analysed NEvents : %ld , NTracks : %ld\n",Detector->NEvents,Detector->NTracks);
ComputeResults();
PrintInfo();
WriteResultsToFile();
WriteSpecialResultsToFile();
CrateGnuplotHeader();
MoveResults();
CloseProgram();

}

////////////////////////////////////////////////////////////////////////

void ReadArguments(int ArgC, char** ArgV)
	{
	if (Verbose>0) printf("Reading arguments ...\n");
	int PrevalenceDotRelative;
	if (ArgC<2) Help(0);
	for (int i=1; i<ArgC; i++)
		if (strcmp(ArgV[i],"-h")==0 || strcmp(ArgV[i],"--help")==0 )
			{ Help(); }
		else if (strcmp(ArgV[i],"-e")==0 || strcmp(ArgV[i],"--ebefile")==0 )
			{ i++; sprintf(EbeFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-l")==0 || strcmp(ArgV[i],"--ebe-list")==0 )
			{ i++; sprintf(EbeListFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-en")==0 || strcmp(ArgV[i],"--ebe-naming")==0 )
			{ i++; sprintf(EbeFileNaming,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-b")==0 || strcmp(ArgV[i],"--basic-info")==0 )
			{ i++; sprintf(DetectorBasicsFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-i")==0 || strcmp(ArgV[i],"--info-in")==0 )
			{ i++; sprintf(DetectorInfoInFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-r")==0 || strcmp(ArgV[i],"--results")==0 )
			{ i++; sprintf(ResultsFilenaming,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-rd")==0 || strcmp(ArgV[i],"--dir-results")==0 )
			{ i++; sprintf(ResultsDirName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-t")==0 || strcmp(ArgV[i],"--tracks")==0 )
			{ i++; ShallWriteTracksFile=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-c")==0 || strcmp(ArgV[i],"--compute")==0 )
			{ i++; ShallComputeResults=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-g")==0 || strcmp(ArgV[i],"--gnuplot")==0 )
			{ i++; ShallCreateGnuplotHeader=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-gp")==0 || strcmp(ArgV[i],"--plot")==0 )
			{ i++; sprintf(GnuplotPlotFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-sp")==0 || strcmp(ArgV[i],"--show-plots")==0 )
			{ i++; ShowPlots=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-ap")==0 || strcmp(ArgV[i],"--analysis-parameters")==0 )
			{ i++; sprintf(AnalysisParametersFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-aps")==0 || strcmp(ArgV[i],"--analysis-parameter-set")==0 )
			{ i++; sprintf(ApExtraParName[NApExtras],"%s",ArgV[i]); i++; ApExtraParValue[NApExtras]=GetPostfixedNumber(ArgV[i]); NApExtras++; }
		else if (strcmp(ArgV[i],"-ma")==0 || strcmp(ArgV[i],"--multi-track-analysis")==0 )
			{ i++; sprintf(MultiTrackAnalysisParametersFileName,"%s",ArgV[i]); DoMultiTrack=true; }
		else if (strcmp(ArgV[i],"-ri")==0 || strcmp(ArgV[i],"--run-level-info")==0 )
			{ i++; sprintf(RunLevelInfoFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-me")==0 || strcmp(ArgV[i],"--max-events")==0 )
			{ i++; MaxNEvents=long(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-ee")==0 || strcmp(ArgV[i],"--every")==0 )
			{ i++; EveryNthEvent=int(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-se")==0 || strcmp(ArgV[i],"--skip-first")==0 )
			{ i++; SkipFirstNEvent=int(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-p")==0 || strcmp(ArgV[i],"--prevalence")==0 )
			{ i++; Prevalence=int(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-pd")==0 || strcmp(ArgV[i],"--prevalence-dot")==0 )
			{ i++; PrevalenceDot=int(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-pdr")==0 || strcmp(ArgV[i],"--prevalence-dot-relative")==0 )
			{ i++; sscanf(ArgV[i],"%d",&PrevalenceDotRelative); if (PrevalenceDotRelative>1) PrevalenceDot=Prevalence/PrevalenceDotRelative;}
		else if (strcmp(ArgV[i],"-s")==0 || strcmp(ArgV[i],"--show-events")==0 )
			{ i++; sscanf(ArgV[i],"%d",&MaxNShownEvents); }
		else if (strcmp(ArgV[i],"--start")==0 || strcmp(ArgV[i],"--start-run")==0 )
			{ ShallCorrectDateTime=true; i++; sscanf(ArgV[i],"%s",StartRunDateTime); }
		else if (strcmp(ArgV[i],"--dtdt")==0 || strcmp(ArgV[i],"--difftime-as-datetime")==0 )
			{ i++; UseDiffTimeAsDateTimeInfo=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-v")==0 || strcmp(ArgV[i],"--verbose")==0 )
			{ i++; sscanf(ArgV[i],"%d",&Verbose); }
		else if (strcmp(ArgV[i],"-vs")==0 || strcmp(ArgV[i],"--speak")==0 )
			{ i++; DoSpeak=GetYesNoAnswer(ArgV[i]);  }
		else if (strcmp(ArgV[i],"-id")==0 || strcmp(ArgV[i],"--id")==0 )
			{ i++; sscanf(ArgV[i],"%d",&IdNumber); }
		else
			{ printf("\nBad argument : %s\n\n",ArgV[i]); Help(ErrWrongArgument); };
	// Check read values
	sprintf(EbeZipTempName,"%s/EbeZip%d.ebe",TempDirName,IdNumber);
	sprintf(EbeListFileNameTemp,"%s/EbeFileList%d.temp",TempDirName,IdNumber);
	if (Prevalence<10) Prevalence=10;
	if (PrevalenceDot<2) PrevalenceDot=2;
	}

void ReadInfo()
	{
	ReadAnalysisParameters();
	ReadDetectorBasics();
	ReadRunLevelInfo();
	InitializeClasses();
	ReadDetectorInfo();
	}

void ReadAnalysisParameters()
	{
	// Load AnalysisParameters, if specified
	AnalysisParameters = new AnalysisParametersClass();
	if (strcmp(AnalysisParametersFileName,"")!=0) 
		{
		RefineFileName(AnalysisParametersFileName,SettingsDirName,(char*)".ap");
		printf("Reading AnalysisParameters from : %s\n",AnalysisParametersFileName);
		AnalysisParameters->ReadFromFile(AnalysisParametersFileName);
		}
	else printf("Analysis Parameters : using the defaults\n");
	for (int NApe=0; NApe<NApExtras; NApe++)
		AnalysisParameters->Set(ApExtraParName[NApe],ApExtraParValue[NApe]);
	}

void ReadDetectorBasics()
	{
	if (strcmp(DetectorBasicsFileName,"")==0) return;
	RefineFileName(DetectorBasicsFileName,SettingsDirName,(char*)".db");
	printf("Reading DetectorBasics from : %s\n",DetectorBasicsFileName);
	// Open file
	DetectorInfoFile=fopen(DetectorBasicsFileName,"r");
	if (DetectorInfoFile==NULL) { printf("Cannot read DetectorBasics file : %s\nExit\n",DetectorBasicsFileName); Help(ErrDetectorInfo); }
	// Read info
	char	ParName[50];
	while (!feof(DetectorInfoFile))
		{
		ParName[0]='\0';
		//sprintf(ParName,"\0"); // reset ParName 
		fscanf(DetectorInfoFile,"%s",ParName);
		if (strlen(ParName)<1) continue;
		if (ParName[0]=='#') { SkipLine(DetectorInfoFile,1); continue; }
		if (strcmp(ParName,"DetectorName")==0)
			fscanf(DetectorInfoFile,"%s",DetectorName);
		else if (strcmp(ParName,"NChambers")==0)
			{
			fscanf(DetectorInfoFile,"%d",&MaxNChambers);
			Chamber = (ChamberClass**) calloc(MaxNChambers,sizeof(ChamberClass*));
			for (int NChamber=0; NChamber<MaxNChambers; NChamber++) Chamber[NChamber] = new ChamberClass();
			for (int NChamber=0; NChamber<MaxNChambers; NChamber++) Chamber[NChamber]->SetAnalysisParameters(AnalysisParameters);
			}
		else if (strcmp(ParName,"NAdcs")==0)
			{
			fscanf(DetectorInfoFile,"%d %*s",&MaxNAdcs);
			AssignAdc = (int*) calloc(MaxNAdcs,sizeof(int));
			AdcArray = (int*) calloc(MaxNAdcs,sizeof(int));
			for (int NAdc=0; NAdc<MaxNAdcs; NAdc++) fscanf(DetectorInfoFile,"%d",&(AssignAdc[NAdc]));
			}
		else if (strcmp(ParName,"NThps")==0)
			{
			fscanf(DetectorInfoFile,"%d",&MaxNThps);
			if (MaxNThps==0) continue;
			Thp = (ThpClass**) calloc(MaxNThps,sizeof(ThpClass*));
			for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp] = new ThpClass(AnalysisParameters->TimeDepBinSizeSeconds);
			for (int NThp=0; NThp<MaxNThps; NThp++) Thp[NThp]->TimeDev->SetAutoScale(AnalysisParameters->MaxNTimeBins);
			}
		else if (strcmp(ParName,"NChecks")==0)
			fscanf(DetectorInfoFile,"%d",&MaxNChecks);
		else if (strcmp(ParName,"HvControl")==0)
			{
			fscanf(DetectorInfoFile,"%d",&HvControlInt);
			if (HvControlInt==1) HvInfo = new HvClass(AnalysisParameters->TimeDepBinSizeSeconds,AnalysisParameters->MaxNTimeBins);
			}
		else if (strcmp(ParName,"Trigger")==0)
			{
			fscanf(DetectorInfoFile,"%d %*s",&MaxNTriggerBits);
			AssignTriggerBit = (int*) calloc(MaxNTriggerBits,sizeof(int));
			for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++) fscanf(DetectorInfoFile,"%d",&(AssignTriggerBit[NTriggerBit]));
			}
		else if (strcmp(ParName,"PadSizeMm")==0)
			{ fscanf(DetectorInfoFile,"%lf",&PadSizeMm);
			for (int NChamber=0; NChamber<MaxNChambers; NChamber++) { Chamber[NChamber]->PadSizeX=PadSizeMm; Chamber[NChamber]->PadSizeY=PadSizeMm; } }
		else if (strcmp(ParName,"ChamberBasicsInfo")==0)
			for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
				Chamber[NChamber]->ReadBasicsFromFile(DetectorInfoFile);
		else if (strcmp(ParName,"TriggerCoinc")==0)
			fscanf(DetectorInfoFile,"%d",&TriggerCoinc);
		else if (strcmp(ParName,"EventFormat")==0)
			{
			char	EventFormatString[50];
			fscanf(DetectorInfoFile,"%s",EventFormatString);
			if (strcmp(EventFormatString,"20181015")==0) ReadEventFormat=&ReadEvent_Format_20181015;
			else if (strcmp(EventFormatString,"20201204")==0) ReadEventFormat=&ReadEvent_Format_20201204;
			else if (strcmp(EventFormatString,"2014")==0) ReadEventFormat=&ReadEvent_Format_2014;
			else if (strcmp(EventFormatString,"Mts4")==0) ReadEventFormat=&ReadEvent_Format_Mts4;
			else if (strcmp(EventFormatString,"PicMt2")==0) ReadEventFormat=&ReadEvent_Format_PicMt2;
			else printf("Unrecognized EventFormat: %s\n",EventFormatString);
			}
		else if (strcmp(ParName,"DeadTime")==0)
			fscanf(DetectorInfoFile,"%lf %lf",&DeadTimePerEventUs,&DeadTimeZipPerEventUsApprox);
		else if (strcmp(ParName,"NSimData")==0)
			{
			fscanf(DetectorInfoFile,"%d",&MaxNSimulationData);
			SimulationData = (double*) calloc(MaxNSimulationData,sizeof(double));
			}
		else 
			{ printf("Error ! DetectorBasics file : %s, unrecognized Parameter: %s\n",DetectorBasicsFileName,ParName); Help(ErrDetectorInfo); }
		}
	printf("Detector name : %s\n",DetectorName);
	fclose(DetectorInfoFile);
	}

void ReadRunLevelInfo()
	{
	// Check the RunLevelInfoFile
	if (strlen(RunLevelInfoFileName)<2) return;
	FILE * RunLevelInfoFile = fopen(RunLevelInfoFileName,"r");
	if (RunLevelInfoFile==NULL) { printf("Warning: Cannot open RunLevelInfo file : %s\n",RunLevelInfoFileName); return; }
	// Extract ThisRunName to compare later with the RunName params
	char	EbeFileNameFirst[200], InString[50];
	fscanf(EbeListFile,"%s",EbeFileNameFirst); rewind(EbeListFile);
	std::string ThisRunName(EbeFileNameFirst);
	char	RunName[50],ParName[50]; int FoundAt;
	while (!feof(RunLevelInfoFile))
		{
		sprintf(RunName,"-");
		fscanf(RunLevelInfoFile,"%s",RunName);
		if (RunName[0]=='#') { SkipLine(RunLevelInfoFile); continue; }
		FoundAt=ThisRunName.find(RunName);
		if (uint(FoundAt)==std::string::npos || FoundAt<0) { SkipLine(RunLevelInfoFile); continue; }
		if (isdigit(ThisRunName.at(FoundAt+strlen(RunName)))!=0) { SkipLine(RunLevelInfoFile); continue; }
		// Reaching here means RunName matching
		fscanf(RunLevelInfoFile,"%s",ParName);
		if (strcmp(ParName,"StartTime")==0)		{ fscanf(RunLevelInfoFile,"%s",StartRunDateTime); ShallCorrectDateTime=true; printf("RunLevelInfo: %s : StartTime : %s\n",RunName,StartRunDateTime); }
		else if (strcmp(ParName,"MaxNEvents")==0) 	{ fscanf(RunLevelInfoFile,"%d",&MaxNEvents); printf("RunLevelInfo: %s : MaxNEvents : %d\n",RunName,MaxNEvents); }
		else if (strcmp(ParName,"SkipFirstNEvents")==0) { fscanf(RunLevelInfoFile,"%d",&SkipFirstNEvent); printf("RunLevelInfo: %s : SkipFirstNEvents : %d\n",RunName,SkipFirstNEvent); }
		else if (strcmp(ParName,"SkipFirst")==0) 	{ fscanf(RunLevelInfoFile,"%d",&SkipFirstNEvent); printf("RunLevelInfo: %s : SkipFirstNEvents : %d\n",RunName,SkipFirstNEvent); }
		else if (strcmp(ParName,"TimeCorrection")==0) 	{ fscanf(RunLevelInfoFile,"%lf",&TimeCorrectionSec); printf("RunLevelInfo: %s : TimeCorrectionSec : %g\n",RunName,TimeCorrectionSec); }
		else if (strcmp(ParName,"DiffTimeAsDateTime")==0) { fscanf(RunLevelInfoFile,"%s",InString); UseDiffTimeAsDateTimeInfo=GetYesNoAnswer(InString); printf("RunLevelInfo: %s : DiffTimeAsDateTime : %c\n",RunName,(UseDiffTimeAsDateTimeInfo?'Y':'N')); }
		else { printf("Warning: Unrecognized RunLevelInfo ParameterName: %s\n",ParName); SkipLine(RunLevelInfoFile); continue; }
		}
	fclose(RunLevelInfoFile);
	// https://www.cplusplus.com/reference/string/string/find/
	
	}

void InitializeClasses()
	{
	// Detector
	AnalysisParameters->SetNChambers(MaxNChambers);
	Detector = new DetectorClass(MaxNChambers,Chamber,DetectorName);
	Detector->Verbose=Verbose;
	Detector->TriggerCoinc=TriggerCoinc;
	Detector->SetThp(MaxNThps,Thp);
	Detector->SetAnalysisParameters(AnalysisParameters);
	Detector->SetHvInfo(HvInfo);
	Detector->DeadTimePerEventUs=DeadTimePerEventUs;
	Detector->DeadTimeZipPerEventUsApprox=DeadTimeZipPerEventUsApprox;
	Detector->TimeCorrectionSec=TimeCorrectionSec;
	// Tracking
	TrackingNHits=AnalysisParameters->TrackingNHits;
	if (TrackingNHits<2 && TrackingNHits>MaxNChambers)
		{ printf("Warning : cannot set TrackingNHits=%d (using %d chambers)\n",TrackingNHits,MaxNChambers); exit(ErrDetectorInfo); }
	TrackMain = new TrackClass();
	TrackMain->Set(Detector,MaxNChambers,CountArray,TrackingNHits);
	TrackMain->SetId(0,(char*)"TrackMain");
	TrackMain->pEventId=&EventN;
	TrackMain->pEventTimeDiff=&EventTimeDiffUs;
	TrackMain->MaxKhi2OverNdf=AnalysisParameters->TrackKhi2Max;
	TrackMain->GoodKhi2OverNdf=AnalysisParameters->TrackKhi2Good;
	// Performance Tracks
	PerformanceTrack = (TrackClass**) calloc (MaxNChambers,sizeof(TrackClass*));
	int ChamberIdList[MaxNChambers];
	if ((MaxNChambers-1-AnalysisParameters->PerformanceTrackMiss)<2) AnalysisParameters->PerformanceTrackMiss=MaxNChambers-1-3;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		PerformanceTrack[NChamber] = new TrackClass();
		ArrayWithout(MaxNChambers,CountArray,NChamber,ChamberIdList);
		PerformanceTrack[NChamber]->Set(Detector,MaxNChambers-1,ChamberIdList,MaxNChambers-1-AnalysisParameters->PerformanceTrackMiss);

		//WarningModified();
		//TEMP : For the MTL-2-HR-extra
		//for (int NChamber=0; NChamber<MaxNChambers; NChamber++) ChamberIdList[NChamber]=NChamber;
		//PerformanceTrack[NChamber]->Set(Detector,8,ChamberIdList,6);
		//if (NChamber<8) PerformanceTrack[NChamber]->Set(Detector,MaxNChambers-1-2,ChamberIdList,MaxNChambers-1-2-AnalysisParameters->PerformanceTrackMiss);
		//else 		PerformanceTrack[NChamber]->Set(Detector,MaxNChambers-1-1,ChamberIdList,MaxNChambers-1-1-AnalysisParameters->PerformanceTrackMiss);
		}
	// DateTime
	EventDt = new DateTimeClass();
	DtShift = new DateTimeClass();
	if (ShallCorrectDateTime) DtShift->ReadString(StartRunDateTime);
	// Random
	srand(IdNumber);
	// MultiTrack analysis
	if (DoMultiTrack)
		{
		RefineFileName(MultiTrackAnalysisParametersFileName,SettingsDirName,(char*)".ma");
		MultiTrack = new MultiTrackAnalysisClass();
		MultiTrack->pEventId=&EventN;
		MultiTrack->pEventTimeDiff=&EventTimeDiffUs;
		MultiTrack->SetDetector(Detector);
		MultiTrack->ReadSettingsFromFile(MultiTrackAnalysisParametersFileName);
		MultiTrack->Verbose=Verbose;
		}
	}

void ReadDetectorInfo()
	{
	if (strcmp(DetectorInfoInFileName,"")==0) return;
	RefineFileName(DetectorInfoInFileName,ResultsDirName,(char*)".res");
	RefineFileName(DetectorInfoInFileName,SettingsDirName,(char*)".res");
	printf("Reading DetectorInfo from : %s\n",DetectorInfoInFileName);
	Detector->ReadInfoFromFile(DetectorInfoInFileName,true,true); // TODO : ez nem teljesen ok
	if (Verbose>1) Detector->Print();
	//// Open file
	//DetectorInfoFile=fopen(DetectorInfoInFileName,"r");
	//if (DetectorInfoFile==NULL) { printf("Cannot read DetectorInfoFile: %s\nExit\n",DetectorInfoInFileName); Help(ErrDetectorInfo); }
	//// Read the file
	//for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
	//	Chamber[NChamber]->ReadInfoFromFile(DetectorInfoFile);
	//Detector->ComputeResults();
	//fclose(DetectorInfoFile);
	}

void AssignData()
	{
	if (Verbose>1) printf("Assigning data and classes ...\n");
	pAdc = (int**) calloc(MaxNAdcs,sizeof(int*));
	for (int NAdc=0; NAdc<MaxNAdcs; NAdc++)
		if (AssignAdc[NAdc]!=-1) 
			pAdc[NAdc]=&(Chamber[AssignAdc[NAdc]]->AdcRaw);
		else pAdc[NAdc]=&(AdcArray[NAdc]);
	
	pTriggerBit = (int**) calloc(MaxNTriggerBits,sizeof(int*));
	for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++)
		if (AssignTriggerBit[NTriggerBit]!=-1)
			pTriggerBit[NTriggerBit]=&(Chamber[AssignTriggerBit[NTriggerBit]]->TriggerBit);
		else pTriggerBit[NTriggerBit]=&NullInt;

	EventDateTime = (char*) calloc(50,sizeof(char));
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		Chamber[NChamber]->DateTimeString = EventDateTime;
	Detector->DateTimeString=EventDateTime;
	Detector->pEventTimeDiffUs=&EventTimeDiffUs;
	// for simulations
	if ( (MaxNSimulationData>1) && (MultiTrack!=NULL) ) MultiTrack->SimulationInfo=&(SimulationData[1]);
	}

void CheckEventFormat()
	{
	//char Line[2000];
	EbeFile=fopen(EbeFileName,"r");
	if (EbeFile==NULL) { printf("Cannot read EbeFile: %s\nExit\n",EbeFileName); Help(ErrFileError); }
	// TODO : read the first line carefully, and estimate the info therein
	/*
	char	Text[20], Type[20];
	double	Value;
	int	NValue;
	MaxNAdcs=0;  MaxNThps=0;
	while ()
		{
		fscanf(EbeFile,"%s",Text);
		if ( sscanf(Text,"%g",&Value)==1 )
			{ NValue++; continue; }
		if ( strcmp(Type,"Adc")==0) )	MaxNAdcs=NValue;
		if ( strcmp(Type,"Thp")==0) )	MaxNThps=NValue/4;
		if ( strcmp(Type,"Adc")==0) )	MaxNAdcs=NValue;
		
		}
	*/
	fclose(EbeFile);
	}

void SetResultsFilenaming()
	{
	std::string HelpString="";
	int  NameBeg,NameEnd; bool ComposedEbe=false;
	// If no ResultsFilenaming is specified take from Ebe,EbeList
	if (strlen(ResultsFilenaming)<1)
		{
		// Example for EbeFileName :  ../Measurements/Mtl1_201808/Mtl1_Run100.part01.ebe.zip
		if (strlen(EbeListFileName)>0) HelpString=EbeListFileName;
		if (strlen(EbeFileName)>2) HelpString=EbeFileName;
		if (strlen(EbeFileNaming)>2) HelpString=EbeFileNaming;
		if (HelpString.find_first_of(' ',1)<HelpString.length()-1) ComposedEbe=true;
		NameBeg=HelpString.find_last_of('/')+1;
		NameEnd=HelpString.find_first_of('.',NameBeg);
		// there are more than one part
		if ( (!ComposedEbe) && (HelpString.compare(NameEnd+1,4,"part")==0) ) NameEnd=HelpString.find_first_of('.',NameEnd+1);
		sprintf(ResultsFilenaming,"%s",HelpString.substr(NameBeg,NameEnd-NameBeg).data());
		}
	// Set the filenames
	sprintf(ResultsFileName,"%s.res",ResultsFilenaming);
	sprintf(GnuplotHeaderFileName,"%s.gph",ResultsFilenaming);
	sprintf(TracksFileName,"%s.tl",ResultsFilenaming);
	sprintf(TracksInfoFileName,"%s.ti",ResultsFilenaming);
	sprintf(Detector->ResFilenaming,"%s",ResultsFilenaming);
	if (Verbose>1) printf("Results will be saved to : %s\n",ResultsFileName);
	if (DoMultiTrack) MultiTrack->SetFileNaming(ResultsFilenaming,ResultsDirName);
	// Open them if needed
	if (ShallWriteTracksFile)
		{
		TracksFile=fopen(TracksFileName,"w");
		if (TracksFile==NULL) 
			{ printf("TracksFile : %s could not be opened for write\n",TracksFileName); ShallWriteTracksFile=false; }
		else fprintf(TracksFile,"#TracksFile for %s\n#TrackSlopeX TrackSlopeY  PosX PosY  KonX KonY  EffArea\n",ResultsFilenaming); 
		}
	}

void InitEbeList()
	{
	if (Verbose>1) printf("Initializing Ebe file list ...\n");
	// Generate list for single file
	if (strlen(EbeFileName)>2)
		{
		sprintf(EbeListFileName,"%s",EbeListFileNameTemp);
		sprintf(Command,"echo %s > %s",EbeFileName,EbeListFileNameTemp);
		system(Command);
		}
	if (strlen(EbeFileNaming)>2)
		{
		sprintf(EbeListFileName,"%s",EbeListFileNameTemp);
		sprintf(Command,"echo %s*ebe* > %s",EbeFileNaming,EbeListFileNameTemp);
		system(Command);
		}
	// Check if List file exists and open it
	if ( strlen(EbeListFileName)<2 )
		{ printf("Ebe file or list shall be specified\nExit\n\n"); exit(100); }
	EbeListFile=fopen(EbeListFileName,"r");
	if (EbeListFile==NULL)
		{ printf("EbeListFile %s cannot be opened\n",EbeListFileName); exit(100); }
	// Q : structure for the list? eg: "Dir" "Mask" "List" ...
	
	NEbeFiles=0;
	while (!feof(EbeListFile))
		{
		fscanf(EbeListFile,"%*s ");
		NEbeFiles++;
		}
	rewind(EbeListFile);
	}

bool OpenNextEbeFile()
	{
	if (EbeFile!=NULL) { fclose(EbeFile); EbeFile=NULL; }
	Detector->NFiles++;
	if (NEvent>1) FirstEventOfFile=true;
	// Only one EbeFile, no list
	// Check next EbeFile
	NThisEbeFile++;
	if (NEventsPerFile<2 && NEvent>1) 
		{
		NEventsPerFile=NEvent;
		if (MaxNEvents>0) NTotalEventsEstimated=MaxNEvents; 
		else NTotalEventsEstimated=NEbeFiles*NEventsPerFile;
		Detector->NEventsPerFile=NEventsPerFile;
		}
	fscanf(EbeListFile,"%s",EbeFileName);
	if (feof(EbeListFile))
		{
		fclose(EbeListFile); EbeListFile=NULL;
		if (NEvent<1) { printf("Warning! No valid Ebe file has been found\nExit\n"); exit(100); }
		printf("\nReached End of EbeListFile\n");
		return false;
		}
	// Open next EbeFile
	if (Verbose>0) printf("\nOpen NextEbeFile : %s\n",EbeFileName);
	if (strcmp(&(EbeFileName[strlen(EbeFileName)-3]),"zip")==0) IsEbeZip=true; else IsEbeZip=false;
	// Unzip if needed
	if (IsEbeZip)
		{
		if (Verbose>1) printf("Unzipping file : %s \n",EbeFileName);
		sprintf(Command,"unzip -p %s > %s 2> /dev/null",EbeFileName,EbeZipTempName);
		CommandExitCode=system(Command);
		if (CommandExitCode!=0)
			{
			printf("\nWarning! Zip error in file : %s\n",EbeFileName);
			return OpenNextEbeFile();
			}
		EbeFile=fopen(EbeZipTempName,"r");
		}
	else
		EbeFile=fopen(EbeFileName,"r");
	// Check if file is ok
	if (EbeFile==NULL) 
		{
		printf("\nWarning! Cannot open EbeFile: %s\n",EbeFileName);
		return OpenNextEbeFile();
		}
	// Get NEventsPerFile from the first file
	if (NEvent<1)
		{
		char FileNameLineNum[200]="Temp/LineNum.txt";
		if (IsEbeZip) sprintf(Command,"wc -l %s > %s",EbeZipTempName,FileNameLineNum);
		else sprintf(Command,"wc -l %s > %s",EbeFileName,FileNameLineNum);
		system(Command);
		FILE * FileLineNum=fopen(FileNameLineNum,"r");
		if (FileLineNum==NULL) return true;
		fscanf(FileLineNum,"%d",&NEventsPerFile);
		if (NEventsPerFile>1) Detector->NEventsPerFile=NEventsPerFile;
		if (MaxNEvents>0) NTotalEventsEstimated=MaxNEvents;
		else NTotalEventsEstimated=NEbeFiles*NEventsPerFile;
		fclose(FileLineNum);
		}
	return true;
	}

void SkipFirstEvents(int InNSkip)
	{
	if (InNSkip<1) return;
	if (Verbose>1) printf("Skipping first %d events\n",InNSkip);
	int NSkipOk = SkipLine(EbeFile,InNSkip);
	if (NSkipOk==InNSkip) return;
	OpenNextEbeFile();
	SkipFirstEvents(InNSkip-NSkipOk);
	return;
	}

bool ReadNextEvent()
	{
	// Check if MaxNEvents reached
	if (NEvent==MaxNEvents) { printf(" Reached MaxNEvents=%d\n",MaxNEvents); return false; }
	// 
	NConversions=fscanf(EbeFile,"%ld",&EventFirstData);
	if (feof(EbeFile))
		{
		if ( ! OpenNextEbeFile() ) return false;
		NConversions=fscanf(EbeFile,"%ld",&EventFirstData);
		}
	if (NConversions==0) { SkipLine(EbeFile,1); printf("Wrong NEvent data in EBE file, skipping line.\n"); return ReadNextEvent(); }
	// Wts info
	WriteInfoToScreen();
	// Read the proper format
	EventReadOk=(*ReadEventFormat)();
	if (!EventReadOk) { SkipLine(EbeFile,1); printf("\nBug in data file, at EventN:%ld\n",EventN); return ReadNextEvent(); }
	// If EveryNthEvent 
	if (EveryNthEvent>1) SkipLine(EbeFile,EveryNthEvent-1);
	// For first event in file, add to Zip time
	if (FirstEventOfFile) { Detector->DeadTimeZipSec+=double(EventTimeDiffUs)/1e6; FirstEventOfFile=false; } 
	if (EventTimeDiffUs<0) printf("Warning: negative EventTimeDiffUs=%ld , at event %d (EventN:%ld)\n",EventTimeDiffUs,NEvent,EventN);
	return true;
	}

// To handle different EventFormats several EventReader had to be implemented,
// and for a more convenient handling it has been moved to a separate file:
#include "ReadEventFormats.cpp"

void CorrectDateTime()
	{
	if (DtShift->Year>1900) // compute DateTimeShift from the first event
		{
		EventDt->ReadString(EventDateTime);
		DtShift->Year-=EventDt->Year; DtShift->Month-=EventDt->Month; DtShift->Day-=EventDt->Day;
		DtShift->Day+=int(DtShift->Year*365.25)+int(DtShift->Month*30.5); DtShift->Month=0; DtShift->Year=0;
		DtShift->Hour-=EventDt->Hour; DtShift->Min-=EventDt->Min; DtShift->Sec-=EventDt->Sec;
		EventDt->AddDateTime(DtShift);
		}
	EventDt->ReadString(EventDateTime);
	EventDt->AddDateTime(DtShift);
	EventDt->MakeString(EventDateTime);
	}

void WriteInfoToScreen()
	{
	if (NEvent%Prevalence==0) printf("(%3.1lfM) ",double(NEvent)/1e6);
	NEvent++;
	if (NEvent%PrevalenceDot==0) printf("."); //fflush(stdout);
	if (NEvent%Prevalence==0) 
		{
		printf("  NEvent: %8d  with EventId: %8ld (T:%s) ",NEvent,EventN+1,EventDateTime);
		if (NTotalEventsEstimated>0) printf(" %4.1lf%% ",100.*double(NEvent)/double(NTotalEventsEstimated));
		if (NEbeFiles>1) printf(" (file:%d/%d)",NThisEbeFile,NEbeFiles);
		printf("\n");
		}
	// Update the statistics
	if (NEvent%PrevalenceUpdateStatistics==0) 
		{ WriteResultsToFile(); sprintf(Command,"mv %s %s/.",ResultsFileName,ResultsDirName); system(Command); }
	}

void ShowEvent()
	{
	if (NShownEvents==MaxNShownEvents) return;
	NShownEvents++;
	printf("\nEventId : %ld (%s) dt=%ld\n",EventN,EventDateTime,EventTimeDiffUs);
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		Chamber[NChamber]->PrintEvent(TrackMain);
	if (MaxNThps>0) printf("THP: %g oC, %g %%, %g mbar\n",Thp[0]->T,Thp[0]->H,Thp[0]->P);
	if (MaxNAdcs>0) { printf("Adcs: "); for (int NAdc=0; NAdc<MaxNAdcs; NAdc++) printf(" %d ",AdcArray[NAdc]); printf("\n"); }
	if (MaxNSimulationData>0) { printf("SimulationData: "); for (int NSimData=0; NSimData<MaxNSimulationData; NSimData++) printf(" %g ",SimulationData[NSimData]); printf("\n"); }
	TrackMain->Print();
	if (DoMultiTrack) MultiTrack->Print();
	}

void ClusterizeAll()
	{
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		Chamber[NChamber]->Clusterize();
	}

void WriteTracksFile()
	{
	if (!(TrackMain->Good)) return;
	fprintf(TracksFile,"%s\t%s\n",TrackMain->GetInfo(),Detector->GetInfoForTrack(TrackMain));


	//// Mtl4-Resolution
	//WarningModified();
	//if (Chamber[8]->MaxNClustersX==1 && Chamber[8]->MaxNClustersY==1 && Chamber[9]->MaxNClustersX==1 && Chamber[9]->MaxNClustersY==1)
	//fprintf(TracksFile,"%s\t %lf %lf %lf %lf\n",TrackMain->GetInfo(),Chamber[8]->ClusterX[0]->GlobalPos,Chamber[8]->ClusterY[0]->GlobalPos,Chamber[9]->ClusterX[0]->GlobalPos,Chamber[9]->ClusterY[0]->GlobalPos);
	//WarningModified();
	// TEMP : Combined-Bhd-Mts50
	//fprintf(TracksFile,"%lf %lf  %lf %lf  %g %d\n",TrackMain->GetXAtZ(Chamber[0]->PositionZ+12),TrackMain->GetYAtZ(Chamber[0]->PositionZ+12),TrackMain->GetSlopeX(),TrackMain->GetSlopeY(),Chamber[0]->AdcScaled,Chamber[0]->TriggerBit);
	//fprintf(TracksFile,"%s\t%d %d %d %d\t%g\n",TrackMain->GetInfo(),AdcArray[0],AdcArray[1],AdcArray[2],AdcArray[3],TrackMain->NClustersAverage);
	// TODO : TEMP : Comis
	//fprintf(TracksFile,"%s\tAdcs ",TrackMain->GetInfo());
	//for (int n=0;n<8;n++) fprintf(TracksFile," %d",AdcArray[n]);
	//fprintf(TracksFile," T");
	//for (int n=0;n<4;n++) fprintf(TracksFile," %d",GetBit(TriggerPattern,n));
	//fprintf(TracksFile," M %g\n",TrackMain->NClustersAverage);
	}

void ComputeResults()
	{
	if (!ShallComputeResults) return;
	if (Verbose>1) printf("Compute results ...\n");
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++) 
		Chamber[NChamber]->ComputeResults();
	Detector->ComputeResults();
	}

void PrintInfo()
	{
	if (ShallComputeResults) Detector->Print();
	if (Verbose>1)
		for (int NChamber=0; NChamber<MaxNChambers; NChamber++) 
			Chamber[NChamber]->Print();
	}

void WriteResultsToFile()
	{
	if (Verbose>1) printf("Write results to file ...\n");
	ResultsFile = fopen(ResultsFileName,"w");
	if (ResultsFile==NULL)
		{ printf("Cannot write ResultsFile : %s \n;",ResultsFileName); return; }
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++) 
		Chamber[NChamber]->WriteResultsToFile(ResultsFile);
	Detector->WriteResultsToFile(ResultsFile);
	TrackMain->WriteStatisticsToFile(ResultsFile);
	WriteCodeRunInfoToFile();
	WriteAnalysisInfoToDatabase();
	AnalysisParameters->WriteToFile(ResultsFile);
	fprintf(ResultsFile,"#DetectorBasicsFile\n");
	fclose(ResultsFile);
	sprintf(Command,"cat %s >> %s",DetectorBasicsFileName,ResultsFileName);
	system(Command);
	// Other output files
	WriteTracksInfoFile();
	if (DoMultiTrack) MultiTrack->WriteMapsToFile();
	}

void WriteSpecialResultsToFile()
	{
	//Detector->DiffTimeHist->WriteToNewFile((char*)"Results/a");
	
	
	// Temporary part for special checks
	/*MultiHistClass * TempMm = new MultiHistClass(2,Detector->AngleDep->XStartValue,Detector->AngleDep->XEndValue,Detector->AngleDep->XBinSize);
	Detector->AngleDep->GetSliceXIntegral(Detector->IdTrackCross,-1,1,TempMm,0);
	Detector->AngleDep->GetSliceYIntegral(Detector->IdTrackCross,-1,1,TempMm,1);
	TempMm->WriteToNewFile((char*)"Results/a");
	*/
	
	/*MultiHistDynTimeClass * SpecialOut = new  MultiHistDynTimeClass(14,Chamber[0]->TimeDep);
	SpecialOut->AddTimeHist(0,Detector->Thp[0]->TimeDev,0);
	SpecialOut->AddTimeHist(1,Detector->Thp[0]->TimeDev,1);
	SpecialOut->AddTimeHist(2,Detector->Thp[0]->TimeDev,2);
	SpecialOut->AddTimeHist(3,Detector->Thp[0]->TimeDev,3);
	SpecialOut->ModifyHistWith(1,0,FunctionDivide);
	SpecialOut->ModifyHistWith(2,0,FunctionDivide);
	SpecialOut->ModifyHistWith(3,0,FunctionDivide);
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++) 
		{
		SpecialOut->Fill(0,0);
		SpecialOut->AddTimeHist(NChamber+4,Chamber[NChamber]->TimeDep,7);
		SpecialOut->AddTimeHist(0,Chamber[NChamber]->TimeDep,1);
		SpecialOut->ModifyHistWith(NChamber+4,0,FunctionDivide);
		}
	SpecialOut->Fill(0,0);
	SpecialOut->AddTimeHist(0,Detector->Rates,3);
	char	SpecialOutFileName[500]; sprintf(SpecialOutFileName,"Results/%s.res",Detector->Name);
	FILE * SpecialOutFile = fopen(SpecialOutFileName,"w");
	SpecialOut->WriteToFile(SpecialOutFile);
	fclose(SpecialOutFile);
	*/

	}

void WriteTracksInfoFile()
	{
	if (!ShallWriteTracksFile) return;
	TracksInfoFile=fopen(TracksInfoFileName,"w");
	if (TracksFile==NULL) { printf("TracksInfoFile : %s could not be opened for write\n",TracksFileName); return; }
	fprintf(TracksInfoFile,"# TracksInfoFile\n");
	fprintf(TracksInfoFile,"Detector\t%s\n",Detector->Name);
	fprintf(TracksInfoFile,"FileNaming\t%s\n",ResultsFilenaming);
	fprintf(TracksInfoFile,"NEvents\t\t%ld\n",Detector->NEvents);
	fprintf(TracksInfoFile,"NTracks\t\t%ld\n",Detector->NTracks);
	fprintf(TracksInfoFile,"PadSizeMm\t%g\n",Detector->PadSizeMm);
	fprintf(TracksInfoFile,"TimeSec\t\t%g\n",Detector->TotalTimeSec);
	fprintf(TracksInfoFile,"TimeLiveSec\t\t%g\n",Detector->TotalTimeLiveSec);
	fprintf(TracksInfoFile,"AnalysisCodeVersion\t%s\n",CodeVerion);
	//fprintf(TracksInfoFile,"TrackListContent\t\t9\tEventN EventTimeDiffUs SlopeX SlopeY PosX PosY KonX KonY EffArea\n");
	fclose(TracksInfoFile);
	}

void WriteCodeRunInfoToFile()
	{
	if (Verbose>1) printf("Write code run info to file ...\n");
	// Write analysis code run info to ResultsFile
	char	TempInfoFileName[200]; sprintf(TempInfoFileName,"%s/AnalysisRunInfo.temp",TempDirName);
	fprintf(ResultsFile,"#Analysis run info\n");
	fprintf(ResultsFile,"CodeName\tTracking\nWrittenBy\tG.Hamar\n");
	fprintf(ResultsFile,"CodeVersion\t%s\n",CodeVerion);
	sprintf(Command,"stat Tracking.run  | grep Modify | awk '{printf \"%%s_%%s\\n\",$2,substr($3,0,8);}' > %s",TempInfoFileName); system(Command);
	sprintf(Command,"date +\"%%Y-%%m-%%d_%%H:%%M:%%S\" >> %s",TempInfoFileName); system(Command);
	sprintf(Command,"hostname >> %s",TempInfoFileName); system(Command);
	sprintf(Command,"whoami >> %s",TempInfoFileName); system(Command);
	FILE * InfoTempFile = fopen(TempInfoFileName,"r");
	char	BashInfo[22];
	fscanf(InfoTempFile,"%s",BashInfo);
	fprintf(ResultsFile,"CompiledAt\t%s\n",BashInfo);
	fscanf(InfoTempFile,"%s",BashInfo);
	fprintf(ResultsFile,"AnalysisRun\t%s\n",BashInfo);
	fscanf(InfoTempFile,"%s",BashInfo);
	fprintf(ResultsFile,"Hostname\t%s\n",BashInfo);
	fscanf(InfoTempFile,"%s",BashInfo);
	fprintf(ResultsFile,"User\t\t%s\n",BashInfo);
	fprintf(ResultsFile,"\n\n");
	}

void WriteAnalysisInfoToDatabase()
	{
	if (Verbose>1) printf("Write Database info to file ...\n");
	fprintf(ResultsFile,"#AnalysisInfoToDatabase\n");
	// ParamName and ParamValue into two lines in corresponding order
	fprintf(ResultsFile,"NEvents,NTracks,TimeHr,TimeDays\n");
	fprintf(ResultsFile,"%d,%ld,%s,%.2lf\n",NEvent,Detector->NTracks,NiceDurationSec(Detector->TotalTimeSec),Detector->TotalTimeSec/3600./24.);
	fprintf(ResultsFile,"\n\n");
	}

void CrateGnuplotHeader()
	{
	if (!ShallCreateGnuplotHeader) return;
	printf("Create Plots and Gnuplot-header\n");
	GnuplotHeaderFile = fopen(GnuplotHeaderFileName,"w");
	if (GnuplotHeaderFile==NULL)
		{ printf("Cannot write GnuplotHeaderFile : %s \n;",GnuplotHeaderFileName); return; }
	fprintf(GnuplotHeaderFile,"# Gnuplot header file : %s\n",GnuplotHeaderFileName);
	// Filenaming
	fprintf(GnuplotHeaderFile,"\n# Filenaming\n");
	fprintf(GnuplotHeaderFile,"ResultsFile=\"%s\"\n",ResultsFileName);
	fprintf(GnuplotHeaderFile,"Filenaming=\"%s\"\n",ResultsFilenaming);
	// General
	fprintf(GnuplotHeaderFile,"\nShortVersion = 1\n");
	fprintf(GnuplotHeaderFile,"\n# General info\n");
	fprintf(GnuplotHeaderFile,"DetectorName=\"%s\"\n",DetectorName);
	// Chamber Name List
	fprintf(GnuplotHeaderFile,"NChambers=%d\n",MaxNChambers);
	std::string ChamberNameList;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++) 
		{ ChamberNameList+=Chamber[NChamber]->Name; ChamberNameList+=" "; }
	fprintf(GnuplotHeaderFile,"ChamberNameList = \"%s\"\n",ChamberNameList.data());
	fprintf(GnuplotHeaderFile,"# Parameters\n");
	// Get max Size of chambers and info of Run
	int	PlotSizeX=0,PlotSizeY=0;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++) 
		{ if (PlotSizeX<Chamber[NChamber]->SizeX) PlotSizeX=Chamber[NChamber]->SizeX; if (PlotSizeY<Chamber[NChamber]->SizeY) PlotSizeY=Chamber[NChamber]->SizeY; }
	fprintf(GnuplotHeaderFile,"SizeX = %d ; SizeY = %d\n",PlotSizeX,PlotSizeY);
	fprintf(GnuplotHeaderFile,"SlopeMax = %g\n",AnalysisParameters->AngleMax);
	fprintf(GnuplotHeaderFile,"GainAverage = %6.1lf\n",Detector->GainAverage);
	if (Detector->EventRate > 0.01) fprintf(GnuplotHeaderFile,"EventRate = %5.2lf ; TrackRate = %5.2lf\n",Detector->EventRate,Detector->TrackRate);
	else fprintf(GnuplotHeaderFile,"EventRate = 1.0 ; TrackRate = 0.1 ; # For scaling, Not valid info!\n");
	//fprintf(GnuplotHeaderFile,"PosBinSize=%g\n",AnalysisParameters->PositionDepBinSize);
	// Indexes for Chambers
	fprintf(GnuplotHeaderFile,"\n# LocalIndexes for the Chambers\n");
	int Index = Chamber[0]->WriteGnuplotInfo(GnuplotHeaderFile);
	fprintf(GnuplotHeaderFile,"NIndexes=%d\n",Index);
	// Indexes for the Detector
	fprintf(GnuplotHeaderFile,"\n# DetectorIndexes\n");
	fprintf(GnuplotHeaderFile,"NThps=%d\n",Detector->MaxNThps);
	fprintf(GnuplotHeaderFile,"NHvs=%d\n",HvControlInt);
	Index*=MaxNChambers;
	Detector->WriteGnuplotInfo(GnuplotHeaderFile,Index);
	// MultiTrackAnalysis settings
	if (DoMultiTrack)
		{
		fprintf(GnuplotHeaderFile,"\n# MultiTrack analysis\n");
		std::string MultiTrackRegionNameList;
		if (MultiTrack->NScatterMaps>0) for (int NRegion=0; NRegion<MultiTrack->ScatterMap[0]->NRegions; NRegion++) 
			{ MultiTrackRegionNameList+=MultiTrack->ScatterMap[0]->Region[NRegion]->Name; MultiTrackRegionNameList+=" "; }
		fprintf(GnuplotHeaderFile,"Region = \"%s\"\n",MultiTrackRegionNameList.data());
		}
	// Calling the plot function
	fprintf(GnuplotHeaderFile,"\n# End of Gnuplot header\n");
	// Close file
	fclose(GnuplotHeaderFile); GnuplotHeaderFile=NULL;
	// Create the plots
	sprintf(Command,"gnuplot %s %s > /dev/null 2> /dev/null",GnuplotHeaderFileName,GnuplotPlotFileName);
	if (Verbose>0) sprintf(Command,"gnuplot %s %s 2> /dev/null",GnuplotHeaderFileName,GnuplotPlotFileName);
	system(Command);
	}

void MoveResults()
	{
	sprintf(Command,"mv %s %s/.",ResultsFileName,ResultsDirName); system(Command);
	if (ShallCreateGnuplotHeader)
		{
		sprintf(Command,"mv %s %s/.",GnuplotHeaderFileName,ResultsDirName); system(Command);
		sprintf(Command,"mv %s.pdf %s/.",ResultsFilenaming,ResultsDirName); system(Command);
		}
	if (ShallWriteTracksFile)
		{
		sprintf(Command,"mv %s %s/.",TracksFileName,ResultsDirName); system(Command);
		sprintf(Command,"mv %s %s/.",TracksInfoFileName,ResultsDirName); system(Command);
		}
	if (Verbose>0)
		{
		printf("Results written to : %s/%s\n",ResultsDirName,ResultsFileName);
		if (ShallWriteTracksFile) printf("TrackList written to : %s/%s\n",ResultsDirName,TracksFileName);
		}
	// TODO ?
	// Something more adjustable, like recognizing "/" and find path
	}

void Help(int ExitCode)
	{
	system("cat Help/HelpTracking.txt");
	exit(ExitCode);
	}

void CloseProgram()
	{
	// Close opened files
	if (EbeFile!=NULL) fclose(EbeFile);
	if (EbeListFile!=NULL) fclose(EbeListFile);
	if (TracksFile!=NULL) fclose(TracksFile);
	// Remove ZipTemp if used
	sprintf(Command,"rm -f %s 2> /dev/null",EbeZipTempName); system(Command);
	// Free memory allocated to the Classes
	delete TrackMain;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		delete PerformanceTrack[NChamber];
	free(PerformanceTrack);
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		delete Chamber[NChamber];
	free(Chamber);
	for (int NThp=0; NThp<MaxNThps; NThp++) delete Thp[NThp];
	free(Thp);
	free(AssignAdc);
	free(AdcArray);
	free(pAdc);
	free(pTriggerBit);
	free(EventDateTime);
	if (DoSpeak) system("espeak -a200 ready");
	printf("End.\n");
	// Show plots if requested
	if (ShowPlots&&ShallCreateGnuplotHeader) { sprintf(Command,"evince %s/%s.pdf 2> /dev/null",ResultsDirName,ResultsFilenaming); system(Command); }
	exit(0);
	}

void WarningModified()
	{
	if (WarningModifiedShown) return;
	printf("\n\n!!! WARNING : MODIFIED CODE IS USED !!!\n\n\n");
	WarningModifiedShown=true;
	}


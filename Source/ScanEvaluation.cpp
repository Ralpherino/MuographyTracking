////////////////////////////////////////////////////////////////////////
// Scan Evaluation - impelementation
//      version : 2019-10-22
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include <string>
#include "ScanEvaluation.h"

int main(int ArgC, char** ArgV)
{
printf("Start: Scan Evaluattion\n");

ReadArguments(ArgC,ArgV);

// Check list file
ResultsListFile=fopen(ResultsListFileName,"r");
if (ResultsListFile==NULL) { printf("Cannot open %s\nExit",ResultsListFileName); exit(2); }

// Get NRuns
sprintf(Command,"grep -c res %s > Temp/LineNum.temp",ResultsListFileName); system(Command);
FILE * TempFile = fopen("Temp/LineNum.temp","r");
fscanf(TempFile,"%d",&NRuns);
fclose(TempFile);

// Init classes
InitClasses();

// Loop
NRun=0;
while (!feof(ResultsListFile))
	{
	fscanf(ResultsListFile,"%s %lf ",ResultsFileName,&ParameterValue);
	RunNumber=GetRunNumber(ResultsFileName);
	if (Verbose>1) printf("(%02d) Checking: %s - ",NRun,ResultsFileName);
	printf("Run: %d - Par: %g\n",RunNumber,ParameterValue);

	Detector->ReadInfoFromFile(ResultsFileName,false,false);

	for (int NChamber=0; NChamber<Detector->NChambers; NChamber++)
		{
		ChamberInfo->Set(IdRunNumber,NChamber,NRun,RunNumber);
		ChamberInfo->Set(IdParameter,NChamber,NRun,ParameterValue);
		ChamberInfo->Set(IdTrackingEff,NChamber,NRun,Detector->Chamber[NChamber]->EfficiencyTracking);
		ChamberInfo->Set(IdTriggerEff,NChamber,NRun,Detector->Chamber[NChamber]->EfficiencyTrigger);
		ChamberInfo->Set(IdGain,NChamber,NRun,Detector->Chamber[NChamber]->AdcDep->Mean(Detector->Chamber[NChamber]->IdTrackAdcScaledSum));
		}
	DetectorInfo->Set(IdRunNumber,NRun,RunNumber);
	DetectorInfo->Set(IdParameter,NRun,ParameterValue);
	DetectorInfo->Set(IdEventRate,NRun,Detector->EventRate);
	DetectorInfo->Set(IdFluxMid,NRun,Detector->AngleDep->Z(Detector->IdFlux,0,0));
	Flux->AddMap(NRun,Detector->AngleDep,Detector->IdFlux);
	
	NRun++;
	Detector->ClearStatistics();
	}

WriteScanEv();

CloseProgram();
}



void ReadArguments(int ArgC, char** ArgV)
	{
	if (Verbose>0) printf("Reading arguments ...\n");
	if (ArgC<2) Help(0);
	for (int i=1; i<ArgC; i++)
		if (strcmp(ArgV[i],"-h")==0 || strcmp(ArgV[i],"--help")==0 )
			{ Help(); }
		else if (strcmp(ArgV[i],"-b")==0 || strcmp(ArgV[i],"--basic-info")==0 )
			{ i++; sprintf(DetectorBasicsFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-l")==0 || strcmp(ArgV[i],"--results-list")==0 )
			{ i++; sprintf(ResultsListFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-s")==0 || strcmp(ArgV[i],"--scan-name")==0 )
			{ i++; sprintf(ScanEvFileNaming,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-v")==0 || strcmp(ArgV[i],"--verbose")==0 )
			{ i++; sscanf(ArgV[i],"%d",&Verbose); }
		else
			{ printf("\nBad argument : %s\n\n",ArgV[i]); Help(ErrWrongArgument); };
	RefineFileName(DetectorBasicsFileName,(char*)"Settings",(char*)".db");
	sprintf(ScanEvFileName,"Results/%s.dat",ScanEvFileNaming);
	
	if (strlen(DetectorBasicsFileName)<2)	{ printf("Missing: DetectorBasicsFileName\nExit\n"); exit(ErrMissingArgument); }
	if (strlen(ResultsListFileName)<2) 	{ printf("Missing: ResultsListFileName\nExit\n"); exit(ErrMissingArgument); }
	
	}

void InitClasses()
	{
	Detector = new DetectorClass();
	Detector->ReadBasics(DetectorBasicsFileName);

	ChamberInfo = new MultiMapClass(NCIds,0,Detector->NChambers-1,1,0,NRuns-1,1);
	ChamberInfo->SetName(IdRunNumber,(char*)"RunNumber");
	ChamberInfo->SetName(IdParameter,(char*)"Parameter");
	ChamberInfo->SetName(IdTrackingEff,(char*)"TrackingEff");
	ChamberInfo->SetName(IdTriggerEff,(char*)"TriggerEff");
	ChamberInfo->SetName(IdGain,(char*)"Gain");

	DetectorInfo = new MultiHistClass(NDIds,0,NRuns-1,1);
	DetectorInfo->SetName(IdRunNumber,(char*)"RunNumber");
	DetectorInfo->SetName(IdParameter,(char*)"Parameter");
	DetectorInfo->SetName(IdEventRate,(char*)"EventRate");
	DetectorInfo->SetName(IdFluxMid,(char*)"FluxMid");
	
	Flux = new MultiMapClass(NRuns,Detector->AngleDep);
	//FluxX = new MultiHistClass(NRuns,Detector->AngleDep->StartValue,Detector->AngleDep->EndValue,Detector->AngleDep->BinSize);
	//FluxY = new MultiHistClass(NRuns,Detector->AngleDep->StartValue,Detector->AngleDep->EndValue,Detector->AngleDep->BinSize);
	
	
	}

int GetRunNumber(char* RunFileName)
	{
	std::string RunFileNameString=RunFileName;
	int RunStringPosition = RunFileNameString.rfind("Run");
	return (int) strtol(&(ResultsFileName[RunStringPosition+3]),NULL,10);
	}

void WriteScanEv()
	{
	sprintf(ScanEvFileName,"Results/%s.sr",ScanEvFileNaming);
	ScanEvFile = fopen(ScanEvFileName,"w");
	if (ScanEvFile==NULL) { printf("Cannot open output file : %s\nExit\n",ScanEvFileName); exit(ErrFileOpen); } 
	ChamberInfo->WriteToFile(ScanEvFile);
	DetectorInfo->WriteToFile(ScanEvFile);
	Flux->WriteToFile(ScanEvFile);
	fclose(ScanEvFile); ScanEvFile=NULL;
	}

void CloseProgram()
	{
	if (ResultsListFile!=NULL) fclose(ResultsListFile);
	if (Detector!=NULL) delete Detector;
	if (ScanEvFile!=NULL) fclose(ScanEvFile);
	printf("End.\n");
	}

void Help(int ExitCode)
	{
	system("cat Help/HelpScanEvaluation.txt");
	exit(ExitCode);
	}



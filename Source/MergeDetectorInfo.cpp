////////////////////////////////////////////////////////////////////////
// Merge DetectorInfo files - impelementation
//      version : 2020-07-31
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MergeDetectorInfo.h"

int main(int ArgC, char** ArgV)
{
printf("Start: Merge Detector Info\n");

ReadArguments(ArgC,ArgV);

AnalysisParametersClass *AnalysisParameters = new AnalysisParametersClass();
AnalysisParameters->ReadFromFile(AnalysisParametersFileName);
//AnalysisParameters->SetNChambers(8);

DetectorClass * DetectorSum = new DetectorClass();
DetectorClass * DetectorAdd = new DetectorClass();
// TODO : AnalysisParapeters shall be read from RES file
DetectorSum->SetAnalysisParameters(AnalysisParameters);
DetectorAdd->SetAnalysisParameters(AnalysisParameters);
DetectorSum->ReadBasics(DetectorBasicsFileName);
DetectorAdd->ReadBasics(DetectorBasicsFileName);
DetectorSum->Verbose=Verbose;

ResultsListFile=fopen(ResultsListFileName,"r");
if (ResultsListFile==NULL) { printf("Cannot open %s\nExit",ResultsListFileName); exit(2); }

fscanf(ResultsListFile,"%s ",ResultsFileName);
if (Verbose>1) printf("InitAt: %s\n",ResultsFileName);
DetectorSum->ReadInfoFromFile(ResultsFileName,false,false);
sprintf(TimeMarkThis,"%s\t%s\n",DetectorSum->TimeDep->TAt(0),DetectorSum->ResFilenaming);
TimeMarks.append(TimeMarkThis);
if (MergedNTimeBins>0) DetectorSum->SetNTimeBins(MergedNTimeBins);

while (!feof(ResultsListFile))
	{
	fscanf(ResultsListFile,"%s ",ResultsFileName);
	if (Verbose>1) printf("Adding: %s\n",ResultsFileName);
	DetectorAdd->ReadInfoFromFile(ResultsFileName,false,false);
	DetectorSum->AddInfoFromDetector(DetectorAdd);
	sprintf(TimeMarkThis,"%s\t%s\n",DetectorAdd->TimeDep->TAt(0),DetectorAdd->ResFilenaming);
	TimeMarks.append(TimeMarkThis);
	DetectorAdd->ClearStatistics();
	}
sprintf(TimeMarkThis,"%s\t%s\n",DetectorSum->TimeDep->TAt(DetectorSum->TimeDep->MaxNBins-1),(char*)"end");
TimeMarks.append(TimeMarkThis);
printf("Compute results...\n");
DetectorSum->ComputeResults(true);
DetectorSum->Print();

sprintf(MergedFileName,"%s/%s.res",OutputDir,MergedFileNaming);
MergedFile = fopen(MergedFileName,"w");
if (MergedFile==NULL) { printf("Cannot open for write : %s\n",MergedFileName); exit(1); }

for (int NChamber=0; NChamber<DetectorSum->NChambers; NChamber++) DetectorSum->Chamber[NChamber]->WriteResultsToFile(MergedFile);
DetectorSum->WriteResultsToFile(MergedFile);
fprintf(MergedFile,"%s\n\n",TimeMarks.c_str());
fclose(MergedFile);
printf("Merged file has been created : %s\n",MergedFileName);

CreateGnuplotHeader();


printf("End.\n\n");
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
		else if (strcmp(ArgV[i],"-ap")==0 || strcmp(ArgV[i],"--analysis-parameters")==0 )
			{ i++; sprintf(AnalysisParametersFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-n")==0 || strcmp(ArgV[i],"--results-naming")==0 )
			{ i++; sprintf(ResultsListNaming,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-l")==0 || strcmp(ArgV[i],"--results-list")==0 )
			{ i++; sprintf(ResultsListFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-m")==0 || strcmp(ArgV[i],"--merged-name")==0 )
			{ i++; sprintf(MergedFileNaming,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-rd")==0 || strcmp(ArgV[i],"--merged-dir")==0 )
			{ i++; sprintf(OutputDir,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-tb")==0 || strcmp(ArgV[i],"--time-bins")==0 )
			{ i++; sscanf(ArgV[i],"%d",&MergedNTimeBins); }
		else if (strcmp(ArgV[i],"-v")==0 || strcmp(ArgV[i],"--verbose")==0 )
			{ i++; sscanf(ArgV[i],"%d",&Verbose); }
		else
			{ printf("\nBad argument : %s\n\n",ArgV[i]); Help(ErrWrongArgument); };
	
	// Generate the list from the naming
	if (strlen(ResultsListNaming)>2) 
		{
		sprintf(Command,"ls %s*res > %s",ResultsListNaming,ResultsListFileNameTemp);
		system(Command);
		sprintf(ResultsListFileName,"%s",ResultsListFileNameTemp);
		}
	
	RefineFileName(DetectorBasicsFileName,(char*)"Settings",(char*)".db");
	
	}

void CreateGnuplotHeader()
	{
	char	GnuplotHeaderFileName[FileNameLength];
	sprintf(GnuplotHeaderFileName,"%s/%s.gph",OutputDir,MergedFileNaming);
	FILE * GnuplotHeaderFile = fopen(GnuplotHeaderFileName,"w");
	if (GnuplotHeaderFile==NULL)
		{ printf("Cannot write GnuplotHeaderFile : %s \n;",GnuplotHeaderFileName); return; }
	fprintf(GnuplotHeaderFile,"# Gnuplot header file : %s\n",GnuplotHeaderFileName);
	// Filenaming
	fprintf(GnuplotHeaderFile,"\n# Filenaming\n");
	fprintf(GnuplotHeaderFile,"Filenaming=\"%s\"\n",MergedFileNaming);
	fprintf(GnuplotHeaderFile,"ResultsFile=\"%s.res\"\n",MergedFileNaming);
	fprintf(GnuplotHeaderFile,"UseTimeMarks=1\n");
	fclose(GnuplotHeaderFile);
	sprintf(Command,"tail -n +6 $(dirname %s)/$(basename %s .res).gph >> %s",ResultsFileName,ResultsFileName,GnuplotHeaderFileName);
	if (system(Command)>0) { sprintf(Command,"tail -n +6 $(ls -1 $(dirname %s)/*.gph | head -n 1) >> %s",ResultsFileName,GnuplotHeaderFileName); system(Command); }
	sprintf(Command,"ThisDir=$(pwd); cd %s; gnuplot %s.gph $ThisDir/PlotScripts/PlotAll.gp; cd $ThisDir",OutputDir,MergedFileNaming);
	system(Command);
	}

void Help(int ExitCode)
	{
	system("cat Help/HelpMergeDI.txt");
	exit(ExitCode);
	}



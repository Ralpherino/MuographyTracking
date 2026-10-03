////////////////////////////////////////////////////////////////////////
// Merge DetectorInfo files - header
//      version : 2019-11-28
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////
#ifndef MergeDetectorInfoGuard
#define MergeDetectorInfoGuard

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "DetectorClass.h"

const int	FileNameLength=500;
char	DetectorBasicsFileName[FileNameLength]="";
char	ResultsListNaming[FileNameLength*20]="",ResultsListFileName[FileNameLength]="";
char	ResultsListFileNameTemp[FileNameLength]="Temp/ResultsList.list";
char	ResultsFileName[FileNameLength]="", AnalysisParametersFileName[FileNameLength]="";
char	OutputDir[FileNameLength]="Results";
char	MergedFileNaming[FileNameLength]="Merged";
char	MergedFileName[FileNameLength];
FILE	*ResultsListFile,*MergedFile;

char	TimeMarkThis[200];
std::string TimeMarks;

char	Command[1000];
int	Verbose=2;
int	MergedNTimeBins=-1;

// Functions
void ReadArguments(int ArgC, char** ArgV);
void CreateGnuplotHeader();
void Help(int ExitCode=0);



// Error Codes
const int	ErrWrongArgument=1;



#endif


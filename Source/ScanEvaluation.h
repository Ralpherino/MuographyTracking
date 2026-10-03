////////////////////////////////////////////////////////////////////////
// ScanEvaluation : header
//      version : 2019-10-22
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////
#ifndef ScanEvaluationGuard
#define ScanEvaluationGuard

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "DetectorClass.h"


const int	FileNameLength=200;
char	DetectorBasicsFileName[FileNameLength]="";
char	ResultsListFileName[FileNameLength]="", ResultsFileName[FileNameLength]="";
char	ScanEvFileNaming[FileNameLength]="ScanEv", ScanEvFileName[FileNameLength]="";
FILE	*ResultsListFile,*ScanEvFile;


DetectorClass * Detector;
double	ParameterValue;
int	NCIds=5,NDIds=4,NRuns=1;
enum {IdParameter=0,IdRunNumber=1,IdTrackingEff=2,IdTriggerEff=3,IdGain=4};
enum {IdEventRate=2,IdFluxMid=3};
int	NRun=0,RunNumber=0;
MultiMapClass	* ChamberInfo;
MultiHistClass	* DetectorInfo;
MultiMapClass	* Flux;
//MultiHistClass	* FluxX, * FluxY;


char	Command[200];
int	Verbose=2;

// Functions
void ReadArguments(int ArgC, char** ArgV);
void InitClasses();
int  GetRunNumber(char* RunFileName);
void WriteScanEv();
void CloseProgram();
void Help(int ExitCode=0);


// Error Codes
const int	ErrMissingArgument=1, ErrWrongArgument=2;
const int	ErrFileOpen=3;




#endif


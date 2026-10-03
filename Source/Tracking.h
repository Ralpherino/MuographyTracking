////////////////////////////////////////////////////////////////////////
// Tracking for a Detector with several 2dim chambers - header
//      version : 2024-04-25
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

char	CodeVerion[22]="2024-04-25";

#ifndef TrackingGuard
#define TrackingGaurd

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>

#include "AnalysisParametersClass.h"
#include "ChamberClass.h"
#include "DetectorClass.h"
#include "EventClass.h"
#include "DateTimeClass.h"
#include "ThpClass.h"
#include "SelectionClass.h"
#include "Track1DimClass.h"
#include "TrackClass.h"
#include "CombinationClass.h"
#include "MultiHistDynTimeClass.h"
#include "CombinedEfficiencyClass.h"
#include "MultiTrackAnalysisClass.h"
#include "ReadEventFormats.h"
#include "ExtraFunctions.h"

// Files and FileNames
char	EbeListFileName[200]="", EbeListFileNameTemp[200]="Temp/EbeFileList.temp", EbeFileNaming[400]="";
FILE	*EbeListFile=NULL;
int	NEbeFiles=1,NThisEbeFile=0,NEventsPerFile=-1,NTotalEventsEstimated=-1;
char	EbeFileName[20000]="", EbeZipTempName[500]="Temp/EbeZip.ebe"; bool IsEbeZip=false;
FILE	*EbeFile=NULL;
char	AnalysisParametersFileName[200]="", MultiTrackAnalysisParametersFileName[200]="", RunLevelInfoFileName[200]="";
int 	NApExtras=0; char ApExtraParName[10][50]; double ApExtraParValue[10];
char	DetectorBasicsFileName[400]="",DetectorInfoInFileName[400]="";
FILE	*DetectorInfoFile;
char	ResultsFilenaming[200]="", ResultsFileName[200]="", GnuplotHeaderFileName[200]="", TracksFileName[200]="" ,TracksInfoFileName[200]="";
char	GnuplotPlotFileName[200]="PlotScripts/PlotAll.gp";
FILE	*ResultsFile=NULL, *GnuplotHeaderFile=NULL, *TracksFile=NULL, *TracksInfoFile=NULL;
char	TempDirName[200]="Temp", SettingsDirName[200]="Settings", ResultsDirName[200]="Results";
// General variables
int	Verbose=0, IdNumber=0;
bool	ShallCreateGnuplotHeader=true, ShowPlots=false, ShallWriteTracksFile=false, ShallComputeResults=true, DoSpeak=false;
int	MaxNEvents=-1,NEvent=0, Prevalence=500000, PrevalenceDot=10000, PrevalenceUpdateStatistics=4000000;
int	EveryNthEvent=0, SkipFirstNEvent=0;
double	TimeCorrectionSec=0;
int	MaxNShownEvents=0, NShownEvents=0;
bool	WarningModifiedShown=false;

char	DetectorName[100];
// TEMPORARY !! 1 line (should be in DetectorClass  only? ..)
double	DeadTimePerEventUs, DeadTimeZipPerEventUsApprox, PadSizeMm;
int	TriggerCoinc=1;
// TEMP : to check DeadTimePerFile
double	DtpfSumTSec=0; bool FirstEventOfFile=false;

AnalysisParametersClass	*AnalysisParameters;
DetectorClass	*Detector;
ChamberClass	**Chamber;
int		MaxNChambers=0;
TrackClass	*TrackMain,**PerformanceTrack;
int		TrackingNHits;
bool		DoMultiTrack=false;
MultiTrackAnalysisClass * MultiTrack;
bool		ShallCorrectDateTime=false, UseDiffTimeAsDateTimeInfo=false;
char		StartRunDateTime[50];

// Event
long	EventFirstData;
long	EventN;
char	*EventDateTime;
DateTimeClass	*EventDt,*DtShift;
long	EventTimeDiffUs; long long int EventTimeAbsUs=0;
int	TriggerPattern;
int	CounterDiff=0;
int	MaxNThps=0;
ThpClass **Thp;
HvClass	*HvInfo;
int	MaxNAdcs=0;
int	**pAdc, *AssignAdc, *AdcArray;
int	MaxNTriggerBits;
int	**pTriggerBit, *AssignTriggerBit;
int	MaxNChecks=0;
int	HvControlInt;
int	MaxNSimulationData=0;
double	*SimulationData;
bool	EventReadOk;
int	NConversions;

char	Command[20000];
int	CommandExitCode=0;


// Functions
void ReadArguments(int ArgC, char** ArgV);
void Help(int ExitCode=0);

void InitializeClasses();
void InitEbeList();
bool OpenNextEbeFile();
void ReadInfo();
void ReadAnalysisParameters();
void ReadDetectorBasics();
void ReadRunLevelInfo();
void ReadDetectorInfo();
void SetResultsFilenaming();
void CheckEventFormat();
void AssignData();
void SkipFirstEvents(int InNSkip);
bool ReadNextEvent();
bool (*ReadEventFormat)() = ReadEvent_Format_20181015;
void CorrectDateTime();
void WriteInfoToScreen();
void ClusterizeAll();
void ConstructTrackX();
void ConstructTrackY();
void RetuneTrack();
void WriteTracksFile();
void ShowEvent();
void ComputeResults();
void PrintInfo();
void WriteResultsToFile();
void WriteAnalysisInfoToDatabase();
void WriteSpecialResultsToFile();
void WriteTracksInfoFile();
void WriteCodeRunInfoToFile();
void CrateGnuplotHeader();
void MoveResults();
void WarningModified();
void CloseProgram();

enum {ErrWrongArgument=1, ErrFileError, ErrDetectorInfo};


#endif



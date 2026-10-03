////////////////////////////////////////////////////////////////////////
// Flux Computation using Tracks data - header
//      version : 2022-02-24
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////
#ifndef FluxComputationGuard
#define FluxComputationGuard

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "VectorClass.h"
#include "MultiMapClass.h"
#include "ZonedMapsClass.h"
#include "GrainedMapsClass.h"
#include "ExtraFunctions.h"

char	CodeVerion[22]="2022-01-08";

// Files and FileNames
char	TracksFileName[200]="", TracksInfoFileName[200]="", ResultsFileName[200]="";
FILE	*TracksFile,*TracksInfoFile,*ResultsFile;
char	ListFileName[200]="",TracksDir[200]="",TempDir[200]="Temp/", TempFileName[200]="Temp/FluxComputation.tmp";
FILE	*ListFile;
char	FluxFileNaming[200]="", FluxFileName[200]="",SimpleFluxFileName[200]="";
FILE	*FluxFile,*SimpleFluxFile;
char	ResultsDirName[200]="Results";

// Track level info
long int EventN=-1,EventNPrevious;
double	SlopeX,SlopeY,EffArea;
double	MapX,MapY;

// Run level info, from TracksInfoFile
double	NFiles=0;
double	TotalTimeSec=1;
double	PadSizeMm=12;
double	Scale;
char	AnalysisInfoText[500]="";
char	ArgRotation[200]=" 0";

// Detector info
MultiMapClass *DetectorAngleDep;

// Transformation
VectorClass	*TrackDirection;
int	NRotations; const int MaxNRotations=4;
char	RotAxis[MaxNRotations]; 
double	RotAngle[MaxNRotations];

// Counters
long int NTracks=0, MaxNTracks=-1, NTracksAll=0;
int	Prevalence=500000,PrevalenceDot=10000;
int	NConversions=0;

// Flux calculation
MultiMapClass	*FineMap, *SimpleFluxMap, *MuographyMap;
enum	FineMapIds { IdSize=0, IdCounts, IdAngleOfView, IdFluxA, IdCos2, IdFlux, IdFluxGrained, IdFluxSmooth, IdSmoothSize,
		IdDetZenith, IdDetAzimuth, IdGeoZenith, IdGeoAzimuth, 
		IdRockLength, IdDensityLengthReyna, IdDensityLengthGuan, IdDensityLengthErr,
		NumberOfFineMapIds};
int	NMaps=NumberOfFineMapIds;

double	MapMax=2.6, MapBs=0.020;
GrainedMapsClass	*FluxGm;
double	GrainCountLimit=2000;
double	SmoothCountLimit=100; 
int	SmoothSizeMax=10;

// Switches
bool	DoGrainedFlux=true, DoSimpleFlux=true, DoPlotFlux=false;
enum	{ModeNone=0, ModeSlope=1, ModePolar=2};
int	CoordinateSystemMode=ModeSlope;
char	CoordinateSystemModeString[50]="Slope";
bool	UseSpecialOrientation=false; int Orientation=0; char OrientaionName[50]="";
enum	{OriNone=0, OriMts8=1};
bool	UseResultsFile=false;

// Various
int	Verbose=0;
char	Command[500];

// Functions
void ReadArguments(int ArgC, char** ArgV);
void SetMode();
void InitializeClasses();
void FillZenithAzimuthMap();
void GetResultsFileInfo();
void OpenListFile();
void SetFluxFileName();
bool GetNextFileInfo();
void SetTracksFileNames();
void ReadTracksInfoFile();
void ReadAnalysisInfo();
void OpenTracksFile();
void ReadTrack();
void GetEffArea();
double AngleOfView(double SX, double SY);
void TransformToMap();
void WriteInfoToScreen();
void ComputeFineMapFlux();
void MakeGrainedMap();
void WriteInfoToFile(FILE* OutFile);
void WriteResultsToFile();
void PlotFlux();
double RadToDeg(double XRad);
double DegToRad(double XDeg);
double FunctionCos2(double SX, double SY);
double FunctionZenith(double SX, double SY);
double FunctionAzimuth(double SX, double SY);
void Help(int ExitCode=0);
void CloseProgram(int ExitCode=0);


#endif



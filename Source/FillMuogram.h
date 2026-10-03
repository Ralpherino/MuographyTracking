////////////////////////////////////////////////////////////////////////////////
// Fill Muogram - header
//	version: 20240425
//	written by: G.Hamar at WignerRCP
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>

#include "MuogramClass.h"

char	CodeVersion[100]="2024-04-25";
// Variables
const int FileNameMaxLength=1000;

MuogramClass * Muogram;
int	Verbose, IdNumber;
char	Command[FileNameMaxLength];
double	BinSizeDeg=2;

char	MuogramFileName[FileNameMaxLength]="";
char	RunLevelInfoFileName[FileNameMaxLength]="";
char	ResultsDirName[FileNameMaxLength]="Results/";
char	TrackListFileName[FileNameMaxLength]="", DemFileName[FileNameMaxLength]="", TunnelsStlFileName[FileNameMaxLength]="";
char	FluxDlConvZfdFileName[FileNameMaxLength]="Reference.zfd";
double	PosN,PosE,PosH, RotX=0,RotY=0,RotZ=0;
double	Density,DensityEst;
// Moduls/Calculation
bool	DoCreate=true;
bool	DoSetRot=false, DoReadTrackList=false, DoFluxToDl=true;
bool	DoSetPos=false, DoReadDemFile=false, DoReadStlFile=false, DoRlToDl=true; 
bool	DoEstimateDensity=true, DoCompareDlMC=true;
bool	DoShowInfo=true, DoWrite=true, DoPlot=true, DoMove=true;

// Functions
void ReadArguments(int ArgC, char** ArgV);
void ReadRunLevelInfo();
void CheckFilenaming();
void Help(int ExitCode=0);


// ErrorCodes
enum {ErrWrongArgument=1, ErrMissingInfo=2, ErrWrongFile=3};

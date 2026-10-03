////////////////////////////////////////////////////////////////////////
//  MultiTrack Parameters Class - header
//      version : 2019-08-24
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef MultiTrackAnalysisClassGuard
#define MultiTrackAnalysisClassGuard

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TrackClass.h"
#include "DetectorClass.h"
#include "MultiMapClass.h"
#include "MultiHistFuncBinClass.h"

class TrackIntersectionClass;
class ScatterMapClass;
class RegionClass;

class MultiTrackAnalysisClass
	{ public:
	// Constructor
	MultiTrackAnalysisClass();
	~MultiTrackAnalysisClass();
	// Variables
	DetectorClass	*Detector;
	TrackClass	**Track;
	int		NTracks;
	int		Verbose;
	bool		DoWriteTracksFile;
	// Intersection
	TrackIntersectionClass	**Intersection;
	int	NIntersections;
	long	*pEventId,EventIdDef,*pEventTimeDiff;
	// ScatterMap
	ScatterMapClass		**ScatterMap;
	int	NScatterMaps;
	// Files
	char	FileNaming[200], DirName[200], MultiTrackListFileName[200];
	FILE	*MultiTrackListFile;
	FILE	*ScatterMapsFile;
	// Extra for simu
	double	*SimulationInfo;
	
	// Functions
	bool	WriteSettingsToFile(FILE* OutFile);
	bool	ReadSettingsFromFile(char* FileName);
	void	SetDetector(DetectorClass * InDetector);
	void	SetFileNaming(char* InFileNaming, char* InDirName=(char*)"");
	void	Get();
	void	Print();
	void	WriteTracksFile();
	void	WriteMapsToFile();
	TrackClass * GetTrackBy(char* Specifier, char* Parameter);
	TrackClass * GetTrackByName(char* InName);
	TrackClass * GetTrackById(int InId);
	};

class TrackIntersectionClass
	{ public:
	TrackIntersectionClass(MultiTrackAnalysisClass	*InMultiTrack, int InId=-1);
	~TrackIntersectionClass();
	
	MultiTrackAnalysisClass	*MultiTrack;
	bool	ReadFromFile(FILE * InFile);
	void	SetTracks(TrackClass * InTrack1, TrackClass * InTrack2);
	void	InitClasses();
	bool	Check();
	void	Print();

	// Variables
	char		Name[50];
	int		Id;
	TrackClass	*Track1,*Track2;
	FILE		*OutFile;
	long int	*pEventId;
	bool		Intersect;
	double		Xc,Zx,dSx,Yc,Zy,dSy,Z,dS;
	MultiMapClass	*ZDsMap;
	
	};

class ScatterMapClass
	{ public:
	ScatterMapClass(MultiTrackAnalysisClass	*InMultiTrack, int InId=-1);
	~ScatterMapClass();
	// Function
	bool	ReadFromFile(FILE * InFile);
	bool	CopyParameters(ScatterMapClass * SM);
	void	InitClasses();
	void	Compute();
	void	Print();
	bool	WriteToFile(FILE * OutFile);
	
	// Variables
	MultiTrackAnalysisClass	*MultiTrack;
	char		Name[50];
	int		Id;
	MultiMapClass	*Map;
	const int	NMapLevels=5;
	enum		{IdN=0, IdNScatter=1, IdSumScatter=2, IdOnlyFirst=3, IdOnlySecond=4};
	TrackClass	*Track1,*Track2;
	double		ZLevel, DMax, BinSize, ShiftX, ShiftY;
	double		ScatterMin,ScatterMax;
	double		X,Y,Scatter;
	RegionClass	**Region;
	int		NRegions;
	//MultiHistClass	*ScatterInRegions;
	MultiHistFuncBinClass *ScatterInRegions;
	};

double	ScatterHistBinSizeFunc(double x);

class RegionClass
	{ public:
	RegionClass();
	bool	ReadFromFile(FILE* InFile);
	bool	IsInside(double X, double Y);
	// Variables
	char	Name[50];
	double	XMin,XMax,YMin,YMax;
	};

#endif


////////////////////////////////////////////////////////////////////////
// Track Class - header
//      version : 2020-05-14
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef TrackClassGuard
#define TrackClassGuard

#include <stdio.h>
#include "ChamberClass.h"
#include "DetectorClass.h"
#include "Track1DimClass.h"
#include "SelectionClass.h"
#include "ExtraFunctions.h"
#include "MultiHistClass.h"
#include "MultiMapClass.h"

class TrackClass
	{ public:
	TrackClass();
	~TrackClass();
	
	// General
	char	Name[20];
	int	Id;
	long	*pEventId,EventIdDef;
	long	TrackTimeDiff,*pEventTimeDiff;
	double	ZMin,ZMax,ZMid;
	bool	UseFixPoint;
	double	FixPointZ,FixPointX,FixPointY;
	int	NTriggers, NCrossed;
	double	Kon;
	// Chamber info
	int	MinNHits,NChambers,*ChamberId;
	ChamberClass	**Chamber;
	DetectorClass	*Detector;
	// Init functions
	void	Set(DetectorClass * InDetector, int InNChambers, int *InChamberId, int InMinNHits);
	void	Set(DetectorClass * InDetector);
	void	Set();
	void	SetId(int InId, char* InName=(char*)"NoName");
	bool	ReadSettingsFromFile(FILE * InFile);
	bool	WriteSettingsToFile(FILE * OutFile);
	// Tracking var
	SelectionClass	**HitSelection;
	int		*NClustersArray;
	Track1DimClass	*TrackX,*TrackXTest,*TrackX0,*TrackX1,*TrackXTemp;
	Track1DimClass	*TrackY,*TrackYTest,*TrackY0,*TrackY1,*TrackYTemp;
	int		NTrackXCandidates,NTrackYCandidates;
	int		NClustersTotalX,NClustersTotalY;
	double		NClustersAverage;
	double		MaxKhi2OverNdf,GoodKhi2OverNdf;
	// Tracking functions
	void	Get();
	void	SearchTrackX();
	void	SearchTrackY();
	void	CheckTriggerContent();
	void	FillStatistics();
	void	SetFixPoint(double Z, double X, double Y);
	void	UnsetFixPoint();
	// Info about the track
	void	PrintDef();
	bool	Good;
	char	Info[100];
	double	GetSlopeX();
	double	GetSlopeY();
	double	GetXAtZ(double InZ);
	double	GetYAtZ(double InZ);
	double	GetLength();
	void	Print();
	char*	GetInfo();
	// Statistics
	MultiMapClass	* KonMap; // KON == Khi2 Over Ndf
	MultiMapClass	* CrossHitsMap; // NChambers crossed, NHits in X and Y
	double	KonBinSize,KonMaxMult;
	enum	{IdAll=0, IdGood=1};
	void	WriteStatisticsToFile(FILE* OutFile);
	};

double GetTrackSlopeDiff(TrackClass*,TrackClass*);

#endif


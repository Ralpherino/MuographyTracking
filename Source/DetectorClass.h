////////////////////////////////////////////////////////////////////////
// Detector Class - header
//      version : 2022-04-19
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef DetectorClassGuard
#define DetectorClassGuard

#include "AnalysisParametersClass.h"
#include "ChamberClass.h"
#include "ThpClass.h"
#include "HvClass.h"
#include "MultiMapClass.h"
#include "CombinedEfficiencyClass.h"
#include "ExtraFunctions.h"

class TrackClass;	// for cross reference

class DetectorClass
	{ public:
	DetectorClass();
	DetectorClass(int InNChambers, ChamberClass ** InChamber, char* InName);
	~DetectorClass();
	
	char	Name[50];
	AnalysisParametersClass * AParameters;
	int	NChambers;
	ChamberClass	**Chamber;
	int	TriggerCoinc;
	int	Verbose;

	// Performance and cumulative hists
	enum	{IdTrackCross=0, IdTriggerEff=1, IdTrackingEff=2, IdAcceptance=3, IdEffArea=4, IdFlux=5};
	MultiMapClass	*AngleDep;
	MultiHistDynTimeClass *TimeDep;
	enum	{IdRateNEvents=0, IdRateNTracks=1, IdRateEventRate=2, IdRateTrackRate=3};
	MultiHistDynTimeClass *Rates;
	enum	{IdTdsT=0, IdTdsH=1, IdTdsP=2, IdTdsRateEvent=3, IdTdsRateTrack=4, IdTdsUMon=5, IdTdsIMon=6, IdTdsNSum=7};
	int	TdsCh;
	MultiHistDynTimeClass *TimeDepSum;
	enum	{IdClEvent=0, IdClTrack=1};
	MultiHistClass	*CoincLevelHist;
	MultiHistClass	*DiffTimeHist;
	char	*DateTimeString;
	long	*pEventTimeDiffUs;
	int	MaxNThps;
	ThpClass **Thp;
	HvClass	*HvInfo;
	std::string FileContents;
	MultiMapClass	*CorrelMapChambersDataTrigger,*CorrelMapChambersDataAdc;
	MultiMapClass	*CoincArea;

	// SingleValue perf
	double	Efficiency,GainAverage;
	double	ZEff,ZMax,ChamberSizeX,ChamberSizeY,ZMiddle; // to be used for calculating Acceptance
	// TrackLevel info	
	int	TrackingNHits;
	double	TrackEffArea,TrackEfficiency; // stores the calculated acceptance for the last Track
	char	InfoForTrack[100];
	double	AngleViewX,AngleViewY, AngleViewXMin,AngleViewXMax,AngleViewYMin,AngleViewYMax;	// Angle of view for the tracking
	// Statistic and Run info
	double	TotalTimeSec,TotalTimeLiveSec,TimeCorrectionSec;
	long	NEvents,NTracks,NFiles;
	double	FluxMid,EventRate,TrackRate,PadSizeMm;
	char	ResFilenaming[500];
	double	DeadTimePerEventUs, DeadTimeZipSec, NEventsPerFile;
	double	DeadTimePerEventUsEstimation, DeadTimeZipPerEventUsEstimation,DeadTimeZipPerEventUsApprox;
	// Functions
	void	ResetVariables();
	void	AllocateMemory();
	void	SetName(char* InName);
	void	SetChambers(int InNChambers, ChamberClass ** InChamber);
	void	SetAnalysisParameters(AnalysisParametersClass * InAParameters);
	void	SetThp(int InMaxNThps, ThpClass ** InThp);
	void	SetHvInfo(HvClass * InHvInfo);
	void	CreateChambers(int InNChambers);
	void	ClearStatistics();
	void	SetNTimeBins(int NewNTimeBins);
	void	AddInfoFromDetector(DetectorClass* SimilarDetector);
	double	CalculateAcceptance(double SlopeX, double SlopeY);
	void	FillAcceptanceMap();
	void	GoodEvent();
	void	GoodTrack(TrackClass* Track);
	char*	GetInfoForTrack(TrackClass* Track);
	void	ComputeResults(bool DoForChambers=false);
	void	CalculateSingleValues();
	void	CalculateAngleOfView();
	void	CalculateAngDepSimpleEfficiencies();
	void	CalculateAngDepEffectiveArea();
	void	CalculateAngDepCoincArea();
	void	CalculateTimeDep();
	void	CalculateFlux();
	void	CheckCorrelationMaps();
	void	Print(bool DoForChambers=false);
	// File operations
	void	ReadBasics(char* InDetectorBasicsFileName);
	void	WriteGeneralInfoToFile(FILE* OutFile);
	void	ReadGeneralInfoFromFile(FILE* InFile);
	void	WriteResultsToFile(FILE* OutFile);
	void	ReadInfoFromFile(char* InDetectorInfoFileName, bool DoClearStat=true, bool DoTuneChambers=false);
	void	WriteResultsToFileName(char* OutResultsFileName);
	int	WriteGnuplotInfo(FILE* OutFile, int StartIndex);
	};

double FunctionJacobiMFlux(double mx, double my);

#endif


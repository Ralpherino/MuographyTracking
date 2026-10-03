////////////////////////////////////////////////////////////////////////
// Chamber Class - header
//      version : 2021-08-26
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef ChamberClassGuard
#define ChamberClassGuard

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include "AnalysisParametersClass.h"
#include "MultiMapClass.h"
#include "MultiHistClass.h"
#include "Histogram2DClass.h"
#include "MultiHistDynTimeClass.h"
#include "PositionSmootherClass.h"
#include "ExtraFunctions.h"

class TrackClass;		// for cross reference

class ClusterClass;		// defined later

extern int Verbose;

class ChamberClass
	{ public:
	ChamberClass(int InSizeX=0, int InSizeY=0);
	~ChamberClass();
	
	// Event level
	int	AdcRaw, TriggerBit;
	int	NHitsX,NHitsY;
	int	*HitPositionX,*HitPositionY;
	void	PrintEvent(TrackClass* TrackShow = NULL);
	double	Adc, AdcScaled;
	char	*DateTimeString;

	// General
	char	Name[200];
	double	PositionX,PositionY,PositionZ;
	int	SizeX,SizeY;
	double	PadSizeX,PadSizeY;
	std::string FileContents;

	// Performance
	double	EfficiencyTracking,EfficiencyTrigger,GainAdc,EffSize,EffGood;
	AnalysisParametersClass * AParameters;
	double	TrackAtX,TrackAtY,TrackSlopeX,TrackSlopeY;
	bool	ClusterOk, ClusterXOk, ClusterYOk, Triggered, TrackCrossed;
	bool	HitFound,HitFoundX,HitFoundY;
	double	DistX,DistY,Dist,NoiseClearance;
	double	AdcPedestal,AdcScale;
	int	NIds,NCluIds,NHitIds;
	enum	{IdAll=0, IdTrackCross=1, IdTriggerOk=2, IdClusterOk=3, IdHitFound=4, IdTrackingEff=5, IdTriggerEff=6,
			IdTrackAdcSum=7, IdTrackAdcScaledSum=8,	IdShiftXSum=9, IdShiftYSum=10, 
			IdFiducial=11, IdFiducialNew=12,
			IdNTriggers=13, IdTriggerRate=14 };
	enum	{IdHitAll=0, IdHitTrack=1, IdHitNoise=2, IdHitNoiseBase=3};
	char	**IdName,**CluIdName,**HitIdName;
	MultiHistClass	*HitXDep,*HitYDep;
	MultiMapClass	*PositionDep;
	MultiMapClass	*AngleDep;
	MultiMapClass	*PosAngXDep,*PosAngYDep;
	MultiHistClass	*AdcDep;
	MultiHistDynTimeClass *TimeDep;
	Histogram2DClass *Resolution;
	MultiHistClass	*PositionTune;
	enum	{IdDeltaXN=0, IdDeltaXSum=1, IdDeltaYN=2, IdDeltaYSum=3};
	MultiHistClass	*TriggerEffSlope;
	double	TesQ;
	enum	{IdTesN=0, IdTesOk=1, IdTesEff=2};
	HistogramClass	*FiducialX,*FiducialY;
	double	ResolutionXSigma,ResolutionYSigma,PositionShiftX,PositionShiftY,PositionShiftZ;
	double	SideGrowX,SideGrowY;

	// Clusters
	int	MaxNClustersX,MaxNClustersY;
	int	TrackClusterX,TrackClusterY;
	ClusterClass	**ClusterX,**ClusterY, *ClusterTemp;
	enum	{IdCluAllRaw=0,IdCluAll=1,IdCluOkRaw=2, IdCluOk=3};
	MultiHistClass	*ClusterSizeXHist,*ClusterSizeYHist;
	MultiHistClass	*ClusterPositionXHist,*ClusterPositionYHist;
	int	NPositionSmoothers;
	MultiHistClass	*ClusterSizePositionXHist,*ClusterSizePositionYHist;
	PositionSmootherClass	**PositionSmootherX,**PositionSmootherY;
	int	NFlipsX,NFlipsY;
	double	*FlipPositionX,*FlipPositionY;
	
	// MisPosition
	bool	UseMisPositionCorrection;
	int	MisPositionXMin,MisPositionXMax,MisPositionXValue;
	int	MisPositionYMin,MisPositionYMax,MisPositionYValue;
	// Broken channels
	bool	UseBrokenChannels;
	int	NBrokenChannelsX; int *BrokenChannelX;
	int	NBrokenChannelsY; int *BrokenChannelY;

	// Functions
	void	SetSize(int InSizeX, int SizeY);
	void	SetAnalysisParameters(AnalysisParametersClass* AParameters=NULL);
	void	AllocateMemory();
	void	FiducialConvertMapToXY();
	void	ClearStatistics();
	void	GoodEvent();
	void	MisPositionCorrection();
	void	BrokenChannelsCorrection();
	void	Clusterize();
	double	ConvertLocalToGlobalX(double LocalX), ConvertLocalToGlobalY(double LocalY);
	double	ConvertGlobalToLocalX(double GlobalX), ConvertGlobalToLocalY(double GlobalY);
	void	FindClusterAtTrack(TrackClass* Track);
	bool	IsTrackCrossed(TrackClass* Track);
	void	GoodTrack(TrackClass* Track);
	void	Print();
	void	ComputeTuning();
	void	ComputeResults();
	void	FitTriggerEffSlope();
	void	ComputePositionShifts();
	void	SuggestFiducial();
	bool	AddInfoFromChamber(ChamberClass* OtherChamber, bool DoComputeResults=true);
	// File operations
	bool	ReadBasicsFromFile(FILE* InFile);
	void	WriteGeneralInfoToFile(FILE* OutFile);
	void	ReadGeneralInfoFromFile(FILE* InFile);
	void	WriteResultsToFile(FILE* OutFile);
	bool	ReadInfoFromFile(FILE*, bool DoClearStat=true, bool DoComputeTuning=true);
	int	WriteGnuplotInfo(FILE* OutFile);
	bool	CheckReadWriteConsystency();
	};

class ClusterClass
	{ public:
	ClusterClass();
	double	Size,CenterRaw,Center,GlobalPos;
	void Start(double InCenter);
	void ExtendOne();
	void Smooth(PositionSmootherClass * PosSmoother);
	};


#endif


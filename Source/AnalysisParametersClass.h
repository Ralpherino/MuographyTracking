////////////////////////////////////////////////////////////////////////
//  Analysis Parameters Class - header
//      version : 2021-02-16
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef AnalysisParametersClassGuard
#define AnalysisParametersClassGuard

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class AnalysisParametersClass
	{ public:
	// Funcions
	AnalysisParametersClass();
	bool	WriteToFile(FILE* OutFile);
	bool	ReadFromFile(char* FileName);
	bool	Set(char* ParName, double ParValue);
	bool	SetNChambers(int InNChambers);
	// General info
	char	Name[200];
	// Chamber level 
	double	AngleMax,DistMax;
	double	PositionDepBinSize,AngleDepBinSize,ResolutionBinSize,TimeDepBinSizeSeconds;
	int	MaxNTimeBins;
	double	AdcMin,AdcMax,AdcBinSize;
	// Clusters
	int	ClusterSizeLimit, NClustersLimit, ClusterBreak;
	double	SmoothingPeriodLength, ClusterPositionBinSize;
	// Detector level
	double	TrackKhi2BinSize,TrackKhi2Max,TrackKhi2Good;
	int	DoSideGrowInt;
	// Tracking
	int	TrackMinNHitsSet, TrackMaxNMissSet;
	int	NChambers,TrackingNHits;
	int	PerformanceTrackMiss;
	};

#endif


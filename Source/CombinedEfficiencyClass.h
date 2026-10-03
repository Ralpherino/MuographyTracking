////////////////////////////////////////////////////////////////////////
// Combined Efficiency Class - header
//	version : 2019-03-19
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef CombinedEfficiencyClassGuard
#define CombinedEfficiencyClassGuard

#include <stdio.h>
#include <stdlib.h>
#include "MultiHistClass.h"
#include "MultiMapClass.h"
#include "CombinationClass.h"
#include "ExtraFunctions.h"

double CalculateCombinedEfficiency(int N, int K, double * Eff, bool CheckArray=false);

void CalculateMeasuredEfficiencies(int N, int K, double * Eff, bool CheckArray=false);

class CombinedEfficiencyHistClass
	{ public:
	CombinedEfficiencyHistClass();
	~CombinedEfficiencyHistClass();
	
	CombinationMinClass	*Combination;
	MultiHistClass		*EffHist, *EffCalculator, *ResultsMultiHist;
	int	NHists, HistIdResult;
	int	* HistIdArray;
	int	HistIdCombEff, HistIdTotalEff;
	
	void	Reset();
	void	SetCombination(int N, int K);
	void	SetHist(MultiHistClass * InHist);
	void	SaveResultsTo(MultiHistClass * InResultsMultiHist, int InHistIdResult);
	void	SaveResultsTo(int InHistIdResult);
	void	SetEfficiencyIdArray(int NHists, int * HistIdArray);
	void	Calculate();
	};


class CombinedEfficiencyMapClass
	{ public:
	CombinedEfficiencyMapClass();
	~CombinedEfficiencyMapClass();
	
	CombinationMinClass	*Combination;
	MultiMapClass		*EffMap, *EffCalculator, *ResultsMultiMap;
	int	NMaps, MapIdResult;
	int	* MapIdArray;
	int	MapIdCombEff, MapIdTotalEff;
	
	void	Reset();
	void	SetCombination(int N, int K);
	void	SetMap(MultiMapClass * InMap);
	void	SaveResultsTo(MultiMapClass * InResultsMultiMap, int InMapIdResult);
	void	SaveResultsTo(int InMapIdResult);
	void	SetEfficiencyIdArray(int NMaps, int * MapIdArray);
	void	Calculate();
	
	};



#endif


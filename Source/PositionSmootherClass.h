////////////////////////////////////////////////////////////////////////
// Position Smoother Class - header
//      version : 2021-10-12
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef PositionSmootherClassGuard
#define PositionSmootherClassGuard

#include <stdio.h>
#include <stdlib.h>
#include "HistogramClass.h"
#include "MultiHistClass.h"
#include "ExtraFunctions.h"

class SmootherPeriodClass;

class PositionSmootherClass
	{ public:
	PositionSmootherClass();
	~PositionSmootherClass();

	void	Set(MultiHistClass * InMultiHist, int InHistId, double InPeriodLengthX, int NFlips=0, double *FlipPosition=NULL);

	char	Name[100];
	void	GetPeriodCoordinates(int Bin, int* CoordPeriod, int* CoordLocal);
	void	CalculateSmoothing();
	double	Smooth(double x);
	bool	CheckPeriodUsability();
	
	bool	IsSet;
	double	PeriodLengthX, PeriodGroupSizeX;
	int	PeriodLength, NPeriods;
	int	PeriodGroupSize, PeriodGroupMultiplicity;
	double	UseRatio;

	double	StartValue,BinSize;
	
	SmootherPeriodClass ** Period;
	SmootherPeriodClass * ThePeriod, *ThePeriodF;
	bool	UseFlip,*IsFlipped;
	};

class SmootherPeriodClass
	{ public:
	SmootherPeriodClass(int InLength);
	~SmootherPeriodClass();
	int	Length;
	bool	Good;
	double	*DataRaw,*DataEq;
	double	AverageRaw,AverageEq;
	double	ComparedDiff;
	double	*Start,*Mult;
	void	CalculateAverage();
	void	Equalize(SmootherPeriodClass * PeriodBefore, SmootherPeriodClass * PeriodAfter);
	void	ClearEq();
	void	Flip();
	void	Shift(int NShift=1);
	void	AddEq(SmootherPeriodClass * InPeriod);
	double	CompareTo(SmootherPeriodClass * InPeriod);
	void	PrepareSmoother();
	double	Smooth(int X);
	};


#endif



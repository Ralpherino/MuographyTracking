////////////////////////////////////////////////////////////////////////
// Histrogram2Dim Class  - header
//	version : 2018-03-21
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef Histogram2DClass_GUARD
#define Histogram2DClass_GUARD

#include "HistogramClass.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


class Histogram2DClass
	{ public:
	Histogram2DClass(double InXStartValue=0., double InXEndValue=1., double InXBinSize=0.1, 
				double InYStartValue=0., double InYEndValue=1., double InYBinSize=0.1, 
				char * InName = (char*)"");
	~Histogram2DClass();
	// Variables
	char	Name[200];
	double	XStartValue, XEndValue, XBinSize, YStartValue, YEndValue, YBinSize;
	int	MaxNXBins, MaxNYBins, NData;
	double	**Hist;
	// Helping functions
	void	AllocateMemory();
	void	FreeMemory();
	// Histogram2D operations 
	void	Clear();
	void	Add(double InX, double InY, double InWeight=1.0);
	bool	Clone(Histogram2DClass *OriginalHist2D);
	bool	SameBinningAs(Histogram2DClass* AnyHist2D);
	bool	Add2DHist(Histogram2DClass* MapToAdd);
	bool	MultiplyBy(Histogram2DClass* MultiplicatorHist2D);
	bool	DivideBy(Histogram2DClass* DivisorHist2D);
	bool	BuildFromTwoHists(HistogramClass* HistX, HistogramClass* HistY);
	void	Fill(double FillValue=1.0);
	void	NormIntegral(double NewIntegral=1.0);
	void	Sink(double SinkLevel);
	void	SinkByAverage(double SinkMultiplicator=1.0);
	void	Crop(double XMin, double XMax, double YMin, double YMax);
	void	Cut(double XMin, double XMax, double YMin, double YMax);
	void	Smooth(int NIterations=1);
	void	SmearSquare(double InRadius);
	void	SmearRound(double InRadius);
	void	CutEdges();
	void	AddXHistAtY(HistogramClass* HistToAdd, double InY);
	void	AddYHistAtX(HistogramClass* HistToAdd, double InX);
	// Getting info
	double	Integral();
	double	MeanX(), MeanY();
	double	SigmaX(), SigmaY(), Sigma();
	double	Peak(double * pPeakX = NULL, double * pPeakY = NULL);
	void	GetZDistribution(HistogramClass* ZHist);
	double	GetZMin(), GetZMax(), GetZAverage();
	// Bin info and operations
	int	GetXBin(double InX), GetYBin(double InY);
	double	XAt(int InNXBin), XMinAt(int InNXBin), XMaxAt(int InNXBin);
	double	YAt(int InNYBin), YMinAt(int InNYBin), YMaxAt(int InNYBin);
	double	ZAt(int InNXBin, int InNYBin), ZScaledAt(int InNXBin, int InNYBin);
	double	Z(double InX, double InY);
	double	GetXBinDouble(double InX);
	double	GetYBinDouble(double InY);
	double	GetWeightedZAtBinDouble(double InNXBinDouble, double InNYBinDouble);
	double	GetWeightedZInXY(double InX, double InY);
	// Projections, slices
	bool	GetXSliceAtYBin(int InNYBin, HistogramClass* HistSlice);
	bool	GetXSliceAtY(double InY, HistogramClass* HistSlice);
	bool	GetYSliceAtXBin(int InNXBin, HistogramClass* HistSlice);
	bool	GetYSliceAtX(double InX, HistogramClass* HistSlice);
	void	GetProjectionToX(HistogramClass* HistProjection);
	void	GetProjectionToY(HistogramClass* HistProjection);
	// Write and File operations
	void	WriteParametersToScreen();
	void	WriteToFile(FILE * OutFile);
	bool	WriteToNewFile(char* NewFileName);
	void	WriteToFileGpBars(FILE * OutFile);
	void	WriteToFileGpRebin(FILE * OutFile, double DensityMult=2.0);
	void	WriteToFileProjectionToX(FILE * OutFile);
	void	WriteToFileProjectionToY(FILE * OutFile);
	bool	ReadFromFile(FILE * InFile);
	};


#endif


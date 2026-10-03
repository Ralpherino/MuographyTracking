////////////////////////////////////////////////////////////////////////
// Multiple Histograms Class - header
//	version : 2020-05-04
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef MultiHistClassGuard
#define MultiHistClassGuard

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "HistogramClass.h"

class MultiHistClass
	{ public:
	MultiHistClass();
	MultiHistClass(int InMaxNHists, double InStartValue = 0., double InEndValue = 1., double InBinSize = 1.0, char* InName=(char*)"MultiHist");
	MultiHistClass(int InMaxNHists, MultiHistClass* SimilarMultiHist);
	~MultiHistClass();
	
	double	StartValue, EndValue, BinSize;
	int	MaxNHists, MaxNBins;
	double	**Hist;
	char	TheName[100];
	char	Name[100][100];

	// Basic functions
	void	AllocateMemory();
	void	SetName(int InNMap, char* InName);
	int	CheckNHist(int InNHist);
	void	ClearAll();
	void	Clear(int InNHist);
	// Bin operations
	int	GetBin(double InX);
	void	Add(int InNHist, double InX, double InWeight=1.0);
	void	Set(int InNHist, double InX, double InValue);
	double	XAt(int InNBin);
	double	YAt(int NHist, int NBin);
	double	Y(int NHist, double X);
	double	XAtDouble(double NBinDouble);
	// Get info
	double	Integral(int NHist);
	double	IntegralFromTo(int InNHist, double InFrom, double InTo);
	double	IntegralFrom(int InNHist, double InFrom);
	double	IntegralTo(int InNHist, double InTo);
	double	Mean(int NHist);
	double	PeakPosition(int NHist);
	double	PeakPositionInRange(int InNHist, double XMin, double XMax);
	double	YAverage(int NHist, bool IncludeZeros=false);
	double	YMin(int NHist), YMax(int NHist);
	bool	IsSameBinningAsHist(HistogramClass* OtherHist);
	bool	IsSameBinningAsHist(MultiHistClass* OtherMultiHist);
	bool	IsSimilarAs(MultiHistClass* OtherMultiHist);
	// Hist operations
	void	Fill(int NHist, double FillValue=1.0);
	void	NormIntegral(int NHist, double NewIntegral);
	void	CutEdges(int NHist);
	void	Crop(int NHist, double XMin, double XMax);
	void	Cut(int NHist, double XMin, double XMax);
	void	Export(int NHist, HistogramClass* OutHist);
	bool	Import(int NHist, HistogramClass* InHist);
	bool	Import(int NHist, MultiHistClass* OtherMultiHist, int InOtherHistIdToImport, bool CopyName=true);
	void	AddHist(int NHist, HistogramClass* AddHist);
	void	AddHist(int NHist, MultiHistClass* SomeMultiHist, int SomeNHist);
	void	MultiplyWithNumber(int NHist, double Multiplicator);
	void	CopyFromTo(int NHistFrom, int NHistTo);
	void	ModifyHist(int NHist, double (*Function) (double));
	void	ModifyHistWith(int NHistToModify, int NHistToFunction, double (*Function) (double,double));
	void	PutSumOfThese(int NHistResult, int NHistsToUse, int* HistNArray);
	void	PutProductOfThese(int NHistResult, int NHistsToUse, int* HistNArray);
	void	ExchangeToIntegral(int NHist);
	bool	AddSimilarMultiHist(MultiHistClass* SomeMultiHist);
	bool	AddSimilarMultiHistSelection(MultiHistClass* SomeMultiHist, int NHistsToAdd, int* pNHistToAdd);
	bool	AddMultiHist(MultiHistClass* SomeMultiHist);
	bool	AddMultiHistSelection(MultiHistClass* SomeMultiHist, int NHistsToAdd, int* pNHistToAdd);
	// File operations
	void	WriteParametersToScreen();
	void	WriteToFile(FILE * Outfile);
	void	WriteToFileZs(FILE * Outfile);
	void	WriteToNewFile(char* NewFileName);
	bool	ReadFromFile(FILE * InFile);
	bool	AddFromFile(FILE * InFile);

	};

#endif

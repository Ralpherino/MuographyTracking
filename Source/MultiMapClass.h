////////////////////////////////////////////////////////////////////////
// Multiple Maps Class - header
//	version : 2021-02-18
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef MultiMapClass_GUARD
#define MultiMapClass_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "HistogramClass.h"
#include "Histogram2DClass.h"
#include "MultiHistClass.h"

class MultiMapClass
	{ public:
	MultiMapClass(int InMaxNMaps, double InXStartValue=0., double InXEndValue=1., double InXBinSize=0.1, 
				double InYStartValue=0., double InYEndValue=1., double InYBinSize=0.1, char* InTheName=(char*)"Map");
	MultiMapClass(int InMaxNMaps, MultiMapClass * SimilarMultiMap, char* InTheName=(char*)"Map");
	~MultiMapClass();
	// Variables
	int	MaxNMaps, MaxNXBins, MaxNYBins;
	double	XStartValue, XEndValue, XBinSize, YStartValue, YEndValue, YBinSize;
	double	***Map; // coordinates: Map[N,x,y]
	char	TheName[100],Name[100][100];

	// Basic Functions
	void	AllocateMemory();
	void	FreeMemory();
	void	SetTheName(char* InTheName);
	void	SetName(int InNMap, char* InName);
	int	CheckNMap(int InNMap);
	void	ClearAll();
	void	Clear(int InNMap);
	void	ResetToSize(double InXStartValue, double InXEndValue, double InYStartValue, double InYEndValue);
	// Bin operations
	void	Add(int InNMap, double InX, double InY, double InWeight=1.0);
	void	Set(int InNMap, double InX, double InY, double InValue);
	void	SetAt(int InNMap, int InNXBin, int InNYBin, double InValue);
	void	SetLineX(int InNMap, double InX, double InValue);
	void	SetLineY(int InNMap, double InY, double InValue);
	void	SetLineXWidth(int InNMap, double InX, double InValue, double InWidth);
	void	SetLineYWidth(int InNMap, double InY, double InValue, double InWidth);
	int	GetXBin(double InX), GetYBin(double InY);
	double	XAt(int InNXBin), XMinAt(int InNXBin), XMaxAt(int InNYBin);
	double	YAt(int InNYBin), YMinAt(int InNYBin), YMaxAt(int InNYBin);
	double	ZAt(int InNMap, int InNXBin, int InNYBin);
	double	Z(int InNMap, double InX, double InY);
	double	ZWeighted(int InNMap, double InX, double InY);
	// Get info
	double	GetMeanX(int InNMap), GetMeanY(int InNMap);
	double	GetZMin(int InNMap), GetZMax(int InNMap);
	bool	SameBinningAsHist2D(Histogram2DClass* AnyHist2D), SameBinningAsMultiMap(MultiMapClass * MMap);
	bool	IsSimilarAs(MultiMapClass* SomeMultiMap);
	double	Integral(int InNMap);
	double	IntegralInRange(int InNMap, double XMin, double XMax, double YMin, double YMax);
	double	AverageInRange(int InNMap, double XMin, double XMax, double YMin, double YMax);
	double	PeakPositionOnLineX(int InNMap, double InX), PeakPositionOnLineY(int InNMap, double InY);
	double	Peak(int InNMap, double* PeakPosX=NULL, double* PeakPosY=NULL);
	bool	GetSliceXIntegral(int InNMap, double YMin, double YMax, MultiHistClass* MHist, int InNHist);
	bool	GetSliceYIntegral(int InNMap, double XMin, double XMax, MultiHistClass* MHist, int InNHist);
	double	AreaAbove(int InNNap, double Min, double* MidX=NULL, double* MidY=NULL);
	// Map opeations
	void	Fill(int InNMap, double FillValue=1.0);
	void	FillFunction(int InNMap, double (*Function) (double,double));
	void	ClearBorders(int InNMap);
	void	Export(int InNMap, Histogram2DClass* OutMap);
	bool	Import(int InNMap, Histogram2DClass* InMap);
	bool	Import(int InNMap, MultiMapClass* OtherMultiMap, int InOtherMapIdToImport, bool InCopyName=true);
	void	AddMap(int InNMap, Histogram2DClass* SomeMap);
	void	AddMap(int InNMap, MultiMapClass* OtherMultiMap, int OtherNMapToAdd);
	void	AddVolumeShifted(int InNMap, MultiMapClass* OtherMultiMap, int OtherNMapToAdd, double ShiftX=0, double ShiftY=0);
	void	CopyFromTo(int InNMapFrom, int InNMapTo);
	void	MultiplyMap(int InNMap, double Multiplicator);
	void	ModifyMap(int InNMap, double (*Function) (double));
	void	ModifyMapWith(int InNMap1, int InMap2, double (*Function) (double,double));
	void	DivideMapWithCutElse(int InNMap1, int InNMap2, double InCut, double InElse);
	void	DivideMapWithAvgCutElse(int InNMap1, int InNMap2, double AvgRadius, double InCut, double InElse, double ElseCut=-1);
	void	PutSumOfThese(int InNMap, int NMapsToUse, int * MapIdArray);
	void	PutProductOfThese(int InNMap, int NMapsToUse, int * MapIdArray);
	bool	AddSimilarMultiMap(MultiMapClass* SomeMultiMap);
	bool	AddSimilarMultiMapSelection(MultiMapClass* SomeMultiMap, int NMapsToAdd, int* pSelection);
	bool	FillGapsRangeCutTemp(int InNMap, int RMax, double Cut, int IdTemp);
	// File operations
	void	WriteToFile(FILE * OutFile);
	void	WriteToNewFile(char* NewFileName);
	void	WriteToFileGpBars(FILE * OutFile);
	void	WriteToFileNameGpBars(char* FileName);
	void	WriteToFileProjections(int InNMap, FILE * OutFile);
	bool	ReadFromFile(FILE * InFile);
	bool	ReadFromFileName(char* InFileName);
	// User and debug
	void	ShowInfo();
	};


#endif


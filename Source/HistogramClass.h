#ifndef HistogramClassGuard
#define HistogramClassGuard
////////////////////////////////////////////
// HistrogramClass - header
//	written by : Gergo" Hamar
// 	version: 2020-06-05
////////////////////////////////////////////
#include <stdio.h>
#include "FitLineClass.h"

class HistogramClass
	{ public:
	HistogramClass(double InStartValue = 0., double InEndValue = 1., double InBinSize = 0.1, char * InName = (char*)"Hist");
	~HistogramClass();

// Variables
	char	Name[200];
	double	StartValue, EndValue, BinSize;
	int	MaxNBins, NData;		// counter
	double	*Hist;
// Helping functions
	void	AllocateMemory();
	void	FreeMemory();
// Histogram operations
	void	Clear();
	void	Add(double InX, double InWeight=1.0);
	bool	Clone(HistogramClass* HistToClone);
	bool	SameBinning(HistogramClass* IsSameBinningHist);
	bool	IsSameAs(HistogramClass* OtherHist);
	bool	AddHist(HistogramClass* AddedHist);
	bool	MultiplyBy(HistogramClass* MultiplicatorHist);
	bool	DivideBy(HistogramClass* DividerHist);
	void	Fill(double FillValue=1.0);
	void	NormIntegral(double NewIntegral=1.0);
	void	Sink(double SinkLevel);
	void	SinkByAverage(double multiplicator=1.0);
	void	Crop(double XMin, double XMax);
	void	Cut(double XMin, double XMax);
	void	Smooth(int NIterations=1);
	void	SmoothStep(int HalfWidth=1);
	void	SmoothStepDouble(double HalfWidth);
	void	CutEdges();
	void	ShiftXBin(int ShiftBin);
	void	ShiftX(double Shift);
// Getting info
	double	Mean();
	double	Sigma();
	double	Median();
	double	TruncatedMean(double TruncMin, double TruncMax);
	double	PeakPosition();
	double	PeakHight();
	double	PeakSigma(int NInterations=3, double SigmaCut=4.0, double * pThisPeakPosition=NULL);
	double	PeakSmooth(int NInterations=3, double SigmaCut=4.0, double * pThisSigma=NULL);
	double	PeakFwhm();
	double	LocalPeakPosition(double XMin, double XMax);
	double	LocalPeakFwhm(double XMin, double XMax);
	double	LocalValleyPosition(double XMin, double XMax);
	double	Integral();
	double	IntegralFromTo(double InFrom, double InTo);
	double	IntegralFrom(double InFrom);
	double	IntegralTo(double InTo);
	double	FirstZeroFromLeft(double From);
	double	FirstZeroFromRight(double From);
// Bin info and operations
	int	GetBin(double InX);
	double	XAt(int InNBin), XMinAt(int InNBin), XMaxAt(int InNBin);
	double	YAt(int InNBin), YScaledAt(int InNBin);
	double	Y(double InX);
	double	XAtDouble(double InNBinDouble);
	double	GetBinDouble(double InX);
	double	GetWeightedYAtDouble(double InNBinDouble);
	double	GetWeightedY(double InX);
// Fit line
	bool	FitLine(FitLineClass * line, double InMinX, double InMaxX);
	bool	FitLog(FitLineClass * logline, double InMinX, double InMaxX);
// Write to / read
	void	WriteToScreen();
	void	WriteParametersToScreen();
	void	WriteToFile(FILE * Outfile);
	void	WriteToNewFile(char * NewFilename);
	void	WriteToFilename(char* Filename, char* OpenType=(char*)"w");
	bool	ReadFromFile(FILE * InFile);
	bool	ReadFromGeneralFile(FILE * InFile, int MaxNReadBins=30000);
	};

#endif


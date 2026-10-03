#ifndef MultiHistFuncBinClassGuard
#define MultiHistFuncBinClassGuard
////////////////////////////////////////////////////////////////////////
// Multiple Histogram with Function defined BinSizes - header
//	version : 2020-03-13
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiHistClass.h"
#include <time.h>
#define AMaxNBins_MultiHistFuncBinClass 10000

class MultiHistFuncBinClass // : public MultiHistClass
	{ public:
	MultiHistFuncBinClass(int InMaxNHists, double InStartValue, double InEndValue, double InBinSize0, double (*InBsFunction)(double) , char* InName);
	//MultiHistFuncBinClass(int InMaxNHists, double InStartValue, double InEndValue, double InBinSize0, double (*InBsFunction)(double,double) , double InFunctionPar);
	~MultiHistFuncBinClass();

	double	StartValue,EndValue,BinSize;
	int	MaxNBins,MaxNHists;
	double	**Hist;
	char	TheName[100];
	char	**Name;
	double (*BsFunction)(double);
	double *BinStart,*BinSizeA;

	// General
	void AllocateMemory();

	// Basic functions : good as is
	// Bin operations
	int	CheckNHist(int InNHist);
	int	CheckNBin(int InNBin);
	int	GetBin(double InX);
	void	Add(int InNHist, double InX, double InWeight=1.0);
	void	Set(int InNHist, double InX, double InValue);
	double	XAt(int InNBin);
	double	YAt(int NHist, int NBin);
	double	Y(int NHist, double X);
	
	// Get info
	double	Integral(int NHist);
/*	double	IntegralFromTo(int InNHist, double InFrom, double InTo);
	double	IntegralFrom(int InNHist, double InFrom);
	double	IntegralTo(int InNHist, double InTo);
	double	Mean(int NHist);
	double	PeakPosition(int NHist);
	double	YAverage(int NHist, bool IncludeZeros=false);
	bool	IsSameBinningAsHist(HistogramClass* OtherHist);
	bool	IsSameBinningAsHist(MultiHistClass* OtherMultiHist);
	bool	IsSimilarAs(MultiHistClass* OtherMultiHist);
	// Hist operations
	void	Fill(int NHist, double FillValue=1.0);
*/	void	NormIntegral(int NHist, double NewIntegral);
/*	void	CutEdges(int NHist);
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
	bool	AddSimilarMultiHist(MultiHistClass* SomeMultiHist);
	bool	AddSimilarMultiHistSelection(MultiHistClass* SomeMultiHist, int NHistsToAdd, int* pNHistToAdd);
	bool	AddMultiHist(MultiHistClass* SomeMultiHist);
	bool	AddMultiHistSelection(MultiHistClass* SomeMultiHist, int NHistsToAdd, int* pNHistToAdd);
	// File operations
	void	WriteParametersToScreen();
*/	void	WriteToFile(FILE * Outfile);
/*	void	WriteToFileZs(FILE * Outfile);
	void	WriteToNewFile(char* NewFileName);
	bool	ReadFromFile(FILE * InFile);
	bool	AddFromFile(FILE * InFile);
*/	};





#endif



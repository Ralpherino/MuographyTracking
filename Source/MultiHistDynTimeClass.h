#ifndef MultiHistDynTimeClassGuard
#define MultiHistDynTimeClassGuard
////////////////////////////////////////////////////////////////////////
// Multiple Histogram Dynamic allocation on Time Class - header
//	version : 2019-03-19
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MultiHistClass.h"
#include <time.h>

class MultiHistDynTimeClass : public MultiHistClass
	{ public:
	MultiHistDynTimeClass(int InMaxNHists, double InBinSize = 1.0, char* InName=(char*)"MultiHistDynTime");
	MultiHistDynTimeClass(int InMaxNHists, MultiHistDynTimeClass* SimilarMhdt);
	~MultiHistDynTimeClass();
	
	time_t	TimeZero;
	struct	tm *ThisDateTime;
	char	DateTimeStringTemp[20], TAtString[20];
	int	DTYear,DTMonth;
	double	TimeSec, LastX;
	bool	AutoScale, HistType;
	int	AutoScaleLimitNBins;

	// General
	void	SetTimeZero(char* InDateTimeString, bool Exact=false);
	void	AllocateMemory(int NewBin);
	void	ClearAll();
	char*	PrintDt(time_t * DtTimet);
	// Bin operations
	int	GetBinDyn(double InX);
	char*	TAt(int NBin);
	void	Add(int InNHist, char* DateTimeString, double InWeight=1.0);
	void	AddX(int InNHist, double InX, double InWeight=1.0);
	void	Set(int InNHist, char* DateTimeString, double InValue);
	void	SetX(int InNHist, double InX, double InValue);
	// Hist operations
	void	ScaleBinSize(int BinSizeScale=2);
	void	SetAutoScale(int InAutoScaleLimitNBins);
	void	ConvertToRate(int InToRate);
	void	RateFromTo(int InFrom, int InTo);
	void	AddTimeHist(int NHist, MultiHistDynTimeClass* OtherMultiTimeHist, int OtherNHist);
	bool	AddMultiTimeHist(MultiHistDynTimeClass* OtherMultiTimeHist);
	// File operations
	void	WriteToFile(FILE* OutFile);
	bool	ReadFromFile(FILE* InFile);
	};





#endif



////////////////////////////////////////////////////////////////////////
// HighVoltage info Class - header
//      version : 2019-03-19
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef HvClassGuard
#define HvClassGuard

#include <stdio.h>
#include <stdlib.h>
#include "MultiHistDynTimeClass.h"

class HvClass
	{ public:
	HvClass(double TimeBinSize=1.0, int AutoScaleLimit=500);
	~HvClass();
	
	double	UIseg,UMon,IMon;
	enum	{IdN=0, IdUIseg=1, IdUMon=2, IdIMon=3};
	int	Id;
	MultiHistDynTimeClass	*TimeDev;
	void	SetDt(char* InDateTime);
	void	SetDtHv(char* InDateTime, double InUIseg, double InUMon, double InIMon);

	};

#endif


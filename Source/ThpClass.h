////////////////////////////////////////////////////////////////////////
// Temperature-Humidity-Pressure Class - header
//      version : 2019-03-19
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef ThpClassGuard
#define ThpClassGuard

#include <stdio.h>
#include <stdlib.h>
#include "MultiHistDynTimeClass.h"

class ThpClass
	{ public:
	ThpClass(double TimeBinSize=1.0, int AutoScaleLimit=500);
	~ThpClass();
	
	double	T,H,P;
	enum	{IdN=0, IdT=1, IdH=2, IdP=3};
	int	Id;
	MultiHistDynTimeClass	*TimeDev;
	void	SetDt(char* InDateTime);
	void	SetDtThp(char* InDateTime,double InT, double InH, double InP);

	};

#endif


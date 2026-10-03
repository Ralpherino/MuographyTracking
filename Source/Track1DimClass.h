////////////////////////////////////////////////////////////////////////
// Track 1Dim Class - header
//      version : 2019-08-26
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef Track1DimClassGuard
#define Track1DimClassGuard

#include "FitLineClass.h"
#include "ChamberClass.h"

class Track1DimClass
	{ public:
	Track1DimClass();
	~Track1DimClass();
	
	int	NPoints;
	double	Resolution;
	FitLineClass*	FittedLine;
	
	void	Clear();
	void	AddPointX(ChamberClass* Ch, int NCluster);
	void	AddPointY(ChamberClass* Ch, int NCluster);
	void	AddPoint(double CoordChamber, double CoordCluster);
	void	Fit();
	double	Khi2OverNdf;
	double	AtChamber(ChamberClass* Ch);
	
	};


#endif


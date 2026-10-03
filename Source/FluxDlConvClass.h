////////////////////////////////////////////////////////////////////////////////
// Flux to DensityLength Conversion Class
//      version : 2023-04-28
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "MultiMapClass.h"

#ifndef FluxDlConversionClassGuard
#define FluxDlConversionClassGuard

// Flux -- Bin  conversion
//	for compact and wide-ranged storage 

double FluxToBin(double InFlux);
double BinToFlux(double InBin);


class FluxDlConversionClass : MultiMapClass
	{ public:
	enum {IdFlux=0,IdDl,IdDlRelErr};
	double ZenithMin,ZenithMax,ZenithBinSize, FluxMin,FluxMax,DlMin,DlMax,Resolution;
	FluxDlConversionClass();
	//~FluxDlConversionClass();
	bool ReadFromFile(FILE * InFile);
	bool ReadFromFileName(char* InFileName);
	bool CheckFluxBinFunction();
	double GetDlFromZF(double InZenith, double InFlux);
	double GetDlFromZdegF(double InZenithDeg, double InFlux);
	double GetDlRelErrFromZF(double InZenith, double InFlux);
	
	};


#endif



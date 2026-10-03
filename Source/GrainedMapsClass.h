////////////////////////////////////////////////////////////////////////
// Grained Maps Class - header
//	version : 2018-10-30
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef GrainedMapsClass_GUARD
#define GrainedMapsClass_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ExtraFunctions.h"
#include "MultiMapClass.h"
#include "ZonedMapsClass.h"

class GrainClass
	{ public:
	GrainClass();
	GrainClass(GrainClass*);
	GrainClass(double**,int,int,int,int);
	~GrainClass();
	double	**MapN;
	int	X,Y,SizeX,SizeY;
	double	N;
	bool	Chop();
	void	ShowInfo();
	bool	Inside(double PX, double PY);
	GrainClass *NewGrain[3];
	const int NNewGrains=3;
	};

class GrainedMapsClass
	{ public:
	GrainedMapsClass();
	~GrainedMapsClass();
	char		*TheName;
	MultiMapClass	*MMap;
	int		IdMapN;
	GrainClass	**Grain;
	int		NGrains;
	double		Limit;
	ZonedMapsClass	*ZMap;
	// Functions
	void	SetMap(MultiMapClass *InMMap, int InIdMapN, double InLimit=100);
	void	SetName(char *InTheName);
	bool	Compute();
	void	FillZonedMaps();
	void	GrainsToBaseMapFxyo(double (*Function) (double,double), int IdFuncArgX, int IdFuncArgY, int IdFuncResult);
	void	WriteToFile(FILE * OutFile);
	void	WriteToFileName(char* OutFileName);
	};



#endif


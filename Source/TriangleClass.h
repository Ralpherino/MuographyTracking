////////////////////////////////////////////////////////////////////////
// TriangleClass - header
//	for operating with 3dim triangles
//      version : 2024-03-08
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "VectorClass.h"

#ifndef TriangleClassGuard
#define TriangleClassGuard

class TriangleClass
	{ public:
	TriangleClass();
	~TriangleClass();
	
	VectorClass A,B,C,Norm;
	double	Offset=0;
	// Functions
	void	ReadFromFile(FILE* InFile);
	void	ReadFromStl(FILE* InFile);
	void	ComputeNorm();
	bool	Inside(VectorClass InQ);
	};
#endif

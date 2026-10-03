////////////////////////////////////////////////////////////////////////
// RayClass - header
//	for operating with 3dim rays
//      version : 2024-03-08
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "VectorClass.h"
#include "TriangleClass.h"
#include "ExtraFunctions.h"

#ifndef RayClassGuard
#define RayClassGuard

class RayClass
	{ public:
	RayClass();
	~RayClass();
	
	VectorClass Base,Dir;
	int	NIntersections=0;
	double	*IntersectionPos=NULL;
	double	Path=0;
	// Functions
	void	Clear();
	void	SetBaseDir(VectorClass InBase, VectorClass InDir);
	void	ReadFromFile(FILE* InFile);
	void	AddIntersection(double InT);
	void	IntersectTriangle(TriangleClass* Triangle);
	void	IntersectTriangleArray(TriangleClass* InTriangles, int InMaxNTriangles);
	double	GetPath();
	};
#endif

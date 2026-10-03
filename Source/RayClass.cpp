////////////////////////////////////////////////////////////////////////
// RayClass - implementation
//	for operating with 3dim rays
//      version : 2024-03-08
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "RayClass.h"

RayClass::RayClass()
	{
	}

RayClass::~RayClass()
	{
	if (IntersectionPos!=NULL) free(IntersectionPos);
	}

void RayClass::Clear()
	{
	if (IntersectionPos!=NULL) { free(IntersectionPos); IntersectionPos=NULL; }
	NIntersections=0;
	}

void RayClass::ReadFromFile(FILE* InFile)
	{
	Base.ReadXyzFromFile(InFile);
	Dir.ReadXyzFromFile(InFile);
	Dir.Norm();
	Clear();
	}

void RayClass::SetBaseDir(VectorClass InBase, VectorClass InDir)
	{
	Base=InBase;
	Dir=InDir;
	Dir.Norm();
	}

void RayClass::IntersectTriangle(TriangleClass* Triangle)
	{
	if ((Triangle->Norm % Dir)==0) return; // Parallel
	double t = (Triangle->Offset - (Triangle->Norm%Base))/(Triangle->Norm%Dir);
	if (t<0) return; // Other sise
	VectorClass Q=Base+Dir*t;
	if (!(Triangle->Inside(Q))) return; // Outside;
	AddIntersection(t);
	//printf("t= %g\n",t);
	}

void RayClass::AddIntersection(double InT)
	{
	NIntersections++;
	IntersectionPos = (double*) realloc(IntersectionPos,NIntersections*sizeof(double));
	IntersectionPos[NIntersections-1]=InT;
	}

void RayClass::IntersectTriangleArray(TriangleClass* InTriangles, int InMaxNTriangles)
	{
	for (int NT=0; NT<InMaxNTriangles; NT++)
		IntersectTriangle(&(InTriangles[NT]));
	if (NIntersections>0) SortArrayAscending(IntersectionPos,NIntersections);
	NIntersections=FilterArrayUnique(IntersectionPos,NIntersections);
	}

double RayClass::GetPath()
	{
	Path=0;
	if (NIntersections==0) return Path;
	if (NIntersections%2==0)
		for (int n=1; n<NIntersections; n+=2) Path+=IntersectionPos[n]-IntersectionPos[n-1];
	else 
		Path=IntersectionPos[0];
		for (int n=2; n<NIntersections; n+=2) Path+=IntersectionPos[n]-IntersectionPos[n-1];
	return Path;
	}




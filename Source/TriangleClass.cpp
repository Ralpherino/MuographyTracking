////////////////////////////////////////////////////////////////////////
// TriangleClass - implementation
//	for operating with 3dim triangles
//      version : 2024-03--8
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "TriangleClass.h"

TriangleClass::TriangleClass()
	{
	}

TriangleClass::~TriangleClass()
	{
	}

void TriangleClass::ReadFromFile(FILE* InFile)
	{
	A.ReadXyzFromFile(InFile);
	B.ReadXyzFromFile(InFile);
	C.ReadXyzFromFile(InFile);
	ComputeNorm();
	}

void TriangleClass::ReadFromStl(FILE* InFile)
	{
	char	Mark[20];
	fscanf(InFile,"%s %*s %*s %*s %*s %*s %*s",Mark);
	if (strcmp(Mark,"facet")!=0) {printf("Wrong STL file. \nExit\n\n"); exit(2); };
	fscanf(InFile,"%*s"); A.ReadXyzFromFile(InFile);
	fscanf(InFile,"%*s"); B.ReadXyzFromFile(InFile);
	fscanf(InFile,"%*s"); C.ReadXyzFromFile(InFile);
	fscanf(InFile,"%*s %*s ");
	ComputeNorm();
	}

void TriangleClass::ComputeNorm()
	{
	// TODO
	// n=(b-a)X(c-a)  v   axb+bxc+cxa
	VectorClass AB=B-A;
	VectorClass AC=C-A;	
	Norm=(AB/AC);
	Norm.Norm();
	Offset=Norm%A;
	}

bool TriangleClass::Inside(VectorClass Q)
	{
	return ( ((((B-A)/(Q-A))%Norm)>=0) && ((((C-B)/(Q-B))%Norm)>=0) && ((((A-C)/(Q-C))%Norm)>=0) );
	}


////////////////////////////////////////////////////////////////////////
// Track 1Dim Class - implementation
//      version : 2019-08-26
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "Track1DimClass.h"


Track1DimClass::Track1DimClass()
	{
	NPoints=0;
	FittedLine = new FitLineClass();
	Resolution=0.3*12; // Shall be rewritten based on the real one
	}

Track1DimClass::~Track1DimClass()
	{
	delete FittedLine;
	}

void Track1DimClass::AddPointX(ChamberClass* Ch, int NCluster)
	{
	FittedLine->Add( Ch->PositionZ , Ch->ClusterX[NCluster]->GlobalPos );
	}

void Track1DimClass::AddPointY(ChamberClass* Ch, int NCluster)
	{
	FittedLine->Add( Ch->PositionZ , Ch->ClusterY[NCluster]->GlobalPos );
	}

void Track1DimClass::AddPoint(double CoordChamber, double CoordCluster)
	{
	FittedLine->Add(CoordChamber,CoordCluster);
	}


void Track1DimClass::Clear()
	{
	FittedLine->Null();
	Khi2OverNdf=100;
	}

void Track1DimClass::Fit()
	{
	FittedLine->Fit();
	NPoints=FittedLine->GetEntries();
	if (NPoints<2) { Khi2OverNdf=100; return; }
	if (NPoints==2) { Khi2OverNdf=0; return; }
	Khi2OverNdf=FittedLine->GetError2()/Resolution/Resolution/double(NPoints-2);
	}

double Track1DimClass::AtChamber(ChamberClass* Ch)
	{
	return (FittedLine->ComputeY(Ch->PositionZ));
	}

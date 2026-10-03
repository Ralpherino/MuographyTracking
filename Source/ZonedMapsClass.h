////////////////////////////////////////////////////////////////////////
// Zoned Maps Class - header
//	version : 2019-01-30
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef ZonedMapsClass_GUARD
#define ZonedMapsClass_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ExtraFunctions.h"
#include "MultiMapClass.h"

class ZonedMapsClass
	{ public :
	ZonedMapsClass(int InNMaps);
	ZonedMapsClass(int InNMaps, double InStartX, double InEndX, double InStartY, double InEndY);
	ZonedMapsClass(ZonedMapsClass * CloneMap);
	ZonedMapsClass(MultiMapClass * MMap, int InNMap);
	ZonedMapsClass(MultiMapClass * MMap, int InNMap, double* ZLevel, int NZLevels, bool TwoDimension=false);
	~ZonedMapsClass();
	
	int	MaxNMaps, MaxNXBins, MaxNYBins;
	double	*XMarker,*YMarker;
	double	***Map;
	char	*TheName,**Name;
	MultiMapClass	*Dummy;
	
	// Basic functions
	void	Init();
	void	AllocateMemory();
	void	FreeMemory();
	void	UseNames();
	void	SetName(char* NewName);
	void	SetMapName(int InNMap, char* NewName);
	void	FreeNames();
	// Marker functions
	void	AddXMarker(double InNewXMarker);
	void	AddYMarker(double InNewYMarker);
	void	AddXMarkerArray(int NNewXMarkers, double* NewXMarkerArray, double ShiftX=0);
	void	AddYMarkerArray(int NNewYMarkers, double* NewYMarkerArray, double ShiftY=0);
	// Map operations
	void	Shift(double ShiftX, double ShiftY);
	void	Scale(double ScaleX, double ScaleY);
	void	AddMap(int NMap, ZonedMapsClass * ZMap, int NZMap=0);
	void	AddMapShifted(int NMap, ZonedMapsClass * ZMap, int NZMap, double ShiftX, double ShiftY);
	void	AddMapShifted(int NMap, ZonedMapsClass * ZMap, double ShiftX, double ShiftY);
	void	CopyFromTo(int InNMapFrom, int InNMapTo);
	void	ModifyMapWith(int InNMap1, int InMap2, double (*Function) (double,double));
	// Dummy
	void	CreateDummy();
	void	DeleteDummy();
	void	GetFromDummy(int NMap);
	// Get Bin level info
	int	CheckMap(int InNMap);
	double	XAt(int NXBin), YAt(int NYBin);
	int	GetXBin(double X), GetYBin(double Y);
	double	ZAt(int InNMap, int NXBin, int NYBin);
	double	Z(int InNMap, double InX, double InY);
	// Get Map level info
	double	Integral(int NMap);
	// File operations
	void	WriteToFile(FILE * OutFile);
	void	WriteToFileName(char* FileName);
	};




#endif


////////////////////////////////////////////////////////////////////////
// Muogram Class - header
//	version : 2023-04-21
//	written by : G.Hamar
// for the R(Zenith)->Descartes system
////////////////////////////////////////////////////////////////////////

#ifndef MuogramClass_GUARD
#define MuogramClass_GUARD

#include "MultiMapClass.h"
#include "ExtraFunctions.h"
#include "VectorClass.h"
#include "FluxDlConvClass.h"
#include "TriangleClass.h"
#include "RayClass.h"

const double rd=180/M_PI;

class MuogramClass : public MultiMapClass
{ public:
	MuogramClass(double InBinSize);
	~MuogramClass();
	enum {IdZenith=0,IdAzimuth,IdOmega,IdEffArea,IdN,IdFluxM,IdFluxMErr,IdDlM,IdDlMErr,IdRlM,IdRlC,IdDlC,IdFluxC,IdNC,IdSurface,IdTunnels,IdTemp,IdTemp2,NMaps};
	double	BinSize=2;
	int	NRuns=0,NTracks=0;
	double	MeasTime=0;
	double	PosN,PosE,PosH, RotX,RotY,RotZ;
	FluxDlConversionClass	*FluxDlConv=NULL;
	HistogramClass *DensityHist, *DlDiffHist;
	double	Density=1.0;
	int	Verbose=2;
	// Bin level functions
	void AddZA(int InNMap, double Z, double A, double InWeight=1);
	void SetZA(int InNMap, double Z, double A, double InValue);
	double GetZA(int InNMap, double Z, double A);
	double GetZAW(int InNMap, double Z, double A);
	// General functions
	bool IsValidAt(int NX, int NY);
	bool IsUsableAt(int NX, int NY);
	bool ReadTrackList(char* TrackListFileName, double InRotX=0, double InRotY=0, double InRotZ=0);
	bool SetFluxDlFileName(char* FluxDlFileName);
	bool FluxToDl(char* FluxDlFileName=NULL);
	bool DlToRl(double InDensity=-1);
	// Calculation
	void SetPosition(double InPosX, double InPosY, double InPosZ);
	bool ReadDemFileName(char* DemFileName);
	bool ReadDemFileName(char* DemFileName, double InPosN, double InPosE, double InPosH);
	bool ReadStlFileName(char* StlFileName);
	bool ReadStlFileName(char* StlFileName, double InPosN, double InPosE, double InPosH);
	bool RlToDl(double InDensity=-1);
	bool DlToFlux(char* FluxFromZenithDlFileName);
	double EstimateDensity();
	bool CompareDlMC();
	// Muogram operations
	// ? kell ilyen ? bool Copy(MuogramClass* M);
	bool AddMuogram(MuogramClass* M); // N,EffA,FluxM; NRuns++; and check compatiblity !!
	// General info
	void ShowInfo();
	char TrackListFileNameBase[500]="NotUsed", DemFileNameBase[500]="NotUsed", TunnelsFileNameBase[500]="NotUsed";
	char ZfdFileNameBase[500]="NotUsed", ZdfFileNameBase[500]="NotUsed";
	// File operations
	bool WriteToFile(FILE* OutFile);
	bool ReadFromFile(FILE* InFile);
	bool WriteToFileName(char* OutFileName);
	bool ReadFromFileName(char* InFileName);
	// Conversion functions
	static double	ZenithToRadius(double Z);
	static double	RadiusToZenith(double R);
	static double	OmegaPerBs2(double x, double y);
	static double	XyToZenith(double X, double Y);
	static double	XyToAzimuth(double X, double Y);
	double	ZaToX(double Z, double A);
	double	ZaToY(double Z, double A);


};

#endif


////////////////////////////////////////////////////////////////////////
// Muogram Class - implemantation
//	version : 2023-08-28
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "MuogramClass.h"

MuogramClass::MuogramClass(double InBinSize) 
	: MultiMapClass(NMaps,-ZenithToRadius(90),ZenithToRadius(90),InBinSize,-ZenithToRadius(90),ZenithToRadius(90),InBinSize,(char*)"MuogramMap")
	{
	// Maps
	BinSize=InBinSize;
	FillFunction(IdZenith,this->XyToZenith);
	FillFunction(IdAzimuth,this->XyToAzimuth);
	FillFunction(IdOmega,this->OmegaPerBs2);
	MultiplyMap(IdOmega,BinSize*BinSize);
	SetName(IdZenith,(char*)"Zenith"); SetName(IdAzimuth,(char*)"Azimuth"); SetName(IdOmega,(char*)"Omega");
	SetName(IdEffArea,(char*)"EffArea"); SetName(IdN,(char*)"NTracksM");
	SetName(IdFluxM,(char*)"FluxM"); SetName(IdFluxMErr,(char*)"FluxMErr"); SetName(IdDlM,(char*)"DlM"); SetName(IdDlMErr,(char*)"DlMErr"); SetName(IdRlM,(char*)"RlM");
	SetName(IdRlC,(char*)"RlC"); SetName(IdDlC,(char*)"DlC"); SetName(IdFluxC,(char*)"FluxC"); SetName(IdNC,(char*)"NTracksC");
	SetName(IdSurface,(char*)"Surface"); SetName(IdTunnels,(char*)"Tunnels");
	SetName(IdTemp,(char*)"Temp"); SetName(IdTemp2,(char*)"Temp2");
	// Spec extra
	DensityHist = new HistogramClass(0,5,0.050,(char*)"DensityHist");
	DlDiffHist = new HistogramClass(-10,10,0.100,(char*)"DlDiffHist");
	}

MuogramClass::~MuogramClass()
	{
	delete DensityHist;
	delete DlDiffHist;
	}

// Conversions

double MuogramClass::RadiusToZenith(double R)
	{ if (R>=0 && R<90) return R; else return -1;}

double MuogramClass::ZenithToRadius(double Z)
	{ if (Z>=0 && Z<=90) return Z; else return -1; }

double MuogramClass::OmegaPerBs2(double X, double Y)
	{ double R=sqrt(X*X+Y*Y); if (R==0) return 1./rd/rd; if (RadiusToZenith(R)>=0) return (sin(R/rd)/R/rd); else return 0;}

double MuogramClass::XyToZenith(double X, double Y)
	{ return RadiusToZenith(sqrt(X*X+Y*Y)); }
	
double MuogramClass::XyToAzimuth(double X, double Y)
	{ return 180-atan2(X,-Y)*rd; }
	
double MuogramClass::ZaToX(double Z, double A)
	{ double R=ZenithToRadius(Z); return R*sin(A/rd); }

double MuogramClass::ZaToY(double Z, double A)
	{ double R=ZenithToRadius(Z); return R*cos(A/rd); }

// Bin functions

void MuogramClass::AddZA(int InNMap, double Z, double A, double InWeight)
	{ if (Z>-0.5 && Z<90) MultiMapClass::Add(InNMap,ZaToX(Z,A),ZaToY(Z,A),InWeight); }
void MuogramClass::SetZA(int InNMap, double Z, double A, double InValue)
	{ if (Z>-0.5 && Z<90) MultiMapClass::Set(InNMap,ZaToX(Z,A),ZaToY(Z,A),InValue); }
double MuogramClass::GetZA(int InNMap, double Z, double A)
	{ return MultiMapClass::Z(InNMap,ZaToX(Z,A),ZaToY(Z,A)); }
double MuogramClass::GetZAW(int InNMap, double Z, double A)
	{ return MultiMapClass::ZWeighted(InNMap,ZaToX(Z,A),ZaToY(Z,A)); }


// Specific operations

bool MuogramClass::ReadTrackList(char* InTrackListFileName, double InRotX, double InRotY, double InRotZ)
	{
	NRuns=1;
	RotX=InRotX; RotY=InRotY; RotZ=InRotZ;
	double BackRotXRad=-RotX/rd,BackRotYRad=-RotY/rd,BackRotZRad=RotZ/rd; // BackRot in Z is wihout a neative sign, az the Geolocogal azimuth increases clock-wise, the opposite as the mathematical one
	if (Verbose>0) printf("Reading TrackListFile : %s (Rot:%g,%g,%g)\n",InTrackListFileName,RotX,RotY,RotZ);
	if (Verbose>1) printf("\t");
	GetBaseName(InTrackListFileName,TrackListFileNameBase);
	FILE *TrackListFile; int ScanfNConv;
	double		DetX,DetY,DetSx,DetSy,EffA,DTMuS,TSec=0, Z,A;
	long int	EventId, EventIdPrev=-1, Wts=100000;
	VectorClass	*TrackV = new VectorClass();
	TrackListFile = fopen(InTrackListFileName,"r");
	if (TrackListFile==NULL) { printf("Cannot open TrackList file : %s\n",InTrackListFileName); return false; }
	SkipLine(TrackListFile,2);
	while (!feof(TrackListFile))// && (NTracks<1000))
		{
		ScanfNConv=fscanf(TrackListFile,"%ld ",&EventId);
		if (ScanfNConv!=1) { SkipLine(TrackListFile); continue;}
		fscanf(TrackListFile,"%lf %*s %*s %lf %lf %lf %lf %*f %*f %lf ",&DTMuS,&DetSx,&DetSy,&DetX,&DetY,&EffA);
		NTracks++;
		if (NTracks%Wts==0 && Verbose>1) { printf("."); fflush(stdout); }
		if (EventId<EventIdPrev) { printf("Suspicious EventId: %ld (previous:%ld)\n",EventId,EventIdPrev); }; EventIdPrev=EventId;
		TSec+=DTMuS/1.e6;
		TrackV->SetXyz(DetSx,DetSy,1);
		if (EffA<1e-3) EffA=1e15;
		TrackV->RotY(BackRotYRad); TrackV->RotX(BackRotXRad); TrackV->RotZ(BackRotZRad);
		Z=TrackV->GetTheta()*rd; A=90-TrackV->GetPhi()*rd; // Geological azimuth is calculated from Y axis and clockwise
		if (Z>90) {Z=180-Z; A+=180; if (A>360) A-=360;}
		AddZA(IdN,Z,A,1);
		AddZA(IdFluxM,Z,A,1./EffA);
		SetZA(IdEffArea,Z,A,EffA);
		//printf("EvnetId: %ld   dT: %g\nDetXY: %g %g\nThetaPhi: %g %g\nMapXY: %g %g\n",EventId,DTMuS,DetSx,DetSy,TrackV->GetTheta(),TrackV->GetPhi(),ZaToX(TrackV->GetTheta(),TrackV->GetPhi()),ZaToY(TrackV->GetTheta(),TrackV->GetPhi()));
		}
	ModifyMapWith(IdFluxM,IdOmega,FunctionDivide);
	MultiplyMap(IdFluxM,1.e6/TSec);
	CopyFromTo(IdFluxM,IdFluxMErr);
	ModifyMapWith(IdFluxMErr,IdN,FunctionDivideWithSquareRoot);
	MeasTime=TSec;
	if (Verbose>1) printf("\t %d Tracks in %d Hours\n",NTracks,int(TSec/3600));
	return true;
	}

bool MuogramClass::SetFluxDlFileName(char* FluxDlFileName)
	{
	FluxDlConv = new FluxDlConversionClass();
	GetBaseName(FluxDlFileName,ZfdFileNameBase);
	return FluxDlConv->ReadFromFileName(FluxDlFileName);
	}

bool MuogramClass::FluxToDl(char* FluxDlFileName)
	{
	bool Ret=true;
	if (FluxDlFileName==NULL && FluxDlConv==NULL) return false;
	if (FluxDlFileName!=NULL) Ret=SetFluxDlFileName(FluxDlFileName);
	if (!Ret) return false;
	if (Verbose>0) printf("Converting: FluxM -> DlM , via %s\n",FluxDlFileName);
	double Zenith,Flux,FluxErr;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (IsValidAt(NXBin,NYBin))
		{
		Zenith=ZAt(IdZenith,NXBin,NYBin);
		Flux=ZAt(IdFluxM,NXBin,NYBin);
		FluxErr=ZAt(IdFluxMErr,NXBin,NYBin);
		SetAt(IdDlM,NXBin,NYBin,FluxDlConv->GetDlFromZF(Zenith,Flux));
		// TODO : kitalalni, h relativ v abszolut error kell, Flux,Dl ?
		SetAt(IdDlMErr,NXBin,NYBin,FluxDlConv->GetDlFromZF(Zenith,Flux)*abs(FluxDlConv->GetDlRelErrFromZF(Zenith,Flux))*FluxErr/Flux);
		}
	return true;
	}

bool MuogramClass::RlToDl(double InDensity)
	{
	if (InDensity>0) Density=InDensity;
	if (Verbose>1) printf("Converting: RlC -> DlC , with Density: %g\n",Density);
	CopyFromTo(IdRlC,IdDlC);
	MultiplyMap(IdDlC,Density);
	return true;
	}

bool MuogramClass::DlToRl(double InDensity)
	{
	if (InDensity>0) Density=InDensity;
	if (Verbose>1) printf("Converting: DlM -> RlM , with Density: %g\n",Density);
	CopyFromTo(IdDlM,IdRlM);
	MultiplyMap(IdRlM,Density);
	return true;
	}


bool MuogramClass::DlToFlux(char* FluxFromZenithDlFileName)
	{
	bool Ret=true;
	MultiMapClass * FluxFromZenithDl = new MultiMapClass(1);
	GetBaseName(FluxFromZenithDlFileName,ZdfFileNameBase);
	Ret=FluxFromZenithDl->ReadFromFileName(FluxFromZenithDlFileName);
	if (!Ret) return false;
	if (Verbose>1) printf("Converting: DlC -> FluxC , via %s\n",FluxFromZenithDlFileName);
	double Zenith,Dl;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (IsValidAt(NXBin,NYBin))
		{
		Zenith=ZAt(IdZenith,NXBin,NYBin);
		Dl=ZAt(IdDlC,NXBin,NYBin);
		SetAt(IdFluxC,NXBin,NYBin,FluxFromZenithDl->ZWeighted(0,Zenith,Dl));
		}
	return true;
	}

double MuogramClass::EstimateDensity()
	{
	DensityHist->Clear();
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) 
		if (IsValidAt(NXBin,NYBin) && ZAt(IdN,NXBin,NYBin)>1 && ZAt(IdRlC,NXBin,NYBin)>1)
			DensityHist->Add(ZAt(IdDlM,NXBin,NYBin)/ZAt(IdRlC,NXBin,NYBin),sqrt(ZAt(IdN,NXBin,NYBin)));
	Density=DensityHist->Median();
	if (Verbose>0) printf("Estimated Density : %g\n",Density);
	return Density;
	}

bool MuogramClass::CompareDlMC()
	{
	if (Verbose>1) printf("Comparing DlM <-> DlC \n");
	DlDiffHist->Clear();
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) 
		if (IsValidAt(NXBin,NYBin) && ZAt(IdN,NXBin,NYBin)>10 && ZAt(IdRlC,NXBin,NYBin)>1)
			DlDiffHist->Add((ZAt(IdDlM,NXBin,NYBin)-ZAt(IdDlC,NXBin,NYBin))/ZAt(IdDlMErr,NXBin,NYBin)); // ,ZAt(IdN,NXBin,NYBin)
	//DlDiffHist->WriteParametersToScreen();
	// somethign better is needed :)
	if (DlDiffHist->Sigma()>3) return false; else return true;
	}


bool MuogramClass::IsValidAt(int NX, int NY)
	{
	return (ZAt(IdZenith,NX,NY) > -0.5);
	}

bool MuogramClass::IsUsableAt(int NX, int NY)
	{
	return (ZAt(IdEffArea,NX,NY) > 1e-10);
	}


void MuogramClass::SetPosition(double InPosN, double InPosE, double InPosH)
	{ PosN=InPosN; PosE=InPosE; PosH=InPosH; }

bool MuogramClass::ReadDemFileName(char* DemFileName)
	{
	if (Verbose>0) printf("Reading DemFile : %s (PosEnh:%g,%g,%g)\n",DemFileName,PosE,PosN,PosH);
	if (Verbose>1) printf("\t");
	if (Integral(IdRlC)>1) printf("ReadDemFile: %s : Warning RockLengthCalc was not empty\n",DemFileName); 
	// for minden DemItem : Add RlC:Rl, add Temp:1; Div. Fill holes from neighb.
	FILE * DemFile = fopen(DemFileName,"r");
	if (DemFile==NULL) { printf("Cannot open DemFile : %s\n",DemFileName); return false; }
	GetBaseName(DemFileName,DemFileNameBase);
	double DemN,DemE,DemH, Zenith,Azimuth,Distance;
	int NLineDemFile=0, Wts=10000;
	SkipLine(DemFile);
	while (!feof(DemFile))
		{ // XYZ -> East,Noth,Height
		fscanf(DemFile,"%lf %lf %lf ",&DemE,&DemN,&DemH); // from 2024-Jul
		NLineDemFile++;
		if ( (NLineDemFile%Wts==0) && (Verbose>1) ) { printf("."); fflush(stdout); }
		// TEST 
		Azimuth=atan2(DemE-PosE,DemN-PosN)*rd;
		//Azimuth=atan2(DemN-PosN,DemE-PosE)*rd; // de miert ez a jo???
		Zenith=atan(sqrt((DemN-PosN)*(DemN-PosN)+(DemE-PosE)*(DemE-PosE))/(DemH-PosH))*rd;
		//printf("Dem: %g %g %g, Pos: %g %g %g, ZA: %g %g, D: %g\n",DemN,DemE,DemH,PosN,PosE,PosH,Zenith,Azimuth,Distance);		
		AddZA(IdRlC,Zenith,Azimuth,DemH);
		AddZA(IdTemp,Zenith,Azimuth,1);
		}
	if (Verbose>1) printf("  %d points\n",NLineDemFile);
	// Get average DemZ map
	DivideMapWithCutElse(IdRlC,IdTemp,0.1,-1);
	// Fill gaps by neighbour average 
	for (int NIter=0; NIter<4; NIter++)
		FillGapsRangeCutTemp(IdRlC,10,0.1,IdTemp);
	// Calculate RockLength from the averaged DemH map
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (IsValidAt(NXBin,NYBin))
		{
		// TODO :: EZZEL kezdeni valamit!
		if (ZAt(IdRlC,NXBin,NYBin)!=-1) Distance=(ZAt(IdRlC,NXBin,NYBin)-PosH)/cos(ZAt(IdZenith,NXBin,NYBin)/rd);
		else Distance=-1;
		SetAt(IdRlC,NXBin,NYBin,Distance);
		}
	CopyFromTo(IdRlC,IdSurface);
	return true;
	}

bool MuogramClass::ReadDemFileName(char* DemFileName, double InPosN, double InPosE, double InPosH)
	{
	SetPosition(InPosN,InPosE,InPosH);
	return ReadDemFileName(DemFileName);
	}

bool MuogramClass::ReadStlFileName(char* StlFileName)
	{
	char	Command[1000],TempFileName[500]="Temp/TriangleCount.temp";
	TriangleClass	*Triangles; // Triangle array
	int		MaxNTriangles;
	RayClass	Ray;
	FILE	*TempFile, *StlFile;
	// Load Stl to Triangles array
	printf("Read STL file %s\n",StlFileName);
	GetBaseName(StlFileName,TunnelsFileNameBase);
	sprintf(Command,"grep -c normal %s > %s",StlFileName,TempFileName); system(Command);
	TempFile = fopen(TempFileName,"r");
	fscanf(TempFile,"%d",&MaxNTriangles); fclose(TempFile);
	printf("\tN.Triangles: %d\n",MaxNTriangles);
	Triangles = (TriangleClass*) calloc(MaxNTriangles,sizeof(TriangleClass));
	StlFile = fopen(StlFileName,"r");
	SkipLine(StlFile);
	for (int NT=0; NT<MaxNTriangles; NT++)
		Triangles[NT].ReadFromStl(StlFile);
	fclose(StlFile);
	// Loop on Rays
	printf("\t");
	enum {ParityEven=0,ParityOdd=1};
	int ParityCount[2], ParityOk, ParityToCorrect; ParityCount[ParityEven]=0; ParityCount[ParityOdd]=0;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (IsValidAt(NXBin,NYBin))
		{
		if (NYBin==MaxNYBins/2) { printf("."); fflush(stdout); }
		Ray.Clear();
		Ray.Base.SetXyz(PosE,PosN,PosH);
		//Ray.Dir.SetZenithAzimuth(ZAt(IdZenith,NXBin,NYBin),ZAt(IdAzimuth,NXBin,NYBin));
		Ray.Dir.SetZenithAzimuth(ZAt(IdZenith,NXBin,NYBin),ZAt(IdAzimuth,NXBin,NYBin));
		Ray.IntersectTriangleArray(Triangles,MaxNTriangles);
		Ray.GetPath();
		SetAt(IdTunnels,NXBin,NYBin,Ray.Path);
		SetAt(IdTemp,NXBin,NYBin,Ray.NIntersections);
		ParityCount[Ray.NIntersections%2]++;
		}
	printf("\n");
	// TODO : check parity
	printf("IntersectionParity: Even: %d , Odd: %d\n",ParityCount[ParityEven],ParityCount[ParityOdd]);
	if (ParityCount[ParityEven]!=0 && ParityCount[ParityOdd]!=0) 
		{
		double CorrectionRatio;
		if (ParityCount[ParityEven]>ParityCount[ParityOdd]) ParityOk=ParityEven; else ParityOk=ParityOdd;
		ParityToCorrect=(ParityOk+1)%2;
		CorrectionRatio=double(ParityCount[ParityToCorrect])/double(ParityCount[ParityEven]+ParityCount[ParityOdd]);
		printf("CorrectionRatio: %4.1lf%%\n",CorrectionRatio*100.);
		for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (IsValidAt(NXBin,NYBin))
			if ( (int(ZAt(IdTemp,NXBin,NYBin)+0.1)%2) == ParityToCorrect ) SetAt(IdTunnels,NXBin,NYBin,-2);
		FillGapsRangeCutTemp(IdTunnels,4,-1,IdTemp2);
		}
	// Subtract TunnelLength from RockLengh, if later exists
	if (Integral(IdRlC)>0.1) ModifyMapWith(IdRlC,IdTunnels,FunctionSubtract);
	else CopyFromTo(IdTunnels,IdRlC);
	return true;
	}

bool MuogramClass::ReadStlFileName(char* StlFileName, double InPosN, double InPosE, double InPosH)
	{
	SetPosition(InPosN,InPosE,InPosH);
	return ReadStlFileName(StlFileName);
	}


// Muogram operations

bool MuogramClass::AddMuogram(MuogramClass* M)
	{
	if (! SameBinningAsMultiMap(M))
		{ printf("Cannot add muogram %s to %s",M->TheName,TheName); return false; }
	double F1,dF1,N1,w1,F2,dF2,N2,A2,w2,t1=MeasTime,t2=M->MeasTime,F;
	if (t1<1e-10 || t2<1e-10) {t1=1; t2=1;}
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) if (IsValidAt(NXBin,NYBin))
		{
		if (IsUsableAt(NXBin,NYBin) && M->IsUsableAt(NXBin,NYBin))
			{
			F1=ZAt(IdFluxM,NXBin,NYBin); dF1=ZAt(IdFluxMErr,NXBin,NYBin); N1=ZAt(IdN,NXBin,NYBin);
			F2=M->ZAt(IdFluxM,NXBin,NYBin); dF2=M->ZAt(IdFluxMErr,NXBin,NYBin); N2=M->ZAt(IdN,NXBin,NYBin);
			if (dF1>0 && dF2>0) { w1=1.0/dF1/dF1; w2=1.0/dF2/dF2; } else {w1=1;w2=1;}
			F=(w1*F1+w2*F2)/(w1+w2);
			SetAt(IdFluxM,NXBin,NYBin,F);
			SetAt(IdFluxMErr,NXBin,NYBin,sqrt((w1*dF1*dF1+w2*dF2*dF2)/(w1+w2)));
			SetAt(IdN,NXBin,NYBin,N1+N2);
			SetAt(IdEffArea,NXBin,NYBin,(N1+N2)/F/(t1+t2));
			}
		if ((!(IsUsableAt(NXBin,NYBin))) && (M->IsUsableAt(NXBin,NYBin)) )
			{
			F2=M->ZAt(IdFluxM,NXBin,NYBin); dF2=M->ZAt(IdFluxMErr,NXBin,NYBin);
			N2=M->ZAt(IdN,NXBin,NYBin); A2=M->ZAt(IdEffArea,NXBin,NYBin);
			SetAt(IdFluxM,NXBin,NYBin,F2);
			SetAt(IdFluxMErr,NXBin,NYBin,dF2);
			SetAt(IdN,NXBin,NYBin,N2);
			SetAt(IdEffArea,NXBin,NYBin,A2);
			}
		//if (IsValidAt(NXBin,NYBin) && !(M->IsValidAt(NXBin,NYBin)) )		// do nothing
		//if (!(IsValidAt(NXBin,NYBin)) && !(M->IsValidAt(NXBin,NYBin)) )	// do nothing
		}	
	MeasTime=t1+t2;
	NRuns++;
	return true;
	}

// General info

void MuogramClass::ShowInfo()
	{
	printf("Muogram info\n");
	printf("\t Measured: \n\t Time: %s , NTracks: %d\n",NiceDurationSec(MeasTime),NTracks);
	printf("\t Flux: Zenith: %g \n",Z(IdFluxM,0,0));
	double EffAPeakX,EffAPeakY,EffAPeakValue;
	EffAPeakValue=Peak(IdEffArea,&EffAPeakX,&EffAPeakY);
	printf("\t Mid at Z:%.1lf A:%.0lf is Flux:%g\n",XyToZenith(EffAPeakX,EffAPeakY),XyToAzimuth(EffAPeakX,EffAPeakY),ZWeighted(IdFluxM,EffAPeakX,EffAPeakY));
	CopyFromTo(IdFluxM,IdTemp);
	ModifyMapWith(IdTemp,IdOmega,FunctionPositiveThan);
	printf("\t Area[deg2]: %g (L:%4.1lf cm)\n",Integral(IdTemp)*rd*rd,sqrt(EffAPeakValue)/10);
	Clear(IdTemp);
	printf("\t Topography:\n");
	CopyFromTo(IdRlC,IdTemp);
	ModifyMapWith(IdTemp,IdOmega,FunctionPositiveThan);
	printf("\t Area[deg2]: %g , depth: %g\n",Integral(IdTemp)*rd*rd,Z(IdRlC,0,0));
	Clear(IdTemp);
	printf("\t Combination:\n");
	printf("\t Estimated Density: med:%g, peaksigma:%g\n",DensityHist->Median(),DensityHist->PeakSigma());
	printf("\t "); DensityHist->WriteParametersToScreen();
	printf("\t "); DlDiffHist->WriteParametersToScreen();
	//printf("\t Outlier points : %g %%\n",double(n));
	}

// File operations
bool MuogramClass::WriteToFile(FILE* OutFile)
	{
	// Write Muogram specific parameters
	fprintf(OutFile,"#Muogram\n");
	fprintf(OutFile,"BinSizeDeg %g\n",BinSize);
	fprintf(OutFile,"NRuns %d\n",NRuns);
	fprintf(OutFile,"TrackListFile %s\n",TrackListFileNameBase);
	fprintf(OutFile,"NTracks %d\n",NTracks);
	fprintf(OutFile,"MeasTimeNice %s\n",NiceDurationSec(MeasTime));
	fprintf(OutFile,"MeasTimeSec %g\n",MeasTime);
	fprintf(OutFile,"DemFile %s\n",DemFileNameBase);
	fprintf(OutFile,"TunnelsFile %s\n",TunnelsFileNameBase);
	fprintf(OutFile,"PosEnh %g %g %g\n",PosE,PosN,PosH);
	fprintf(OutFile,"ZfdFile %s\n",ZfdFileNameBase);
	fprintf(OutFile,"Density %g\n",Density);
	fprintf(OutFile,"Verbose %d\n",Verbose);
	fprintf(OutFile,"End\n\n\n");
	// Write the multimap
	MultiMapClass::WriteToFile(OutFile);
	DensityHist->WriteToFile(OutFile);
	DlDiffHist->WriteToFile(OutFile);
	return true;
	}

bool MuogramClass::ReadFromFile(FILE* InFile)
	{
	// Read Muogram specific parameters
	bool	DoRead=true;
	char	Parameter[50];
	// Check definition line
	fscanf(InFile,"%s ",Parameter);
	if (strcmp(Parameter,"#Muogram")!=0) printf("WARNING: Suspicious Muogram defline: %s\n",Parameter);
	while (DoRead)
		{
		fscanf(InFile,"%s",Parameter);
		if (feof(InFile)) DoRead=false;
		if      (strcmp(Parameter,"#NRuns")==0)		fscanf(InFile,"%d",&NRuns);
		else if (strcmp(Parameter,"#MeasTime")==0)	fscanf(InFile,"%lf",&MeasTime);
		else if (strcmp(Parameter,"#PosEnh")==0)	fscanf(InFile,"%lf %lf %lf",&PosE,&PosN,&PosH);
		else if (strcmp(Parameter,"#Verbose")==0)	fscanf(InFile,"%d",&Verbose);
		else if (strcmp(Parameter,"#End")==0)	DoRead=false;
		else { printf("\nWARNING : Incorrect parameter: %s\n\n",Parameter); return false; }
		}
	// Read the multimap
	return MultiMapClass::ReadFromFile(InFile);
	}


bool MuogramClass::WriteToFileName(char* OutFileName)
	{
	FILE * OutFile;
	if ( (OutFile=fopen(OutFileName,"w")) == NULL ) 
		{printf("\nERROR : file: %s could not be opened\n\n",OutFileName); return false;}
	WriteToFile(OutFile);
	fclose(OutFile);
	return true;
	}

bool MuogramClass::ReadFromFileName(char* InFileName)
	{
	FILE * InFile; bool Ret;
	if ( (InFile=fopen(InFileName,"r")) == NULL ) 
		{printf("\nERROR : file: %s could not be opened\n\n",InFileName); return false;}
	Ret=ReadFromFile(InFile);
	fclose(InFile);
	return Ret;
	}



////////////////////////////////////////////////////////////////////////
// Flux Computation using Tracks data - implementation
//      version : 2022-02-24
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "FluxComputation.h"

int main(int ArgC, char** ArgV)
{
printf("Start : Flux Computation\n");

ReadArguments(ArgC,ArgV);
SetMode();
InitializeClasses();

OpenListFile();
SetFluxFileName();

while (GetNextFileInfo())
	{
		
// TEMP : To have GlobalCoord rot
for (int x=0;x<FineMap->MaxNXBins;x++) for (int y=0;y<FineMap->MaxNYBins;y++)
	{
	TrackDirection->SetXyz(FineMap->XAt(x),FineMap->YAt(y),1.0);
	for (int NRot=0; NRot<NRotations; NRot++)
		TrackDirection->RotAxis(RotAxis[NRot],DegToRad(-RotAngle[NRot]));
	FineMap->SetAt(IdDetZenith,x,y,RadToDeg(TrackDirection->GetTheta()));
	FineMap->SetAt(IdDetAzimuth,x,y,RadToDeg(PI/2.-TrackDirection->GetPhi()));
	}

		
	SetTracksFileNames();
	ReadTracksInfoFile();
	ReadAnalysisInfo();
	OpenTracksFile();
	while (!feof(TracksFile) && (NTracks!=MaxNTracks))
		{
		ReadTrack();
		TransformToMap();
		FineMap->Add(IdCounts,MapX,MapY);
		FineMap->Add(IdFluxA,MapX,MapY,1.0/EffArea/Scale);
		WriteInfoToScreen();
		}
	if (Verbose>1) printf("\nRead NTracks : %ld\n",NTracks);
	}
printf("\n");
FineMap->MultiplyMap(IdFluxA,1.0/NFiles);
ComputeFineMapFlux();
MakeGrainedMap();
WriteResultsToFile();
PlotFlux();
CloseProgram();
}

////////////////////////////////////////////////////////////////////////

void ReadArguments(int ArgC, char** ArgV)
	{
	if (ArgC<2) Help();
	for (int i=1; i<ArgC; i++)
		if (strcmp(ArgV[i],"-h")==0 || strcmp(ArgV[i],"--help")==0 )
			{ Help(); }
		else if (strcmp(ArgV[i],"-t")==0 || strcmp(ArgV[i],"--tracks-file")==0 )
			{ i++; sprintf(TracksFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-rot")==0 || strcmp(ArgV[i],"--rotation")==0 )
			{ i++; sprintf(ArgRotation,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-l")==0 || strcmp(ArgV[i],"--list-file")==0 )
			{ i++; sprintf(ListFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-f")==0 || strcmp(ArgV[i],"--flux-file")==0 )
			{ i++; sprintf(FluxFileNaming,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-r")==0 || strcmp(ArgV[i],"--results-file")==0 )
			{ i++; sprintf(ResultsFileName,"%s",ArgV[i]); UseResultsFile=true; }
		else if (strcmp(ArgV[i],"-s")==0 || strcmp(ArgV[i],"--simple-flux")==0 )
			{ i++; DoSimpleFlux=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-g")==0 || strcmp(ArgV[i],"--grained-flux")==0 )
			{ i++; DoGrainedFlux=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-rd")==0 || strcmp(ArgV[i],"--dir-results")==0 )
			{ i++; sprintf(ResultsDirName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-p")==0 || strcmp(ArgV[i],"--plot")==0 )
			{ i++; DoPlotFlux=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-mt")==0 || strcmp(ArgV[i],"--max-tracks")==0 )
			{ i++; MaxNTracks=long(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-gl")==0 || strcmp(ArgV[i],"--grain-count-limit")==0 )
			{ i++; GrainCountLimit=GetPostfixedNumber(ArgV[i]); }
		else if (strcmp(ArgV[i],"-bs")==0 || strcmp(ArgV[i],"--bin-size")==0 )
			{ i++; MapBs=GetPostfixedNumber(ArgV[i]); }
		else if (strcmp(ArgV[i],"-mm")==0 || strcmp(ArgV[i],"--map-max")==0 )
			{ i++; MapMax=GetPostfixedNumber(ArgV[i]); }
		else if (strcmp(ArgV[i],"-scl")==0 || strcmp(ArgV[i],"--smooth-count-limit")==0 )
			{ i++; SmoothCountLimit=long(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-ssm")==0 || strcmp(ArgV[i],"--smooth-size-max")==0 )
			{ i++; SmoothSizeMax=long(GetPostfixedNumber(ArgV[i])); }
		else if (strcmp(ArgV[i],"-o")==0 || strcmp(ArgV[i],"--orientation")==0 )
			{ i++; if ( (strcmp(ArgV[i],"mts8")==0) || (strcmp(ArgV[i],"Mts8")==0) ) { UseSpecialOrientation=true; Orientation=OriMts8; sprintf(OrientaionName,"Mts8"); }
				else { printf("Incorrect orientation name : %s",ArgV[i]); Help(); } }
		else if (strcmp(ArgV[i],"-v")==0 || strcmp(ArgV[i],"--verbose")==0 )
			{ i++; Verbose=GetPostfixedNumber(ArgV[i]); }
		else if (strcmp(ArgV[i],"-m")==0 || strcmp(ArgV[i],"--mode")==0 )
			{ i++; sprintf(CoordinateSystemModeString,"%s",ArgV[i]); }
		// and some non-official switches
		else if (strcmp(ArgV[i],"-id")==0 || strcmp(ArgV[i],"--id")==0 )
			{ i++; }
		else
			{ printf("\nBad argument : %s\n\n",ArgV[i]); Help(); };
	if (Verbose>1)
		{
		printf("FluxComputation : used settings :\n");
		printf("\t Orientation : "); if (UseSpecialOrientation) printf("Special: %s\n",OrientaionName); else printf(" Normal\n");
		printf("\t TracksFile : %s\n",TracksFileName);
		printf("\t Rotation : %s\n",ArgRotation);
		printf("\t ListFile : %s\n",ListFileName);
		printf("\t ResultsFile : %s\n",ResultsFileName);
		printf("\t FluxFileNaming : %s\n",FluxFileNaming);
		//printf("\t Output SimpleFlux : "); if (DoSimpleFlux) printf("Yes\n"); else printf("No\n");
		}
	}

void SetMode()
	{
	if ( (strcmp(CoordinateSystemModeString,"Slope")==0) || (strcmp(CoordinateSystemModeString,"slope")==0) || (strcmp(CoordinateSystemModeString,"s")==0) )
		CoordinateSystemMode=ModeSlope;
	else if ( (strcmp(CoordinateSystemModeString,"Polar")==0) || (strcmp(CoordinateSystemModeString,"polar")==0) || (strcmp(CoordinateSystemModeString,"p")==0) )
		CoordinateSystemMode=ModePolar;
	else { printf("Unrecognized mode : %s\n\n",CoordinateSystemModeString); Help();}
	}

void InitializeClasses()
	{
	TrackDirection = new VectorClass();
	DetectorAngleDep = new MultiMapClass(1);
	
	if (CoordinateSystemMode==ModeSlope)
		FineMap = new MultiMapClass(NMaps,-MapMax,MapMax,MapBs,-MapMax,MapMax,MapBs,(char*)"FineMap");
	if (CoordinateSystemMode==ModePolar)
		FineMap = new MultiMapClass(NMaps,0+MapBs/2,MapMax,MapBs,0+MapBs/2,360-MapBs/2,MapBs,(char*)"FineMap");
	FineMap->SetName(IdSize,(char*)"Size");
	FineMap->SetName(IdCounts,(char*)"Counts");
	FineMap->SetName(IdAngleOfView,(char*)"AngleOfView");
	FineMap->SetName(IdFluxA,(char*)"FluxA");
	FineMap->SetName(IdCos2,(char*)"Cos2");
	FineMap->SetName(IdFlux,(char*)"Flux");
	FineMap->SetName(IdFluxGrained,(char*)"FluxGrained");
	FineMap->SetName(IdFluxSmooth,(char*)"FluxSmooth");
	FineMap->SetName(IdSmoothSize,(char*)"SmoothSize");
	FineMap->SetName(IdDetZenith, (char*)"DetZenith");
	FineMap->SetName(IdDetAzimuth,(char*)"DetAzimuth");
	FineMap->SetName(IdGeoZenith, (char*)"GeoZenith");
	FineMap->SetName(IdGeoAzimuth,(char*)"GeoAzimuth");
	FineMap->Fill(IdSize,1);
	FineMap->FillFunction(IdAngleOfView,AngleOfView);
	FineMap->FillFunction(IdCos2,FunctionCos2);
	SimpleFluxMap = new MultiMapClass(3,FineMap,(char*)"SimpleFluxMap");
	FluxGm = new GrainedMapsClass();
	if (UseResultsFile) GetResultsFileInfo();
	FillZenithAzimuthMap();
	}

void FillZenithAzimuthMap()
	{
	FineMap->FillFunction(IdDetZenith,FunctionZenith);
	FineMap->FillFunction(IdDetAzimuth,FunctionAzimuth);
	}

void GetResultsFileInfo()
	{
	FILE * ResFile = fopen(ResultsFileName,"r");
	if (ResFile==NULL) { printf("Cannot open ResultsFile : %s\nExit\n\n",ResultsFileName); CloseProgram(1); }
	char	ReadString[50];
	while ( (strcmp(ReadString,"!Detector")!=0) && (!feof(ResFile)) )
		{ fscanf(ResFile,"%s",ReadString); SkipBlockInFile(ResFile); }
	if (feof(ResFile)) { printf("Cannot find Detector info in file : %s\n",ResultsFileName); return; };
	// Let's assume the Detector:AngleDep is the first... shall be upgraded later
	// TODO : itt tartok!
	DetectorAngleDep->ReadFromFile(ResFile);
	fclose(ResFile);
	}

void OpenListFile()
	{
	if ( (strlen(ListFileName)<1) && (strlen(TracksFileName)<1) ) 
		{ printf("TrackFile or ListFile is required\nExit\n"); Help(1); }
	// Single file processing
	if (strlen(ListFileName)<1)
		{
		char	TempString[200];
		sprintf(ListFileName,"%s/Temp.fl",TempDir);
		if (sscanf(TracksFileName,"%s %s",TempString,TempString)==2) sprintf(Command,"echo \"%s\" > %s",TracksFileName,ListFileName);
		else sprintf(Command,"echo \"%s %s\" > %s",TracksFileName,ArgRotation,ListFileName);
		system(Command);
		}
	ListFile=fopen(ListFileName,"r");
	if (ListFile==NULL) { printf("List file cannot be opened : %s\nExit\n\n",ListFileName); CloseProgram(1); }
	}

void SetFluxFileName()
	{
	std::string HelpString="";
	int  NameBeg,NameEnd;
	if (strlen(FluxFileNaming)<1)
		{
		if (strlen(ListFileName)>0) HelpString=ListFileName;
		if (strlen(TracksFileName)>0) HelpString=TracksFileName;
		NameBeg=HelpString.find_last_of('/')+1;
		NameEnd=HelpString.find_first_of('.',NameBeg);
		sprintf(FluxFileNaming,"%s",HelpString.substr(NameBeg,NameEnd-NameBeg).data());
		}
	sprintf(FluxFileName,"%s/%s.flux",ResultsDirName,FluxFileNaming);
	sprintf(SimpleFluxFileName,"%s/%s.sflux",ResultsDirName,FluxFileNaming);
	}

bool GetNextFileInfo()
	{
	char	FirstItem[200];
	fscanf(ListFile,"%s",FirstItem);
	if (feof(ListFile)) return false;
	if (FirstItem[0]=='#')
		{
		char Line[202];
		fgets(Line,200,ListFile);
		return GetNextFileInfo();
		}
	if (strcmp(FirstItem,"DIR")==0)
		{
		fscanf(ListFile,"%s",TracksDir);
		return GetNextFileInfo();
		}
	// Read the run info
	sprintf(TracksFileName,"%s%s",TracksDir,FirstItem);
	// Read rotation info
	if (fscanf(ListFile,"%d",&NRotations)!=1) NRotations=0;
	for (int NRot=0; NRot<NRotations; NRot++)
		fscanf(ListFile,"%s %lf",&(RotAxis[NRot]),&(RotAngle[NRot]));
	NFiles++;
	return true;
	}

void SetTracksFileNames()
	{
	// Check names for the TracksFile and TracksInfoFile
	RefineFileName(TracksFileName,ResultsDirName,(char*)".tl");
	sprintf(TracksInfoFileName,"%s",TracksFileName);
	TracksInfoFileName[strlen(TracksInfoFileName)-1]='i'; // *.tl -> *.ti
	if (strlen(ResultsFileName)>2) return;
	sprintf(ResultsFileName,"%s",TracksFileName);
	sprintf(&(ResultsFileName[strlen(ResultsFileName)-2]),"res");
	}

void ReadTracksInfoFile()
	{
	TracksInfoFile=fopen(TracksInfoFileName,"r");
	if (TracksInfoFile==NULL)
		{
		printf("Warning: Cannot open TracksInfoFile : %s\n",TracksInfoFileName);
		return;
		}
	// Read the relevant info
	char Line[202], ParameterName[50];
	double	ParameterValue;
	while (!feof(TracksInfoFile))
		{
		fgets(Line,200,TracksInfoFile);
		sscanf(Line,"%s %lf",ParameterName,&ParameterValue);
		if (strcmp(ParameterName,"TimeLiveSec")==0) TotalTimeSec=ParameterValue;
		if (strcmp(ParameterName,"PadSizeMm")==0) PadSizeMm=ParameterValue;
		if (strcmp(ParameterName,"NTracks")==0) NTracksAll=ParameterValue;
		}
	Scale=TotalTimeSec*1e-6; // to have: 1/s/m2
	if (MaxNTracks>0) Scale*=double(MaxNTracks)/double(NTracksAll);
	fclose(TracksInfoFile);
	}

void ReadAnalysisInfo()
	{
	sprintf(Command,"basename %s .res > %s",ResultsFileName,TempFileName); system(Command);
	sprintf(Command,"grep CodeVersion %s >> %s",ResultsFileName,TempFileName); system(Command);
	sprintf(Command,"grep AnalysisRun %s >> %s",ResultsFileName,TempFileName); system(Command);
	FILE * TempFile = fopen(TempFileName,"r");
	sprintf(AnalysisInfoText,"#");
	char	TempStr[50];
	for (int n=0;n<5;n++) { fscanf(TempFile,"%s",TempStr); sprintf(AnalysisInfoText,"%s %s",AnalysisInfoText,TempStr); }
	fclose(TempFile);
	}

void OpenTracksFile()
	{
	if (Verbose>1) printf("\nOpen TracksFile : %s\n",TracksFileName);
	TracksFile=fopen(TracksFileName,"r");
	if (TracksFile==NULL) { printf("Error: Cannot open TracksFile: %s\nExit\n",TracksFileName); CloseProgram(1); }
	// Skip the header the first two lines
	char	Line[501];
	fgets(Line,500,TracksFile); fgets(Line,500,TracksFile);
	EventN=0;
	}

void ReadTrack()
	{
	EventNPrevious=EventN;
	//NConversions=fscanf(TracksFile,"%ld %*d %lf %lf %*f %*f %lf ",&EventN,&SlopeX,&SlopeY,&EffArea);
	//NConversions=fscanf(TracksFile,"%ld %*d %lf %lf %*f %*f %*f %*f %lf ",&EventN,&SlopeX,&SlopeY,&EffArea);
	NConversions=fscanf(TracksFile,"%ld %*d %*s %*d %lf %lf %*f %*f %*f %*f %lf ",&EventN,&SlopeX,&SlopeY,&EffArea);
	if (UseResultsFile) GetEffArea();
	// this basically filters out the non-valid values, if any, shall be rechecked are traced
	//if (EffArea<1e-3) { printf("\n\tSuspicious data: EffArea=%g  at EventN=%ld , this event will be neglected\n",EffArea,EventN); EffArea=1e10; }
	if (EffArea<1e-3) EffArea=1e10;
	// Muograph level orientation, read as a switch
	if (UseSpecialOrientation && Orientation==OriMts8) { double t=SlopeX; SlopeX=-SlopeY; SlopeY=-t; }
	if (NConversions!=4) { printf("Error: Incorrect TrackListFile data\nExit\n"); exit(1); };
	if (EventN<EventNPrevious) printf("Suspicious data : EventN : %ld !< %ld\n",EventNPrevious,EventN);
	NTracks++;
	TrackDirection->SetXyz(SlopeX,SlopeY,1.0);
	}

void GetEffArea()
	{
	EffArea = DetectorAngleDep->ZWeighted(4,SlopeX,SlopeY);
	}

void TransformToMap()
	{
// TEMP : to change Track-by-Track rot into GlobalCoord rot.
//	for (int NRot=0; NRot<NRotations; NRot++)
//		TrackDirection->RotAxis(RotAxis[NRot],-RotAngle[NRot]/180.*PI);
	if (CoordinateSystemMode==ModeSlope)
		{ MapX=TrackDirection->GetMx();	MapY=TrackDirection->GetMy(); }
	if (CoordinateSystemMode==ModePolar)
		{ MapX=RadToDeg(TrackDirection->GetTheta()); MapY=RadToDeg(TrackDirection->GetPhi()); }
	}

double AngleOfView(double SX, double SY)
	{
	if (CoordinateSystemMode==ModeSlope)
		return 1.0/pow(SX*SX+SY*SY+1.0,1.5) * FineMap->XBinSize * FineMap->YBinSize; // 1.5 == cos3
	if (CoordinateSystemMode==ModePolar)
		//return cos(SX)*sin(SX) * FineMap->XBinSize * FineMap->YBinSize;
		return cos(DegToRad(SX))*sin(DegToRad(SX)) * DegToRad(FineMap->XBinSize) * DegToRad(FineMap->YBinSize) ;
	return 0;	
	}

void WriteInfoToScreen()
	{
	if (NTracks%PrevalenceDot==0) printf("."); fflush(stdout);
	if (NTracks%Prevalence==0) printf("  NTracks: %8ld  at EventN : %8ld\n",NTracks,EventN);
	}

void ComputeFineMapFlux()
	{
	if (CoordinateSystemMode==ModeSlope) FineMap->ClearBorders(IdFluxA);
	if (CoordinateSystemMode==ModePolar) FineMap->SetLineX(IdFluxA,MapMax,0);
	FineMap->CopyFromTo(IdFluxA,IdFlux);
	FineMap->ModifyMapWith(IdFlux,IdAngleOfView,FunctionDivide);
	
	double N, SumFluxA, SumAov, SmoothFlux;
	double SmoothSize;
	bool IncreaseSmoothSize;
	// Smooth
	for (int x=1; x<FineMap->MaxNXBins-1; x++) for (int y=1; y<FineMap->MaxNYBins-1; y++)
		{
		N=0;
		SmoothSize=0;
		IncreaseSmoothSize=true;
		while (IncreaseSmoothSize)
			{
			SmoothSize++;
			for (int sx=x-SmoothSize; sx<x+SmoothSize+1; sx++) for (int sy=y-SmoothSize; sy<y+SmoothSize+1; sy++)
				N+=FineMap->ZAt(IdCounts,sx,sy);
			if (N>=SmoothCountLimit) IncreaseSmoothSize=false;
			if (SmoothSize>=SmoothSizeMax) IncreaseSmoothSize=false;
			if (x-SmoothSize==0 || x+SmoothSize==FineMap->MaxNXBins-1 ) IncreaseSmoothSize=false;
			if (y-SmoothSize==0 || y+SmoothSize==FineMap->MaxNYBins-1 ) IncreaseSmoothSize=false;
			}
		SumFluxA=0; SumAov=0;
		for (int sx=x-SmoothSize; sx<x+SmoothSize+1; sx++) for (int sy=y-SmoothSize; sy<y+SmoothSize+1; sy++)
			{ SumFluxA+=FineMap->ZAt(IdFluxA,sx,sy); SumAov+=FineMap->ZAt(IdAngleOfView,sx,sy); }
		if (SumAov>0) SmoothFlux=SumFluxA/SumAov; else SmoothFlux=0;
		FineMap->SetAt(IdSmoothSize,x,y,SmoothSize);
		FineMap->SetAt(IdFluxSmooth,x,y,SmoothFlux);
		}
	}

void MakeGrainedMap()
	{
	// Make Grains
	FluxGm->SetMap(FineMap,IdCounts,GrainCountLimit);
	FluxGm->Compute();
	FluxGm->FillZonedMaps();
	FluxGm->ZMap->CopyFromTo(IdFluxA,IdFlux);
	FluxGm->ZMap->ModifyMapWith(IdFlux,IdAngleOfView,FunctionDivide);
	FluxGm->GrainsToBaseMapFxyo(FunctionDivide,IdFluxA,IdAngleOfView,IdFluxGrained);
	}

void WriteInfoToFile(FILE* OutFile)
	{
	TracksInfoFile=fopen(TracksInfoFileName,"r");
	fprintf(OutFile,"#GeneralInfo\nTrackingInfo");
	// Copy Tracking-TrackList info
	char Line[202];
	while (!feof(TracksInfoFile))
		{
		fgets(Line,200,TracksInfoFile);
		if (strlen(Line)>1) fprintf(OutFile,"%s",Line);
		}
	fclose(TracksInfoFile);
	// Write FluxComputation params
	fprintf(OutFile,"# FluxComputation parameters\n");
	fprintf(OutFile,"ArgRotation\t%s\n",ArgRotation);
	fprintf(OutFile,"Map: MapMax %g MapBinsize %g\n",MapMax,MapBs);
	fprintf(OutFile,"Smooth: SmoothCountLimit %g SmoothSizeMax %d\n",SmoothCountLimit,SmoothSizeMax);
	fprintf(OutFile,"NUsedTracks %ld\n",NTracks);
	fprintf(OutFile,"FluxCodeVersion %s\n",CodeVerion);
	MultiMapClass * AMaps = FineMap;
	fprintf(OutFile,"MapDetails %s Columns\nX Y",AMaps->TheName);
	for (int n=0; n<AMaps->MaxNMaps; n++) fprintf(OutFile," %s",AMaps->Name[n]);
	AMaps=SimpleFluxMap;
	fprintf(OutFile,"\nMapDetails %s Columns\nX Y",AMaps->TheName);
	for (int n=0; n<AMaps->MaxNMaps; n++) fprintf(OutFile," %s",AMaps->Name[n]);
	fprintf(OutFile,"\n\n\n");
	}

void WriteResultsToFile()
	{
	// Write GrainMaps file
	if (DoGrainedFlux)
		{
		FluxFile = fopen(FluxFileName,"w");
		if (FluxFile==NULL)
			{ printf("Output file cannot be opened for write: %s\nExit\n",FluxFileName); return; }
		//FluxGm->WriteToFile(FluxFile);
		FineMap->WriteToFile(FluxFile);
		if (Verbose>1) printf("FluxGm witten to : %s\n",FluxFileName);
		fclose(FluxFile); FluxFile=NULL;
		}
	// Simple flux format for Surda's analysis
	if (DoSimpleFlux)
		{
		SimpleFluxMap->Import(0,FineMap,IdCounts);
		SimpleFluxMap->Import(1,FineMap,IdFluxSmooth);
		SimpleFluxMap->Import(2,FineMap,IdFlux);
		//SimpleFluxMap->Import(1,FineMap,IdFlux);
		//SimpleFluxMap->Import(2,FineMap,IdFlux);
		//SimpleFluxMap->SetName(2,(char*)"FluxUncertainty");
		//SimpleFluxMap->ModifyMapWith(2,0,FunctionDivideWithSquareRoot);
		SimpleFluxFile=fopen(SimpleFluxFileName,"w");
		WriteInfoToFile(SimpleFluxFile);
		SimpleFluxMap->WriteToFile(SimpleFluxFile);
		fclose(SimpleFluxFile);
		if (Verbose>1) printf("SimpleFlux witten to : %s\n",SimpleFluxFileName);
		}
	}

void PlotFlux()
	{
	if (!DoPlotFlux) return;
	usleep(100); // wait for file to be closed... strange 
	if (Verbose>1) printf("Plot flux ...\n");
	sprintf(Command,"gnuplot -e \"fn='%s/%s'\" PlotScripts/PlotFlux.gp",ResultsDirName,FluxFileNaming);
	system(Command);
	//sprintf(Command,"display %s/%s_flux.png",ResultsDirName,FluxFileNaming);
	//system(Command);
	}

double RadToDeg(double XRad)
	{
	double XDeg=XRad*180./M_PI;
	if (XDeg<0) XDeg+=360;
	if (XDeg>360) XDeg-=360.;
	return XDeg;
	}
double DegToRad(double XDeg)
	{
	return XDeg/180.*M_PI;
	}

double FunctionCos2(double SX, double SY)
	{
	if (CoordinateSystemMode==ModeSlope)
		return 1.0/(1.0+SX*SX+SY*SY);
	if (CoordinateSystemMode==ModePolar)
		return pow(cos(DegToRad(SX)),2);
	return 0;
	}

double FunctionZenith(double SX, double SY)
	{
	return RadToDeg(atan2(sqrt(SX*SX+SY*SY),1.0));
	}

double FunctionAzimuth(double SX, double SY)
	{
	//return RadToDeg(atan2(-SY,SX));
	return RadToDeg(atan2(SX,SY)); // North towards Y
	}


void CloseProgram(int ExitCode)
	{
	printf("End.\n");
	if (TracksFile!=NULL) fclose(TracksFile);
	if (FluxFile!=NULL) fclose(FluxFile);
	if (ListFile!=NULL) fclose(ListFile);
	delete TrackDirection;
	delete FineMap;
	delete FluxGm;
	delete SimpleFluxMap;
	exit(ExitCode);
	}

void Help(int ExitCode)
	{
	system("cat Help/HelpFluxComputation.txt");
	exit(ExitCode);
	}


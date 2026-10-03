////////////////////////////////////////////////////////////////////////
// Zoned Maps Class - implemetation
//	version : 2019-02-05
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "ZonedMapsClass.h"

// Basic functions

ZonedMapsClass::ZonedMapsClass(int InNMaps)
	{
	Init();
	if (InNMaps<1) return;
	MaxNMaps=InNMaps;
	AllocateMemory();
	UseNames();
	}

ZonedMapsClass::ZonedMapsClass(int InNMaps, double InStartX, double InEndX, double InStartY, double InEndY)
	{
	Init();
	if (InNMaps<1) return;
	MaxNMaps=InNMaps;
	MaxNXBins=1;
	MaxNYBins=1;
	AllocateMemory();
	XMarker[0]=InStartX; XMarker[1]=InEndX;
	YMarker[0]=InStartY; YMarker[1]=InEndY;
	UseNames();
	}

ZonedMapsClass::ZonedMapsClass(ZonedMapsClass * CloneMap)
	{
	Init();
	MaxNMaps=CloneMap->MaxNMaps;
	MaxNXBins=CloneMap->MaxNXBins;
	MaxNYBins=CloneMap->MaxNYBins;
	AllocateMemory();
	int NMap,NXBin,NYBin;
	for (NXBin=0; NXBin<MaxNXBins+1; NXBin++) XMarker[NXBin]=CloneMap->XMarker[NXBin];
	for (NYBin=0; NYBin<MaxNYBins+1; NYBin++) YMarker[NYBin]=CloneMap->YMarker[NYBin];
	for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=CloneMap->Map[NMap][NXBin][NYBin];
	if (CloneMap->TheName!=NULL) { TheName=(char*)calloc(strlen(CloneMap->TheName)+1,sizeof(char)); sprintf(TheName,"%s",CloneMap->TheName); }
	UseNames();
	}

ZonedMapsClass::ZonedMapsClass(MultiMapClass * MMap, int InNMap)
	{
	Init();
	MaxNMaps=1;
	int	NMap=MMap->CheckNMap(InNMap);
	MaxNXBins=1; MaxNYBins=1;
	AllocateMemory();
	XMarker[0]=MMap->XMinAt(0); XMarker[1]=MMap->XMaxAt(MMap->MaxNXBins-1);
	YMarker[0]=MMap->YMinAt(0); YMarker[1]=MMap->YMaxAt(MMap->MaxNYBins-1);
	Map[0][0][0]=MMap->Integral(NMap)/double(MMap->MaxNXBins*MMap->MaxNYBins);
	}

ZonedMapsClass::ZonedMapsClass(MultiMapClass * MMap, int InNMap, double* ZLevel, int NZLevels, bool TwoDimension)
	{
	Init();
	MaxNMaps=1;
	int	NMap=MMap->CheckNMap(InNMap);
	double	NewXMarker[MMap->MaxNXBins+1]; int NNewXMarkers=0;
	double	NewYMarker[MMap->MaxNYBins+1]; int NNewYMarkers=0;
	// Define markers using Dim1 or Dim2 levelling
	if (TwoDimension)
		{
		// Define levels for merging to Zones
		double	LevelMap[MMap->MaxNXBins][MMap->MaxNYBins];
		for (int NX=0; NX<MMap->MaxNXBins; NX++) for (int NY=0; NY<MMap->MaxNYBins; NY++) 
			{
			LevelMap[NX][NY]=0;
			for (int NZL=0; NZL<NZLevels; NZL++)
				if (MMap->Map[NMap][NX][NY]>ZLevel[NZL]) LevelMap[NX][NY]++;
			}
		// Search for the markers : X
		bool	AddThisMarker;
		NewXMarker[NNewXMarkers]=MMap->XMinAt(0); NNewXMarkers++;
		for (int NX=0; NX<MMap->MaxNXBins-1; NX++)
			{
			AddThisMarker=false;
			for (int NY=0; NY<MMap->MaxNYBins; NY++) if (LevelMap[NX][NY]!=LevelMap[NX+1][NY]) AddThisMarker=true;
			if (AddThisMarker) { NewXMarker[NNewXMarkers]=MMap->XMaxAt(NX); NNewXMarkers++; }
			}
		NewXMarker[NNewXMarkers]=MMap->XMaxAt(MMap->MaxNXBins-1); NNewXMarkers++;
		MaxNXBins=NNewXMarkers-1;
		// Search for the markers : Y
		NewYMarker[NNewYMarkers]=MMap->YMinAt(0); NNewYMarkers++;
		for (int NY=0; NY<MMap->MaxNYBins-1; NY++)
			{
			AddThisMarker=false;
			for (int NX=0; NX<MMap->MaxNXBins; NX++) if (LevelMap[NX][NY]!=LevelMap[NX][NY+1]) AddThisMarker=true;
			if (AddThisMarker) { NewYMarker[NNewYMarkers]=MMap->YMaxAt(NY); NNewYMarkers++; }
			}
		NewYMarker[NNewYMarkers]=MMap->YMaxAt(MMap->MaxNYBins-1); NNewYMarkers++;
		MaxNYBins=NNewYMarkers-1;
		}
	else
		{
		// Search for the markers : X
		double LevelX[MMap->MaxNXBins], AverageX;
		NewXMarker[NNewXMarkers]=MMap->XMinAt(0); NNewXMarkers++;
		for (int NX=0; NX<MMap->MaxNXBins; NX++) 
			{
			AverageX=0; LevelX[NX]=0;
			for (int NY=0; NY<MMap->MaxNYBins; NY++) AverageX+=MMap->Map[NMap][NX][NY];
			AverageX/=double(MMap->MaxNYBins);
			for (int NZL=0; NZL<NZLevels; NZL++) if (AverageX>ZLevel[NZL]) LevelX[NX]++;
			if (NX>0 && (LevelX[NX]!=LevelX[NX-1])) { NewXMarker[NNewXMarkers]=MMap->XMinAt(NX); NNewXMarkers++; }
			}
		NewXMarker[NNewXMarkers]=MMap->XMaxAt(MMap->MaxNXBins-1); NNewXMarkers++;
		MaxNXBins=NNewXMarkers-1;
		// Search for the markers : Y
		double LevelY[MMap->MaxNYBins], AverageY;
		NewYMarker[NNewYMarkers]=MMap->YMinAt(0); NNewYMarkers++;
		for (int NY=0; NY<MMap->MaxNYBins; NY++) 
			{
			AverageY=0; LevelY[NY]=0;
			for (int NX=0; NX<MMap->MaxNXBins; NX++) AverageY+=MMap->Map[NMap][NX][NY];
			AverageY/=double(MMap->MaxNXBins);
			for (int NZL=0; NZL<NZLevels; NZL++) if (AverageY>ZLevel[NZL]) LevelY[NY]++;
			if (NY>0 && (LevelY[NY]!=LevelY[NY-1])) { NewYMarker[NNewYMarkers]=MMap->YMinAt(NY); NNewYMarkers++; }
			}
		NewYMarker[NNewYMarkers]=MMap->YMaxAt(MMap->MaxNYBins-1); NNewYMarkers++;
		MaxNYBins=NNewYMarkers-1;
		}
	// Contruct
	AllocateMemory();
	int NXBin,NYBin;
	for (NXBin=0; NXBin<MaxNXBins+1; NXBin++) XMarker[NXBin]=NewXMarker[NXBin];
	for (NYBin=0; NYBin<MaxNYBins+1; NYBin++) YMarker[NYBin]=NewYMarker[NYBin];
	for (int NX=0; NX<MMap->MaxNXBins; NX++) for (int NY=0; NY<MMap->MaxNYBins; NY++) 
		Map[0][GetXBin(MMap->XAt(NX))][GetYBin(MMap->YAt(NY))]+=MMap->Map[NMap][NX][NY];
	double	MMapBinSize=MMap->XBinSize*MMap->YBinSize;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++) 
		Map[0][NXBin][NYBin]*=MMapBinSize/(XMarker[NXBin+1]-XMarker[NXBin])/(YMarker[NYBin+1]-YMarker[NYBin]);
	}

ZonedMapsClass::~ZonedMapsClass()
	{
	DeleteDummy();
	FreeNames();
	FreeMemory();
	}

void ZonedMapsClass::Init()
	{
	Map=NULL;
	MaxNMaps=0;
	MaxNXBins=0;
	MaxNYBins=0;
	TheName=NULL; SetName((char*)"ZonedMap");
	Name=NULL;
	Dummy=NULL;
	}

void ZonedMapsClass::AllocateMemory()
	{
	int	NMap, NXBin;
	Map = (double***) calloc(MaxNMaps+1,sizeof(double**));
	for (NMap=0; NMap<MaxNMaps+1; NMap++)
		 Map[NMap] = (double**) calloc(MaxNXBins,sizeof(double*));
	for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++)
		 Map[NMap][NXBin] = (double*) calloc (MaxNYBins,sizeof(double)); 
	XMarker = (double*) calloc(MaxNXBins+1,sizeof(double));
	YMarker = (double*) calloc(MaxNYBins+1,sizeof(double));
	//for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++) Map[NMap][NXBin][NYBin]=0;
	}

void ZonedMapsClass::UseNames()
	{
	Name = (char**) calloc(MaxNMaps+1,sizeof(char*));
	for (int NMap=0; NMap<MaxNMaps+1; NMap++) Name[NMap]=NULL;
	}

void ZonedMapsClass::FreeMemory()
	{
	int NMap,NXBin;
	for (NMap=0; NMap<MaxNMaps+1; NMap++) for (NXBin=0; NXBin<MaxNXBins; NXBin++) free(Map[NMap][NXBin]);
	for (NMap=0; NMap<MaxNMaps+1; NMap++) free(Map[NMap]);
	free(Map);
	free(XMarker);
	free(YMarker);
	}

void ZonedMapsClass::SetName(char* NewName)
	{
	free(TheName);
	TheName=(char*)calloc(strlen(NewName)+2,sizeof(char));
	sprintf(TheName,"%s",NewName);
	}

void ZonedMapsClass::SetMapName(int InNMap, char* NewName)
	{
	if (Name==NULL) { printf("Name usage shall be turned on\n"); return;}
	int NMap=CheckMap(InNMap);
	if (Name[NMap]!=NULL) free(Name[NMap]);
	Name[NMap]=(char*)calloc(strlen(NewName)+2,sizeof(char));
	sprintf(Name[NMap],"%s",NewName);
	}

void ZonedMapsClass::FreeNames()
	{
	free(TheName);
	if (Name==NULL) return;
	for (int NMap=0; NMap<MaxNMaps+1; NMap++) free(Name[NMap]); 
	free(Name); 
	}


// Marker functions

void ZonedMapsClass::AddXMarker(double InNewXMarker)
	{
	double NewXMarker=InNewXMarker;
	AddXMarkerArray(1,&NewXMarker);
	}

void ZonedMapsClass::AddYMarker(double InNewYMarker)
	{
	double NewYMarker=InNewYMarker;
	AddYMarkerArray(1,&NewYMarker);
	}

void ZonedMapsClass::AddXMarkerArray(int NNewXMarkers, double* NewXMarkerArray, double ShiftX)
	{
	// Save the recent map
	ZonedMapsClass * OldMaps = new ZonedMapsClass(this);
	FreeMemory();
	// Check the new markers
	int MaxNXMarkers=OldMaps->MaxNXBins+1+NNewXMarkers;
	double * MergedXMarker = (double*) calloc(MaxNXMarkers,sizeof(double));
	for (int NMerged=0; NMerged<OldMaps->MaxNXBins+1; NMerged++) MergedXMarker[NMerged]=OldMaps->XMarker[NMerged];
	for (int NNew=0; NNew<NNewXMarkers; NNew++) MergedXMarker[OldMaps->MaxNXBins+1+NNew]=NewXMarkerArray[NNew]+ShiftX;
	SortArrayAscending(MergedXMarker,MaxNXMarkers);
	RemoveDuplicatesFromArray(MergedXMarker,MaxNXMarkers,&MaxNXBins);
	MaxNXBins--;
	MaxNYBins=OldMaps->MaxNYBins;
	// Set markers
	AllocateMemory();
	for (int NXMarker=0; NXMarker<MaxNXBins+1; NXMarker++) XMarker[NXMarker]=MergedXMarker[NXMarker];
	for (int NYMarker=0; NYMarker<MaxNYBins+1; NYMarker++) YMarker[NYMarker]=OldMaps->YMarker[NYMarker];
	// Fill Maps
	if ( OldMaps->MaxNXBins==0 || OldMaps->MaxNYBins==0 ) { delete OldMaps; free(MergedXMarker); return; }
	int OldNXBin=0;
	int NXBin=0;
	while (XMarker[NXBin]<OldMaps->XMarker[0]) NXBin++;
	for ( ; NXBin<MaxNXBins; NXBin++) 
		{
		if ( XMarker[NXBin] >= OldMaps->XMarker[OldNXBin+1] ) OldNXBin++;
		if (OldNXBin==OldMaps->MaxNXBins) break;
		for (int NMap=0; NMap<MaxNMaps+1; NMap++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
			Map[NMap][NXBin][NYBin]=OldMaps->Map[NMap][OldNXBin][NYBin];
		}
	delete OldMaps;
	free(MergedXMarker);
	}

void ZonedMapsClass::AddYMarkerArray(int NNewYMarkers, double* NewYMarkerArray, double ShiftY)
	{
	// Save the recent map
	ZonedMapsClass * OldMaps = new ZonedMapsClass(this);
	FreeMemory();
	// Check the new markers
	int MaxNYMarkers=OldMaps->MaxNYBins+1+NNewYMarkers;
	double * MergedYMarker = (double*) calloc(MaxNYMarkers,sizeof(double));
	for (int NMerged=0; NMerged<OldMaps->MaxNYBins+1; NMerged++) MergedYMarker[NMerged]=OldMaps->YMarker[NMerged];
	for (int NNew=0; NNew<NNewYMarkers; NNew++) MergedYMarker[OldMaps->MaxNYBins+1+NNew]=NewYMarkerArray[NNew]+ShiftY;
	SortArrayAscending(MergedYMarker,MaxNYMarkers);
	RemoveDuplicatesFromArray(MergedYMarker,MaxNYMarkers,&MaxNYBins);
	MaxNYBins--;
	MaxNXBins=OldMaps->MaxNXBins;
	// Set markers
	AllocateMemory();
	for (int NYMarker=0; NYMarker<MaxNYBins+1; NYMarker++) YMarker[NYMarker]=MergedYMarker[NYMarker];
	for (int NXMarker=0; NXMarker<MaxNXBins+1; NXMarker++) XMarker[NXMarker]=OldMaps->XMarker[NXMarker];
	// Fill Maps
	if ( OldMaps->MaxNXBins==0 || OldMaps->MaxNYBins==0 ) { delete OldMaps; free(MergedYMarker); return; }
	int OldNYBin=0;
	int NYBin=0;
	while (YMarker[NYBin]<OldMaps->YMarker[0]) NYBin++;
	for ( ; NYBin<MaxNYBins; NYBin++) 
		{
		if ( YMarker[NYBin] >= OldMaps->YMarker[OldNYBin+1] ) OldNYBin++;
		if (OldNYBin==OldMaps->MaxNYBins) break;
		for (int NMap=0; NMap<MaxNMaps+1; NMap++) for (int NXBin=0; NXBin<MaxNXBins; NXBin++)
			Map[NMap][NXBin][NYBin]=OldMaps->Map[NMap][NXBin][OldNYBin];
		}
	delete OldMaps;
	free(MergedYMarker);
	}

// Map operations

void ZonedMapsClass::Shift(double ShiftX, double ShiftY)
	{
	for (int NXBin=0; NXBin<MaxNXBins+1; NXBin++) XMarker[NXBin]+=ShiftX;
	for (int NYBin=0; NYBin<MaxNYBins+1; NYBin++) YMarker[NYBin]+=ShiftY;
	}

void ZonedMapsClass::Scale(double ScaleX, double ScaleY)
	{
	for (int NXBin=0; NXBin<MaxNXBins+1; NXBin++) XMarker[NXBin]*=ScaleX;
	for (int NYBin=0; NYBin<MaxNYBins+1; NYBin++) YMarker[NYBin]*=ScaleY;
	}

void ZonedMapsClass::AddMap(int InNMap, ZonedMapsClass * ZMap, int InNZMap)
	{
	if (this==ZMap) { printf("Cannot add to itself\n"); return; }
	int NMap=CheckMap(InNMap);
	int NZMap=ZMap->CheckMap(InNZMap);
	AddXMarkerArray(ZMap->MaxNXBins+1,ZMap->XMarker);
	AddYMarkerArray(ZMap->MaxNYBins+1,ZMap->YMarker);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		if ( (XAt(NXBin)>ZMap->XMarker[0]) && (XAt(NXBin)<ZMap->XMarker[ZMap->MaxNXBins]) && (YAt(NYBin)>ZMap->YMarker[0]) && (YAt(NYBin)<ZMap->YMarker[ZMap->MaxNYBins]) )
			Map[NMap][NXBin][NYBin]+=ZMap->Map[NZMap][ZMap->GetXBin(XAt(NXBin))][ZMap->GetYBin(YAt(NYBin))];
	}

void ZonedMapsClass::AddMapShifted(int InNMap, ZonedMapsClass * ZMap, double ShiftX, double ShiftY)
	{ AddMapShifted(InNMap,ZMap,0,ShiftX,ShiftY); }

void ZonedMapsClass::AddMapShifted(int InNMap, ZonedMapsClass * ZMap, int InNZMap, double ShiftX, double ShiftY)
	{
	if (this==ZMap) { printf("Cannot add to itself\n"); return; }
	int NMap=CheckMap(InNMap);
	int NZMap=ZMap->CheckMap(InNZMap);
	AddXMarkerArray(ZMap->MaxNXBins+1,ZMap->XMarker,ShiftX);
	AddYMarkerArray(ZMap->MaxNYBins+1,ZMap->YMarker,ShiftY);
	double	X,Y; // of the Other ZMap
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		{
		X=XAt(NXBin)-ShiftX; Y=YAt(NYBin)-ShiftY;
		if ( (X>ZMap->XMarker[0]) && (X<ZMap->XMarker[ZMap->MaxNXBins]) && (Y>ZMap->YMarker[0]) && (Y<ZMap->YMarker[ZMap->MaxNYBins]) )
			Map[NMap][NXBin][NYBin]+=ZMap->Map[NZMap][ZMap->GetXBin(X)][ZMap->GetYBin(Y)];
		}
	}

void ZonedMapsClass::CopyFromTo(int InNMapFrom, int InNMapTo)
	{
	int NMapFrom=CheckMap(InNMapFrom);
	int NMapTo=CheckMap(InNMapTo);
	if (NMapTo==NMapFrom) return;
	int NXBin,NYBin;
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMapTo][NXBin][NYBin]=Map[NMapFrom][NXBin][NYBin];
	}

void ZonedMapsClass::ModifyMapWith(int InNMap1, int InNMap2, double (*Function) (double,double))
	{
	int	NXBin, NYBin, NMap1=CheckMap(InNMap1), NMap2=CheckMap(InNMap2);
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap1][NXBin][NYBin]=Function(Map[NMap1][NXBin][NYBin],Map[NMap2][NXBin][NYBin]);
	}

// Get Bin level info

int ZonedMapsClass::CheckMap(int InNMap)
	{
	if (InNMap<0 || InNMap>MaxNMaps) return MaxNMaps;
	return InNMap;
	}

double ZonedMapsClass::XAt(int NXBin)
	{
	if (NXBin<0) return XMarker[0];
	if (NXBin>=MaxNXBins) return XMarker[MaxNXBins];
	return (XMarker[NXBin]+XMarker[NXBin+1])/2.0;
	}

double ZonedMapsClass::YAt(int NYBin)
	{
	if (NYBin<0) return YMarker[0];
	if (NYBin>=MaxNYBins) return YMarker[MaxNYBins];
	return (YMarker[NYBin]+YMarker[NYBin+1])/2.0;
	}

int ZonedMapsClass::GetXBin(double X)
	{
	int NBin=0;
	while ( (XMarker[NBin]<X) && (NBin<MaxNXBins) ) NBin++;
	return (NBin-1);
	}

int ZonedMapsClass::GetYBin(double Y)
	{
	int NBin=0;
	while ( (YMarker[NBin]<Y) && (NBin<MaxNYBins) ) NBin++;
	return (NBin-1);
	}

double ZonedMapsClass::ZAt(int InNMap, int NXBin, int NYBin)
	{
	return Map[InNMap][NXBin][NYBin];
	}

double ZonedMapsClass::Z(int InNMap, double InX, double InY)
	{
	return Map[CheckMap(InNMap)][GetXBin(InX)][GetYBin(InY)];
	}

// Get Map level info

double ZonedMapsClass::Integral(int InNMap)
	{
	int NMap=CheckMap(InNMap);
	double TheIntegral=0;
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		TheIntegral+=Map[NMap][NXBin][NYBin]*(XMarker[NXBin+1]-XMarker[NXBin])*(YMarker[NYBin+1]-YMarker[NYBin]);
	return TheIntegral;
	}

// DummyMap operations

void ZonedMapsClass::CreateDummy()
	{
	Dummy = new MultiMapClass(MaxNMaps,0,MaxNXBins,1,0,MaxNYBins,1);
	for (int NMap=0; NMap<MaxNMaps; NMap++) for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		Dummy->Map[NMap][NXBin][NYBin]=Map[NMap][NXBin][NYBin];
	}

void ZonedMapsClass::DeleteDummy()
	{
	if (Dummy==NULL) return;
	delete Dummy;
	}

void ZonedMapsClass::GetFromDummy(int InNMap)
	{
	int NMap=CheckMap(InNMap);
	for (int NXBin=0; NXBin<MaxNXBins; NXBin++) for (int NYBin=0; NYBin<MaxNYBins; NYBin++)
		Map[NMap][NXBin][NYBin]=Dummy->Map[NMap][NXBin][NYBin];
	}

// File operations

void ZonedMapsClass::WriteToFile(FILE * OutFile)
	{
	int	NMap, NXBin, NYBin;
	fprintf(OutFile,"#%s\n",TheName);
	fprintf(OutFile,"# X\tY");
	if (Name!=NULL) for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%s",Name[NMap]);
	fprintf(OutFile,"\n");
	// Very first line
	for (NYBin=0; NYBin<MaxNYBins+1; NYBin++) for (int y=0; y<2; y++)
		{
		fprintf(OutFile,"%g\t%g",XMarker[0],YMarker[NYBin]);
		for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t0"); fprintf(OutFile,"\n");
		}
	fprintf(OutFile,"\n");
	// Real map
	for (NXBin=0; NXBin<MaxNXBins; NXBin++) for (int x=0; x<2; x++)
		{ 
		fprintf(OutFile,"%g\t%g",XMarker[NXBin+x],YMarker[0]);
		for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t0"); fprintf(OutFile,"\n");
		for (NYBin=0; NYBin<MaxNYBins; NYBin++)
			{
			fprintf(OutFile,"%g\t%g",XMarker[NXBin+x],YMarker[NYBin]);
			for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",Map[NMap][NXBin][NYBin]); fprintf(OutFile,"\n"); 
			fprintf(OutFile,"%g\t%g",XMarker[NXBin+x],YMarker[NYBin+1]);
			for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t%g",Map[NMap][NXBin][NYBin]); fprintf(OutFile,"\n"); 
			}
		fprintf(OutFile,"%g\t%g",XMarker[NXBin+x],YMarker[MaxNYBins]);
		for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t0"); fprintf(OutFile,"\n\n");
		}
	// Very last line
	for (NYBin=0; NYBin<MaxNYBins+1; NYBin++) for (int y=0; y<2; y++)
		{
		fprintf(OutFile,"%g\t%g",XMarker[MaxNXBins],YMarker[NYBin]);
		for (NMap=0; NMap<MaxNMaps; NMap++) fprintf(OutFile,"\t0"); fprintf(OutFile,"\n");
		}
	fprintf(OutFile,"\n\n");
	}

void ZonedMapsClass::WriteToFileName(char* FileName)
	{
	FILE * NewFile;
	if ( (NewFile=fopen(FileName,"w")) == NULL ) 
		{printf("\nERROR : File: %s could not be opened\n\n",FileName); return;}
	WriteToFile(NewFile);
	fclose(NewFile);
	}





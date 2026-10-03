////////////////////////////////////////////////////////////////////////
// Grained Maps Class - implementation
//	version : 2018-11-01
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "GrainedMapsClass.h"

////////////////////////////////////////////////////////////////////////
// Grain Class

GrainClass::GrainClass()
	{
	}

GrainClass::GrainClass(GrainClass* Mother)
	{
	MapN=Mother->MapN;
	X=Mother->X; SizeX=Mother->SizeX;
	Y=Mother->Y; SizeY=Mother->SizeY;
	N=Mother->N;
	}

GrainClass::GrainClass(double** InMapN, int InX, int InSizeX, int InY, int InSizeY)
	{
	MapN=InMapN;
	X=InX; SizeX=InSizeX;
	Y=InY; SizeY=InSizeY;
	N=0;
	for (int nx=0; nx<SizeX; nx++) for (int ny=0; ny<SizeY; ny++)
		N+=MapN[X+nx][Y+ny];
	}

GrainClass::~GrainClass()
	{
	}

bool GrainClass::Chop()
	{
	if (SizeX<2 || SizeY<2) return false;
	NewGrain[0] = new GrainClass(MapN,X+SizeX/2,SizeX-SizeX/2,Y,SizeY/2);
	NewGrain[1] = new GrainClass(MapN,X,SizeX/2,Y+SizeY/2,SizeY-SizeY/2);
	NewGrain[2] = new GrainClass(MapN,X+SizeX/2,SizeX-SizeX/2,Y+SizeY/2,SizeY-SizeY/2);
	SizeX/=2; SizeY/=2;
	N=0;
	for (int nx=0; nx<SizeX; nx++) for (int ny=0; ny<SizeY; ny++)
		N+=MapN[X+nx][Y+ny];
	return true;
	}

void GrainClass::ShowInfo()
	{
	printf("Grain : %d+%d,%d+%d : %g\n",X,SizeX,Y,SizeY,N);
	}

bool GrainClass::Inside(double PX, double PY)
	{
	if ( (PX>=X) && (PX<(X+SizeX)) && (PY>=Y) && (PY<(Y+SizeY)) ) return true;
	else return false;
	}

////////////////////////////////////////////////////////////////////////
// Grained Maps Class

GrainedMapsClass::GrainedMapsClass()
	{
	MMap=NULL; IdMapN=-1; Limit=-1;
	NGrains=0; Grain=NULL;
	ZMap=NULL; TheName=NULL;
	SetName((char*)"GrainedMap");
	}

GrainedMapsClass::~GrainedMapsClass()
	{
	if (ZMap!=NULL) delete ZMap;
	for (int NGrain=0; NGrain<NGrains; NGrain++) delete Grain[NGrain];
	free(Grain);
	free(TheName);
	}

void GrainedMapsClass::SetMap(MultiMapClass *InMMap, int InIdMapN, double InLimit)
	{
	MMap=InMMap;
	IdMapN=InIdMapN;
	Limit=InLimit;
	}

void GrainedMapsClass::SetName(char* InTheName)
	{
	free(TheName);
	TheName=(char*)calloc(strlen(InTheName)+1,sizeof(char));
	sprintf(TheName,"%s",InTheName);
	}

bool GrainedMapsClass::Compute()
	{
	if (MMap==NULL || IdMapN==-1 || Limit<=0) 
		{ printf("GrainedMaps : Parameters are missing\n"); return false; }
	// Set the initial grain as the full area
	GrainClass *GrainFirst = new GrainClass(MMap->Map[IdMapN],0,MMap->MaxNXBins,0,MMap->MaxNYBins);
	NGrains=1;
	Grain=(GrainClass**) realloc(Grain,sizeof(GrainClass*)*NGrains);
	Grain[0]=GrainFirst;
	// Let's chop the gains until they are small enough
	int	ThisGrain=0;
	bool	ChopOk;
	while (ThisGrain<NGrains)
		{
		//printf("Checking: %d ::",ThisGrain);
		//Grain[ThisGrain]->ShowInfo();
		if (Grain[ThisGrain]->N<Limit || Grain[ThisGrain]->SizeX<2 || Grain[ThisGrain]->SizeY<2)
			{
			ThisGrain++;
			}
		else
			{
			ChopOk=Grain[ThisGrain]->Chop();
			if (!ChopOk) { ThisGrain++; continue; }
			Grain=(GrainClass**) realloc(Grain,sizeof(GrainClass*)*(NGrains+3));
			for (int g=0; g<3; g++) Grain[NGrains+g]=Grain[ThisGrain]->NewGrain[g];
			NGrains+=3;
			}
		}
	return true;
	}

void GrainedMapsClass::FillZonedMaps()
	{
	// Define the ZonedMap
	ZMap = new ZonedMapsClass(MMap->MaxNMaps);
	if (TheName!=NULL) ZMap->SetName(TheName);
	for (int NMap=0; NMap<MMap->MaxNMaps; NMap++) ZMap->SetMapName(NMap,MMap->Name[NMap]);
	double	MarkerX[NGrains+1],MarkerY[NGrains+1];
	for (int g=0;g<NGrains;g++)
		{ MarkerX[g]=Grain[g]->X; MarkerY[g]=Grain[g]->Y; }
	MarkerX[NGrains]=MMap->MaxNXBins;
	MarkerY[NGrains]=MMap->MaxNYBins;
	ZMap->AddXMarkerArray(NGrains+1,MarkerX);
	ZMap->AddYMarkerArray(NGrains+1,MarkerY);
	// Fill the ZonedMap
	int	GrainForZone=0;
	for (int nx=0;nx<ZMap->MaxNXBins;nx++) for (int ny=0;ny<ZMap->MaxNYBins;ny++)
		{
		for (int g=0;g<NGrains;g++)
			if (Grain[g]->Inside(ZMap->XAt(nx),ZMap->YAt(ny))) GrainForZone=g;
		for (int m=0;m<MMap->MaxNMaps;m++)
			for (int mapx=Grain[GrainForZone]->X; mapx<Grain[GrainForZone]->X+Grain[GrainForZone]->SizeX; mapx++)
			for (int mapy=Grain[GrainForZone]->Y; mapy<Grain[GrainForZone]->Y+Grain[GrainForZone]->SizeY; mapy++)
				ZMap->Map[m][nx][ny]+=MMap->Map[m][mapx][mapy];
		}
	// Redefine the Markers
	for (int mx=0; mx<ZMap->MaxNXBins; mx++)
		ZMap->XMarker[mx]=MMap->XMinAt(round(ZMap->XMarker[mx]));
	ZMap->XMarker[ZMap->MaxNXBins]=MMap->XMaxAt(MMap->MaxNXBins-1);
		for (int my=0; my<ZMap->MaxNYBins; my++)
		ZMap->YMarker[my]=MMap->YMinAt(round(ZMap->YMarker[my]));
	ZMap->YMarker[ZMap->MaxNYBins]=MMap->YMaxAt(MMap->MaxNYBins-1);
	}

void GrainedMapsClass::GrainsToBaseMapFxyo(double (*Function) (double,double), int IdFuncArgX, int IdFuncArgY, int IdFuncResult)
	{
	// practical use: Function==Divide ; ArgX=FluxA, ArgY=Aov; Result=Flux;
	// for all Grains , for sizex , for sizey, : 
	//	sum fluxa, sum aov; f=fluxa/aov;
	double SumArgX, SumArgY, Result; // Flux=SumFluxA/SumAov;
	for (int g=0;g<NGrains;g++)
		{
		SumArgX=0; SumArgY=0;
		for (int x=0; x<Grain[g]->SizeX; x++) for (int y=0; y<Grain[g]->SizeY; y++)
			{
			SumArgX+=MMap->ZAt(IdFuncArgX,Grain[g]->X+x,Grain[g]->Y+y); // eg. FluxA
			SumArgY+=MMap->ZAt(IdFuncArgY,Grain[g]->X+x,Grain[g]->Y+y); // eg. Aov
			}
		Result=Function(SumArgX,SumArgY);
		for (int x=0; x<Grain[g]->SizeX; x++) for (int y=0; y<Grain[g]->SizeY; y++)
			MMap->SetAt(IdFuncResult,Grain[g]->X+x,Grain[g]->Y+y,Result);
		}
	}

void GrainedMapsClass::WriteToFile(FILE * OutFile)
	{
	// Write the Grain info
	fprintf(OutFile,"#%s : Grains\n",TheName);
	fprintf(OutFile,"#  X  Y\tSizeX  SizeY\t");
	for (int NMap=0; NMap<MMap->MaxNMaps; NMap++) fprintf(OutFile,"\t%s",MMap->Name[NMap]);
	fprintf(OutFile,"\n");
	for (int NGrain=0; NGrain<NGrains; NGrain++)
		{
		fprintf(OutFile,"%g %g\t%g %g\t",MMap->XMinAt(Grain[NGrain]->X)+Grain[NGrain]->SizeX*MMap->XBinSize*0.5,MMap->YMinAt(Grain[NGrain]->Y)+Grain[NGrain]->SizeY*MMap->YBinSize*0.5,Grain[NGrain]->SizeX*MMap->XBinSize,Grain[NGrain]->SizeY*MMap->YBinSize);
		for (int NMap=0; NMap<ZMap->MaxNMaps; NMap++) fprintf(OutFile,"\t%g",ZMap->Z(NMap,MMap->XAt(Grain[NGrain]->X),MMap->YAt(Grain[NGrain]->Y)));
		fprintf(OutFile,"\n");
		}
	fprintf(OutFile,"\n\n");
	// Write the ZonedMap as well
	if (ZMap==NULL) FillZonedMaps();
	ZMap->WriteToFile(OutFile);
	}

void GrainedMapsClass::WriteToFileName(char* OutFileName)
	{
	if (ZMap==NULL) FillZonedMaps();
	FILE * OutFile = fopen(OutFileName,"w");
	WriteToFile(OutFile);
	fclose(OutFile);
	}


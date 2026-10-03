////////////////////////////////////////////////////////////////////////
//  Analysis Parameters Class - implementaion
//      version : 2021-02-16
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "AnalysisParametersClass.h"

AnalysisParametersClass::AnalysisParametersClass()
	{
	// These are the defaults
	// General info
	sprintf(Name,"Default");
	// Chamber level
	PositionDepBinSize=1;
	AngleMax=1.5;
	AngleDepBinSize=0.050;
	DistMax=2.1;
	ResolutionBinSize=0.1;
	// Time
	TimeDepBinSizeSeconds=1;
	MaxNTimeBins=200;
	// Adc
	AdcMin=-5e3; AdcMax=5e3; AdcBinSize=8;
	// Clusters
	ClusterSizeLimit=8;
	ClusterBreak=0;
	NClustersLimit=4;
	ClusterPositionBinSize=0.5;
	SmoothingPeriodLength=4;
	// Detector, Tracking level
	DoSideGrowInt=0;
	TrackKhi2BinSize=0.100;
	TrackKhi2Max=5.0;
	TrackKhi2Good=0.0;
	TrackingNHits=-1;
	TrackMinNHitsSet=-1;
	TrackMaxNMissSet=2;
	PerformanceTrackMiss=0;
	NChambers=0;
	}


bool AnalysisParametersClass::Set(char* ParName, double ParValue)
	{
	//printf("AnalysisParameter : Set : %s=%g\n",ParName,ParValue);
	// Chamber level parameters
	if      (strcmp(ParName,"PositionDepBinSize")==0) PositionDepBinSize=ParValue;
	else if (strcmp(ParName,"AngleMax")==0) 	AngleMax=ParValue;
	else if (strcmp(ParName,"AngleDepBinSize")==0)	AngleDepBinSize=ParValue;
	else if (strcmp(ParName,"DistMax")==0)		DistMax=ParValue;
	else if (strcmp(ParName,"ResolutionBinSize")==0) ResolutionBinSize=ParValue;
	else if (strcmp(ParName,"TimeDepBinSizeSeconds")==0) TimeDepBinSizeSeconds=ParValue;
	else if (strcmp(ParName,"MaxNTimeBins")==0)	MaxNTimeBins=int(ParValue);
	else if (strcmp(ParName,"AdcMin")==0)		AdcMin=ParValue;
	else if (strcmp(ParName,"AdcMax")==0)		AdcMax=ParValue;
	else if (strcmp(ParName,"AdcBinSize")==0)	AdcBinSize=ParValue;
	// Clusters parameters
	else if (strcmp(ParName,"ClusterSizeLimit")==0)	ClusterSizeLimit=ParValue;
	else if (strcmp(ParName,"ClusterBreak")==0)	{ int cb=int(ParValue); if (!(cb==0 || cb==1 || cb==2 || cb==3)) {printf("ClusterBreak=0,1,2,3\n");return false;} ClusterBreak=cb; }
	else if (strcmp(ParName,"NClustersLimit")==0)	NClustersLimit=ParValue;
	else if (strcmp(ParName,"ClusterPositionBinSize")==0) ClusterPositionBinSize=ParValue;
	else if (strcmp(ParName,"SmoothingPeriodLength")==0) SmoothingPeriodLength=ParValue;
	// Detector and Tracking level parameters
	else if (strcmp(ParName,"DoSideGrowInt")==0)	DoSideGrowInt=int(ParValue);
	else if (strcmp(ParName,"TrackKhi2BinSize")==0)	TrackKhi2BinSize=ParValue;
	else if (strcmp(ParName,"TrackKhi2Max")==0)	TrackKhi2Max=ParValue;
	else if (strcmp(ParName,"TrackKhi2Good")==0)	TrackKhi2Good=ParValue;
	else if (strcmp(ParName,"TrackMinNHits")==0)	{ TrackMinNHitsSet=int(ParValue); if (TrackMinNHitsSet>0) TrackMaxNMissSet=-1; }
	else if (strcmp(ParName,"TrackMaxNMiss")==0)	{ TrackMaxNMissSet=int(ParValue); if (TrackMaxNMissSet>0) TrackMinNHitsSet=-1; }
	else if (strcmp(ParName,"TrackingNHits")==0)	TrackingNHits=ParValue;
	else if (strcmp(ParName,"PerformanceTrackMiss")==0)	PerformanceTrackMiss=ParValue;
	// unrecognized
	else { printf("Waring! AnalysisParametersFile : Unrecognized parameter name: %s\n",ParName); return false; }
	return true;
	}


bool AnalysisParametersClass::ReadFromFile(char* FileName)
	{
	FILE * APFile = fopen(FileName,"r");
	if (APFile==NULL) { printf("Warning ! Analysis Parameters File : %s cannot be opened\n",FileName); return false; }
	sprintf(Name,"%s",FileName);
	char	Line[401]; int NLine=0;
	char	ParName[50]; double ParValue; bool SetParOk;
	while (!feof(APFile))
		{
		NLine++;
		fgets(Line,400,APFile);
		if (Line[0]=='#' || strlen(Line)<5) continue;
		sscanf(Line,"%s %lf",ParName,&ParValue);
		SetParOk=Set(ParName,ParValue);
		if (!SetParOk) printf("Warning : Cannot set : %s = %g\n",ParName,ParValue);
		}
	fclose(APFile);
	if (NChambers>1) SetNChambers(NChambers);
	return true;
	}

bool AnalysisParametersClass::WriteToFile(FILE* OutFile)
	{
	if (OutFile==NULL) return false;
	fprintf(OutFile,"#AnalysisParameters\n");
	fprintf(OutFile,"NChambers\t\t%d\n",NChambers);
	fprintf(OutFile,"TrackingNHits\t\t%d\n",TrackingNHits);
	fprintf(OutFile,"PerformanceTrackMiss\t\t%d\n",PerformanceTrackMiss);
	fprintf(OutFile,"AngleMax\t\t%g\n",AngleMax);
	fprintf(OutFile,"AngleDepBinSize\t\t%g\n",AngleDepBinSize);
	fprintf(OutFile,"PositionDepBinSize\t%g\n",PositionDepBinSize);
	fprintf(OutFile,"DistMax\t\t\t%g\n",DistMax);
	fprintf(OutFile,"ResolutionBinSize\t%g\n",ResolutionBinSize);
	fprintf(OutFile,"TimeDepBinSizeSeconds\t%g\n",TimeDepBinSizeSeconds);
	fprintf(OutFile,"MaxNTimeBins\t\t%d\n",MaxNTimeBins);
	fprintf(OutFile,"AdcMin\t\t\t%g\n",AdcMin);
	fprintf(OutFile,"AdcMax\t\t\t%g\n",AdcMax);
	fprintf(OutFile,"AdcBinSize\t\t%g\n",AdcBinSize);
	fprintf(OutFile,"ClusterSizeLimit\t%d\n",ClusterSizeLimit);
	fprintf(OutFile,"ClusterBreak\t%d\n",ClusterBreak);
	fprintf(OutFile,"NClustersLimit\t\t%d\n",NClustersLimit);
	fprintf(OutFile,"ClusterPositionBinSize\t%g\n",ClusterPositionBinSize);
	fprintf(OutFile,"SmoothingPeriodLength\t%g\n",SmoothingPeriodLength);
	fprintf(OutFile,"DoSideGrowInt\t\t%d\n",DoSideGrowInt);
	fprintf(OutFile,"TrackKhi2BinSize\t%g\n",TrackKhi2BinSize);
	fprintf(OutFile,"TrackKhi2Max\t\t%g\n",TrackKhi2Max);
	fprintf(OutFile,"\n\n");
	return true;
	}


bool AnalysisParametersClass::SetNChambers(int InNChambers)
	{
	if (InNChambers<0) return false;
	NChambers=InNChambers;
	if (TrackingNHits>0) return true;
	if (TrackMinNHitsSet==-1 && TrackMaxNMissSet==-1) return false;
	if (TrackMinNHitsSet>1) TrackingNHits=TrackMinNHitsSet;
	if (TrackMaxNMissSet>-1) TrackingNHits=NChambers-TrackMaxNMissSet;
	return true;
	}



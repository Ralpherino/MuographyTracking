////////////////////////////////////////////////////////////////////////////////
// Fill Muogram
//	version: 20240313
//	written by: G.Hamar at WignerRCP
////////////////////////////////////////////////////////////////////////////////

#include "FillMuogram.h"

int main(int ArgC, char** ArgV)
{
ReadArguments(ArgC,ArgV);
CheckFilenaming();
ReadRunLevelInfo();
Muogram = new MuogramClass(BinSizeDeg);

// Do steps one-by-one in proper order, if requested
if (!DoCreate)
	Muogram->ReadFromFileName(MuogramFileName);
if (DoSetRot)
	{ Muogram->RotX=RotX; Muogram->RotY=RotY; Muogram->RotZ=RotZ; }
if (DoReadTrackList)
	Muogram->ReadTrackList(TrackListFileName,RotX,RotY,RotZ);
if (DoFluxToDl)
	Muogram->FluxToDl(FluxDlConvZfdFileName);
if (DoSetPos)
	{ Muogram->PosN=PosN; Muogram->PosE=PosE; Muogram->PosH=PosH; }
if (DoReadDemFile)
	Muogram->ReadDemFileName(DemFileName,PosN,PosE,PosH);
if (DoReadStlFile)
	Muogram->ReadStlFileName(TunnelsStlFileName,PosN,PosE,PosH);
if (DoEstimateDensity || (Density<0.1))
	DensityEst=Muogram->EstimateDensity();
if (Density<0.1) Density=DensityEst;
if (DoRlToDl)
	Muogram->RlToDl(Density);
if (DoCompareDlMC)
	Muogram->CompareDlMC();
if (DoWrite)
	Muogram->WriteToFileName(MuogramFileName);
if (DoShowInfo)
	Muogram->ShowInfo();
if (DoPlot)
	{ sprintf(Command,"gnuplot -e \"m='%s'\" PlotScripts/PlotMuogram.gp;",MuogramFileName); system(Command); }
if (DoMove)
	{ sprintf(Command,"mv %s* %s/.",MuogramFileName,ResultsDirName); system(Command); }


}




void ReadArguments(int ArgC, char** ArgV)
	{
	if (Verbose>0) printf("Reading arguments ...\n");
	if (ArgC<2) Help(0);
	for (int i=1; i<ArgC; i++)
		if (strcmp(ArgV[i],"-h")==0 || strcmp(ArgV[i],"--help")==0 )
			{ Help(); }
		else if (strcmp(ArgV[i],"-m")==0 || strcmp(ArgV[i],"--muogam-file")==0 )
			{ i++; sprintf(MuogramFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-bs")==0 || strcmp(ArgV[i],"--bin-size")==0 )
			{ i++;  sscanf(ArgV[i],"%lf",&BinSizeDeg); }
		else if (strcmp(ArgV[i],"-c")==0 || strcmp(ArgV[i],"--create")==0 )
			{ i++; DoCreate=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-rli")==0 || strcmp(ArgV[i],"--run-level-info-file")==0 )
			{ i++; sprintf(RunLevelInfoFileName,"%s",ArgV[i]); }
		else if (strcmp(ArgV[i],"-rd")==0 || strcmp(ArgV[i],"--results-dir")==0 )
			{ i++; sprintf(ResultsDirName,"%s",ArgV[i]); DoMove=true; }
		else if (strcmp(ArgV[i],"-tl")==0 || strcmp(ArgV[i],"--track-list-file")==0 )
			{ i++; sprintf(TrackListFileName,"%s",ArgV[i]); DoReadTrackList=true; }
		else if (strcmp(ArgV[i],"--rot-xyz")==0 )
			{ i++; sscanf(ArgV[i],"%lf",&RotX); i++; sscanf(ArgV[i],"%lf",&RotY); i++; sscanf(ArgV[i],"%lf",&RotZ); }
		else if (strcmp(ArgV[i],"--rot-yxz")==0 )
			{ i++; sscanf(ArgV[i],"%lf",&RotY); i++; sscanf(ArgV[i],"%lf",&RotX); i++; sscanf(ArgV[i],"%lf",&RotZ); }
		else if (strcmp(ArgV[i],"-fd")==0 || strcmp(ArgV[i],"--flux-dl-conv")==0 )
			{ i++; DoFluxToDl=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-zfd")==0 || strcmp(ArgV[i],"--flux-dl-conv-file")==0 )
			{ i++; sprintf(FluxDlConvZfdFileName,"%s",ArgV[i]); DoFluxToDl=true; }
		else if (strcmp(ArgV[i],"-t")==0 || strcmp(ArgV[i],"--topography")==0 )
			{ i++; DoReadDemFile=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-dem")==0 || strcmp(ArgV[i],"--dem-file-name")==0 )
			{ i++; sprintf(DemFileName,"%s",ArgV[i]); DoReadDemFile=true; }
		else if (strcmp(ArgV[i],"-stl")==0 || strcmp(ArgV[i],"--stl-file-name")==0 )
			{ i++; sprintf(TunnelsStlFileName,"%s",ArgV[i]); DoReadStlFile=true; }
		else if (strcmp(ArgV[i],"-p")==0  || strcmp(ArgV[i],"--pos-enh")==0 )
			{ i++; sscanf(ArgV[i],"%lf",&PosE); i++; sscanf(ArgV[i],"%lf",&PosN); i++; sscanf(ArgV[i],"%lf",&PosH); }
		else if (strcmp(ArgV[i],"-d")==0 || strcmp(ArgV[i],"--estimate-density")==0 )
			{ i++;  sscanf(ArgV[i],"%lf",&Density); DoEstimateDensity=true; DoRlToDl=true; }
		else if (strcmp(ArgV[i],"-com")==0 || strcmp(ArgV[i],"--compare")==0 )
			{ i++; DoCompareDlMC=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-gp")==0 || strcmp(ArgV[i],"--plot")==0 )
			{ i++; DoPlot=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-s")==0 || strcmp(ArgV[i],"--show-info")==0 )
			{ i++; DoShowInfo=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-w")==0 || strcmp(ArgV[i],"--write")==0 )
			{ i++; DoWrite=GetYesNoAnswer(ArgV[i]); }
		else if (strcmp(ArgV[i],"-v")==0 || strcmp(ArgV[i],"--verbose")==0 )
			{ i++; sscanf(ArgV[i],"%d",&Verbose); }
		else if (strcmp(ArgV[i],"-id")==0 || strcmp(ArgV[i],"--id")==0 )
			{ i++; sscanf(ArgV[i],"%d",&IdNumber); }
		else
			{ printf("\nBad argument : %s\n\n",ArgV[i]); Help(ErrWrongArgument); };
	}

void ReadRunLevelInfo()
	{
	// Check the RunLevelInfoFile
	if (strlen(RunLevelInfoFileName)<2) return;
	FILE * RunLevelInfoFile = fopen(RunLevelInfoFileName,"r");
	if (RunLevelInfoFile==NULL) { printf("Warning: Cannot open RunLevelInfo file : %s\n",RunLevelInfoFileName); return; }
	// Extract ThisRunName to compare later with the RunName params
	printf("Check RunLevelInfo file: %s\n",RunLevelInfoFileName);
	char	*RunNaming;
	if (strlen(MuogramFileName)>2) RunNaming=MuogramFileName;
	if (strlen(TrackListFileName)>2) RunNaming=TrackListFileName;
	std::string ThisRunNaming(RunNaming);
	char	RunName[50],ParName[50]; int FoundAt;
	printf("\tUsing naming: %s\n",RunNaming);
	// Loop over the RliFile
	while (!feof(RunLevelInfoFile))
		{
		sprintf(RunName,"-");
		fscanf(RunLevelInfoFile,"%s",RunName);
		if (RunName[0]=='#') { SkipLine(RunLevelInfoFile); continue; }
		FoundAt=ThisRunNaming.find(RunName);
		if (uint(FoundAt)==std::string::npos || FoundAt<0) { SkipLine(RunLevelInfoFile); continue; }
		if (isdigit(ThisRunNaming.at(FoundAt+strlen(RunName)))!=0) { SkipLine(RunLevelInfoFile); continue; }
		// Reaching here means RunName matching, set variables according to names
		fscanf(RunLevelInfoFile,"%s",ParName);
		if (strcmp(ParName,"PosNeh")==0)
			{ fscanf(RunLevelInfoFile,"%lf %lf %lf",&PosN,&PosE,&PosH); printf("\t%s: PosEnh: %g %g %g\n",RunName,PosE,PosN,PosH); }
		else if (strcmp(ParName,"PosEnh")==0)
			{ fscanf(RunLevelInfoFile,"%lf %lf %lf",&PosE,&PosN,&PosH); printf("\t%s: PosEnh: %g %g %g\n",RunName,PosE,PosN,PosH); }
		else if (strcmp(ParName,"RotYxz")==0)
			{ fscanf(RunLevelInfoFile,"%lf %lf %lf",&RotY,&RotX,&RotZ); printf("\t%s: RotXyz: %g %g %g\n",RunName,RotX,RotY,RotZ); }
		else if (strcmp(ParName,"RotXyz")==0)
			{ fscanf(RunLevelInfoFile,"%lf %lf %lf",&RotX,&RotY,&RotZ); printf("\t%s: RotXyz: %g %g %g\n",RunName,RotX,RotY,RotZ); }
		else if (strcmp(ParName,"DemFile")==0)
			{ DoReadDemFile=true; fscanf(RunLevelInfoFile,"%s",DemFileName); printf("\t%s: DemFile: %s\n",RunName,DemFileName); }
		else if ( (strcmp(ParName,"Tunnels")==0) || (strcmp(ParName,"TunnelsFile")==0) )
			{ DoReadStlFile=true; fscanf(RunLevelInfoFile,"%s",TunnelsStlFileName); printf("\t%s: TunnelsFile: %s\n",RunName,TunnelsStlFileName); }
		else if (strcmp(ParName,"ZfdFile")==0)
			{ DoFluxToDl=true; fscanf(RunLevelInfoFile,"%s",FluxDlConvZfdFileName); printf("\t%s: ZfdFile: %s\n",RunName,FluxDlConvZfdFileName); }
		else if (strcmp(ParName,"ResultsDir")==0)
			{ DoMove=true; fscanf(RunLevelInfoFile,"%s",ResultsDirName); printf("\t%s: ResultsDir: %s\n",RunName,ResultsDirName); }
		else
			{ printf("Warning: Unrecognized RunLevelInfo ParameterName: %s\n",ParName); SkipLine(RunLevelInfoFile); continue; }
		}
	fclose(RunLevelInfoFile);
	}

void CheckFilenaming()
	{
	// If MuogramFileName is defined, no need to guess it, just check extension
	if (strlen(MuogramFileName)>2)
		{
		if (strcmp(".mgm",&(MuogramFileName[strlen(MuogramFileName)-4]))!=0)
			sprintf(&(MuogramFileName[strlen(MuogramFileName)]),".mgm");
		return;
		}
	// If no name is defined, get it from the TrackList data
	if (strlen(TrackListFileName)>2)
		sprintf(MuogramFileName,"%s.mgm",BaseFileName(TrackListFileName));
	else
		{ printf("Please indicate name for MuogramFile\nExit\n"); exit(ErrMissingInfo); }
	}

void Help(int ExitCode)
	{
	system("cat Help/HelpFillMuogram.txt");
	exit(ExitCode);
	}







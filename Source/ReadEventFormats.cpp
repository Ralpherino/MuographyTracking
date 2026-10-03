////////////////////////////////////////////////////////////////////////
// Event reader for different formats - implemantaions
//	This piece of code is a part of the Tracking.cpp
//		moved to separate file for a more convenient operation
//      version : 2021-04-06
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

//#include "ReadEventFormats.h"

bool ReadEvent_Format_20201204()
	{
	char	TempString[200];
	// The first data is the EventNumber
	EventN=EventFirstData;
	// DateTime info
	fscanf(EbeFile,"%s %s %ld",TempString,EventDateTime,&EventTimeDiffUs);
	EventTimeAbsUs+=EventTimeDiffUs;
	if (strcmp(TempString,"T")!=0) 
		{
		printf("\n\tUnrecognized string (%s!=T) in EbeFile: %s, at event %ld (%d)\n\t Read new event\n",TempString,EbeFileName,EventN,NEvent);
		//SkipLine(EbeFile,1);
		//return ReadNextEvent();  // would be nicer to add an error counter as well
		return false;
		}
	if (ShallCorrectDateTime) CorrectDateTime();
	// ADCs
	if (MaxNAdcs>0) fscanf(EbeFile,"%*s");
	for (int NAdc=0; NAdc<MaxNAdcs; NAdc++) fscanf(EbeFile,"%d",pAdc[NAdc]);
	// THP
	if (MaxNThps>0) fscanf(EbeFile,"%*s");
	for (int NThp=0; NThp<MaxNThps; NThp++) 
		{ fscanf(EbeFile,"%lf %lf %lf %d",&(Thp[NThp]->T),&(Thp[NThp]->H),&(Thp[NThp]->P),&(Thp[NThp]->Id)); Thp[NThp]->SetDt(EventDateTime); }
	// Counter
	fscanf(EbeFile,"%*s");
	fscanf(EbeFile,"%d %*d",&CounterDiff);
	// Trigger Pattern
	fscanf(EbeFile,"%*s %d",&TriggerPattern);
	for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++)
		*(pTriggerBit[NTriggerBit]) = (TriggerPattern>>NTriggerBit)%2;
	// HvControl
	if (HvInfo!=NULL)
		{ fscanf(EbeFile,"%*s %lf %lf %lf %*s %*f",&(HvInfo->UIseg),&(HvInfo->UMon),&(HvInfo->IMon)); HvInfo->SetDt(EventDateTime); }
		// TODO : the LV info is here, where should it be put ?
	// Check
	int NChecksFailed=0;
	if (MaxNChecks>0) fscanf(EbeFile,"%*s %d",&NChecksFailed); //Cf
	for (int NCf=0; NCf<NChecksFailed; NCf++) fscanf(EbeFile,"%*d");
	// Q: how to handle check bits
	// Chamber hits
	fscanf(EbeFile,"%s",TempString);
	if (strcmp(TempString,"Ch")!=0) { printf("Unrecognized string (%s!=Ch) in EbeFile: %s, at event %ld\n",TempString,EbeFileName,EventN); return false; }
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		fscanf(EbeFile,"%d",&(Chamber[NChamber]->NHitsX));
		for (int NHitX=0; NHitX<Chamber[NChamber]->NHitsX; NHitX++)
			fscanf(EbeFile,"%d",&(Chamber[NChamber]->HitPositionX[NHitX]));
		fscanf(EbeFile,"%d",&(Chamber[NChamber]->NHitsY));
		for (int NHitY=0; NHitY<Chamber[NChamber]->NHitsY; NHitY++)
			fscanf(EbeFile,"%d ",&(Chamber[NChamber]->HitPositionY[NHitY]));
		}
	return true;
	}

bool ReadEvent_Format_20181015()
	{
	char	TempString[200];
	// The first data is the EventNumber
	EventN=EventFirstData;
	// DateTime info
	fscanf(EbeFile,"%s %s %ld",TempString,EventDateTime,&EventTimeDiffUs);
	if (EventTimeDiffUs<0 || EventTimeDiffUs>3500*1e6) EventTimeDiffUs=0;
	EventTimeAbsUs+=EventTimeDiffUs;
	if (UseDiffTimeAsDateTimeInfo)
		{
		// if the absolute time info is not reliable, the sum of microsends can be used for time evolution
		// TimeZero == 2000-01-01_00:00:00
		DateTimeTm.tm_year=100; DateTimeTm.tm_mon=0; DateTimeTm.tm_mday=1;
		DateTimeTm.tm_hour=0; DateTimeTm.tm_min=0;
		DateTimeTm.tm_sec=EventTimeAbsUs/1000000;
		mktime(&DateTimeTm);
		sprintf(EventDateTime,"%04d-%02d-%02d_%02d:%02d:%02d",DateTimeTm.tm_year+1900,DateTimeTm.tm_mon+1,DateTimeTm.tm_mday,DateTimeTm.tm_hour,DateTimeTm.tm_min,DateTimeTm.tm_sec);
		}
	if (strcmp(TempString,"T")!=0) 
		{
		printf("\n\tUnrecognized string (%s!=T) in EbeFile: %s, at event %ld (%d)\n\t Read new event\n",TempString,EbeFileName,EventN,NEvent);
		//SkipLine(EbeFile,1);
		//return ReadNextEvent();  // would be nicer to add an error counter as well
		return false;
		}
	if (ShallCorrectDateTime) CorrectDateTime();
	// ADCs
	if (MaxNAdcs>0) fscanf(EbeFile,"%*s");
	for (int NAdc=0; NAdc<MaxNAdcs; NAdc++) fscanf(EbeFile,"%d",pAdc[NAdc]);
	// THP
	if (MaxNThps>0) fscanf(EbeFile,"%*s");
	for (int NThp=0; NThp<MaxNThps; NThp++) 
		{ fscanf(EbeFile,"%lf %lf %lf %d",&(Thp[NThp]->T),&(Thp[NThp]->H),&(Thp[NThp]->P),&(Thp[NThp]->Id)); Thp[NThp]->SetDt(EventDateTime); }
	// Counter
	fscanf(EbeFile,"%*s");
	fscanf(EbeFile,"%d %*d",&CounterDiff);
	// Check
	if (MaxNChecks>0) fscanf(EbeFile,"%*s");
	for (int NCheck=0; NCheck<MaxNChecks; NCheck++) fscanf(EbeFile,"%*d");
	// Q: how to handle check bits
	// Trigger Pattern
	fscanf(EbeFile,"%*s %d",&TriggerPattern);
	for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++)
		*(pTriggerBit[NTriggerBit]) = (TriggerPattern>>NTriggerBit)%2;
	// HvControl
	if (HvInfo!=NULL)
		{ fscanf(EbeFile,"%*s %lf %lf %lf",&(HvInfo->UIseg),&(HvInfo->UMon),&(HvInfo->IMon)); HvInfo->SetDt(EventDateTime); }
	// Chamber hits
	fscanf(EbeFile,"%s",TempString);
	if (strcmp(TempString,"Ch")!=0) { printf("Unrecognized string (%s!=Ch) in EbeFile: %s, at event %ld\n",TempString,EbeFileName,EventN); return false; }
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		fscanf(EbeFile,"%d",&(Chamber[NChamber]->NHitsX));
		for (int NHitX=0; NHitX<Chamber[NChamber]->NHitsX; NHitX++)
			fscanf(EbeFile,"%d",&(Chamber[NChamber]->HitPositionX[NHitX]));
		fscanf(EbeFile,"%d",&(Chamber[NChamber]->NHitsY));
		for (int NHitY=0; NHitY<Chamber[NChamber]->NHitsY; NHitY++)
			fscanf(EbeFile,"%d ",&(Chamber[NChamber]->HitPositionY[NHitY]));
		}
	// Simulation Info array
	if (MaxNSimulationData>0)
		{
		fscanf(EbeFile,"%*s");
		for (int NSimData=0; NSimData<MaxNSimulationData; NSimData++)
			fscanf(EbeFile,"%lf",&(SimulationData[NSimData]));
		}
	return true;
	}


bool ReadEvent_Format_2014()
	{
	EventN=EventFirstData;
	// DateTime info
	fscanf(EbeFile,"%ld",&EventTimeDiffUs);
	EventTimeAbsUs+=EventTimeDiffUs;
	// No absolute time info is recorded, however the sum of microsends can show the time evolution
	// TimeZero == 2000-01-01_00:00:00
	DateTimeTm.tm_year=100; DateTimeTm.tm_mon=0; DateTimeTm.tm_mday=1;
	DateTimeTm.tm_hour=0; DateTimeTm.tm_min=0;
	DateTimeTm.tm_sec=EventTimeAbsUs/1000000;
	mktime(&DateTimeTm);
	sprintf(EventDateTime,"%04d-%02d-%02d_%02d:%02d:%02d",DateTimeTm.tm_year+1900,DateTimeTm.tm_mon+1,DateTimeTm.tm_mday,DateTimeTm.tm_hour,DateTimeTm.tm_min,DateTimeTm.tm_sec);
	if (ShallCorrectDateTime) CorrectDateTime();
	// Chamber hits
	int NHits, HitPos;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		Chamber[NChamber]->NHitsX=0; Chamber[NChamber]->NHitsY=0;
		fscanf(EbeFile,"%d",&NHits);
		for (int NHit=0; NHit<NHits; NHit++)
			{
			fscanf(EbeFile,"%d",&HitPos);
			if (HitPos<Chamber[NChamber]->SizeX)
				{ Chamber[NChamber]->HitPositionX[Chamber[NChamber]->NHitsX]=HitPos; Chamber[NChamber]->NHitsX++; }
			else 
				{ Chamber[NChamber]->HitPositionY[Chamber[NChamber]->NHitsY]=HitPos-Chamber[NChamber]->SizeX; Chamber[NChamber]->NHitsY++; }
			}
		}
	// Counter
	fscanf(EbeFile,"%d",&CounterDiff);
	// ??
	fscanf(EbeFile,"%*d");
	// Trigger Pattern
	// ????? TODO : CHECK
	fscanf(EbeFile,"%d",&TriggerPattern);
	for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++)
		*(pTriggerBit[NTriggerBit]) = (TriggerPattern>>NTriggerBit)%2;
	fscanf(EbeFile,"%*d ");
	return true;
}


bool ReadEvent_Format_Mts4()
	{
	EventN=EventFirstData;
	// DateTime info
	fscanf(EbeFile,"%ld",&EventTimeDiffUs);
	EventTimeAbsUs+=EventTimeDiffUs;
	// No absolute time info is recorded, however the sum of microsends can show the time evolution
	// TimeZero == 2000-01-01_00:00:00
	DateTimeTm.tm_year=100; DateTimeTm.tm_mon=0; DateTimeTm.tm_mday=1;
	DateTimeTm.tm_hour=0; DateTimeTm.tm_min=0;
	DateTimeTm.tm_sec=EventTimeAbsUs/1000000;
	mktime(&DateTimeTm);
	sprintf(EventDateTime,"%04d-%02d-%02d_%02d:%02d:%02d",DateTimeTm.tm_year+1900,DateTimeTm.tm_mon+1,DateTimeTm.tm_mday,DateTimeTm.tm_hour,DateTimeTm.tm_min,DateTimeTm.tm_sec);
	if (ShallCorrectDateTime) CorrectDateTime();
	// Chamber hits
	int NHits, HitPos;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		Chamber[NChamber]->NHitsX=0; Chamber[NChamber]->NHitsY=0;
		fscanf(EbeFile,"%d",&NHits);
		for (int NHit=0; NHit<NHits; NHit++)
			{
			fscanf(EbeFile,"%d",&HitPos);
			if (HitPos<Chamber[NChamber]->SizeX)
				{ Chamber[NChamber]->HitPositionX[Chamber[NChamber]->NHitsX]=HitPos; Chamber[NChamber]->NHitsX++; }
			else 
				{ Chamber[NChamber]->HitPositionY[Chamber[NChamber]->NHitsY]=HitPos-Chamber[NChamber]->SizeX; Chamber[NChamber]->NHitsY++; }
			}
		}
	// Counter
	fscanf(EbeFile,"%d",&CounterDiff);
	// ??
	fscanf(EbeFile,"%*d");
	// Trigger Pattern
	// ????? TODO : CHECK
	fscanf(EbeFile,"%d",&TriggerPattern);
	for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++)
		*(pTriggerBit[NTriggerBit]) = (TriggerPattern>>NTriggerBit)%2;
	return true;
}

bool ReadEvent_Format_PicMt2()
	{
	EventN=EventFirstData;
	// DateTime info
	fscanf(EbeFile,"%ld",&EventTimeDiffUs);
	EventTimeAbsUs+=EventTimeDiffUs;
	// No absolute time info is recorded, however the sum of microsends can show the time evolution
	// TimeZero == 2000-01-01_00:00:00
	DateTimeTm.tm_year=100; DateTimeTm.tm_mon=0; DateTimeTm.tm_mday=1;
	DateTimeTm.tm_hour=0; DateTimeTm.tm_min=0;
	DateTimeTm.tm_sec=EventTimeAbsUs/1000000;
	mktime(&DateTimeTm);
	sprintf(EventDateTime,"%04d-%02d-%02d_%02d:%02d:%02d",DateTimeTm.tm_year+1900,DateTimeTm.tm_mon+1,DateTimeTm.tm_mday,DateTimeTm.tm_hour,DateTimeTm.tm_min,DateTimeTm.tm_sec);
	if (ShallCorrectDateTime) CorrectDateTime();
	// Chamber hits
	int DecCode, NBitsInCode=16;
	for (int NChamber=0; NChamber<MaxNChambers; NChamber++)
		{
		Chamber[NChamber]->NHitsX=0; Chamber[NChamber]->NHitsY=0;
		// Readout starts with "Y" to have the orientation as standard
		for (int NCode=0; NCode<Chamber[NChamber]->SizeY/NBitsInCode; NCode++)
			{
			fscanf(EbeFile,"%d",&DecCode);
			for (int NBit=0; NBit<NBitsInCode; NBit++)
				if (GetBit(DecCode,NBitsInCode-1-NBit)==1) 
					{ Chamber[NChamber]->HitPositionY[Chamber[NChamber]->NHitsY]=NCode*NBitsInCode+NBit; Chamber[NChamber]->NHitsY++; }
			}
		for (int NCode=0; NCode<Chamber[NChamber]->SizeX/NBitsInCode; NCode++)
			{
			fscanf(EbeFile,"%d",&DecCode);
			for (int NBit=0; NBit<NBitsInCode; NBit++)
				if (GetBit(DecCode,NBitsInCode-1-NBit)==1) // if forward, replace with : (GetBit(DecCode,NBit)==1)
					{ Chamber[NChamber]->HitPositionX[Chamber[NChamber]->NHitsX]=NCode*NBitsInCode+NBit; Chamber[NChamber]->NHitsX++; }
			}
		}
	// Trigger Pattern ?
	fscanf(EbeFile,"%d %*d",&TriggerPattern);
	for (int NTriggerBit=0; NTriggerBit<MaxNTriggerBits; NTriggerBit++)
		*(pTriggerBit[NTriggerBit]) = (TriggerPattern>>(NBitsInCode-1-NTriggerBit))%2;
	// Laca : AnalogData 0-9
	//mt2_2011_2012.cpp:	Pressure[TimeBin] = ActualEvent->AnalogData[0];
	//mt2_2011_2012.cpp:	Hum_Board[TimeBin] = ActualEvent->AnalogData[1]; Hum_Out[TimeBin] = ActualEvent->AnalogData[2]; Hum_In[TimeBin] = ActualEvent->AnalogData[3];	
	//mt2_2011_2012.cpp:	Battery[TimeBin] = ActualEvent->AnalogData[4]; HighVoltage[TimeBin] = ActualEvent->AnalogData[5]; HighCurrent[TimeBin] = ActualEvent->AnalogData[6];
	//mt2_2011_2012.cpp:	Temp_Board[TimeBin] = ActualEvent->AnalogData[7]; Temp_Out[TimeBin] = ActualEvent->AnalogData[8]; Temp_In[TimeBin] = ActualEvent->AnalogData[9];
	// The REAL conversion is not known...  assuming a 10**x factor have been used below
	int NAnalogData=10; double AnalogData[10];
	for (int NAData=0; NAData<NAnalogData; NAData++) fscanf(EbeFile,"%lf ",&(AnalogData[NAData]));
	//for (int NAData=0; NAData<NAnalogData; NAData++) printf(" %g ",AnalogData[NAData]/1000.); printf("\n");
	if (MaxNThps>0) { Thp[0]->T=AnalogData[9]/1000.; Thp[0]->H=AnalogData[3]/1000.; Thp[0]->P=AnalogData[0]/100.; Thp[0]->SetDt(EventDateTime); }
	if (MaxNThps>1) { Thp[1]->T=AnalogData[8]/1000.; Thp[1]->H=AnalogData[2]/1000.; Thp[1]->P=AnalogData[0]/100.; Thp[1]->SetDt(EventDateTime); }
	if (HvInfo!=NULL) { HvInfo->UIseg=AnalogData[4]/10.; HvInfo->UMon=AnalogData[5]; HvInfo->IMon=AnalogData[6]; HvInfo->SetDt(EventDateTime); }
	return true;
	}




////////////////////////////////////////////////////////////////////////
// HighVoltage info Class - implementation
//      version : 2019-03-19
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "HvClass.h"

HvClass::HvClass(double TimeBinSize, int AutoScaleLimit)
	{
	TimeDev = new MultiHistDynTimeClass(4,TimeBinSize,(char*)"HvTimeDev");
	TimeDev->SetName(IdN,(char*)"N");
	TimeDev->SetName(IdUIseg,(char*)"UIsegxN");
	TimeDev->SetName(IdUMon,(char*)"UMonxN");
	TimeDev->SetName(IdIMon,(char*)"IMonxN");
	TimeDev->SetAutoScale(AutoScaleLimit);
	Id=0;
	}

HvClass::~HvClass()
	{
	delete TimeDev;
	}

void HvClass::SetDt(char* InDateTime)
	{
	if ( (UIseg<-100) || (UMon<-100) ) return; 	// not valid
	TimeDev->Add(IdN,InDateTime);
	TimeDev->Add(IdUIseg,InDateTime,UIseg);
	TimeDev->Add(IdUMon,InDateTime,UMon);
	TimeDev->Add(IdIMon,InDateTime,IMon);
	}

void HvClass::SetDtHv(char* InDateTime, double InUIseg, double InUMon, double InIMon)
	{
	UIseg=InUIseg;
	UMon=InUMon;
	IMon=InIMon;
	SetDt(InDateTime);
	}

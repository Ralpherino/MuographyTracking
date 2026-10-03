////////////////////////////////////////////////////////////////////////
// Temperature-Humidity-Pressure Class - implementation
//      version : 2019-03-19
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "ThpClass.h"

ThpClass::ThpClass(double TimeBinSize, int AutoScaleLimit)
	{
	TimeDev = new MultiHistDynTimeClass(4,TimeBinSize,(char*)"ThpTimeDev");
	TimeDev->SetName(IdN,(char*)"N");
	TimeDev->SetName(IdT,(char*)"TxN");
	TimeDev->SetName(IdH,(char*)"HxN");
	TimeDev->SetName(IdP,(char*)"PxN");
	TimeDev->SetAutoScale(AutoScaleLimit);
	}

ThpClass::~ThpClass()
	{
	delete TimeDev;
	}

void ThpClass::SetDt(char* InDateTime)
	{
	if ( (T<-40) || (H<1) ) return; // not valid
	TimeDev->Add(IdN,InDateTime);
	TimeDev->Add(IdT,InDateTime,T);
	TimeDev->Add(IdH,InDateTime,H);
	TimeDev->Add(IdP,InDateTime,P);
	}

void ThpClass::SetDtThp(char* InDateTime,double InT, double InH, double InP)
	{
	T=InT; H=InH; P=InP;
	SetDt(InDateTime);
	}

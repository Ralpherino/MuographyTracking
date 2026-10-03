////////////////////////////////////////////////////////////////////////
// DateTime Class - implementation
//      version : 2020-11-03
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "DateTimeClass.h"

DateTimeClass::DateTimeClass()
	{
	Set();
	MakeString();
	}

DateTimeClass::~DateTimeClass()
	{
	}

void DateTimeClass::ReadString(char* InString)
	{
	if (InString!=NULL) sprintf(DtString,"%s",InString);
	sscanf(DtString,"%04d-%02d-%02d_%02d:%02d:%02d",&Year,&Month,&Day,&Hour,&Min,&Sec);	
	}

void DateTimeClass::MakeString(char * OutString)
	{
	sprintf(DtString,"%04d-%02d-%02d_%02d:%02d:%02d",Year,Month,Day,Hour,Min,Sec);
	if (OutString!=NULL) sprintf(OutString,"%s",DtString);
	}

void DateTimeClass::Set(int InYear, int InMonth, int InDay, int InHour, int InMin, int InSec)
	{
	Year=InYear; Month=InMonth; Day=InDay;
	Hour=InHour; Min=InMin; Sec=InSec;
	}

void DateTimeClass::AddDateTime(DateTimeClass* DtAdd)
	{
	if (DtAdd->Year>0) { DtAdd->Day+=int(DtAdd->Year*365.25); DtAdd->Year=0; }
	if (DtAdd->Month>0) { DtAdd->Day+=int(DtAdd->Month*30.43); DtAdd->Month=0; } // Shall be tricked unfortunately... not trivial why..
	DateTimeTm.tm_year=Year+DtAdd->Year-1900; DateTimeTm.tm_mon=Month+DtAdd->Month-1; DateTimeTm.tm_mday=Day+DtAdd->Day;
	DateTimeTm.tm_hour=Hour+DtAdd->Hour; DateTimeTm.tm_min=Min+DtAdd->Min; DateTimeTm.tm_sec=Sec+DtAdd->Sec;
	mktime(&DateTimeTm);
	Year=DateTimeTm.tm_year+1900; Month=DateTimeTm.tm_mon+1; Day=DateTimeTm.tm_mday;
	Hour=DateTimeTm.tm_hour; Min=DateTimeTm.tm_min; Sec=DateTimeTm.tm_sec;
	MakeString();
	}



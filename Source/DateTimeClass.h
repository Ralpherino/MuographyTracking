////////////////////////////////////////////////////////////////////////
// DateTime Class - header
//      version : 2020-11-03
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef DateTimeClassGuard
#define DateTimeClassGuard

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

class DateTimeClass
	{ public:
	DateTimeClass();
	~DateTimeClass();

	time_t	DateTimeTimet;
	struct tm DateTimeTm;
	char	DtString[50];
	int	Year,Month,Day,Hour,Min,Sec;
	
	// Functions
	void	ReadString(char* InString=NULL);
	void	MakeString(char* OutString=NULL);
	void	Set(int InYear=0, int InMonth=0, int InDay=0, int InHour=0, int InMin=0, int InSec=0);
	void	AddDateTime(DateTimeClass* DtAdd);

	};

#endif


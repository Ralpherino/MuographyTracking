////////////////////////////////////////////////////////////////////////
// Event Class - header
//      version : 2018-01-06
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef EventClassGuard
#define EventClassGuard

#include <stdio.h>
#include <stdlib.h>

class EventClass
	{ public:
	EventClass();
	~EventClass();
	
	//FILE*	SettFile;
	//void	SetSettFile(FILE*);
	FILE	*EbeFile;
	void	SetEbeFile(FILE*);
	bool	ReadNext();
	
	};

#endif


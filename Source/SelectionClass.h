////////////////////////////////////////////////////////////////////////
// Selection Class - header
// 	to select the combination of clusters from Chambers
//      version : 2019-02-25
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef	SelectionClassGuard
#define	SelectionClassGuard

#include <stdio.h>
#include <stdlib.h>

struct PointStruct { int n,c; };

class SelectionClass
	{ public:
	SelectionClass();
	SelectionClass(int InN=0, int InK=0);
	~SelectionClass();
	int	N,K;
	int	*Max;
	PointStruct *P;
	bool	Valid;
	void Set(int InN, int InK);
	void SetArray(int *InMax);
	void Increment();
	void IncrementS(int s);
	void Reset(int s, int n);
	void Print();
	void PrintAll();
	int  GetChamber(int s);
	int  GetCluster(int s);
	bool IsValid();
	};

class SelectionMinClass
	{ public:
	SelectionMinClass(int InN, int InK);
	~SelectionMinClass();
	
	void SetArray(int *NClustersArray);
	void NextSelection();
	void Increment();
	bool IsValid();
	int  GetChamber(int s);
	int  GetCluster(int s);
	
	int	N,KMin,K;
	bool	Valid;
	int	NSelections;
	SelectionClass	**Selection;
	};

# endif


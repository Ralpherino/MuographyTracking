////////////////////////////////////////////////////////////////////////
// Combination Class - header
//      version : 2018-03-01
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef CombinationClassGuard
#define CombinationClassGuard

#include <stdio.h>
#include <stdlib.h>

class CombinationClass
	{ public:
	CombinationClass() {};
	CombinationClass(int InN, int InK);
	int	N,K;//Collection, Selection;
	int	*S;
	bool	Valid;
	int	NCombinations;
	void	SetK(int InK);
	void	CalculateNCombinations();
	void	Increment();
	void	IncrementIndex(int i);
	void	NoMoreCombinations();
	int	Get(int i);
	void	GetArray(int* Array);
	void	GetSelectionArray(int* Array);
	void	Print();
	void	PrintAll();
	void	Reset();
	};

class CombinationMinClass : public CombinationClass
	{ public:
	CombinationMinClass(int InN, int InKMin);

	int	KMin;
	
	void	CalculateNCombinations();
	void	Increment();
	void	Reset();
	};


// Functions
int Factorial(int N);
int Binomial(int N, int K);


#endif




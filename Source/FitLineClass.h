////////////////////////////////////////////////////////////////////////
//  FitLineClass - header
//  	for fitting a line
//	verion : 2015
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#ifndef FitLineClassGuard
#define FitLineClassGuard

#include <stdio.h>
#include <math.h>


class FitLineClass
	{ public:
	FitLineClass();
	~FitLineClass();
	
	void	Null();
	void	Add(double InX, double InY, double InWeight=1.0);
	bool	Fit();
	double	GetSlope();
	double	GetB();
	double	GetError(), GetError2();
	double	GetEntries();
	void	PutParams(double *pInM, double *pInB);
	double	ComputeY(double InX);
	void	Print();
	bool	IsFitted();
	
	private:
	bool	fitted;
	double	x,y,Sx,Sy,Sxx,Sxy,Syy,N;
	double	m,b,err,err2;
	};

#endif


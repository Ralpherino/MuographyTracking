////////////////////////////////////////////////////////////////////////
// VectorClass - header
//	for operating with 3dim vectors
//      version : 2018-01-11
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef VectorClassGuard
#define VectorClassGuard

class VectorClass
	{ public:
	VectorClass();
	VectorClass(double InX, double InY, double InZ);
	~VectorClass();
	
	double	X,Y,Z;
	// Functions
	void	SetXyz(double InX, double InY, double InZ);
	void	SetThetaPhiR(double InTheta, double InPhi, double InR);
	void	SetZenithAzimuth(double InZDeg, double InADeg);
	void	Copy(VectorClass InV);
	void	RotX(double Alpha), RotY(double Alpha), RotZ(double Alpha);
	void	RotAxis(char Axis, double Alpha);
	double	Length();
	void	Norm(double N=1.0);
	double	GetTheta(), GetPhi();
	double	GetMx(), GetMy();
	void	ReadXyzFromFile(FILE* InFile);
	void	WriteToScreen();
	// Operators test
	friend VectorClass operator+(VectorClass V1, VectorClass V2);
	friend VectorClass operator-(VectorClass V1, VectorClass V2);
	friend VectorClass operator*(VectorClass V1, double a);
	friend VectorClass operator/(VectorClass V1, VectorClass V2);
	friend double operator%(VectorClass V1, VectorClass V2);
	friend bool operator||(VectorClass V1, VectorClass V2);
	};


#endif


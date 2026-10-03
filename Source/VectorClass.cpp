////////////////////////////////////////////////////////////////////////
// VectorClass - implementation
//	for operating with 3dim vectors
//      version : 2019-01-25
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "VectorClass.h"

VectorClass::VectorClass()
	{
	X=0;Y=0;Z=1;
	}

VectorClass::VectorClass(double InX, double InY, double InZ)
	{
	SetXyz(InX,InY,InZ);
	}

VectorClass::~VectorClass()
	{
	}

// Set Vector
void VectorClass::SetXyz(double InX, double InY, double InZ)
	{
	X=InX; Y=InY; Z=InZ;
	}

void VectorClass::SetThetaPhiR(double InTheta, double InPhi, double InR)
	{
	Z=InR*cos(InTheta);
	X=InR*sin(InTheta)*cos(InPhi);
	Y=InR*sin(InTheta)*sin(InPhi);
	}

void VectorClass::SetZenithAzimuth(double InZDeg, double InADeg)
	{
	double rd=180/3.1415;
	SetThetaPhiR(InZDeg/rd,(90-InADeg)/rd,1);
	}

void VectorClass::Copy(VectorClass InV)
	{
	X=InV.X; Y=InV.Y; Z=InV.Z;
	}

// Length
double VectorClass::Length()
	{
	return sqrt(X*X+Y*Y+Z*Z);
	}

void VectorClass::Norm(double N)
	{
	double L=Length();
	if (L==0) return;
	X/=L; Y/=L; Z/=L;
	}

// Rotations
void VectorClass::RotX(double Alpha)
	{
	double y=Y; double z=Z;
	Y=cos(Alpha)*y+sin(Alpha)*z;
	Z=cos(Alpha)*z-sin(Alpha)*y;
	}
	
void VectorClass::RotY(double Alpha)
	{
	double z=Z; double x=X;
	Z=cos(Alpha)*z+sin(Alpha)*x;
	X=cos(Alpha)*x-sin(Alpha)*z;
	}

void VectorClass::RotZ(double Alpha)
	{
	double x=X; double y=Y;
	X=cos(Alpha)*x+sin(Alpha)*y;
	Y=cos(Alpha)*y-sin(Alpha)*x;
	}

void VectorClass::RotAxis(char Axis, double Alpha)
	{
	if (Axis=='x' || Axis=='X') RotX(Alpha);
	else if (Axis=='y' || Axis=='Y') RotY(Alpha);
	else if (Axis=='z' || Axis=='Z') RotZ(Alpha);
	else printf("VectorClass: Incorrect axis : .%c.\n",Axis);
	}

// Get Info
double VectorClass::GetTheta()
	{
	return atan2(sqrt(X*X+Y*Y),Z);
	}

double VectorClass::GetPhi()
	{
	return atan2(Y,X);
	}

double VectorClass::GetMx()
	{
	return X/Z;
	}

double VectorClass::GetMy()
	{
	return Y/Z;
	}


// Write

void VectorClass::WriteToScreen()
	{
	printf("Vector: XYZ: %g %g %g\n",X,Y,Z);
	}


// File operations

void VectorClass::ReadXyzFromFile(FILE* InFile)
	{
	fscanf(InFile,"%lf %lf %lf ",&X,&Y,&Z);
	}


// Operators

VectorClass operator+(VectorClass V1, VectorClass V2)
	{
	VectorClass Result;
	Result.X=V1.X+V2.X;
	Result.Y=V1.Y+V2.Y;
	Result.Z=V1.Z+V2.Z;
	return Result;
	}

VectorClass operator-(VectorClass V1, VectorClass V2)
	{
	VectorClass Result;
	Result.X=V1.X-V2.X;
	Result.Y=V1.Y-V2.Y;
	Result.Z=V1.Z-V2.Z;
	return Result;
	}

VectorClass operator*(VectorClass V1, double a)
	{
	VectorClass Result;
	Result.X=V1.X*a;
	Result.Y=V1.Y*a;
	Result.Z=V1.Z*a;
	return Result;
	}

double operator%(VectorClass V1, VectorClass V2)
	{
	return (V1.X*V2.X + V1.Y*V2.Y + V1.Z*V2.Z);
	}

VectorClass operator/(VectorClass V1, VectorClass V2)
	{
	VectorClass Result;
	Result.X = V1.Y*V2.Z - V1.Z*V2.Y;
	Result.Y = V1.Z*V2.X - V1.X*V2.Z;
	Result.Z = V1.X*V2.Y - V1.Y*V2.X;
	return Result;
	}

bool operator==(VectorClass V1, VectorClass V2)
	{
	return ((V1.X==V2.X) && (V1.Y==V2.Y) && (V1.Z==V2.Z) );
	}

bool operator||(VectorClass V1, VectorClass V2)
	{
	if (V1.Length()==0) return true;
	double L=V2.Length()/V1.Length();
	if ((V2*L)==V1) return true;
	return false;
	}

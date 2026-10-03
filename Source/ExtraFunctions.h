////////////////////////////////////////////////////////////////////////
// Extra Functions - header
//	written by : Gergo" Hamar
// 	version: 2020-05-03
////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

#ifndef ExtraFunctionsGuard
#define ExtraFunctionsGuard

////////////////////////////////////////////////////////////////////////
// Coinstant definitions
#define PI 3.1415926535897932384626433832795
////////////////////////////////////////////////////////////////////////
// Practical variables
extern int CountArray[21];
extern int NullInt; extern long NullLong;
extern double NullDouble; extern float NullFloat;
extern char NullChar;
////////////////////////////////////////////////////////////////////////
// Simple functions reinterpreted
double FunctionAdd(double,double);
double FunctionSubtract(double,double);
double FunctionMultiply(double,double);
double FunctionDivide(double,double);
double FunctionOneMinusX(double);
double FunctionDivideWithSquareRoot(double,double);
double FunctionDivideIfElse(double,double,double,double);
double FunctionPositiveThan(double,double);
double GetMin(double,double);
double GetMax(double,double);
////////////////////////////////////////////////////////////////////////
// Sort with a bubble sort algorithm
void SortArrayDescending(double * a, int size);
void SortArrayAscending(double * a, int size);
void SortDouble(double arr[], int beg, int end);
void SwapDouble(double * a, double * b);
int FilterArrayUnique(double * arr, int n, double resolution=0.0000001);
void ShiftArrayLeft(double * arr, int n);
////////////////////////////////////////////////////////////////////////
// Practical functions
double	Interpolate(double x1, double y1, double x2, double y2, double x);
double	PoissonMax(double n);
double	PoissonMin(double n);
bool	GetYesNoAnswer(char * Answer);
double	GetPostfixedNumber(char* NumberString);
bool	SkipBlockInFile(FILE * InFile);
bool	CopyBlockInFileOutFile(FILE * InFile, FILE* OutFile);
void	RefineFileName(char* FileName, char* Dir, char* Extension);
void	RemoveDuplicatesFromArray(double *a, int size, int * newsize);
void	ArrayWithout(int ArraySize, double *ArrayOriginal, int NSkip, double *ArrayOut);
void	ArrayWithout(int ArraySize, int *ArrayOriginal, int NSkip, int *ArrayOut);
int	SkipLine(FILE* InFile, int NLines=1);
char*	NiceDurationSec(int TSec, int Depth=2);
int	CountWordsInString(std::string ThisString);
void	ReadStringToArrayInt(char* InString, int ArrayLength, int* Array, bool LeadingString=false);
char*	BaseFileName(char* InPathFileName);
void	GetBaseName(char* InPathFileName, char* InBaseName);
////////////////////////////////////////////////////////////////////////
// Bitwise
int GetBit(int BitCode, int WhichBit);

#endif



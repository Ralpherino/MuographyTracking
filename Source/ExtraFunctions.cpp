////////////////////////////////////////////////////////////////////////
// Extra Functions - header
//	written by : Gergo" Hamar
// 	version: 2020-05-03
////////////////////////////////////////////////////////////////////////

#include "ExtraFunctions.h"

#include <stdio.h>
#include <math.h>

////////////////////////////////////////////////////////////////////////
// Practical variables
int CountArray[21]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int NullInt=0; long NullLong=0;
double NullDouble=0; float NullFloat=0;
char NullChar='.';
////////////////////////////////////////////////////////////////////////
// Simple functions reinterpreted

double FunctionAdd(double x, double y)
	{ return (x+y); }

double FunctionSubtract(double x, double y)
	{ return (x-y); }

double FunctionMultiply(double x ,double y)
	{ return x*y; }

double FunctionDivide(double x, double y)
	{
	if (y==0) return 0;
	return x/y;
	}

double FunctionOneMinusX(double x)
	{ return (1.0-x); }

double FunctionDivideWithSquareRoot(double x, double y)
	{
	if (y<=0) return 0;
	return x/sqrt(y);
	}

double FunctionDivideIfElse(double x, double y, double ymin, double velse)
	{
	if (ymin<0) { printf("Warning: FunctionDivideIfElse :: YMin shall be larger than Zero, %g\n",ymin); return FunctionDivide(x,y); }
	if (y>ymin) return x/y;
	else return velse;
	}

double FunctionPositiveThan(double x, double y)
	{
	if (x>0) return y;
	else return 0;
	}

double GetMin(double x, double y)
	{ if (x<y) return x; else return y; }

double GetMax(double x, double y)
	{ if (x<y) return y; else return x; }

////////////////////////////////////////////////////////////////////////
// Sort with a bubble sort algorithm

void SortArrayAscending(double * a, int size)
	{
	SortArrayDescending(a,size);
	for (int n=0; n<size/2; n++) SwapDouble(&(a[n]),&(a[size-1-n]));
	}

void SortArrayDescending(double * a, int size)
	{
	SortDouble(a,0,size);
	}

void SwapDouble(double * a, double * b)
	{
	double t=*a; *a=*b; *b=t;
	}

void SortDouble(double arr[], int beg, int end)
	{
	if (end > beg + 1)
		{
		double piv = arr[beg];
		int l = beg + 1, r = end;
		while (l < r)
			{
			if (arr[l] >= piv) l++;
			else SwapDouble(&arr[l], &arr[--r]);
			}
		SwapDouble(&arr[--l], &arr[beg]);
		SortDouble(arr, beg, l);
		SortDouble(arr, r, end);
		}
	}

int FilterArrayUnique(double * arr, int n, double resolution)
	{
	for (int i=1; i<n; i++)
		if ( fabs(arr[i]-arr[i-1])<resolution )
			{
			ShiftArrayLeft( &(arr[i]),n-i);
			i--;
			n--;
			}
	return n;
	}

void ShiftArrayLeft(double * arr, int n)
	{
	for (int i=0;i<n-1;i++) arr[i]=arr[i+1];
	}

////////////////////////////////////////////////////////////////////////
// Practical functions

double Interpolate(double x1, double y1, double x2, double y2, double x)
	{
	if (x1==x2) return ((y1+y2)/2.0);
	
	if (x1>x2)
		{
		double xx=x1; x1=x2; x2=xx;
		double yy=y1; y1=y2; y2=yy;
		}
	
	double w1=(x2-x)/(x2-x1);
	double w2=(x-x1)/(x2-x1);
	return (y1*w1+y2*w2);
	}

double PoissonMax(double n)
	{
	if (n<0) return n;
	return (n+sqrt(n)); 
	}
	
double PoissonMin(double n)
	{
	if (n<0) return n;
	return (n-sqrt(n)); 
	}

bool GetYesNoAnswer(char * Answer)
	{
	if ( strcmp(Answer,"Y")==0 || strcmp(Answer,"y")==0 || strcmp(Answer,"yes")==0 || \
		strcmp(Answer,"Yes")==0 || strcmp(Answer,"YES")==0  || strcmp(Answer,"1")==0 )
		return true;
	if ( strcmp(Answer,"N")==0 || strcmp(Answer,"n")==0 || strcmp(Answer,"no")==0 || \
		strcmp(Answer,"No")==0 || strcmp(Answer,"NO")==0  || strcmp(Answer,"0")==0 )
		return false;
	printf("Warning! Incorrect yes/no answer: %s\n",Answer);
	return false;
	}

double GetPostfixedNumber(char* NumberString)
	{
	double Number; char Postfix[10]="";
	int Conv=sscanf(NumberString,"%lf%s",&Number,Postfix);
	if (Conv==0) { Number=1.0; Conv=sscanf(NumberString,"%s",Postfix); if (Conv==0) return 0; }
	if (strcmp(Postfix,"")==0) Number*=1;
	if (strcmp(Postfix,"u")==0) Number*=1e-6;
	if (strcmp(Postfix,"m")==0) Number*=1e-3;
	if (strcmp(Postfix,"k")==0) Number*=1e3;
	if (strcmp(Postfix,"K")==0) Number*=1e3;
	if (strcmp(Postfix,"M")==0) Number*=1e6;
	if (strcmp(Postfix,"G")==0) Number*=1e9;
	return Number;
	}

bool SkipBlockInFile(FILE * InFile)
	{
	const int NBytes=500;
	char	Line[NBytes+2];
	int	NConsecutiveEmptyLines=0;
	fscanf(InFile,"%*s"); // to jump to the first real data after initial empty lines
	while (NConsecutiveEmptyLines<2)
		{
		if (fgets(Line,NBytes,InFile)==NULL) return false;
		if ( Line[0] == '\n' ) NConsecutiveEmptyLines++;
		else NConsecutiveEmptyLines=0;
		// if partial line, the EOL character can go single to next read
		if ( Line[strlen(Line)-1] != '\n' ) NConsecutiveEmptyLines--;
		}
	return true;
	}

bool CopyBlockInFileOutFile(FILE * InFile, FILE* OutFile)
	{
	const int NBytes=500;
	char	Line[NBytes+2];
	int	NConsecutiveEmptyLines=0;
	//fscanf(InFile,"%*s"); // to jump to the first real data after initial empty lines
	while (NConsecutiveEmptyLines<2)
		{
		if (fgets(Line,NBytes,InFile)==NULL) return false;
		if ( Line[0] == '\n' ) NConsecutiveEmptyLines++;
		else NConsecutiveEmptyLines=0;
		// if partial line, the EOL character can go single to next read
		if ( Line[strlen(Line)-1] != '\n' ) NConsecutiveEmptyLines--;
		fprintf(OutFile,"%s",Line);
		}
	return true;
	
	}


void RefineFileName(char* FileNameOriginal, char* DirName, char* Extension)
	{
	char	FileNameCheck[200];
	FILE	*FileCheck;
	if (Extension[0]=='.') Extension++;
	sprintf(FileNameCheck,"%s",FileNameOriginal);
	FileCheck = fopen(FileNameCheck,"r");
	if (FileCheck!=NULL) { sprintf(FileNameOriginal,"%s",FileNameCheck); fclose(FileCheck); return; }
	sprintf(FileNameCheck,"%s.%s",FileNameOriginal,Extension);
	FileCheck = fopen(FileNameCheck,"r");
	if (FileCheck!=NULL) { sprintf(FileNameOriginal,"%s",FileNameCheck); fclose(FileCheck); return; }
	sprintf(FileNameCheck,"%s/%s",DirName,FileNameOriginal);
	FileCheck = fopen(FileNameCheck,"r");
	if (FileCheck!=NULL) { sprintf(FileNameOriginal,"%s",FileNameCheck); fclose(FileCheck); return; }
	sprintf(FileNameCheck,"%s/%s.%s",DirName,FileNameOriginal,Extension);
	FileCheck = fopen(FileNameCheck,"r");
	if (FileCheck!=NULL) { sprintf(FileNameOriginal,"%s",FileNameCheck); fclose(FileCheck); return; }
	}

void RemoveDuplicatesFromArray(double *a, int size, int * pnewsize)
	{
	int newsize=size;
	for (int n=0;n<newsize-1;n++)
		{
		if (a[n]==a[n+1]) 
			{ for (int i=n; i<size; i++) a[i]=a[i+1]; newsize--; n--; }
		}
	*pnewsize=newsize;
	}

void ArrayWithout(int ArraySize, double *ArrayOriginal, int NSkip, double *ArrayOut)
	{
	for (int n=0; n<ArraySize-1; n++)
		{
		if (n<NSkip) ArrayOut[n]=ArrayOriginal[n];
		else ArrayOut[n]=ArrayOriginal[n+1];
		}
	}

void ArrayWithout(int ArraySize, int *ArrayOriginal, int NSkip, int *ArrayOut)
	{
	for (int n=0; n<ArraySize-1; n++)
		{
		if (n<NSkip) ArrayOut[n]=ArrayOriginal[n];
		else ArrayOut[n]=ArrayOriginal[n+1];
		}
	}

int SkipLine(FILE* InFile, int NLines)
	{
	int NLine=0;
	fseek(InFile,-1,SEEK_CUR);
	while ( (NLine<NLines) && (!feof(InFile)) )
		{
		//fscanf(InFile,"%*[^\n]\n"); // Could not move FTell sometimes..  HG solution below
		fscanf(InFile,"%*c%*[^\n]\n");
		NLine++;
		//printf("\t at : %d  :: FTell: %ld \n",NLine,ftell(InFile));
		}
	return NLine;
	}

char NiceDurationSec_String[20];
char * NiceDurationSec(int T, int Depth)
	{
	if (T==0) { sprintf(NiceDurationSec_String,"0s"); return NiceDurationSec_String; }
	NiceDurationSec_String[0]='\0';
	char Unit[7]="smhdmy"; // sec,min,hour,day,month,year
	int V[6];
	V[0]=T%60; T/=60; V[1]=T%60; T/=60; V[2]=T%24; T/=24;
	V[3]=T%30; T/=30; V[4]=T%12; T/=12; V[5]=T;
	int s=5;
	while (V[s]==0) s--;
	while (s>0 && Depth>0)
		{ sprintf(NiceDurationSec_String+strlen(NiceDurationSec_String),"%d%c",V[s],Unit[s]); Depth--; s--;}
	return NiceDurationSec_String;
	}

int CountWordsInString(std::string ThisString)
	{
	//std::string WordString;
	int PosBegin=0,PosEnd=1, NWords=0;
	while ( PosEnd > 0)
		{
		PosEnd = ThisString.find_first_of(' ',PosBegin);
		//WordString=ThisString.substr(PosBegin,PosEnd-PosBegin);
		PosBegin=PosEnd+1;
		NWords++;
		}
	return NWords;
	}

void ReadStringToArrayInt(char* InString, int ArrayLength, int* Array, bool LeadingString)
	{
	int Position=0, PositionInc=0;
	if (LeadingString) sscanf(InString,"%*s %n",&Position);
	for (int n=0; n<ArrayLength; n++)
		{
		sscanf(&(InString[Position]),"%d %n",&(Array[n]),&PositionInc);
		Position+=PositionInc;
		}
	}

char BaseFileNameString[500];
char* BaseFileName(char* InPathFileName)
	{
	std::string PathFileName(InPathFileName);
	std::string FileName = PathFileName.substr(PathFileName.find_last_of('/')+1);
	int ExtenstionStartsHere=FileName.find_last_of('.');
	FileName = FileName.substr(0,ExtenstionStartsHere);
	sprintf(BaseFileNameString,"%s",FileName.c_str());
	return BaseFileNameString;
	}

void GetBaseName(char* InPathFileName, char* InBaseName)
	{
	std::string PathFileName(InPathFileName);
	std::string FileName = PathFileName.substr(PathFileName.find_last_of('/')+1);
	sprintf(InBaseName,"%s",FileName.c_str());
	}



////////////////////////////////////////////////////////////////////////
// Bitwise
int GetBit(int BitCode, int WhichBit)
	{ return ((BitCode & (1<<WhichBit)) / (1<<WhichBit)); }


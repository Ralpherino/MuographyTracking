////////////////////////////////////////////////////////////////////////
// Combination Class - implemenetaion
//      version : 2018-03-01
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "CombinationClass.h"

// Combination Class

CombinationClass::CombinationClass(int InN, int InK)
	{
	N=InN; K=InK;
	if (N<2 || K<1 || N<K) { printf("Combination needs N>K>0 , N=%d, K=%d\n",N,K); exit(1); }
	S=NULL;
	SetK(K);
	}

void CombinationClass::SetK(int InK)
	{
	K=InK;
	if (S!=NULL) free(S);
	S = (int*) calloc(K,sizeof(int));
	CalculateNCombinations();
	Reset();
	}

void CombinationClass::CalculateNCombinations()
	{
	NCombinations=Binomial(N,K);
	}

void CombinationClass::Reset()
	{
	for (int i=0;i<K;i++) S[i]=i;
	Valid=true;
	}

void CombinationClass::Increment()
	{
	IncrementIndex(K-1);
	}

void CombinationClass::IncrementIndex(int i)
	{
	if (i<0) { Valid=false; return; }
	if ( (S[i]+1==S[i+1]) || (S[i]==(N-1)) )
		{
		IncrementIndex(i-1);
		return;
		}
	else 
		{
		S[i]++;
		for (int j=i+1; j<K; j++) S[j]=S[i]+j-i;
		}
	}

int CombinationClass::Get(int i)
	{
	return S[i];
	}

void CombinationClass::GetArray(int* Array)
	{
	for (int i=0; i<K; i++) Array[i]=S[i];
	}

void CombinationClass::GetSelectionArray(int* Array)
	{
	for (int i=0; i<N; i++) Array[i]=0;
	for (int i=0; i<K; i++) Array[S[i]]=1;
	}

void CombinationClass::Print()
	{
	for (int i=0; i<K; i++) printf(" %d ",S[i]);
	printf("\n");
	}

void CombinationClass::PrintAll()
	{
	printf("Combinations : %d from %d\n",K,N);
	for (int i=0; i<K; i++) S[i]=i;
	for (int i=0; i<NCombinations; i++)
		{ Print(); Increment(); }
	Reset();
	}

// Combination Min Class

CombinationMinClass::CombinationMinClass(int InN, int InKMin) : CombinationClass(InN,InKMin)
	{
	KMin=InKMin;
	CalculateNCombinations();
	}

void CombinationMinClass::Increment()
	{
	IncrementIndex(K-1);
	if (Valid) return;
	if (K==N) return;
	SetK(K+1);
	CalculateNCombinations();
	}

void CombinationMinClass::CalculateNCombinations()
	{
	NCombinations=0;
	for (int KThis=KMin; KThis<=N; KThis++) NCombinations+=Binomial(N,KThis);
	}

void CombinationMinClass::Reset()
	{
	SetK(KMin);
	}

// Functions

int Factorial(int N)
	{
	if (N<2) return 1;
	return N*Factorial(N-1);
	}

int Binomial(int N, int K)
	{
	int B=1;
	for (int i=0; i<K; i++) B*=(N-i);
	for (int i=0; i<K; i++) B/=(i+1);
	return B;
	}

////////////////////////////////////////////////////////////////////////
// Selection Class - implemenetaion
// 	to select the combination of clusters from Chambers
//      version : 2019-02-25
//      written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "SelectionClass.h"

// Selection Class

SelectionClass::SelectionClass()
	{
	N=0; K=0;
	Max=NULL; P=NULL;
	Valid=false;
	};

SelectionClass::SelectionClass(int InN, int InK)
	{
	Max=NULL; P=NULL;
	Valid=false;
	Set(InN,InK);
	}

SelectionClass::~SelectionClass()
	{
	if (Max!=NULL) free(Max);
	if (P!=NULL) free(P);
	}

void SelectionClass::Set(int InN, int InK)
	{
	N=InN; K=InK;
	if (K>N || N<1) { printf("Undefined selection 0<K<N :: N=%d, K=%d\n",N,K); exit(1); };
	if (Max!=NULL) free(Max);
	if (P!=NULL) free(P);
	Max = (int*) calloc(N,sizeof(int));
	P = (PointStruct*) calloc(K+1,sizeof(PointStruct));
	P[K].n=N;
	if (K>0) Valid=true; else Valid=false;
	}

void SelectionClass::SetArray(int *InMax)
	{
	for (int n=0; n<N; n++) Max[n]=InMax[n];
	Valid=true;
	Reset(0,0);
	//printf("Array set to : "); for (int n=0; n<N; n++) printf("%d ",Max[n]); printf("\n");
	}

void SelectionClass::Increment()
	{ IncrementS(K-1); }

void SelectionClass::IncrementS(int s)
	{
	if (s<0) { Valid=false; return; }
	if (P[s].c<Max[P[s].n]-1) { (P[s].c)++; Reset(s+1,P[s].n+1); return;}
	Reset(s,P[s].n+1);
	if (!Valid) { Valid=true; IncrementS(s-1); }
	}

void SelectionClass::Reset(int s, int n)
	{
	if (s>=K) return; // end of reset loop
	if (n>=N) { Valid=false; return; }
	if (Max[n]>0) 
		{
		P[s].n=n; P[s].c=0;
		Reset(s+1,n+1);
		}
	else Reset(s,n+1);
	}

void SelectionClass::Print()
	{
	printf("Select:  ");
	for (int s=0; s<K; s++) printf(" %d:%d  ",P[s].n,P[s].c);
	printf("\n");
	}

void SelectionClass::PrintAll()
	{
	printf("Selecting %d of %d , with clusters as: ",K,N);
	for (int n=0; n<N; n++) printf("%d ",Max[n]);
	printf("\n");
	if (!Valid) printf("There is no valid selection\n");
	while (Valid)
		{
		Print();
		Increment();
		}
	}

int SelectionClass::GetChamber(int s)
	{ return P[s].n; }

int SelectionClass::GetCluster(int s)
	{ return P[s].c; }

bool SelectionClass::IsValid()
	{ return Valid; }

// SelectionMin Class

SelectionMinClass::SelectionMinClass(int InN, int InK)
	{
	N=InN; KMin=InK;
	NSelections=N-KMin+1;
	Selection = (SelectionClass**) calloc(N,sizeof(SelectionClass*));
	for (int K=KMin; K<=N; K++)
		Selection[K] = new SelectionClass(N,K);
	}

SelectionMinClass::~SelectionMinClass()
	{
	for (int K=KMin; K<=N; K++)
		delete Selection[K];
	free(Selection);
	}

void SelectionMinClass::SetArray(int *NClustersArray)
	{
	for (int K=KMin; K<=N; K++)
		Selection[K]->SetArray(NClustersArray);
	K=KMin-1;
	Valid=true;
	NextSelection();
	}

void SelectionMinClass::NextSelection()
	{
	K++;
	if (K>N || K<KMin) { Valid=false; return; }
	if (!(Selection[K]->Valid)) NextSelection();
	}

void SelectionMinClass::Increment()
	{
	Selection[K]->Increment();
	if (Selection[K]->Valid) return;
	else NextSelection();
	}

bool SelectionMinClass::IsValid()
	{ return Valid; }

int SelectionMinClass::GetChamber(int s)
	{
	return Selection[K]->GetChamber(s);
	}

int SelectionMinClass::GetCluster(int s)
	{
	return Selection[K]->GetCluster(s);
	}




////////////////////////////////////////////////////////////////////////
// Combined Efficiency Class - implementation
//	version : 2019-03-19
//	written by : G.Hamar
////////////////////////////////////////////////////////////////////////

#include "CombinedEfficiencyClass.h"


////////////////////////////////////////////////////////////////////////
// CombinedEfficiencyHistClass

CombinedEfficiencyHistClass::CombinedEfficiencyHistClass()
	{
	// Reset the used variables
	Combination=NULL;
	EffHist=NULL;
	EffCalculator=NULL;
	HistIdArray=NULL;	NHists=-1;
	ResultsMultiHist=NULL;	HistIdResult=-1;
	HistIdCombEff=-1; HistIdTotalEff=-1;
	}

CombinedEfficiencyHistClass::~CombinedEfficiencyHistClass()
	{
	if (Combination!=NULL) delete Combination;
	if (EffCalculator!=NULL) delete EffCalculator;
	if (HistIdArray!=NULL) free(HistIdArray);
	}

void CombinedEfficiencyHistClass::Reset()
	{
	// Could be used to make sure all parameters are set after
	if (Combination!=NULL) { delete Combination; Combination=NULL; }
	if (EffCalculator!=NULL) { delete EffCalculator; EffCalculator=NULL; }
	if (HistIdArray!=NULL) { free(HistIdArray); HistIdArray=NULL; }
	EffHist=NULL;
	NHists=-1; HistIdResult=-1;
	HistIdCombEff=-1; HistIdTotalEff=-1;
	}

void CombinedEfficiencyHistClass::SetCombination(int N, int K)
	{
	if (Combination!=NULL) delete Combination;
	Combination = new CombinationMinClass(N,K);
	}

void CombinedEfficiencyHistClass::SetHist(MultiHistClass * InEffHist)
	{
	EffHist = InEffHist;
	}

void CombinedEfficiencyHistClass::SaveResultsTo(int InHistIdResult)
	{
	HistIdResult=InHistIdResult;
	}

void CombinedEfficiencyHistClass::SaveResultsTo(MultiHistClass * InResultsMultiHist, int InHistIdResult)
	{
	ResultsMultiHist=InResultsMultiHist;
	HistIdResult=InHistIdResult;
	}


void CombinedEfficiencyHistClass::SetEfficiencyIdArray(int InNHists, int * InHistIdArray)
	{
	NHists=InNHists;
	if (HistIdArray!=NULL)
		{ free(HistIdArray); HistIdArray=NULL; }
	HistIdArray = (int*) calloc(NHists,sizeof(int));
	for (int NHist=0; NHist<NHists; NHist++)
		HistIdArray[NHist]=InHistIdArray[NHist];
	}

void CombinedEfficiencyHistClass::Calculate()
	{
	// Check if all the needed info is there
	if ( Combination==NULL || EffHist==NULL || NHists<0 || HistIdResult<0 )
		{ printf("Warning! : CombinedEfficiency could not be calculated, due to lack of information\n"); return; }
	if ( Combination->N != NHists )
		{ printf("Warning! : Inconsystency in settings\n"); return; }
	Combination->Reset();
	// Check if Hist values are ok : 0<=Eff<=1
	for (int NHist=0; NHist<NHists; NHist++)
		if ( (EffHist->YMin(HistIdArray[NHist]) < -0.0001) || (EffHist->YMax(HistIdArray[NHist]) > 1.0001) )
			{ printf("Warning: suspicious efficiency in Hist:%d : id=%d\n",NHist,HistIdArray[NHist]); return; }
	// Create Hist to calculate
	EffCalculator = new MultiHistClass(NHists*2+2,EffHist);
	HistIdCombEff=NHists*2;
	HistIdTotalEff=NHists*2+1;
	for (int NHist=0; NHist<NHists; NHist++)
		{
		EffCalculator->Import(NHist,EffHist,HistIdArray[NHist]);
		EffCalculator->Import(NHist+NHists,EffHist,HistIdArray[NHist]);
		EffCalculator->ModifyHist(NHist+NHists,FunctionOneMinusX);
		}
	// Create combinations
	int IsIncluded[NHists];
	int ProductArray[NHists];
	while (Combination->Valid)
		{
		EffCalculator->Fill(HistIdCombEff,1.0);
		Combination->GetSelectionArray(IsIncluded);
		//Combination->Print();
		for (int NHist=0; NHist<NHists; NHist++)
			if (IsIncluded[NHist]==1) ProductArray[NHist]=NHist;
			else ProductArray[NHist]=NHist+NHists;
		EffCalculator->PutProductOfThese(HistIdCombEff,NHists,ProductArray);
		EffCalculator->ModifyHistWith(HistIdTotalEff,HistIdCombEff,FunctionAdd);
		Combination->Increment();
		}
	// Copy the results
	if (ResultsMultiHist==NULL) EffHist->Import(HistIdResult,EffCalculator,HistIdTotalEff);
	else ResultsMultiHist->Import(HistIdResult,EffCalculator,HistIdTotalEff);
	delete EffCalculator; EffCalculator=NULL;
	}


////////////////////////////////////////////////////////////////////////
// CombinedEfficiencyMapClass

CombinedEfficiencyMapClass::CombinedEfficiencyMapClass()
	{
	// Reset the used variables
	Combination=NULL;
	EffMap=NULL;
	EffCalculator=NULL;
	MapIdArray=NULL;	NMaps=-1;
	ResultsMultiMap=NULL;	MapIdResult=-1;
	MapIdCombEff=-1; MapIdTotalEff=-1;
	}

CombinedEfficiencyMapClass::~CombinedEfficiencyMapClass()
	{
	if (Combination!=NULL) delete Combination;
	if (EffCalculator!=NULL) delete EffCalculator;
	if (MapIdArray!=NULL) free(MapIdArray);
	}

void CombinedEfficiencyMapClass::Reset()
	{
	// Could be used to make sure all parameters are set after
	if (Combination!=NULL) { delete Combination; Combination=NULL; }
	if (EffCalculator!=NULL) { delete EffCalculator; EffCalculator=NULL; }
	if (MapIdArray!=NULL) { free(MapIdArray); MapIdArray=NULL; }
	EffMap=NULL;
	NMaps=-1; MapIdResult=-1;
	MapIdCombEff=-1; MapIdTotalEff=-1;
	}

void CombinedEfficiencyMapClass::SetCombination(int N, int K)
	{
	if (Combination!=NULL) delete Combination;
	Combination = new CombinationMinClass(N,K);
	}

void CombinedEfficiencyMapClass::SetMap(MultiMapClass * InEffMap)
	{
	EffMap = InEffMap;
	}

void CombinedEfficiencyMapClass::SaveResultsTo(int InMapIdResult)
	{
	MapIdResult=InMapIdResult;
	}

void CombinedEfficiencyMapClass::SaveResultsTo(MultiMapClass * InResultsMultiMap, int InMapIdResult)
	{
	ResultsMultiMap=InResultsMultiMap;
	MapIdResult=InMapIdResult;
	}


void CombinedEfficiencyMapClass::SetEfficiencyIdArray(int InNMaps, int * InMapIdArray)
	{
	NMaps=InNMaps;
	if (MapIdArray!=NULL)
		{ free(MapIdArray); MapIdArray=NULL; }
	MapIdArray = (int*) calloc(NMaps,sizeof(int));
	for (int NMap=0; NMap<NMaps; NMap++)
		MapIdArray[NMap]=InMapIdArray[NMap];
	}

void CombinedEfficiencyMapClass::Calculate()
	{
	// Check if all the needed info is there
	if ( Combination==NULL || EffMap==NULL || NMaps<0 || MapIdResult<0 )
		{ printf("Warning! : CombinedEfficiency could not be calculated, due to lack of information\n"); return; }
	if ( Combination->N != NMaps )
		{ printf("Warning! : Inconsystency in settings\n"); return; }
	Combination->Reset();
	// Check if Map values are ok : 0<=Eff<=1
	for (int NMap=0; NMap<NMaps; NMap++)
		if ( (EffMap->GetZMin(MapIdArray[NMap]) < -0.0001) || (EffMap->GetZMax(MapIdArray[NMap]) > 1.0001) )
			{ printf("Warning: suspicious efficiency in Map:%d : id=%d\n",NMap,MapIdArray[NMap]); return; }
	// Create Map to calculate
	EffCalculator = new MultiMapClass(NMaps*2+2,EffMap);
	MapIdCombEff=NMaps*2;
	MapIdTotalEff=NMaps*2+1;
	for (int NMap=0; NMap<NMaps; NMap++)
		{
		EffCalculator->Import(NMap,EffMap,MapIdArray[NMap]);
		EffCalculator->Import(NMap+NMaps,EffMap,MapIdArray[NMap]);
		EffCalculator->ModifyMap(NMap+NMaps,FunctionOneMinusX);
		}
	// Create combinations
	int IsIncluded[NMaps];
	int ProductArray[NMaps];
	while (Combination->Valid)
		{
		EffCalculator->Fill(MapIdCombEff,1.0);
		Combination->GetSelectionArray(IsIncluded);
		//Combination->Print();
		for (int NMap=0; NMap<NMaps; NMap++)
			if (IsIncluded[NMap]==1) ProductArray[NMap]=NMap;
			else ProductArray[NMap]=NMap+NMaps;
		EffCalculator->PutProductOfThese(MapIdCombEff,NMaps,ProductArray);
		EffCalculator->ModifyMapWith(MapIdTotalEff,MapIdCombEff,FunctionAdd);
		Combination->Increment();
		}
	// Copy the results
	if (ResultsMultiMap==NULL) EffMap->Import(MapIdResult,EffCalculator,MapIdTotalEff);
	else ResultsMultiMap->Import(MapIdResult,EffCalculator,MapIdTotalEff);
	delete EffCalculator; EffCalculator=NULL;
	}




////////////////////////////////////////////////////////////////////////
// Calculation for single values

double CalculateCombinedEfficiency(int N, int K, double * Eff, bool CheckArray)
	{
	if (CheckArray) for (int n=0; n<N; n++) if ((Eff[n]<0)||(Eff[n]>1.0)) { printf("Warning: Suspicious efficiency : Eff=%g\n",Eff[n]); return 0; }
	double	EfficiencyTotal=0, EfficiencyOfCombination;
	int	IsIncluded[N];
	CombinationMinClass * Combination = new CombinationMinClass(N,K);
	while (Combination->Valid)
		{
		EfficiencyOfCombination=1;
		Combination->GetSelectionArray(IsIncluded);
		//Combination->Print();
		for (int n=0; n<N; n++)
			if (IsIncluded[n]==1) EfficiencyOfCombination*=Eff[n];
			else EfficiencyOfCombination*=(1.0-Eff[n]);
		//printf("\tEfficiencyOfCombination : %g\n",EfficiencyOfCombination);
		EfficiencyTotal+=EfficiencyOfCombination;
		Combination->Increment();
		}
	//printf("EfficiencyTotal= %g\n",EfficiencyTotal);
	delete Combination;
	return EfficiencyTotal;
	}


void CalculateMeasuredEfficiencies(int N, int K, double * Eff, bool CheckArray)
	{
	if (CheckArray) for (int n=0; n<N; n++) if ((Eff[n]<0)||(Eff[n]>1.0)) { printf("Warning: Suspicious efficiency : Eff=%g\n",Eff[n]); }
	// EffMeas = e*Pm(N-1,K-1)/Pm(N,K)
	double EffMeas[N];
	double EffN1[N-1];
	for (int n=0; n<N; n++)
		{
		ArrayWithout(N,Eff,n,EffN1);
		// The formula via using CalculateCombinedEfficiency() function 
		EffMeas[n]=Eff[n]*CalculateCombinedEfficiency(N-1,K-1,EffN1)/CalculateCombinedEfficiency(N,K,Eff);
		}
	printf("CalculateMeasuredEfficiencies:\n");
	for (int n=0; n<N; n++) printf(" %5.2lf -> %5.2lf\n",Eff[n]*100.,EffMeas[n]*100.);
	}




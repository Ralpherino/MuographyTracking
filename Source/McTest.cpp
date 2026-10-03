// MuogramClass Test

int Verbose;
#include "MuogramClass.h"

int main()
{

//
//  Mindenfele filenevek meg hardkodolva vannak, mert ez csak egy teszt
//  ki lehet probalni dolgokat, es fejleszteni magat a MuogramClass-t
//  illetve mimdenki a sajat dolgat irja majd persze :)
//  Probaltam ugy irni (mar ilyen allapotban is), hogy ertheto legyen
//  meg rengeteg dolog van, ez egy vaz amit tudunk hasznalni
//  ha van otlet, akkor szoljatok ill csinaljatok :)
//  jo szorakozast !
//  udv!  Gergo"
//


// Create MuogramClass with BinSize = 2 deg
MuogramClass * M = new MuogramClass(2);

// Esztramos : info az Excel tablazatbol
//146	VII-es 5, Rákóczi-táró vége, omlás előtt	776034.5	354222.2	182	266.9	0
//147	VII-es 5, Rákóczi-táró vége, omlás előtt	776034.5	354222.2	182	356.9	0
//148	VII-es 5, Rákóczi-táró vége, omlás előtt	776034.5	354222.2	182	356.9	-14.2

// Read TrackList and thus get the Flux

//M->ReadTrackList((char*)"Data/Esztramos/Mtl1_Run147.tl",0,0,356.9);
M->ReadTrackList((char*)"Data/Esztramos/Mtl1_Run148.tl",-14.2,0,356.9);

// Convert Flux to DensityLength
M->FluxToDl((char*)"Reference.zfd");
// Read DEM file for Surface info, and calculate RockLength

M->ReadDemFileName((char*)"Data/Esztramos/Esztramos.dem",776034.5,354222.2,182); // 147,148


// From RockLengthCalculted and DensityLengthMeasured one can estimate the average density
M->EstimateDensity();
// With the estimated density convert RockLengthCalc to DensityLengthCalc
M->RlToDl(2.6);
// Then one can compare the calcualted and measured DensityLength
// [ this will be insteresting, no real exiting thing implemented, but a shell]
M->CompareDlMC();
// Write results to file
M->WriteToFileName((char*)"Results/Mtl1_Run148.m");
// Plot the Muogram plots
char Command[500];
sprintf(Command,"cd Results; gnuplot -e \"m='Mtl1_Run148'\" MuogramPlots.gp;"); system(Command);

//// Test read+write functions
//MuogramClass * M2 = new MuogramClass(2);
//M2->ReadFromFileName((char*)"Results/Mtl1_Run148.m");
//M2->WriteToFileName((char*)"Results/Test.m");


}

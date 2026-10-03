#
# Makefile for the MuographyTracking Codes
#	version: 2024-04-25
#	wriiten by G.Hamar
#

SHELL:=/bin/bash

Classes = $(wildcard Source/*Class.cpp) Source/ExtraFunctions.cpp
Objects = $(subst .cpp,.o,$(subst Source,Objects,$(Classes)))

Switches = -std=c++11 -O2 -Wall -Wno-unused-result -fno-stack-protector -Wno-format-overflow -Wno-misleading-indentation


tracking : $(Objects) Source/Tracking.cpp
	@#echo Compiling Tracking ...
	@g++ -o Tracking.run $(Switches) $^		&& echo "Tracking.run updated"

merge : $(Objects) Source/MergeDetectorInfo.cpp
	@#echo Compiling MergeDetectorInfo
	@g++ -o MergeDI.run $(Switches) $^		&& echo "MergeDI.run updated"

flux : $(Objects) Source/FluxComputation.cpp
	@#echo Compiling FluxComputation
	@g++ -o FluxComputation.run $(Switches) $^	&& echo "FluxComputation.run updated"

scanev : $(Objects) Source/ScanEvaluation.cpp
	@#echo Compiling ScanEvaluation
	@g++ -o ScanEvaluation.run $(Switches) $^	&& echo "ScanEvaluation.run updated"

test : $(Objects) Source/Test.cpp
	@#echo Compiling Test
	@g++ -o Test.run $(Switches) $^	&& echo "Test.run updated"

fillm : $(Objects) Source/FillMuogram.cpp
	@#echo Compiling FillMuogram
	@g++ -o FillMuogram.run $(Switches) $^	&& echo "FillMuogram.run updated"

help :
	@echo -e "Targets are : \n\t t : tracking\n\t m : merge\n\t f : flux\n\t s : scanev"

all : tracking merge flux scanev fillm

Objects/%.o : Source/%.cpp
	@echo -n Compiling $< ...
	@g++ $(Switches) -c -o $@ $<		&& echo " ok"


#Short forms
t : tracking
m : merge
s : scanev
f : fillm
h : help
clear : clean

clean : 
	@rm Objects/*.o 2> /dev/null	&& echo "Objects cleaned"

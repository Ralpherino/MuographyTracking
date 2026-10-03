#clear;
echo;

cd Source

Classes="*Class.cpp ExtraFunctions.cpp"
Switches="-std=c++11 -O2 -Wall -Wno-unused-result"

echo -e "Compiling : Tracking ..."
g++ $Switches Tracking.cpp $Classes -o ../Tracking.run \
	&& echo -e "\t ... ok\n"

#echo -e "Compiling : MergeDetectorInfo ..."
#g++ $Switches MergeDetectorInfo.cpp $Classes -o ../MergeDetectorInfo.run \
#	&& echo -e "\t ... ok\n"

#echo -e "\nCompiling : FluxComputation ..."
#g++ $Switches FluxComputation.cpp $Classes -o ../FluxComputation.run \
#	&& echo -e "\t ... ok"
#
#echo

cd ..


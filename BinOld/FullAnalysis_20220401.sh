#!/bin/bash

# Full Analysis : Tracking on Raw, Good, and Flux computation
# Version : 2022-03-30
# Written by : Gergo HAMAR
# Parts are : CreateLists, AnalyseRaw, MergeRaw, AnalyseGood, MergeGood, ComputeFlux, MergeFlux
# Requires a definition of commands,files,runs,etc
# then performes the requested actions on the runs in a fix format

ThisDir=$(pwd)
# Usual things..
ColourRed='\033[1;31m'; ColourBlue='\033[1;34m'; ColourBlack='\033[1;30m'; ColourNc='\033[0m';

# Init variables
DoCreateLists=0
DoAnalyseRaw=0
DoMergeRaw=0
DoAnalyseGood=0
DoMergeGood=0
DoComputeFlux=0
DoMergeFlux=0
DefFile=""
NewRun=""
Naming=""
AnalysisDir=""
MainDir=""
FluxSwitches=""
ListDir=""
SettDb=""
SettAp=""
SettRliRaw=""
SettRliGood=""
CommandAnalyseRaw=""
CommandAnalyseGood=""
CommandFlux=""
# Defaults
FluxSwitchesDefault="-bs 50m -scl 400 -ssm 3 -mm 2.0"


# Proper usage
(return 0 2> /dev/null)
if [ $? -eq 0 ]
	then
	echo "Please run the script via bash, nor source, eg:"
	echo "./FullAnalysis.sh -h"
	return 1
	fi
# The Help
if [ $# = 0 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]
	then
	echo -e "$ColourRed\nFullAnalysis.sh $ColourNc --- Tracking of Raw+Good, Merge, and Flux computation"
	echo -e "\nSynopsis: ./FullAnalysis.sh [-switch [arg]] .."
	echo -e "\t -h,--help \t\t - Shows this very help"
	echo -e "\t -d,--definitions DefFile - the file containing the definitions, mandatory"
	echo -e "\t -cl\t\t - Create lists"
	echo -e "\t -ar\t\t - Analyse raw data"
	echo -e "\t -mr\t\t - Merge raw results"
	echo -e "\t -ag\t\t - Analyse good data"
	echo -e "\t -mg\t\t - Merge good results"
	echo -e "\t -cf\t\t - Compute flux"
	echo -e "\t -mf\t\t - Merge flux plots"
	echo -e "\t --all\t\t - Do all: cl+ar+mr+ag+mg+cf+mf"
	echo -e "\t -r RunNumber \t - Do only for a single run"
	exit;
	fi



# Read arguments
while (( $# > 0 ))
	do
	if [ "$1" = "-d" ]
		then
		shift
		DefFile=$1
	elif [ "$1" = "-cl" ]
		then
		DoCreateLists=1
	elif [ "$1" = "-ar" ]
		then
		DoAnalyseRaw=1
	elif [ "$1" = "-mr" ]
		then
		DoMergeRaw=1
	elif [ "$1" = "-ag" ]
		then
		DoAnalyseGood=1
	elif [ "$1" = "-mg" ]
		then
		DoMergeGood=1
	elif [ "$1" = "-cf" ]
		then
		DoComputeFlux=1
	elif [ "$1" = "-mf" ]
		then
		DoMergeFlux=1
	elif [ "$1" = "--all" ]
		then
		DoCreateLists=1; DoAnalyseRaw=1; DoMergeRaw=1;
		DoAnalyseGood=1; DoMergeGood=1; DoComputeFlux=1; DoMergeFlux=1;
	elif [ "$1" = "-r" ]
		then
		shift
		NewRun=$1
		Runs=$1
	else 
		echo "Incorrect switch : " $1
		exit
		fi
	shift
	done



# Read DefFile

##################
## DEF.FILE Example
#Naming="Mts40_Kiralylaki2021"
#AnalysisDir="/home/ghamar/Programozas/Analysis/MuographyTracking_Newest/"
#MainDir="/home/ghamar/Programozas/Analysis/MuographyTracking_Newest/Results/Mts40_Kiralylaki2021/"
#DataDir="../../Measurements/DsMountIncoming/Mts40_202105-08_Kiralylaki/"
#DataNaming="Mts40_Run"
#Runs="122 123 124 126 127 128"
#FluxSwitches="-bs 20m -gl 900"
#### OPTIONAL
#ListDir=$MainDir"/Lists/"
#SettDb=$MainDir"/Settings/Mts40.db"
#SettAp=$MainDir"/Settings/Mts40.ap"
#SettRliRaw=$MainDir"/Settings/Mts40.rli"
#SettRliGood=$MainDir"/Settings/Mts40-Good.rli"
###
#CommandAnalyseRaw="./Tracking.run -b $SettDb -ap $SettAp -ri $SettRli -t 0 -g 1 -v 2 -rd $MainDir/ResultsRaw -en "
#CommandAnalyseGood="./Tracking.run -b $SettDb -ap $SettAp -ri $SettRliGood -i $MainDir/ResultsRaw/"$Naming"_Raw.res -t 1 -g 1 -v 2 -rd $MainDir/ResultsGood -en "
#CommandComputeFlux="./FluxComputation.run -s y -g y -p y -bs 20m -gl 900 -v 2 -rd $MainDir/Flux -t"
##################

# Check settings
echo -e $ColourRed"Checking settings and files" $ColourNc

if [ -f $DefFile ]
	then
	echo -e "Read DefinitionFile :" $ColourBlue $DefFile $ColourNc
	else
	echo -e "Error:\nDefinitionFile :" $DefFile "cannot be found\nExit"
	exit 1
	fi
source $DefFile
# Check definitions
if [ "$Naming" = "" ]
	then
	echo -e "Error:\nDefFile:" $DefFile "\n\tParameter: Naming is missing\nExit"
	exit 2
	else
	echo -e "Naming:" $ColourBlue $Naming $ColourNc
	fi
if [ "$AnalysisDir" = "" ]
	then
	echo "AnalysisDir: not defined, looking for newest..."
	AnalysisDir=$(find ~/Pr*/*/M* -path "*MuographyTracking*" -name "Tracking.run" -print0 | xargs -0r ls -1 -t | head -n 1 | xargs -0r dirname)
	fi
if [ -f $AnalysisDir/Tracking.run ]
	then
	echo "AnalysisDir:" $AnalysisDir
	else
	echo -e "Error\nAnalysisDir:" $AnalysisDir "does not contain Tracking.run file\nExit\n"
	exit 2
	fi
if [ "$MainDir" = "" ]
	then
	echo "MainDir: not defined, the location of DefFile will be used"
	MainDir=$(realpath $(dirname $DefFile))
	fi
if [ -d $MainDir ]
	then
	echo "MainDir:" $MainDir
	else
	echo -e "Error\nMainDir:" $MainDir "does not exists\nExit\n"
	exit 2
	fi
if [ "$ListDir" = "" ]
	then
	echo "ListDir: not defined, default will be used"
	ListDir=$MainDir"/Lists/"
	fi
echo "ListDir:" $ListDir
if [ -d $DataDir ]
	then
	echo "DataDir:" $DataDir
	else
	echo -e "Error\nDataDir:" $DataDir "does not exists\nExit\n"
	exit 2
	fi
if [ "$DataNaming" = "" ]
	then
	echo -e "DataNaming: parameter is missing, will be automatically checked"
	DataNamingExample=$(basename $(ls $DataDir/*ebe* | head -n 1))
	DataNaming=${DataNamingExample%%Run*}"Run"
	fi
echo "DataNaming:" $DataNaming
if [ "$Runs" = "" ]
	then
	echo "Runs: parameter is missing, will be automatically listed"
	DataFileList=$(basename -a $(ls $DataDir/*ebe*) )
	DataFilePartPrev=""
	for DataFile in $DataFileList
		do
		DataFilePart=${DataFile%%.*}
		if [ "$DataFilePartPrev" != "$DataFilePart" ]
			then
			Runs+="${DataFilePart##*Run} "
			fi
		DataFilePartPrev=$DataFilePart
		done
	fi
echo "Runs:" $Runs
if [ "$FluxSwitches" = "" ]
	then
	echo "FluxSwitches: parameter missing, default will be used"
	FluxSwitches=$FluxSwitchesDefault
	fi
echo "FluxSwitches:" $FluxSwitches
# Sett files
if [ "$SettDb" = "" ]
	then
	echo "SettDb: not defined, default will be used: MainDir/Settings/*db"
	SettDb=$(ls $MainDir/Settings/*db 2> /dev/null | head -n 1 )
	fi
if [ -f "$SettDb" ]
	then
	echo "SettDb:" $SettDb
	else
	echo -e "Error: SettDb: cannot be found\nExit"
	exit 3
	fi
if [ "$SettAp" = "" ]
	then
	echo "SettAp: not defined, search one in MainDir/Settings/*ap"
	SettAp=$(ls $MainDir/Settings/*ap 2> /dev/null | head -n 1 )
	fi
if [ "$SettAp" = "" ]
	then
	echo "SettAp: not found, code defaults will be used"
	SettApAdd=""
	else
	echo "SettAp:" $SettAp
	SettApAdd="-ap $SettAp"
	fi
if [ "$SettRliRaw" = "" ]
	then
	echo "SettRliRaw: not defined, search one in MainDir/Settings/*Raw*rli"
	SettRliRaw=$(ls $MainDir/Settings/*Raw*rli 2> /dev/null | head -n 1 )
	fi
if [ "$SettRliRaw" = "" ]
	then
	echo "SettRliRaw: not found, will not be used"
	SettRliRawAdd=""
	else
	echo "SettRliRaw:" $SettRliRaw
	SettRliRawAdd="-ri $SettRliRaw"
	fi
if [ "$SettRliGood" = "" ]
	then
	echo "SettRliGood: not defined, search one in MainDir/Settings/*Good*rli"
	SettRliGood=$(ls $MainDir/Settings/*Good*rli 2> /dev/null | head -n 1 )
	fi
if [ "$SettRliGood" = "" ]
	then
	echo "SettRliGood: not found, will not be used"
	SettRliGoodAdd=""
	else
	echo "SettRliGood:" $SettRliGood
	SettRliGoodAdd="-ri $SettRliGood"
	fi
# Define commands
if [ "$CommandAnalyseRaw" = "" ]
	then
	echo "CommandAnalyseRaw: not defined, will be generated"
	CommandAnalyseRaw="./Tracking.run -b $SettDb $SettApAdd $SettRliRawAdd -t 0 -g 1 -v 2 -rd $MainDir/ResultsRaw -en "
	fi
echo "CommandAnalyseRaw:" $CommandAnalyseRaw
if [ "$CommandAnalyseGood" = "" ]
	then
	echo "CommandAnalyseGood: not defined, will be generated"
	CommandAnalyseGood="./Tracking.run -b $SettDb $SettApAdd $SettRliGoodAdd -i $MainDir/ResultsRaw/"$Naming"_Raw.res -t 1 -g 1 -v 2 -rd $MainDir/ResultsGood -en "
	fi
echo "CommandAnalyseGood:" $CommandAnalyseGood
if [ "$CommandComputeFlux" = "" ]
	then
	echo "CommandComputeFlux: not defined, will be generated"
	CommandComputeFlux="./FluxComputation.run -s y -g y -p y $FluxSwitches -v 2 -rd $MainDir/Flux -t"
	fi
echo "CommandComputeFlux:" $CommandComputeFlux



# Start the real processing
echo -e $ColourRed"Start processing" $ColourNc

# Create dirs if needed, usually at first run
if [ -d $ListDir ] && [ -d $MainDir/ResultsRaw ] && [ -d $MainDir/ResultsGood ] && [ -d $MainDir/Flux ]
	then
	DirsReady=1
	else
	echo -e $ColourRed"Create Dirs" $ColourNc
	mkdir $ListDir 2> /dev/null
	mkdir $MainDir/ResultsRaw 2> /dev/null
	mkdir $MainDir/ResultsGood 2> /dev/null
	mkdir $MainDir/Flux 2> /dev/null
	fi

# Create Lists
if [ "$DoCreateLists" = 1 ]
	then
	echo -e $ColourRed"Create Lists" $ColourNc
	# Create CommandList for Raw
	echo $CommandAnalyseRaw > $ListDir/$Naming\_Raw.cl
	for Run in $Runs
		do
		echo $DataDir$DataNaming$Run >> $ListDir/$Naming\_Raw.cl
		done
	# Create MergeList for Raw
	rm -f $ListDir/$Naming\_MergeRaw.rl
	for Run in $Runs
		do
		echo $MainDir/ResultsRaw/$DataNaming$Run.res >> $ListDir/$Naming\_MergeRaw.rl
		done
	# Create CommandList for Good
	echo $CommandAnalyseGood > $ListDir/$Naming\_Good.cl
	for Run in $Runs
		do
		echo $DataDir$DataNaming$Run >> $ListDir/$Naming\_Good.cl
		done
	# Create MergeList for Good
	rm -f $ListDir/$Naming\_MergeGood.rl
	for Run in $Runs
		do
		echo $MainDir/ResultsGood/$DataNaming$Run.res >> $ListDir/$Naming\_MergeGood.rl
		done
	# Create CommandList for Flux
	echo $CommandComputeFlux > $ListDir/$Naming\_Flux.cl
	for Run in $Runs
		do
		echo $MainDir/ResultsGood/$DataNaming$Run >> $ListDir/$Naming\_Flux.cl
		done
	
	fi

# Enter AnalysisDir
cd $AnalysisDir

if [ "$DoAnalyseRaw" = 1 ]
	then
	echo -e $ColourRed"Analyse Raw data" $ColourNc
	./DoList.sh -id 1 -a 0 -f $ListDir/$Naming\_Raw.cl
	fi

if [ "$DoMergeRaw" = 1 ]
	then
	echo -e $ColourRed"Merge Raw results" $ColourNc
	./MergeDI.run -b $SettDb $SettApAdd -l $ListDir/$Naming\_MergeRaw.rl -rd $MainDir/ResultsRaw -m $Naming\_Raw
	cp $MainDir/ResultsRaw/$Naming\_Raw.pdf $MainDir/.
	fi

if [ "$DoAnalyseGood" = 1 ]
	then
	echo -e $ColourRed"Analyse Good data" $ColourNc
	./DoList.sh -id 1 -a 0 -f $ListDir/$Naming\_Good.cl
	fi

if [ "$DoMergeGood" = 1 ]
	then
	echo -e $ColourRed"Merge Good results" $ColourNc
	./MergeDI.run -b $SettDb $SettApAdd -l $ListDir/$Naming\_MergeGood.rl -rd $MainDir/ResultsGood -m $Naming\_Good
	cp $MainDir/ResultsGood/$Naming\_Good.pdf $MainDir/.
	fi

if [ "$DoComputeFlux" = 1 ]
	then
	echo -e $ColourRed"Compute Flux" $ColourNc
	./DoList.sh -u 2 -a 0 -f $ListDir/$Naming\_Flux.cl
	fi

if [ "$DoMergeFlux" = 1 ]
	then
	echo -e $ColourRed"Merge Flux plots" $ColourNc
	rm -f $MainDir/Flux/$Naming\_Fluxes.pdf
	pdfunite $MainDir/Flux/*pdf $MainDir/Flux/$Naming\_Fluxes.pdf
	cp $MainDir/Flux/$Naming\_Fluxes.pdf $MainDir/.
	fi

# Finished
cd $MainDir
echo -e $ColourRed"Done" $ColourNc
#espeak -a200 "Full Done"

# End of script


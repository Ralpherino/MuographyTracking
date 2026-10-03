#!/bin/bash

# MultiCore usage via parallel running
# Version : 2018-08-21
# Written by : Gergo HAMAR

# Usage : ./MultiCore*.sh FILENAMING COMMAND

# Set colours
ColourRed='\033[1;31m'
ColourBlue='\033[1;34m'
ColourNc='\033[0m'

# The Help
if [ $# = 0 ] || [ $1 = "-h" ] || [ $1 = "--help" ]
        then
        echo -e "$ColourRed\n MultiCore*.sh $ColourNc --- executes codes in parallel"
        echo -e " Synposis:  $ColourRed ./MultiCore.sh $ColourBlue FileNaming Command $ColourNc"
        echo -e "$ColourBlue\tCommand $ColourNc - is the command that shall be executed parallelly"
        echo -e "$ColourBlue\tFileNaming $ColourNc - Command will issued with all FileNaming\* arguments"
        echo -e "\n"
        return &> /dev/null; exit;
	fi

# Clear temp directory
TempDir="Temp"
mkdir $TempDir 2> /dev/null
rm -f $TempDir/Std*temp &> /dev/null

# Expand the FileList
List=($(ls $1*ebe*))
ListLength=${#List[@]}
if [ $ListLength -le 0 ]
	then
	echo "No file with Filenaming: $1"
	return &> /dev/null ; exit ;
	fi
echo "List of $ListLength files"

# Copy the command, and set the ProcessName
shift
Command=$(echo $*)
ProcessNameArray=($(echo $Command))
ProcessName=${ProcessNameArray[0]}
if [ $ProcessName = "bash" ]
	then
	ProcessName=${ProcessNameArray[1]}
	fi

# Set the number usable cores to total-1
NCores=$(nproc --all)
NCores=$[$NCores - 1]
NTotalProcesses=0
NActiveProcesses=1

# Start multiple processes for the parallel work
while [ $NTotalProcesses -lt $ListLength ] || [ $NActiveProcesses -gt 0 ]
	do
	clear
	echo -e "\nStatus of MultiCore of $ColourRed $ProcessName $ColourNc ($NTotalProcesses/$ListLength)"
	NActiveProcesses=$(ps x -o pid,pcpu,command | grep -v MultiCore | grep -v grep | grep -c "$ProcessName")
	echo "Active processes : $NActiveProcesses"
	ps x -o pid,pcpu,command | grep -v MultiCore | grep -v grep | grep $ProcessName
	if [ $NActiveProcesses -lt $NCores ] && [ $NTotalProcesses -lt $ListLength ]
		then
		echo -e "\nStart process " $NTotalProcesses
		$Command -e ${List[$NTotalProcesses]} -id $NTotalProcesses &> $TempDir/StdDump$NTotalProcesses.temp &
		NTotalProcesses=$[$NTotalProcesses + 1]
		fi
	echo -e "\n Recent StdDump file updates:\n"
	tail -n 2 $(ls -t1 $TempDir/Std*temp | head -n $NCores | sort)
	echo
	sleep 5
	done

# Finished
clear
echo -e "Status of MultiCore of $ColourRed $ProcessName $ColourNc ($NTotalProcesses/$ListLength)"
echo -e "\nDone all ($ListLength)\n"

# Nice exit
return &> /dev/null
exit


#!/bin/bash

# Parallel run of commands using multiple cores
# Version : 2019-11-28
# Written by : Gergo HAMAR

# Set colours
ColourRed='\033[1;31m'
ColourBlue='\033[1;34m'
ColourNc='\033[0m'

# Settings
UpdateSec=5
NohupBackground=0
UseId=1
UseAudio=1
IdText=""
NCoresForce=0
# Set temp dir and files
TempDir="Temp"
mkdir $TempDir 2> /dev/null
rm -f $TempDir/*temp &> /dev/null
rm -f $TempDir/EbeZip*ebe &> /dev/null
rm -f $TempDir/LineNum.txt &> /dev/null
PidFile=$TempDir"/PidList.temp"
echo "9999999" > $PidFile
touch $TempDir/StdDump1.temp
LogFile="DoList.log"
CommandListFileTemp=$TempDir"/CommList.temp"

# The Help
if [ $# = 0 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]
	then
	echo -e "$ColourRed\nDoList.sh $ColourNc --- executes commands in parallel from a list"
	echo -e "\nSynopsis: ./DoTheseList.sh [-h] [-f ListFile] [-nb] [-nc Num] [-u UpdateSec] [-id 0/1] [-s Comm1 [Comm2 ..]]"
	echo -e "\t -h,--help               - Shows this very help"
	echo -e "\t -f,--list-file ListFile - the file containing one command per line to be executed parallelly"
	echo -e "\t -nb,--nohup-background  - Process will be detaches in the background"
	echo -e "\t -nc,--n-cores Num       - Num parallel jobs will be allowed"
	echo -e "\t -u,--update UpdateSec   - Sets the update interval to UpdateSec"
	echo -e "\t -id,--use-id 0/1        - Turns use of id off/on"
	echo -e "\t -s,--in-line Comm...    - Run the followed commands instead of taking them from a file"
	echo -e "\t -a,--audio 0/1          - Audio alert if done : off/on"
	echo -e "\nStructure of the ListFile : \nFirst line is the common command, while all the following ones are like parameters to it. \nIf one needs separate commands the first line shall be empty.\n"
	return &> /dev/null; exit;
	fi

# Read arguments
while (( $# > 0 ))
	do
	if [ "$1" = "-u" ] || [ "$1" = "--update" ]
		then
		shift
		UpdateSec=$1
	elif [ "$1" = "-id" ] || [ "$1" = "--use-id" ]
		then
		shift
		UseId=$1
	elif [ "$1" = "-a" ] || [ "$1" = "--audio" ]
		then
		shift
		UseAudio=$1
	elif [ "$1" = "-nc" ] || [ "$1" = "--n-cores" ]
		then
		shift
		NCoresForce=$1
	elif [ "$1" = "-nb" ] || [ "$1" = "--nohup-background" ]
		then
		NohupBackground=1
	elif [ "$1" = "-f" ] || [ "$1" = "--list-file" ]
		then
		shift
		CommandListFile="$1"
	elif [ "$1" = "-s" ] || [ "$1" = "--in-line" ]
		then
		shift
		CommandListFile=$CommandListFileTemp
		rm -f $CommandListFileTemp &> /dev/null
		while (( $# > 0 ))
			do
			echo $1 >> $CommandListFileTemp
			shift
			done
	else 
		echo "Incorrect switch : " $1
		exit
		fi
	shift
	done

# If Nohup-Background is requested
if [ "$NohupBackground" = "1" ]
	then
	nohup ./DoList.sh -f $CommandListFile -id $UseId -u $UpdateSec -nc $NCoresForce &> /dev/null &
	exit
	fi

# Check CommandList
Command=$(head -n 1 $CommandListFile)
sed '${/^ *$/d;}' $CommandListFile > $CommandListFileTemp
ListLength=$(wc -l $CommandListFileTemp | awk '{print $1-1;}')

# Set the number of usable cores to total-1
NCores=$(nproc --all)
NCores=$(($NCores-1))
if [ $NCoresForce -gt 0 ]
	then
	NCores=$NCoresForce
	fi
NTotalProcesses=0
NActiveProcesses=1

# Start multiple processes for the parallel work
echo -e $(date +"%F_%T")  -  "DoList start\tN.Commands : $ListLength ; File : $CommandListFile" >> $LogFile
while [ $NTotalProcesses -lt $ListLength ] || [ $NActiveProcesses -gt 0 ]
	do
	clear
	echo -e " Status of $ColourRed $CommandListFile $ColourNc ($NTotalProcesses/$ListLength)"
	NActiveProcesses=$(ps --pid "$(paste -s -d ' ' $PidFile)" -o pid | grep -c -f $PidFile)
	echo -e "$ColourBlue Active processes $ColourNc : $NActiveProcesses "
	ps --pid "$(paste -s -d ' ' $PidFile)" -o pid,pcpu,pmem,etime,command
	if [ $NActiveProcesses -lt $NCores ] && [ $NTotalProcesses -lt $ListLength ]
		then
		NTotalProcesses=$(($NTotalProcesses+1))
		echo -e "\n$ColourRed Start process " $NTotalProcesses $ColourNc
		if [ $UseId -gt 0 ]
			then
			IdText="-id $NTotalProcesses"
			fi
		ThisCommand=$(tail -n +$(($NTotalProcesses+1)) $CommandListFile | head -n 1)
		$Command $ThisCommand $IdText  &> $TempDir/StdDump$NTotalProcesses.temp &
		echo $! >> $PidFile
		sleep 0.01
		echo $(date +"%F_%T")  -  "[$NTotalProcesses/$ListLength]"  -  $Command $ThisCommand $IdText >> $LogFile
		fi
	echo -e "\n $ColourBlue Recent StdDump file updates: $ColourNc\n"
	tail -n 7 $(ls -t1 $TempDir/Std*temp | head -n $NCores | sort)
	sleep $UpdateSec
	NActiveProcesses=$(ps --pid "$(paste -s -d ' ' $PidFile)" -o pid | grep -c -f $PidFile)
	done
echo -e $(date +"%F_%T")  -  "All Done\n" >> $LogFile

# Finished
clear
echo -e "\n$ColourBlue Tail of the StdDump files:\n $ColourNc"
tail -n 2 $(ls -t1 $TempDir/Std*temp | sort)
echo -e "\n----------------------------------------------------------\n"
echo -e "$ColourBlue Log of the run $ColourNc\n"
tail -n $(($ListLength+3)) $LogFile
echo -e "\n----------------------------------------------------------\n"
echo -e "$ColourRed $CommandListFile $ColourNc ($NTotalProcesses/$ListLength) : All Done\n\n"
if [ $UseAudio -gt 0 ]
	then
	espeak -a200 "All Done" 2> /dev/null
	fi

# Nice exit
return &> /dev/null
exit


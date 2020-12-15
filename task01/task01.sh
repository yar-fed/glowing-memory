#!/bin/bash

clear

[[ -d $1 ]] && cd $1
DM='long'
LS="ls -a"
PS3="$($LS)"$'\n'
OPTIONS=("Toggle" "cd" "copy" "move" "remove" "exit")
while true; do
	OPTIONS[0]="Toggle display mode (currently $DM)"
	IFSBKP=$IFS
	IFS=''
	select COMMAND in "${OPTIONS[@]}"; do
		IFS=$IFSBKP
		clear
		case $REPLY in
			1 )
				if [[ $DM == "short" ]]; then
					LS="ls -a"
					DM='long'
				else
					LS="ls"
					DM='short'
				fi
				
				;;
			2 )
				PS3=""
				select DIR in `ls -a` "custom"; do
					if [[ $DIR == "custom" ]]; then
						clear
						echo -n "Enter directory: "
						DIR=''
						until [[ -d DIR ]]; do
							read DIR
							echo -en "\nEnter again: "
						done
						clear
					fi
					break
				done
				cd $DIR
				;;
			3 )
				PS3="source file: "
				select SRC in `ls -a` "custom"; do
					if [[ $SRC == "custom" ]]; then
						echo -n "Enter destiantion file: "
						SRC=''
						until [[ -e SRC ]]; do
							read SRC
							echo -en "\nEnter again: "
						done
						clear
					fi
					break
				done
				PS3="destination file: "
				select DST in `ls -a` "custom"; do
					if [[ $DST == "custom" ]]; then
						echo -n "Enter destiantion file: "
						DST=''
						until [[ -e $DST || -d ${DST%/*} ]]; do
							read DST
							echo -en "\nEnter again: "
						done
						clear
					fi
					break
				done
				cp $SRC $DST
				;;
			4 )
				PS3="source file: "
				select SRC in `ls -a` "custom"; do
					if [[ $SRC == "custom" ]]; then
						echo -n "Enter source file: "
						SRC=''
						until [[ -e SRC ]]; do
							read SRC
							echo -en "\nEnter again: "
						done
						clear
					fi
					break
				done
				PS3="destination file: "
				select DST in `ls -a` "custom"; do
					if [[ $DST == "custom" ]]; then
						echo -n "Enter destiantion file: "
						DST=''
						until [[ -e $DST || -d ${DST%/*} ]]; do
							read DST
							echo -en "\nEnter again: "
						done
						clear
					fi
					break
				done
				mv $SRC $DST
				;;
			5 )
				PS3=""
				select DST in `ls -a` "custom"; do
					if [[ $DST == "custom" ]]; then
						echo -en "Enter file: "
						DST=''
						until [[ -e $DST ]]; do
							read DST
							echo -en "\nEnter again: "
						done
						clear
					fi
					break
				done
				rm $DST
				;;
			6 )
				break 2
				echo $COMMAND
				;;
			* )
				break
				;;
		esac
		break
	done
	PS3="$($LS)"$'\n'
	clear
done


#!/bin/bash

declare -A DMODES
declare -A FLAGS
for i in $@; do
	case $i in
		-h|--help )
			echo -e "Usage: task01 [-h|-a|-l|--help|--all|--long] [<starting_dir>]

	<starting_dir>: path to directory where program starts, if multiple dirs are given, the last one is used
	-h|--help: print this message and exit
	-a|--all: toggle 'all' mode (all files are listed in directory)
	-l|--long: toggle 'long' mode (files are listed in long format)
"
			exit 0
			;;
		-a|--all )
			DMODES[all]=all
			FLAGS[a]="-a"
			;;
		-l|--long )
			DMODES[long]=long
			FLAGS[l]="-l"
			;;
		-* )
			echo "Unknown option '${i#-}'"
			exit 1
			;;
		* )
			[[ -d $i ]] && cd $i
			;;
	esac
done

clear

PS3="$(ls ${FLAGS[*]})"$'\n'
OPTIONS=("Toggle" "cd" "copy" "move" "remove" "exit")
while true; do
	OPTIONS[0]="Toggle display mode (currently ${DMODES[*]})"
	IFSBKP=$IFS
	IFS=''
	select COMMAND in "${OPTIONS[@]}"; do
		IFS=$IFSBKP
		clear
		case $REPLY in
			1 )
				PS3=""
				select MODE in "toggle all" "toggle long"; do
					case $REPLY in
						1 )
							if [[ ${DMODES[all]} == "all" ]]; then
								DMODES[all]=""
								FLAGS[a]=""
							else
								DMODES[all]="all"
								FLAGS[a]="-a"
							fi
							;;
						2 )
							if [[ ${DMODES[long]} == "long" ]]; then
								DMODES[long]=""
								FLAGS[l]=""
							else
								DMODES[long]="long"
								FLAGS[l]="-l"
								echo ${FLAGS[*]}
								read a
							fi
							;;
					esac
					break
				done
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
	PS3="$(ls ${FLAGS[*]})"$'\n'
	clear
done


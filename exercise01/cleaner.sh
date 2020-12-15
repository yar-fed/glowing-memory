#!/bin/bash

if [[ "$1" == "-y" || "$1" == "--yes" ]]; then
	AUTOACCEPT=YES
	shift
fi

declare -a DIRS

while [[ "$#" -gt 0 ]]; do
	case $1 in
		-h|--help)
			HELP=YES
			;;
		-t|--test)
			TEST=YES
			;;
		-r|--recursive)
			RECURSIVE=YES
			;;
		-y|--yes)
			if [[ -z ${AUTOACCEPT} ]]; then
				echo -e "Ignoring '$1'\n Try passing it as the first argument"
				AUTOACCEPT=NO
			fi
			;;
		-*|--*)
			if [ -d "$1" ]; then
				DIRS+=($1)
			else
				UNKNOWN_OPTIONS+=($1)
			fi
			;;
		*)
			if [ -d "$1" ]; then
				DIRS+=($1)
			elif [ -e "$1" ]; then
				FILES+=($1)
			else
				NOT_FOUND+=($1)
			fi
			;;
	esac
	shift
done

if [[ ${#UNKNOWN_OPTIONS[@]} -gt 0 ]]; then
	echo "Unknown options passed: ${UNKNOWN_OPTIONS[*]}"
	echo "Use -h or --help for help"
fi

if [[ ${#FILES[@]} -gt 0 ]]; then
	for i in ${FILES[@]}; do
		echo "'$i' is not a directory"
	done
fi

if [[ ${#NOT_FOUND[@]} -gt 0 ]]; then
	for i in ${NOT_FOUND[@]}; do
		echo "No such file or directory '$i'"
	done
fi

if [[ ${#NOT_FOUND[@]} -eq 0 && ${#FILES[@]} -eq 0 && ${#DIRS[@]} -eq 0 ]]; then
	DIRS+=(".")
fi

if [[ $HELP == "YES" ]]; then
	echo "Usage: cleaner.sh [-y|--yes] [options] [directories]"
	echo "options:"
	echo "	-y, --yes"
	echo "		don't ask confirmation, should be first argument"
	echo "	-h, --help"
	echo "		print help message"
	echo "	-t, --test"
	echo "		do not delete, instead print files to stdout"
	echo "	-r, --recursive"
	echo "		recursive removal"
fi

if [[ ${#DIRS[@]} -gt 0 ]]; then
	if [[ $TEST == "YES" ]]; then
		EXEC_COMMAND="-print"
	else
		EXEC_COMMAND="-delete"
	fi
	if [[ $RECURSIVE == "YES" ]]; then
		RECURSIVE=""
	else
		RECURSIVE="-maxdepth 1"
	fi
	for i in ${DIRS[@]}; do
		find $i -depth $RECURSIVE \( -name "[\-_~]*" -or -name "*.tmp" -or \( -type d -empty ! -name "$(echo $i | cut -d/ -f2)" \) \) $EXEC_COMMAND
	done
fi


#!/bin/bash

function genrand ()
{
	echo $(seq 0 $1 | shuf - | head -1)
}

function compare_numbers ()
{
	if [[ $1 -lt $2 ]]; then
		echo "$1 is less than $2"
	elif [[ $1 -gt $2 ]]; then
		echo "$1 is greater than $2"
	else
		echo "$1 is equal to $2"
	fi
}

if [[ -z "$1" ]]; then
	read -p "Please enter a number: " NUMBER
	if [[ ! "$NUMBER" =~ [0-9]+ ]]; then
		echo "'$NUMBER' is not a number"
		exit 2
	fi
elif [[ ! "$1" =~ [0-9]+ ]]; then
	echo "'$1' is not a number"
	exit 2
else
	NUMBER=$1
fi

if [[ -z "$2" ]]; then
	read -p "Please enter a number (leave blank to use default 5): " UPPER_LIMIT
	if [[ -z "$UPPER_LIMIT" ]]; then
		UPPER_LIMIT=5
	elif [[ ! "$UPPER_LIMIT" =~ [0-9]+ ]]; then
		echo "'$UPPER_LIMIT' is not a number"
		exit 2
	fi
elif [[ ! "$2" =~ [0-9]+ ]]; then
	echo "'$2' is not a number"
	exit 2
elif [[ $2 -lt 1 || $2 -gt 100 ]]; then
	echo "'$2' should be in range [1;100]"
	exit 3
else
	UPPER_LIMIT=$2
fi

compare_numbers "$NUMBER" "$(genrand $UPPER_LIMIT)"


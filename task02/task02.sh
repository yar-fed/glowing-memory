#!/bin/bash

function genrand ()
{
	echo $(seq 0 5 | shuf - | head -1)
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
	echo "No argument found"
	exit 1
elif [[ ! "$1" =~ [0-9]+ ]]; then
	echo "'$1' is not a number"
	exit 2
fi

compare_numbers "$1" "$(genrand)"


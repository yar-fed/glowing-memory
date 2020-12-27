#!/bin/bash

function genrand ()
{
	echo $(seq 0 $1 | shuf - | head -1)
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

if [[ -z "$3" ]]; then
	read -p "Please enter number of guesses (leave blank to use default 1): " GUESSES_N
	if [[ -z "$GUESSES_N" ]]; then
		GUESSES_N=1
	elif [[ ! "$GUESSES_N" =~ [1-9][0-9]* ]]; then
		echo "'$GUESSES_N' is not a number"
		exit 2
	fi
elif [[ ! "$3" =~ [1-9][0-9]* ]]; then
	echo "'$3' is not a valid number"
	exit 2
else
	GUESSES_N=$3
fi


while true; do
	RAND_NUMBER="$(genrand $UPPER_LIMIT)"
	if [[ $NUMBER -lt $RAND_NUMBER ]]; then
		echo "$NUMBER is less than $RAND_NUMBER"
	elif [[ $NUMBER -gt $RAND_NUMBER ]]; then
		echo "$NUMBER is greater than $RAND_NUMBER"
	else
		echo "$NUMBER is equal to $RAND_NUMBER"
		break
	fi

	GUESSES_N=$((GUESSES_N - 1))
	if [[ $GUESSES_N -eq 0 ]]; then
		break
	fi

	echo "Guesses left: $GUESSES_N"
	read -p "Try again: " NUMBER
	while [[ ! "$NUMBER" =~ [0-9]+ ]]; do
		echo "'$NUMBER' is not a number"
		read -p "Try again: " NUMBER
	done
done


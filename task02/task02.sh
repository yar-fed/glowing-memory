#!/bin/bash

# random number generator funtion
# takes an upper limit as first positional argument
function genrand ()
{
	lim=$1
	echo $(( RANDOM % (lim + 1) ))
}

# parsing and validating first positional argument (user guess)
# if it is not a number ask user to enter a number instead
if [[ -z "$1" ]]; then
	read -p "Please enter your guess: " NUMBER
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
shift

# parsing options
while [[ "$#" -gt 0 ]]; do
	case $1 in
		-u|--upper=* )
			if [[ "${1#=}" =~ [0-9]+ ]]; then
				UPPER_LIMIT=${1#=}
			elif [[ "$2" =~ [0-9]+ ]]; then
				UPPER_LIMIT=$2
				shift
			else
				echo "'$UPPER_LIMIT' is not a number"
				exit 2
			fi
			if [[ ! -v UPPER_LIMIT && $UPPER_LIMIT -gt 0 && $UPPER_LIMIT -lt 101 ]]; then
				echo "'$UPPER_LIMIT ' should be in range [1;100]"
				exit 3
			fi
			;;
		-g|--guesses=* )
			echo $1 $2
			if [[ "${1#=}" =~ [1-9][0-9]* ]]; then
				GUESSES_N=${1#=}
			elif [[ "$2" =~ [1-9][0-9]* ]]; then
				GUESSES_N=$2
				shift
			else
				echo "'$GUESSES_N' is not a valid number of guesses"
				exit 2
			fi
			;;
		-* )
			echo "Unknown option '${1#-}'"
			exit 4
	esac
	shift
done

# if upper limit is not given ask user to enter it
if [[ -z "$UPPER_LIMIT" ]]; then
	read -p "Please enter upper limit (leave blank to use default 5): " UPPER_LIMIT
	if [[ -z "$UPPER_LIMIT" ]]; then
		UPPER_LIMIT=5
	elif [[ ! "$UPPER_LIMIT" =~ [0-9]+ ]]; then
		echo "'$UPPER_LIMIT' is not a number"
		exit 2
	elif [[ $UPPER_LIMIT -lt 1 && $UPPER_LIMIT -gt 100 ]]; then
		echo "'$UPPER_LIMIT ' should be in range [1;100]"
		exit 3
	fi
fi

# if number of guesses is not given ask user to enter it
if [[ -z "$GUESSES_N" ]]; then
	read -p "Please enter number of guesses (leave blank to use default 1): " GUESSES_N
	if [[ -z "$GUESSES_N" ]]; then
		GUESSES_N=1
	elif [[ ! "$GUESSES_N" =~ [1-9][0-9]* ]]; then
		echo "'$GUESSES_N' is not a number"
		exit 2
	fi
fi

GUESSES_N_BKP=$GUESSES_N

# guess loop
while true; do
	RAND_NUMBER="$(genrand $UPPER_LIMIT)"
	if [[ $NUMBER -lt $RAND_NUMBER ]]; then
		echo "$NUMBER is less than $RAND_NUMBER"
	elif [[ $NUMBER -gt $RAND_NUMBER ]]; then
		echo "$NUMBER is greater than $RAND_NUMBER"
	else
		echo "$NUMBER is equal to $RAND_NUMBER"
		read -p "Would you like to play again? (y/n): " YN
		if [[ ! $YN =~ [Yy] ]]; then
			break
		fi
		GUESSES_N=$GUESSES_N_BKP
		continue
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


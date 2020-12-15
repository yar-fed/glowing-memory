#!/bin/bash
#by Fedoriachenko Yaroslav
		

if [[ -d $1 ]]; then
	FILES=(`find $1 -maxdepth 1 -mtime +31 -type f -printf "%f"`)
	for i in $FILES; do
		mv $i $1/\~$i
	done
	[[ ! -z "$FILES" ]] && find $1 -depth -maxdepth 1 \( -name "[-_~]*" -or -name "*.tmp" \) -print
else
	echo "'$1' is not a directory"
fi


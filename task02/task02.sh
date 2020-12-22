#!/bin/bash

function genrand ()
{
	echo $(seq 0 5 | shuf - | head -1)
}

echo $(genrand)


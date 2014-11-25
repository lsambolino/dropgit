#!/bin/sh

TOT=`/root/bin/script_raritan.sh 18`

TOT=$(echo "$TOT" | awk '{ print ($1/2) }' )

echo $TOT

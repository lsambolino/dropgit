#!/bin/sh

PORT=$1
FILE=/root/bin/$PORT.txt
BASE=`cat $FILE`
NEW=`echo "$BASE $(( ( $(od -An -N2 -i /dev/urandom) ) )) $(( ( $(od -An -N2 -i /dev/urandom) ) ))" | awk '{ print $1 + $3" "$2 + $4 }'`

echo "$NEW" > $FILE
echo "$NEW" | awk '{ rxp = ($1); rxb = ($1*340); txp = ($2); txb = ($2*340) } END  { printf ("%8d %8d %8d %8d\n", rxp, rxb, txp, txb) }'

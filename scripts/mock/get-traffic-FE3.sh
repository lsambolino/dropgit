#!/bin/sh

#sleep 6

echo `od -An -N4 -tu -i /dev/urandom|head -n1 | awk '{ v = ($0) } END { printf("%8d\n", v) }'`


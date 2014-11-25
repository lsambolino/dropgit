#!/bin/sh

for i in $(seq 0 20)
do
    IFACE=deth$i

    if [ `ip l | grep -o "$IFACE"` ]
    then
        ip l d $IFACE
    fi
done


#!/bin/sh

MODULE="deth"

if [ `lsmod | grep -o "^$MODULE"` ]
then
    exit 0
else
    insmod /root/drop/src/kernel_modules/virtual_interfaces/deth.ko

    while [ 1 ]
    do

        if [ `lsmod | grep -o ^$MODULE` ]
        then
            exit 0
        fi

        sleep 1
    done
fi

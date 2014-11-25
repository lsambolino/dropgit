#!/bin/sh

NAME=`hostname`

case "$NAME" in
    "salicone")
        PORT=12
        ;;
    "noce")
        PORT=11
        ;;
    "pino")
        PORT=2
        ;;
    "nocciolo")
        PORT=1
        ;;
    "*")
        echo "0"
        exit 1
        ;;
esac

/root/bin/script_raritan.sh $PORT


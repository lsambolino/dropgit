#!/bin/sh

MAC=`/sbin/ip link show eth0 | grep ether | awk '{ print $2 }'`

SUBNET0=192.168.1
SUBNET1=130.251.17

case "$MAC" in
    "00:25:90:4a:47:38")
        IP1=175
        IP2=41
        NAME="salicone"
        ;;
    "00:25:90:2c:59:62")
        IP1=17
        IP2=43
        NAME="noce"
        ;;
    "00:25:90:1e:63:30")
        IP1=164
        IP2=42
        NAME="pino"
        ;;
    "00:30:48:cf:36:40")
        IP1=27
        IP2=44
        NAME="nocciolo"
        ;;
    "*")
        ;;
esac

case "$1" in
    "eth0")
        /sbin/ip addr add $SUBNET0.$IP1/16 broadcast 192.168.255.255 dev eth0
#        /sbin/ip addr add $SUBNET0.$IP1/24 dev eth0
        /sbin/ip addr add $SUBNET1.$IP1/24 dev eth0

        /sbin/ip route add default via $SUBNET1.1

        hostname $NAME
        ;;
    "*")
        ;;
esac

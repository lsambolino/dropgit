#!/bin/sh

/sbin/ip addr add 0.0.0.0 dev eth0

/sbin/ip addr add 130.251.17.243/24 dev eth0
/sbin/ip route add default via 130.251.17.1

/sbin/ip addr add 192.168.1.243/16 broadcast 192.168.255.255 dev eth0

#/sbin/ip addr add 192.168.1.243/24 dev eth0


#!/bin/sh

ADDRESS=192.168.1.4
OPTIONS="-v 2c -c tntw"

LAST_INDEX=$(/usr/local/bin/snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogLatestIndex.0 | /usr/bin/awk '{ print $4 }' )

current=$(/usr/local/bin/snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletCurrent.$LAST_INDEX.$1 |  /usr/bin/awk '{ print ($4) }' )
voltage=$(/usr/local/bin/snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletVoltage.$LAST_INDEX.$1 |  /usr/bin/awk '{ print ($4) }' )
pf=$(/usr/local/bin/snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletPowerFactor.$LAST_INDEX.$1 |  /usr/bin/awk '{ print ($4) }' )
AP=$(echo "$current $voltage $pf" | /usr/bin/awk '{ print ($1*$2*$3/100000000) }' )

echo "$AP"


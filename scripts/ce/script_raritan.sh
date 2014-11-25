#!/bin/sh

ADDRESS=192.168.1.4
OPTIONS="-v 2c -c tntw"

LAST_INDEX=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogLatestIndex.0 | awk '{ print $4 }' )

current=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletCurrent.$LAST_INDEX.$1 |  awk '{ print ($4) }' )
voltage=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletVoltage.$LAST_INDEX.$1 |  awk '{ print ($4) }' )
pf=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletPowerFactor.$LAST_INDEX.$1 |  awk '{ print ($4) }' )
AP=$(echo "$current $voltage $pf" | awk '{ print ($1*$2*$3/100000000) }' )

echo "$AP"


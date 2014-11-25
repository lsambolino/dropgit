#!/bin/sh

ADDRESS=192.168.1.4
OPTIONS="-v 2c -c tntw"

LAST_INDEX=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogLatestIndex.0 | awk '{ print $4 }' )
#echo "Last index = $LAST_INDEX"

ROUTER1=`seq 11 14`
ROUTER2=`seq 1 4`
MLX=18

if [ $1 = 1 ]
then
#	echo "Router 1 selected"
	SEQ=$ROUTER1
else
#	echo "Router 2 selected"
	SEQ=$ROUTER2
fi
SUM=0
for i in $SEQ
do
	current=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletCurrent.$LAST_INDEX.$i |  awk '{ print ($4) }' )
	voltage=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletVoltage.$LAST_INDEX.$i |  awk '{ print ($4) }' )
	pf=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletPowerFactor.$LAST_INDEX.$i |  awk '{ print ($4) }' )
	AP=$(echo "$current $voltage $pf" | awk '{ print ($1*$2*$3/100000000) }' )
#	echo "$AP"
	SUM=$(echo "$SUM $AP" | awk '{ print ($1+$2) }' )
done

current=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletCurrent.$LAST_INDEX.$MLX |  awk '{ print ($4) }' )
voltage=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletVoltage.$LAST_INDEX.$MLX |  awk '{ print ($4) }' )
pf=$(snmpget $OPTIONS $ADDRESS PDU-MIB::dataLogOutletPowerFactor.$LAST_INDEX.$MLX |  awk '{ print ($4) }' )
AP=$(echo "$current $voltage $pf" | awk '{ print ($1*$2*$3/100000000) }' )
# "$AP"
SUM=$(echo "$SUM $AP" | awk '{ print ($1+$2/2) }' )

echo "$SUM"




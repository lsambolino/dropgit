#!/bin/sh

P1=97 # PORT 10
P2=69 # PORT 11
P3=67 # PORT 12
P4=72 # PORT 13
P5=70 # PORT 14
P6=75 # PORT 15

GetData ()
{
IN=`wget --load-cookies cookies.txt -qO- "http://192.168.1.3/admin/launch?script=rh&template=get-ports-info-counters-eth&var_name=mlx-eth&port=$1" | awk '/id_60_seconds_ingress_rate_value/{print $5;exit}'`; IN_RATE=${IN#title=\"}

if [ "$IN_RATE" = "" ]
then
    wget --save-cookies cookies.txt --keep-session-cookies -qO/dev/null --post-data 'd_user_id=user_id&t_user_id=string&c_user_id=string&e_user_id=true&f_user_id=admin&f_password=admin' "http://192.168.1.3/admin/launch?script=rh&template=login&action=login"
fi

echo "$IN_RATE"
}

T1=`GetData $P1`
T2=`GetData $P2`
T3=`GetData $P3`
T4=`GetData $P4`
T5=`GetData $P5`
T6=`GetData $P6`

echo "$(echo "$T1 $T2 $T3 $T4 $T5 $T6" | awk '{ sum = ($1+$2+$3+$4+$5+$6) } END { printf ("%8d\n", sum) }' )"


#!/bin/sh

P1=106 # PORT 3

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

echo "$T1"


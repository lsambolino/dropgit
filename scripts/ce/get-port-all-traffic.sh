#!/bin/sh

P0=109 # PORT 9
P1=97 # PORT 10
P2=69 # PORT 11
P3=67 # PORT 12
P4=72 # PORT 13
P5=70 # PORT 14
P6=75 # PORT 15

case "$1" in
    "10")
        PORT=97
        ;;
    "11")
        PORT=69
        ;;
    "12")
        PORT=67
        ;;
    "13")
        PORT=72
        ;;
    "14")
        PORT=70
        ;;
    "15")
        PORT=75
        ;;
    *)
        echo "-1"
        exit 1
        ;;
esac

GetData ()
{
    DATA=`wget --load-cookies cookies.txt -qO- "http://192.168.1.3/admin/launch?script=rh&template=get-ports-info-counters-eth&var_name=mlx-eth&port=$PORT"`
    TEST=`echo "$DATA" | grep ingress_rate_value`

    if [ "$TEST" = "" ]
    then
        wget --save-cookies cookies.txt --keep-session-cookies -qO/dev/null --post-data 'd_user_id=user_id&t_user_id=string&c_user_id=string&e_user_id=true&f_user_id=admin&f_password=admin' "http://192.168.1.3/admin/launch?script=rh&template=login&action=login"

        DATA=`wget --load-cookies cookies.txt -qO- "http://192.168.1.3/admin/launch?script=rh&template=get-ports-info-counters-eth&var_name=mlx-eth&port=$PORT"`
    fi

    TEST=`echo "$DATA" | grep ingress_rate_value`
    if [ "$TEST" = "" ]
    then
        echo "-1"
        exit 1
    fi

    echo "$DATA" | awk '/id_60_seconds_(e|in)gress_rate_value/{a=a" "$7" "$9}END{print a}'
}

GetData


i=0;
while :
do
	echo '["1",'$i']' > FE1_g0.json
	echo '["1",'$i']' > FE1_g1.json
	echo '["data",'$i']' > FE1_g2.json
	i=$(($i+1))
	if [ $i -gt 100 ]
	then
		i=0;
	fi

	sleep 1
done

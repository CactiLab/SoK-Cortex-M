#!/bin/bash

BIN_PATH=/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/Nordic_DFU

echo 'Strat!'
cd /home/andy/Documents/FirmXRay-master
start=`date +%s`

count=0
total=$(find $BIN_PATH -name '*.bin' | wc -l)

find $BIN_PATH -type f -name "*.bin" -print0 | while IFS= read -r -d '' file; do
	echo $file
	while ! make run PATH="$file"
	do
		rm -rf /tmp/FirmXRay*
		echo 'Retry...'
		sleep 1
	done
	rm -rf /tmp/FirmXRay*

	# calculate the progress
	cur=`date +%s`
	count=$(( $count + 1 ))
	pd=$(( $count * 73 / $total ))
	runtime=$(( $cur-$start ))
	estremain=$(( ($runtime * $total / $count)-$runtime ))
	printf "\r%d.%d%% complete ($count of $total) - est %d:%0.2d remaining\e[K" $(( $count*100/$total )) $(( ($count*1000/$total)%10)) $(( $estremain/60 )) $(( $estremain%60 ))
	echo
done
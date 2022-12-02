#!/bin/sh

BIN_PATH=/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/ELF
ANALYZER=/home/andy/library/ghidra_10.1.2_PUBLIC/support/analyzeHeadless
SCRIPT_PATH=/home/andy/ghidra_scripts
RUN_PATH=/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/export_elf_ascii.py
LOG_PATH=/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/disass_log
RESULT_PATH=/home/andy/Documents/GitHub/code-CortexM-binary-analysis/data/monolithic/results

echo 'Strat!'
cd $BIN_PATH

count=0
total=$(find $BIN_PATH -name '*.elf' | wc -l)
start=`date +%s`


for file in $(find $BIN_PATH -name '*.elf'); do
	base_name=$(basename $file)
	echo
	echo $base_name
	log_file=$LOG_PATH/"$base_name".txt
	result_file=$RESULT_PATH/"$base_name".txt
	echo $file

	if [ -e hs_err_pid* ]; then
		rm -f hs_err_pid*
	fi
	if [ -e replay_pid* ]; then
		rm -f replay_pid*
	fi

	if [ -e $log_file ] && tail -n 1 $log_file | grep -q "end of the log"; then
		echo 'Result found!'
		total=$((total-1))
	else
		$ANALYZER /tmp demo.bin -scriptPath $SCRIPT_PATH -preScript $RUN_PATH \"$file\" \"$result_file\" -noanalysis -deleteProject > $log_file
		
		while [ ! -e $log_file ] || ! tail -n 1 $log_file | grep -q "end of the log"
		do
			echo "\nRetry..."
			$ANALYZER /tmp demo.bin -scriptPath $SCRIPT_PATH -preScript $RUN_PATH \"$file\" \"$result_file\" -noanalysis -deleteProject > $log_file
			sleep 1
		done
		sleep 1
		# calculate the progress
		cur=`date +%s`
		count=$(( $count + 1 ))
		pd=$(( $count * 73 / $total ))
		runtime=$(( $cur-$start ))
		estremain=$(( ($runtime * $total / $count)-$runtime ))
		printf "\r%d.%d%% complete ($count of $total) - est %d:%0.2d remaining\e[K" $(( $count*100/$total )) $(( ($count*1000/$total)%10)) $(( $estremain/60 )) $(( $estremain%60 ))
		echo
	fi

done



#!/usr/bin/bash

CUTSEL=$1

SEED=0

NOSOL=0 # 1: evaluate without providing a solution

#####

target_dict="targets.json"

if jq -e --arg value "$CUTSEL" '.all_targets + ["hybrid", "boundimp"] | index($value)' "$target_dict" >/dev/null; then
    echo "'$CUTSEL' is a valid target."
else
    echo "'$CUTSEL' is NOT a valid target."
	exit 1
fi

INSTDIR=$PWD/instances
SOLDIR=$PWD/solutions

mapfile -t ev_inst_array < $PWD/problem-lists/evaluation.txt

N=${#ev_inst_array[@]}

BEGIN=0
END=9

# to solve all evaluation problems:
#END=$((N - 1))

NTASKS=$((END - BEGIN + 1))

SEED_LIST=($(<$PWD/evaluation-seed-list.csv))
SEED_LIST=${SEED_LIST[SEED]}

IFS=$','; split=($SEED_LIST); unset IFS;

SEED=${split[0]}
    
S1=${split[1]}
S2=${split[2]}
S3=${split[3]}

if [[ "$CUTSEL" == "hybrid" || "$CUTSEL" == "boundimp" ]]; then
    cd scip-runs/build
	make
else
    cd collect-data-deploy/deploy/build
	make
fi

i=0


for (( run="$BEGIN"; run<="$END"; run++ ));
do
	i=$((i+1))

	echo "Solving problem $i / $NTASKS"
	echo "Problem: ${ev_inst_array[$run]}"
	PRB=$INSTDIR/${ev_inst_array[$run]}

	if [[ "$NOSOL" -eq 1 ]]; then
    	SOLFILENAME=-
	else
		SOLFILENAME=$SOLDIR/${ev_inst_array[$run]}.csv
	fi

	./data $SEED $PRB $S1 $S2 $S3 evaluation $CUTSEL $SOLFILENAME
	echo 'done'

done
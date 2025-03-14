#!/usr/bin/bash

CUT_SELECTOR=Score
MAIN_SEED=0
#####

INST_DIR=$PWD/instances
SOLDIR=$PWD/solutions

mapfile -t ev_inst_array < problem-lists/evaluation_instances_list.txt

N=${#ev_inst_array[@]}

echo $N

BEGIN=0
#END=$((N - 1))
END=0

SEED_LIST=($(<${HOME}/p-nsahinidis3-0/learn2branch-experiments/ac-tiny-cU-001p/main_sub_seeds.txt))
SEED_LIST=${SEED_LIST[MAIN_SEED]}

IFS=$','; split=($SEED_LIST); unset IFS;

MAIN_SEED=${split[0]}
    
S1=${split[1]}
S2=${split[2]}
S3=${split[3]}

if [[ "$CUT_SELECTOR" == "hybrid" || "$CUT_SELECTOR" == "bound_imp" ]]; then
    cd scip-runs/build
else
    cd collect-data-deploy/deploy/build
fi

for (( run="$BEGIN"; run<="$END"; run++ ));
do
	echo $run
	echo "${ev_inst_array[$run]}"
	PRB=$INST_DIR/${ev_inst_array[$run]}
	echo $PRB
	SOLFILENAME=$SOLDIR/${ev_inst_array[$run]}.csv
	echo $S2
	./data $MAIN_SEED $PRB $S1 $S2 $S3 evaluation $CUT_SELECTOR $SOLFILENAME
	echo 'done'

done
#!/bin/bash
ds_group=$1

#################

if [ "${ds_group}" = "train" ]; then
    ARRAY_END=250
elif [ "${ds_group}" = "valid" ]; then
    ARRAY_END=100
elif [ "${ds_group}" = "test" ]; then
    ARRAY_END=100
else
    exit 1
fi

DATADIR=data/${ds_group}

rm -rf $DATADIR

mkdir -p $DATADIR

INST_MAIN_DIR=$PWD/instances

SOL_DIR=$PWD/solutions

INST_SEED_LIST=($(<data-problem-seed-lists/${ds_group}.txt))

cd collect-data-deploy/collect-data/build

for (( run=0; run<="$ARRAY_END"; run++ )); do

    SAMPLE=${INST_SEED_LIST[$run]}
    IFS=$','; split=($SAMPLE); unset IFS;

    EP=${split[0]}
    INST_ID=${split[1]}
    S1=${split[2]}
    S2=${split[3]}
    S3=${split[4]}

    echo "Episode $EP, Solving instance $INST_ID with seeds $S1, $S2 and $S3"

    ./data $EP $INST_MAIN_DIR/orlib_10_0_1_w_${INST_ID}.lp $S1 $S2 $S3 $ds_group s ${SOL_DIR}/orlib_10_0_1_w_${INST_ID}.lp.csv

done

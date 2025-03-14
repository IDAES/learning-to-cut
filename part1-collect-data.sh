#!/bin/bash
DS=$1

#################

if [ -z "$DS" ]; then
    echo "Missing dataset group"
    exit 1
else
    if [ "$DS" = "train" ]; then
        ARRAY_END=20
    elif [ "$DS" = "valid" ]; then
        ARRAY_END=5
    elif [ "$DS" = "test" ]; then
        ARRAY_END=5
    else
        echo "Incorrect dataset group $DS"
        exit 1
    fi
fi

DATADIR=data/${DS}

mkdir -p $DATADIR

INST_MAIN_DIR=$PWD/instances

SOL_DIR=$PWD/solutions

INST_SEED_LIST=($(<data-problem-seed-lists/${DS}.txt))

cd collect-data-deploy/collect-data/build

for (( run=0; run<="$ARRAY_END"; run++ )); do

    SAMPLE=${INST_SEED_LIST[$run]}
    IFS=$','; split=($SAMPLE); unset IFS;

    EP=${split[0]}
    INST_ID=${split[1]}
    S1=${split[2]}
    S2=${split[3]}
    S3=${split[4]}

    echo "Episode $EP, solving instance $INST_ID with seeds $S1, $S2 and $S3"

    ./data $EP $INST_MAIN_DIR/orlib_10_0_1_w_${INST_ID}.lp $S1 $S2 $S3 $DS s ${SOL_DIR}/orlib_10_0_1_w_${INST_ID}.lp.csv

done

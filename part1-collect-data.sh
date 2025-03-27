#!/bin/bash
DS=$1

#################

if [ -z "$DS" ]; then
    echo "Missing dataset group"
    exit 1
else
    if [ "$DS" = "train" ]; then
        N=20
    elif [ "$DS" = "valid" ]; then
        N=5
    elif [ "$DS" = "test" ]; then
        N=5
    else
        echo "Incorrect dataset group $DS"
        exit 1
    fi
fi

DATADIR=data/${DS}

rm -rf $DATADIR; mkdir -p $DATADIR

INSTDIR=$PWD/instances

SOLDIR=$PWD/solutions

INST_SEED_LIST=($(<problem-seed-lists/$DS.txt))

cd collect-data-deploy/collect-data/build

for (( run=0; run<="$N"; run++ )); do

    SAMPLE=${INST_SEED_LIST[$run]}
    IFS=$','; split=($SAMPLE); unset IFS;

    EP=${split[0]}
    INST_ID=${split[1]}
    #MAINSEED=${split[2]}
    S1=${split[3]}
    S2=${split[4]}
    S3=${split[5]}

    echo "Episode $EP, solving instance $INST_ID with seeds $S1, $S2 and $S3"

    ./data $EP $INSTDIR/orlib_10_0_1_w_${INST_ID}.lp $S1 $S2 $S3 $DS s $SOLDIR/orlib_10_0_1_w_${INST_ID}.lp.csv
done

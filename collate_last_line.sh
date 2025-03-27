#!/bin/bash

geometric_mean_colname() {
    local file="$1"
    local column="$2"
    
    awk -v col="$column" '
    BEGIN {
        FS = ","  # Change if needed (e.g., "\t" for tab-separated files)
        sum_log = 0
        count = 0
    }
    NR == 1 {
        for (i = 1; i <= NF; i++) {
            if ($i == col) {
                colnum = i
                break
            }
        }
        if (!colnum) {
            print "Column not found" > "/dev/stderr"
            exit 1
        }
        next
    }
    {
        if ($colnum != "" && $colnum > 0) {
            sum_log += log($colnum + 1)
            count++
        }
    }
    END {
        if (count > 0) {
            print exp(sum_log / count) - 1
        } else {
            print "No valid numbers found" > "/dev/stderr"
            exit 1
        }
    }' "$file"
}

DIR=$1
HEADER=$2

RES_FILE=results.csv

cd $DIR

echo $HEADER > $RES_FILE

for filename in $( ls -d *.lp.csv | sort -V ) ; do
    #echo $filename
    echo "$(tail -1 $filename)" >> $RES_FILE
done

echo -n "Mean time: "; geometric_mean_colname ${DIR}/${RES_FILE} stime
echo -n "Mean memory used (MB): "; geometric_mean_colname ${DIR}/${RES_FILE} mem_used
echo -n "Mean memory allocated (MB): "; geometric_mean_colname ${DIR}/${RES_FILE} mem_total

if grep -m 1 -q "^.*igc.*$" "${DIR}/${RES_FILE}"; then
    echo -n "Mean IGC at root: "; geometric_mean_colname "${DIR}/${RES_FILE}" igc
else
    echo 'igc column not found'
fi

printf "\n"

cd ${HOME}/p-nsahinidis3-0/learn2branch-experiments/learning-to-cut

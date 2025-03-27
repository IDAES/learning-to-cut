METHODS=(hybrid boundimp logScore)

HEADER="policy,seed,instance,nnodes,nlps,nlpiterations,nnodelps,ndivinglps,nsbs,stime,gap,status,mem_used,mem_total,walltime,proctime,cpu_user_time,cpu_system_time,primal_bound,dual_bound,first_dual_bound,igc"

for M in "${METHODS[@]}"
do
    echo "Method: $M"
    DIR=$PWD/results/$M
    ./collate_last_line.sh $DIR $HEADER
done

cd deploy/build
make

INS=orlib_10_0_1_w_1.lp

score_type=normScore

./data 0 /storage/home/hcoda1/0/sbayramoglu3/p-nsahinidis3-0/learn2branch-experiments/learning-to-cut/instances/$INS 1178568023 1273124120 1535857467 evaluation ${score_type} /storage/home/hcoda1/0/sbayramoglu3/p-nsahinidis3-0/learn2branch-experiments/learning-to-cut/solutions/$INS.csv
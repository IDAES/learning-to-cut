Learning to Cut
============

Selin Bayramoglu, Georgia Institute of Technology

Nick Sahinidis, Georgia Institute of Technology

A repository to learn simple and interpretable models for cutting plane selection within the open-source solver SCIP.

### Copyright ###

Please see COPYRIGHT.md .

### SCIP ###

This project uses the open-source solver SCIP 8.0.0. To install it, run

`conda install -c conda-forge scip=8.0.0`

## Instances

Our base instance is `or-lib/10_0_1_w`, a unit commitment problem from the UnitCommitment.jl package ( [XaKaYuHeQi04], [ORLIB], [FrGe06]). It involves an hourly time horizon of 24 hours, a single bus and ten generators. For our experiments, we generate a total of 50 instances by multiplying each load of the base instance with (1 + x), where x ~ N(0, 0.05^2) and independently distributed.

All instances reside in the `instances/` folder. The split of problems into training, validation, test and evaluation sets are provided in the `problem-lists/` folder.

## Data Collection
To collect data for the training dataset, run

`bash part1-collect-data.sh train` .

For the validation and test datasets, replace the argument `train` with `valid` and `test`, respectively.

## Datasets

To build datasets, run

`bash part2-create-datasets.sh 0` .

Changing the argument will produce a different training dataset by selecting a different subset of all collected training observations.

## Building models

To build a models with one of the targets (e.g., logScore) in `targets.json`, run

`bash part3-build-model.sh logScore` .

## Evaluating cut selectors

To evaluate the learned models, SCIP's default cut selector or the bound improvement target, select a cut selector (e.g., logScore) and run 

`bash part4-evaluate-cut-selector.sh logScore` .

If no solution file is provided, set NOSOL to 1 in the script.

## Compare results

To summarize and compare results, run

`bash summarize_results.sh` .

This script produces statistics such as mean solving time and integrality gap closed (IGC) over the evaluated instances.

For runs with NOSOL=1, the solver is not likely to find a primal solution at the root, therefore the IGC will be 0. These instances can be compared by the final dual bounds, where higher bounds indicate more progress.

## References

[XaKaYuHeQi04] Alinson S. Xavier, Aleksandr M. Kazachkov, Ogün Yurdakul, Jun He, Feng Qiu, "UnitCommitment.jl: A Julia/JuMP Optimization Package for Security-Constrained Unit Commitment (Version 0.4)". Zenodo (2024). DOI: 10.5281/zenodo.4269874.

[ORLIB] J.E.Beasley. "OR-Library: distributing test problems by electronic mail", Journal of the Operational Research Society 41(11) (1990). DOI: 10.2307/2582903

[FrGe06] A. Frangioni, C. Gentile. "Solving nonlinear single-unit commitment problems with ramping constraints" Operations Research 54(4), p. 767 - 775, 2006. DOI: 10.1287/opre.1060.0309

#!/usr/bin/bash

target=$1

# target can be Score, normScore, relativeScore or logScore

Rscript sparse_regression_lasso.R -c $target

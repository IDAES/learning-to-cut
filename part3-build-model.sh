#!/usr/bin/bash

target=Score

# target can be normScore, relativeScore or logScore

Rscript sparse_regression_lasso.R -c $target

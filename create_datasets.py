import os
import argparse
import numpy as np
import pandas as pd
import pathlib
import copy
import os
import sys
import glob

import natsort

EPS=10**-6
HOME=os.environ["HOME"]

from cutting_plane_features import feat_names

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument(
        '-s', '--seed',
        help='Random generator seed.',
        type=int,
        default=0,
    )

    HOME = os.environ["HOME"]
    SCRATCHDIR = f"{HOME}/scratch"

    args = parser.parse_args()

    seed = args.seed

    max_size = {}

    max_size["train"] = 5000
    max_size["valid"] = 2000
    max_size["test"] = 2000

    rng = np.random.RandomState(args.seed) #seed 0,1,2,3,4 are for training
    rng_valid = np.random.RandomState(5)
    rng_test = np.random.RandomState(6)

    feat_names_dict = feat_names()

    target_names = ["Score", "logScore", "normScore", "relativeScore"]

    n_targets = len(target_names)
    
    feat_names = [f"Feat_{i}_{feat_names_dict[i]}" for i in range(len(feat_names_dict))]

    col_names = np.concatenate((feat_names, target_names))

    n_targets = len(target_names)

    sample_folder = f"data"

    x_dict = {}
    y_dict = {}

    ncands_dict = {}

    for dataset_type in ["train", "valid", "test"]:
        print("Dataset type: ", dataset_type)

        nsamples = 0
        
        x_dict[dataset_type] = []
        y_dict[dataset_type] = []

        ncands_dict[dataset_type] = 0

        files = glob.glob(f"{sample_folder}/{dataset_type}" + "/**/*_features.txt", recursive=True)

        print(f"{dataset_type}")
        print(f"Total # LP solves: ", len(files))
        total_lines = 0
        for file_path in files:
            with open(file_path, 'r', encoding='utf-8') as f:
                total_lines += sum(1 for _ in f)

        print(f"Total # observations in all files: {total_lines}")

        files = natsort.natsorted(files)

        if dataset_type == "train":
            files = rng.permutation(files)
        elif dataset_type == "valid":
            files = rng_valid.permutation(files)
        else:
            files = rng_test.permutation(files)

        for i,f in enumerate(files):
            feat_f_name = str(os.path.basename(f))
            node_name = feat_f_name[:len(feat_f_name)-13]
            
            # Empty feature file
            if not os.path.isfile(f) or os.path.getsize(f) == 0:
                continue
            
            dirname = os.path.dirname(f) 
            
            feat_df = pd.read_csv(f, header=None)

            score_df = pd.read_csv(f"{dirname}/{node_name}_scores.txt", header=None)

            cut_scores = np.array(score_df.iloc[:,0])
            cut_scores = [float(sc) for sc in cut_scores]
            cut_scores = np.array(cut_scores)

            feat_arr = np.array(copy.deepcopy(feat_df))

            nsamples += 1

            norm_factor = np.sqrt(sum(np.square(cut_scores)))
            if norm_factor <= 0:
                norm_factor = 1

            score1 = cut_scores

            score2 = np.array([np.log(x) if x>EPS else 0 for x in score1])

            score3 = cut_scores / norm_factor

            max_score = np.max(cut_scores)

            if max_score <= 0:
                score4 = cut_scores
            else:
                score4 = cut_scores / max_score

            cand_targets = np.column_stack((score1,score2, score3, score4))

            x_dict[dataset_type].append(feat_arr)
            y_dict[dataset_type].append(cand_targets)

            ncands_dict[dataset_type] += feat_arr.shape[0]

            if ncands_dict[dataset_type] >= max_size[dataset_type]:
                break
   

        print("Total # cands: ", ncands_dict[dataset_type])

        print("# LP solves in dataset: ", nsamples)

        if ncands_dict[dataset_type] > 0:
            x_dict[dataset_type] = np.concatenate(x_dict[dataset_type])
            y_dict[dataset_type] = np.concatenate(y_dict[dataset_type])

        if ncands_dict[dataset_type] > max_size[dataset_type]:
            n = max_size[dataset_type]
            x_dict[dataset_type] = x_dict[dataset_type][0:n]
            y_dict[dataset_type] = y_dict[dataset_type][0:n]

            ncands_dict[dataset_type] = n

    train_dataset = np.concatenate((x_dict["train"], y_dict["train"]), axis = 1)
    valid_dataset = np.concatenate((x_dict["valid"], y_dict["valid"]), axis = 1)
    test_dataset = np.concatenate((x_dict["test"], y_dict["test"]), axis = 1)

    train_df = pd.DataFrame(train_dataset, columns = col_names)
    valid_df = pd.DataFrame(valid_dataset, columns = col_names)
    test_df = pd.DataFrame(test_dataset, columns = col_names)

    p_dataset_dir = f"datasets/{seed}"

    os.makedirs(f"{p_dataset_dir}", exist_ok=True)

    try:
        os.remove(f"{p_dataset_dir}/train.csv")
    except OSError:
        pass
    try:
        os.remove(f"{p_dataset_dir}/valid.csv")
    except OSError:
        pass
    try:
        os.remove(f"{p_dataset_dir}/test.csv")
    except OSError:
        pass
    
    # Ignore original datasets, keep preprocessed only
    #train_df.to_csv(f"{dataset_dir}/train.csv", index = False)
    #valid_df.to_csv(f"{dataset_dir}/valid.csv", index = False)
    #test_df.to_csv(f"{dataset_dir}/test.csv", index = False)

    train_features = train_df.iloc[:,:-n_targets]
    train_targets = train_df.iloc[:,-n_targets:]

    valid_features = valid_df.iloc[:,:-n_targets]
    valid_targets = valid_df.iloc[:,-n_targets:]

    test_features = test_df.iloc[:,:-n_targets]
    test_targets = test_df.iloc[:,-n_targets:]

    # Remove constant columns

    #pd.set_option('display.max_rows', None)

    nonconstant_columns = train_features.std() > (10 ** -10) # Ignore target columns

    train_features = train_features.loc[:, nonconstant_columns]
    valid_features = valid_features.loc[:, nonconstant_columns]
    test_features = test_features.loc[:, nonconstant_columns]
    
    # Remove correlated columns
    correlation_matrix = train_features.corr().abs()
    
    upper = correlation_matrix.where(np.triu(np.ones(correlation_matrix.shape), k=1).astype(bool))
    to_drop = [column for column in upper.columns if any(upper[column] > 0.95)]

    train_features.drop(train_features[to_drop], axis=1, inplace=True)
    valid_features.drop(valid_features[to_drop], axis=1, inplace=True)
    test_features.drop(test_features[to_drop], axis=1, inplace=True)

    train_df = pd.concat([train_features, train_targets], axis=1)
    valid_df = pd.concat([valid_features, valid_targets], axis=1)
    test_df = pd.concat([test_features, test_targets], axis=1)

    train_df.to_csv(f"{p_dataset_dir}/train.csv", index = False)
    valid_df.to_csv(f"{p_dataset_dir}/valid.csv", index = False)
    test_df.to_csv(f"{p_dataset_dir}/test.csv", index = False)

    print("Prep. Regression df shapes: ", train_df.shape, valid_df.shape, test_df.shape)

    print("Selected features: ")
    print(train_features.columns.to_list())

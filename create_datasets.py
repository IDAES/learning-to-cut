import os
import argparse
import numpy as np
import pandas as pd
import copy
import os
import glob
import json
import shutil
import natsort

EPS=10**-6

def load_allowed_targets():
    with open("targets.json", "r") as f:
        return json.load(f)["all_targets"]

from cutting_plane_features import feat_names

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument(
        '-s', '--seed',
        help='Random generator seed.',
        type=int,
        choices=[0,1,2,3,4],
        default=0,
    )

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

    target_names = load_allowed_targets()
    
    feat_names = [f"F_{i+1}_{feat_names_dict[i]}" for i in range(len(feat_names_dict))]

    n_targets = len(target_names)

    sample_folder = "data"

    x = {}

    ncands = {}

    for dataset_type in ["train", "valid", "test"]:
        print("\nDataset: ", dataset_type)

        nsamples = 0
        
        x[dataset_type] = []

        ncands[dataset_type] = 0

        files = glob.glob(f"{sample_folder}/{dataset_type}" + "/**/*_features.txt", recursive=True)

        print(f"\nTotal # LP solves: ", len(files))
        total_lines = 0
        for file_path in files:
            with open(file_path, 'r', encoding='utf-8') as f:
                total_lines += sum(1 for _ in f)
        
        print(f"Total # observations in all files: {total_lines}")

        if total_lines < max_size[dataset_type]:
            raise Exception("Total # observations not enough")

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

            norm_factor = np.sqrt(sum(np.square(cut_scores)))
            if norm_factor <= 0:
                norm_factor = 1
            max_score = np.max(cut_scores)

            for t in target_names:
                if t == "Score":
                    feat_df["Score"] = cut_scores
                elif t == "normScore":
                    feat_df["normScore"] = cut_scores / norm_factor
                elif t == "relativeScore":
                    if max_score <= 0:
                        feat_df["relativeScore"] = cut_scores
                    else:
                        feat_df["relativeScore"] = cut_scores / max_score
                elif t == "logScore":
                    feat_df["logScore"] = np.array([np.log(x) if x>EPS else 0 for x in cut_scores])
                else:
                    raise Exception

            nsamples += 1

            if ncands[dataset_type] + feat_df.shape[0] > max_size[dataset_type]:
                cands_in_excess = ncands[dataset_type] + feat_df.shape[0] - max_size[dataset_type]
                row_ids = feat_df.shape[0] - cands_in_excess
                if row_ids > 0:
                    x[dataset_type].append(feat_df.iloc[0:row_ids,:])
                    ncands[dataset_type] += row_ids
                    break
            else:
                x[dataset_type].append(feat_df)
                ncands[dataset_type] += feat_df.shape[0]

            if ncands[dataset_type] >= max_size[dataset_type]:
                break

        print("\n# cands in dataset: ", ncands[dataset_type])

        print("# LP solves in dataset: ", nsamples)

        x[dataset_type] = np.concatenate(x[dataset_type])

    train_df = pd.DataFrame(x["train"], columns = feat_names + target_names)
    valid_df = pd.DataFrame(x["valid"], columns = feat_names + target_names)
    test_df = pd.DataFrame(x["test"], columns = feat_names + target_names)

    p_dataset_dir = f"datasets/{seed}"
    
    shutil.rmtree(p_dataset_dir)

    os.makedirs(p_dataset_dir)
    
    train_features = train_df.iloc[:,:-n_targets]
    train_targets = train_df.iloc[:,-n_targets:]

    valid_features = valid_df.iloc[:,:-n_targets]
    valid_targets = valid_df.iloc[:,-n_targets:]

    test_features = test_df.iloc[:,:-n_targets]
    test_targets = test_df.iloc[:,-n_targets:]

    nonconstant_columns = train_features.std() > (10 ** -10)

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

    print("Preprocessed datasets nobs: ", train_df.shape[0], valid_df.shape[0], test_df.shape[0])

    print("N features: ", train_features.shape[1])

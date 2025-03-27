library("glmnet")
library("optparse")
library("gsubfn")
library("jsonlite")

source("utilities.R")

load_allowed_targets <- function() {
  allowed_targets <- fromJSON("targets.json")
  return(allowed_targets$all_targets)
}

a = Sys.time()

parser <- OptionParser()
option_list <- list(
    make_option(c("-c", "--score"), type="character", default="Score",
                dest = "score", help="Score type [default %default]"),
    make_option(c("-s", "--seed"), type="numeric", default=0,
                dest = "seed", help="Seed [default %default]")
)

DIR_str = "./"

opt <- parse_args(OptionParser(option_list=option_list), positional_arguments = TRUE)

seed = opt$options$seed

sc_type = opt$options$score

print(paste0("Target: ", sc_type, ", Seed: ", seed))

target_columns <- load_allowed_targets()

if ((sc_type %in% target_columns) == FALSE)
stop("Target not allowed")

feature_type="quadratic"

main_dir = "models"

dir.create(main_dir, showWarnings = FALSE, recursive = TRUE)

sub_model_dir <- sc_type

dir.create(file.path(main_dir, sub_model_dir), showWarnings = FALSE)

sub_model_dir = paste(main_dir, "/", sub_model_dir, sep="")

dir.create(file.path(sub_model_dir, seed), showWarnings = FALSE)

sub_model_dir = paste(sub_model_dir, "/", seed, "/", sep="")

dataset_dir = "./datasets"

train_df = read.csv(paste(dataset_dir, "/", seed, "/train.csv", sep=""))

valid_df = read.csv(paste(dataset_dir, "/", seed, "/valid.csv", sep=""))

elim_columns = c()

x_columns = c(target_columns, elim_columns)

train_x = train_df[ , -which(names(train_df) %in% x_columns)]
valid_x = valid_df[ , -which(names(valid_df) %in% x_columns)]

train_y = train_df[, sc_type]
valid_y = valid_df[, sc_type]

formula_str_start= paste(sc_type,"~",".",sep= " ")

if (feature_type == "simple") {
  formula_str = paste(formula_str_start, "-1", sep=" ")

} else {
  formula_str=paste(formula_str_start, "^2 + ", sep="")
  formula_str=paste(formula_str,paste('poly(',colnames(train_x),',2, raw=TRUE)[, 2]',collapse = ' + '), " -1")
}

formula=as.formula(formula_str)

train_x[, sc_type] <- train_y
valid_x[, sc_type] <- valid_y

if (nrow(which(is.na(train_x), arr.ind = TRUE)) > 0) stop("NAs in training dataset")
if (nrow(which(is.na(valid_x), arr.ind = TRUE)) > 0) stop("NAs in validation dataset")

train_X = model.matrix(formula, train_x)
valid_X = model.matrix(formula, valid_x)

## DO NOT ADD INTERCEPT WITH THE FORMULA, GLMNET ADDS IT!

feat_names = c("(Intercept)", colnames(train_X)) # glmnet adds a feature with the name '(Intercept)'

# The algorithm selects the grid
my_model = glmnet(train_X, train_y, alpha = 1) # default options: intercept = TRUE, standardize = TRUE
grid = my_model$lambda
  
n_lambda = length(grid)

train_preds = predict(my_model, train_X)
valid_preds = predict(my_model, valid_X)

logfile = paste(sub_model_dir, "lasso_", feature_type, "_log.txt", sep="")

cat(paste("Seed: ", seed, "\n\n", sep=""), file = logfile)

r_model_file = paste(sub_model_dir, "lasso_", feature_type, "_model_R.txt", sep="")

best_valid_r2 = -Inf

for (i in 1:n_lambda) {

  train_res <- goodness_of_fit_reg(train_y, train_preds[,i])

  list[train_mse, train_r2] = train_res

  valid_res <- goodness_of_fit_reg(valid_y, valid_preds[,i])
  
  list[valid_mse, valid_r2] = valid_res


  if (valid_r2 > best_valid_r2) {

    list[best_train_mse, best_train_r2] <- train_res
    list[best_valid_mse, best_valid_r2] <- valid_res

    best_model = i

    best_model_coefs = coef(my_model)[,best_model]
    best_model_coefs_vec = unname(best_model_coefs)
    best_model_indices = which(best_model_coefs_vec != 0)
    
    best_model_nnz = length(best_model_indices)
  }
}

my_R_df = cbind(feat_names[best_model_indices], coef(my_model)[best_model_indices,best_model])
colnames(my_R_df) = c("Name", "Coefficient")
write.csv(my_R_df, r_model_file, row.names = FALSE)


line0 = paste("Best model: alpha = ", grid[best_model], " Train R2: ", best_train_r2, " Train MSE: ", best_train_mse, " Valid R2: ", best_valid_r2, " Valid MSE: ", best_valid_mse, "\n", sep="")
cat(line0, file = logfile, append = TRUE)
line0 = paste("Nonzeros: ", best_model_nnz, "\n", sep="")
cat(line0, file = logfile, append = TRUE)

b = Sys.time()
cat("Elapsed time: \n", file = logfile, append = TRUE)    
line = paste0(round(as.numeric(difftime(time1 = b, time2 = a, units = "secs")), 3), " Seconds")
cat(line, file = logfile, append = TRUE)

# Plot

x_lab_str = paste("True", sc_type, sep=" ")
y_lab_str = paste("Predicted", sc_type, sep=" ")

train_plotfile_name = paste(sub_model_dir, "lasso_", feature_type, "_train_plot.png", sep="")

p = plot_parity(train_plotfile_name, "Training", train_df[,sc_type], train_preds[,best_model], x_lab_str, y_lab_str)

valid_plotfile_name = paste(sub_model_dir, "lasso_", feature_type, "_valid_plot.png", sep="")

p = plot_parity(valid_plotfile_name, "Valid", valid_df[,sc_type], valid_preds[,best_model], x_lab_str, y_lab_str)

rm(train_df)
rm(train_x)
rm(train_X)

rm(valid_df)
rm(valid_x)
rm(valid_X)

test_df = read.csv(paste(dataset_dir, "/", seed, "/test.csv", sep=""))

test_x = test_df[ , -which(names(test_df) %in% x_columns)]

test_y = test_df[, sc_type]

test_x[, sc_type] = test_y

if (nrow(which(is.na(test_x), arr.ind = TRUE)) > 0) stop("NAs in test dataset")

test_X = model.matrix(formula, test_x)

test_file = paste(sub_model_dir, "lasso_", feature_type, "_test_log.txt", sep="")

test_preds = predict(my_model, test_X)

test_preds = test_preds[,best_model]

list[test_mse, test_r2] <- goodness_of_fit_reg(test_y, test_preds)

cat("Test R2: ", test_r2, "\n", file = test_file)
cat("Test MSE: ", test_mse, "\n", file = test_file, append = TRUE)

test_plotfile_name = paste(sub_model_dir, "lasso_", feature_type, "_test_plot.png", sep="")

p = plot_parity(test_plotfile_name, "Test", test_df[,sc_type], test_preds, x_lab_str, y_lab_str)

# Create model file that the c program (sparse) will read

logfile = paste(sub_model_dir, "lasso_", feature_type, "_model.txt", sep="")

cat(length(best_model_indices), file = logfile)
cat("\n", file = logfile, append = TRUE)

for (i in 1:best_model_nnz) {

  index = best_model_indices[i]
  name = feat_names[index]

  if (startsWith(name, "(Intercept)")) {
    nterms = 1

    feat1_id = -2
    feat1_pow = 1

    feat2_id = -1
    feat2_pow = 1
  } else {
    comps = strsplit(name,":")[[1]]

    nterms = 1

    feat1_id = -1
    feat1_pow = 1

    feat2_id = -1
    feat2_pow = 1

    if ( length(comps) == 1 ) { #single feature
      
      feat1 = comps[1]
      if( startsWith(feat1, "poly") ) { # squared term
        feat1_pow = 2
      }

      subcomps_list = strsplit(feat1,"_")[[1]]
      feat1_id = strtoi(subcomps_list[2]) - 1
      
    } else if ( length(comps) > 1 ) { #interaction feature

      nterms = 2

      feat1 = comps[1]
      feat2 = comps[2]

      subcomps_list_1 = strsplit(feat1,"_")[[1]]

      feat1_id = strtoi(subcomps_list_1[2]) - 1

      subcomps_list_2 = strsplit(feat2,"_")[[1]]

      feat2_id = strtoi(subcomps_list_2[2]) - 1

    }
  }
  line = paste(nterms, feat1_id, feat1_pow, feat2_id, feat2_pow, best_model_coefs_vec[index], sep=",")
  if (i != best_model_nnz) {
    line = paste(line,"\n",sep="")
    cat(line, file = logfile, append = TRUE)
  } else {
    cat(line, file = logfile, append = TRUE)
  }
}

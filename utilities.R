goodness_of_fit_reg <- function(y, yhat) {

  mse = mean((y - yhat)^2)

  r2 = 1 - sum( (y - yhat) ^2) / sum( (y - mean(y)) ^2)

  result <- c(mse, r2)

  return(result)

}

plot_parity <- function(plot_f_name, data_type, y, yhat, x_lab_str, y_lab_str) {

  png(filename=plot_f_name)
  plot(y, yhat, xlab=x_lab_str, ylab=y_lab_str)
  title_str = paste(data_type, " Data (", length(y), " obs)", sep="")
  title(title_str)
  abline(a=0,b=1,col="red")
  dev.off()

}

plot_parity_max_diff <- function(y, yhat) {

  ind = which.max(abs(y - yhat))

  print(y[ind] - yhat[ind])

}

plot_roc <- function(fname, roc_obj) {

  png(filename=fname)
  plot(roc_obj, print.thres=TRUE)
  dev.off()

}

max_dev <- function(y, yhat) {

  abs_deviation = abs(yhat - y)
  max_dev_pred = max(abs_deviation)
  max_dev_pred_id = which(max_dev_pred == abs_deviation)

  obs = max_dev_pred_id

  print(paste("Max dev pred id: ", obs))
  print(paste("Max dev: ", max_dev_pred))

  print(paste("Pred: ", yhat[obs]))
  print(paste("True value: ", y[obs]))
}

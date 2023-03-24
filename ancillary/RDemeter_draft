## Marcus Perry, PNNL 03/23/23
## draft R script for running Demeter in R


# install.packages("reticulate")
library(reticulate)

source_python("RDemeter.py")
# now python function can be called() as normal within R body
config_file_path = '~\\demodata\\config_gcam_reference.ini' # your path here
write_outputs_flag=TRUE
run_demeterR(config_file_path, TRUE)


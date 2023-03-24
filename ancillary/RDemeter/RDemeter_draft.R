## Marcus Perry, PNNL/JCGRI 03/23/23
## R script for running Demeter in R

# install.packages("reticulate") # Do the first time
library(reticulate)

source_python("RDemeter.py")
# now python function can be called() as normal within R body
config_file_path = '~\\config_gcam_reference.ini' # your path here
write_outputs_flag = TRUE
# write_log_flag=FALSE # Maybe add in later
run_demeterR(config_file_path, write_outputs_flag)

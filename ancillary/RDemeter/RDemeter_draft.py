# Marcus Perry, PNNL 03/24/23
# draft python script using demo data from Demeter to run Demeter within R using R Reticulate package.

# pip install h5py, netCDF4 # (if not already installed)

import demeter

def run_demeterR(config_file_path, write_outputs_flag):
  config_file = config_file_path
  write_outputs = write_outputs_flag
  demeter.run_model(config_file=config_file, write_outputs=write_outputs)
  

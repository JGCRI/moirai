# Marcus Perry, PNNL 03/24/23
# draft python script using demo data from Demeter to run Demeter within R using R Reticulate package.

# pip install h5py
# pip install netcdf4 (requires h5py)

import demeter
import h5py
import netCDF4

def run_demeterR(config_file_path, write_outputs):
  config_file = '~\\demodata\\config_gcam_reference.ini' # your path here
  demeter.run_model(config_file=config_file, write_outputs=True)

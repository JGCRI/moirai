Marcus Perry PNNL 3/24/23

RDemeter draft Readme 

1. Ensure compatible version of Python is installed e.g. 3.9.9.
2. Clone Demeter repo and follow instructions to download demo data.
3. Create a virtual environment in the repo directory using a compatible version of Python and activate the venv.
4. Install Demeter requirements, netCDF4, maybe H5PY depending on versions, then Demeter.
5. Create .Rprofile with Sys.setenv(RETICULATE_PYTHON = '.venv/Script/python.exe')
6. Check for correct python version in R with reticulate::py_config(). Restart and try again if it is not correct.
7. Uncomment install.packages("reticulate") the first time running in R.

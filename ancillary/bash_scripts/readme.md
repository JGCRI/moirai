# Description
These bash scripts were used to get the soil grids data (https://soilgrids.org/) into the requisite format for moirai (binary raster files)

#Usage

This script will download the soil grids data and then generate 6 separate .tif files for each state of carbon (average, median, min, max, q1 and q3), These tifs will then be converted to .bil files. The code requires the `gdal` library to be installed and loaded. We have tested these scripts on a Windows machine with cygwin but should be able to be replicated on other machines.

##Warning: The script takes 8 hours to generate the files for each bash script. This is just because of the size of the soil_grids data. 
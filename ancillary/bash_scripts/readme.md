# 1. Scripts to get soil carbon data (MgC/ha) from soil grids data for mean and 95th percentile 

## Script names
`get_soil_grids_mean.sh` and `get_soil_grids_95th_percentile.sh`

## Description
These bash scripts were used to get the soil grids data (https://soilgrids.org/) into the requisite format for moirai (binary raster files)

## Usage
This script will download the soil grids data and then generate 6 separate .tif files for each state of carbon (average, median, min, max, q1 and q3), These tifs will then be converted to .bil files. The code requires the `gdal` library to be installed and loaded. We have tested these scripts on a Windows machine with cygwin but should be able to be replicated on other machines.

### Warning: The script takes 8 hours to generate the files for each bash script. This is just because of the size of the soil_grids data. 

# 2. Scripts to get soil carbon data (MgC/ha) from FAO Harmonized World Soil Database (FAOHWSD)

## Script names
`get_FAO_HWS_data.sh`

## Description
These bash scripts were used to get the soil carbon data from the harmonized world soil database (https://daac.ornl.gov/SOILS/guides/HWSD.html) into the requisite format for moirai (binary raster files)

## Usage
This script will download the FAO HWSD data (in nc format) and then generate 6 separate .tif files for each state of carbon (average, median, min, max, q1 and q3), These tifs will then be converted to .bil files. The code requires the `gdal` library to be installed and loaded. We have tested these scripts on a Windows machine with cygwin but should be able to be replicated on other machines. The raw units are in kg/m^2. These are converted into MgC/ha by the bash script.

# 3. Scripts to convert raster files (.tif) to moirai grid CRS

## Script names
`warp_raster_to_moirai_crs.sh`

## Description
This bash script is used to convert raw raster files in GeoTiff(.tif) format to the moirai CRS so they can be used as inputs within the LDS  

## Usage
* In order to use the script replace the values `input.tif`,`output.tif` with the input file name and the desired output file name. Also replace `output.bil` with the desired output name to be used within the LDS.
* The script "warps" the input file and converts the CRS, format, pizel size to the moirai grid CRS, format and pixel size (0.5 arc min). The moirai grid size parameters are as follows,

(1) Number of rows and columns - 4320,2160
(2) Extent - -180 -90 180 90 (The number of rows and columns and the extent will yield the 0.5 arc min pixel size)
(3) CRS - WGS84 ("+proj=longlat +ellps=WGS84")

* The output format is set to Float32
* The rasters are warped using an average method.  



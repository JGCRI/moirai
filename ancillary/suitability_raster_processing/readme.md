
# Script to convert EPA suitability raster files (.tif) to moirai grid CRS

## Script names
`warp_raster_to_moirai_crs.sh`

## Description
This bash script is used to convert raw raster files in GeoTiff(.tif) format to the moirai CRS so they can be used as inputs within the LDS. Each of the original six EPA suitability files were preprocessed using this script. These EPA files are thematic with each pixel being 1 (is in the filename class) or 0 (not in the filename class). The first step transforms these data, and the second step converts the transformed tif into a .bil file for use by Moirai. The final files for Moirai contain the fraction of pixel that is in the filename class. For example, L1_processed.bil contains the fraction of each pixel that has EPA L1 suitability.

* The script "warps" the input file to convert the CRS, format, and pizel size to the moirai grid CRS, format and pixel size (0.5 arc min). 
* The input pixels are either 1 or 0, and the average method calculates the fraction of the output pixel that is represented by input pixels with a value of 1
* Nodata pixels are set to zero in order for the averaging method to return proper fraction of pixel in a coastal pixel
* The output format is set to Float32

The moirai grid size parameters are as follows:

* (1) Number of rows and columns - 4320,2160
* (2) Extent - -180 -90 180 90 (The number of rows and columns and the extent will yield the 0.5 arc min pixel size)
* (3) CRS - WGS84 ("+proj=longlat +ellps=WGS84")  

## Usage
* In order to use the script replace the values `input.tif`,`output.tif` with the input file name and the desired output file name. Also replace `output.bil` with the desired output name to be used within the LDS.



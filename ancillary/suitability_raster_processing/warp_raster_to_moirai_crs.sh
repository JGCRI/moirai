#!/bin/bash
echo  "Please update input.tif, output.tif and output.bil with the neccessary names when using this bash script" 
# Warp raster to moirai CRS 
gdalwarp -ts 4320 2160 -r average input.tif output.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Convert to binary files
gdal_translate -of ENVI output.tif output.bil -a_nodata none

#!/bin/bash
echo  "Please make sure folder contains T_C.nc4 file" 
# Translate nc file to tiff 
gdal_translate T_C.nc4 FAO.tif -of GTiff
# Units are in kg/m2. Convert these to MgC/ha 
gdal_calc.py -A FAO_HWS.tif --calc="A*10" --outfile=FAO_HWS.tif --overwrite
# Get weighted average value 
gdalwarp -ts 4320 2160 -r average FAO_HWS.tif FAO_HWS_wavg.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Median
gdalwarp -ts 4320 2160 -r med FAO_HWS.tif FAO_HWS_median.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Minimum
gdalwarp -ts 4320 2160 -r min FAO_HWS.tif FAO_HWS_min.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Maximum
gdalwarp -ts 4320 2160 -r max FAO_HWS.tif FAO_HWS_max.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Q1
gdalwarp -ts 4320 2160 -r q1 FAO_HWS.tif FAO_HWS_q1.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Q3
gdalwarp -ts 4320 2160 -r q3 FAO_HWS.tif FAO_HWS_q3.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Convert to binary files
gdal_translate -of ENVI FAO_HWS_wavg.tif FAO_HWS_wavg.bil -a_nodata none
gdal_translate -of ENVI FAO_HWS_median.tif FAO_HWS_median.bil -a_nodata none
gdal_translate -of ENVI FAO_HWS_min.tif FAO_HWS_min.bil -a_nodata none
gdal_translate -of ENVI FAO_HWS_max.tif FAO_HWS_max.bil -a_nodata none
gdal_translate -of ENVI FAO_HWS_q1.tif FAO_HWS_q1.bil -a_nodata none
gdal_translate -of ENVI FAO_HWS_q3.tif FAO_HWS_q3.bil -a_nodata none
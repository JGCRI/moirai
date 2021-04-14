#!/bin/bash


echo "Starting download of soil grids 95 percentile"


gdal_translate -co "TILED=YES" -co "COMPRESS=DEFLATE" -co "PREDICTOR=2" -co "BIGTIFF=YES"  "/vsicurl?max_retry=3&retry_delay=1&list_dir=no&url=https://files.isric.org/soilgrids/latest/data//ocs/ocs_0-30cm_Q0.95.vrt" "ocs_0-30cm_Q0.95.tif"


#Warp rasters to moirai format

gdalwarp -ts 4320 2160 -r average ocs_0-30cm_Q0.95.tif ocs_95_wavg.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 

gdalwarp -ts 4320 2160 -r med ocs_0-30cm_Q0.95.tif ocs_95_median.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999

gdalwarp -ts 4320 2160 -r min ocs_0-30cm_Q0.95.tif ocs_95_min.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999

gdalwarp -ts 4320 2160 -r max ocs_0-30cm_Q0.95.tif ocs_95_max.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999

gdalwarp -ts 4320 2160 -r q1 ocs_0-30cm_Q0.95.tif ocs_95_q1.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999

gdalwarp -ts 4320 2160 -r q3 ocs_0-30cm_Q0.95.tif ocs_95_q3.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999


#Translate to binary files

gdal_translate -of ENVI ocs_95_wavg.tif soil_carbon_weighted_average_95pct.bil -a_nodata none

gdal_translate -of ENVI ocs_95_median.tif soil_carbon_median_95pct.bil -a_nodata none

gdal_translate -of ENVI ocs_95_min.tif soil_carbon_min_95pct.bil -a_nodata none

gdal_translate -of ENVI ocs_95_max.tif soil_carbon_max_95pct.bil -a_nodata none

gdal_translate -of ENVI ocs_95_q1.tif soil_carbon_q1_95pct.bil -a_nodata none

gdal_translate -of ENVI ocs_95_q3.tif soil_carbon_q3_95pct.bil -a_nodata none

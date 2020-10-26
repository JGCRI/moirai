#!/bin/bash
echo "Starting download of soil grids mean"
echo  " Make sure you have gdal installed and loaded" 
gdal_translate -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" -co "PREDICTOR=2" -co "BIGTIFF=YES" "/vsicurl?max_retry=3&retry_delay=1&list_dir=no&url=https://files.isric.org/soilgrids/latest/data//ocs/ocs_0-30cm_mean.vrt" "ocs_0-30cm_mean.tif"
# now warp the rasters using gdal
#Weighted average
gdalwarp -ts 4320 2160 -r average ocs_0-30cm_mean.tif soil_carbon_weighted_average.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Median
gdalwarp -ts 4320 2160 -r med ocs_0-30cm_mean.tif soil_carbon_median.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Minimum
gdalwarp -ts 4320 2160 -r min ocs_0-30cm_mean.tif soil_carbon_min.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Maximum
gdalwarp -ts 4320 2160 -r max ocs_0-30cm_mean.tif soil_carbon_max.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Q1
gdalwarp -ts 4320 2160 -r q1 ocs_0-30cm_mean.tif soil_carbon_q1.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Q3
gdalwarp -ts 4320 2160 -r q3 ocs_0-30cm_mean.tif soil_carbon_q3.tif -ot Float32 -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata 0
#Now translate to bil file
gdal_translate -of ENVI soil_carbon_weighted_average.tif soil_carbon_weighted_average.bil -a_nodata none
gdal_translate -of ENVI soil_carbon_median.tif soil_carbon_median.bil -a_nodata none
gdal_translate -of ENVI soil_carbon_min.tif soil_carbon_min.bil -a_nodata none
gdal_translate -of ENVI soil_carbon_max.tif soil_carbon_max.bil -a_nodata none
gdal_translate -of ENVI soil_carbon_q1.tif soil_carbon_q1.bil -a_nodata none
gdal_translate -of ENVI soil_carbon_q3.tif soil_carbon_q3.bil -a_nodata none

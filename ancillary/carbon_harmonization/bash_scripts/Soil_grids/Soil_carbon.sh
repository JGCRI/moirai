#First cut the data to fit ESA resolution 
gdalwarp -ts 129588 52198 -r average soil_grids_data.tif soil_carbon_clipped.tif -ot Int16 -t_srs "+proj=longlat +ellps=WGS84" -te -179.9977320 -61.0048800 179.997320 84.0011640 -dstnodata 0 -co TILED=YES -co COMPRESS=DEFLATE -co BIGTIFF=YES



##Unknown_Herb

#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Unknown_Herb.tif --outfile=LC/Unknown_Herb_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0 

#Start 
gdalwarp -ts 4320 2160 -r average LC/Unknown_Herb_soil.tif Soil_carbon_rasters/Unknown_Herb_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Unknown_Herb_soil.tif Soil_carbon_rasters/Unknown_Herb_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Unknown_Herb_soil.tif Soil_carbon_rasters/Unknown_Herb_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Unknown_Herb_soil.tif Soil_carbon_rasters/Unknown_Herb_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Unknown_Herb_soil.tif Soil_carbon_rasters/Unknown_Herb_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Unknown_Herb_soil.tif Soil_carbon_rasters/Unknown_Herb_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Unknown_Tree

#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Unknown_Tree.tif --outfile=LC/Unknown_Tree_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Unknown_Tree_soil.tif Soil_carbon_rasters/Unknown_Tree_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Unknown_Tree_soil.tif Soil_carbon_rasters/Unknown_Tree_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Unknown_Tree_soil.tif Soil_carbon_rasters/Unknown_Tree_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Unknown_Tree_soil.tif Soil_carbon_rasters/Unknown_Tree_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Unknown_Tree_soil.tif Soil_carbon_rasters/Unknown_Tree_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Unknown_Tree_soil.tif Soil_carbon_rasters/Unknown_Tree_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Broadleaf_Decidous

#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Broadleaf_Decidous.tif --outfile=LC/Broadleaf_Decidous_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Broadleaf_Decidous_soil.tif Soil_carbon_rasters/Broadleaf_Decidous_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Broadleaf_Decidous_soil.tif Soil_carbon_rasters/Broadleaf_Decidous_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Broadleaf_Decidous_soil.tif Soil_carbon_rasters/Broadleaf_Decidous_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Broadleaf_Decidous_soil.tif Soil_carbon_rasters/Broadleaf_Decidous_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Broadleaf_Decidous_soil.tif Soil_carbon_rasters/Broadleaf_Decidous_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Broadleaf_Decidous_soil.tif Soil_carbon_rasters/Broadleaf_Decidous_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Broadleaf_Evergreen

#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Broadleaf_Evergreen.tif --outfile=LC/Broadleaf_Evergreen_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Broadleaf_Evergreen_soil.tif Soil_carbon_rasters/Broadleaf_Evergreen_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Broadleaf_Evergreen_soil.tif Soil_carbon_rasters/Broadleaf_Evergreen_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Broadleaf_Evergreen_soil.tif Soil_carbon_rasters/Broadleaf_Evergreen_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Broadleaf_Evergreen_soil.tif Soil_carbon_rasters/Broadleaf_Evergreen_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Broadleaf_Evergreen_soil.tif Soil_carbon_rasters/Broadleaf_Evergreen_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Broadleaf_Evergreen_soil.tif Soil_carbon_rasters/Broadleaf_Evergreen_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Desert

#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Desert.tif --outfile=LC/Desert_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Desert_soil.tif Soil_carbon_rasters/Desert_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Desert_soil.tif Soil_carbon_rasters/Desert_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Desert_soil.tif Soil_carbon_rasters/Desert_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Desert_soil.tif Soil_carbon_rasters/Desert_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Desert_soil.tif Soil_carbon_rasters/Desert_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Desert_soil.tif Soil_carbon_rasters/Desert_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Flood_Tree_Cover
#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Flood_Tree_Cover.tif --outfile=LC/Flood_Tree_Cover_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Flood_Tree_Cover_soil.tif Soil_carbon_rasters/Flood_Tree_Cover_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Flood_Tree_Cover_soil.tif Soil_carbon_rasters/Flood_Tree_Cover_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Flood_Tree_Cover_soil.tif Soil_carbon_rasters/Flood_Tree_Cover_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Flood_Tree_Cover_soil.tif Soil_carbon_rasters/Flood_Tree_Cover_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Flood_Tree_Cover_soil.tif Soil_carbon_rasters/Flood_Tree_Cover_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Flood_Tree_Cover_soil.tif Soil_carbon_rasters/Flood_Tree_Cover_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Flood_Shrub


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Flood_Shrub.tif --outfile=LC/Flood_Shrub_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Flood_Shrub_soil.tif Soil_carbon_rasters/Flood_Shrub_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Flood_Shrub_soil.tif Soil_carbon_rasters/Flood_Shrub_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r min  LC/Flood_Shrub_soil.tif Soil_carbon_rasters/Flood_Shrub_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r max  LC/Flood_Shrub_soil.tif Soil_carbon_rasters/Flood_Shrub_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Flood_Shrub_soil.tif Soil_carbon_rasters/Flood_Shrub_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Flood_Shrub_soil.tif Soil_carbon_rasters/Flood_Shrub_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Grasslands


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Grasslands.tif --outfile=LC/Grasslands_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Grasslands_soil.tif Soil_carbon_rasters/Grasslands_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Grasslands_soil.tif Soil_carbon_rasters/Grasslands_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Grasslands_soil.tif Soil_carbon_rasters/Grasslands_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Grasslands_soil.tif Soil_carbon_rasters/Grasslands_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Grasslands_soil.tif Soil_carbon_rasters/Grasslands_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Grasslands_soil.tif Soil_carbon_rasters/Grasslands_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Mixed_Forests


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Mixed_Forests.tif --outfile=LC/Mixed_Forests_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Mixed_Forests_soil.tif Soil_carbon_rasters/Mixed_Forests_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Mixed_Forests_soil.tif Soil_carbon_rasters/Mixed_Forests_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Mixed_Forests_soil.tif Soil_carbon_rasters/Mixed_Forests_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Mixed_Forests_soil.tif Soil_carbon_rasters/Mixed_Forests_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Mixed_Forests_soil.tif Soil_carbon_rasters/Mixed_Forests_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Mixed_Forests_soil.tif Soil_carbon_rasters/Mixed_Forests_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Mosaic_Herb


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Mosaic_Herb.tif --outfile=LC/Mosaic_Herb_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Mosaic_Herb_soil.tif Soil_carbon_rasters/Mosaic_Herb_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Mosaic_Herb_soil.tif Soil_carbon_rasters/Mosaic_Herb_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Mosaic_Herb_soil.tif Soil_carbon_rasters/Mosaic_Herb_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Mosaic_Herb_soil.tif Soil_carbon_rasters/Mosaic_Herb_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Mosaic_Herb_soil.tif Soil_carbon_rasters/Mosaic_Herb_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Mosaic_Herb_soil.tif Soil_carbon_rasters/Mosaic_Herb_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Mosaic_Tree


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Mosaic_Tree.tif --outfile=LC/Mosaic_Tree_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Mosaic_Tree_soil.tif Soil_carbon_rasters/Mosaic_Tree_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Mosaic_Tree_soil.tif Soil_carbon_rasters/Mosaic_Tree_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Mosaic_Tree_soil.tif Soil_carbon_rasters/Mosaic_Tree_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Mosaic_Tree_soil.tif Soil_carbon_rasters/Mosaic_Tree_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Mosaic_Tree_soil.tif Soil_carbon_rasters/Mosaic_Tree_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Mosaic_Tree_soil.tif Soil_carbon_rasters/Mosaic_Tree_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Needleleaved_Decidous


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Needleleaved_Decidous.tif --outfile=LC/Needleleaved_Decidous_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Needleleaved_Decidous_soil.tif Soil_carbon_rasters/Needleleaved_Decidous_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Needleleaved_Decidous_soil.tif Soil_carbon_rasters/Needleleaved_Decidous_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Needleleaved_Decidous_soil.tif Soil_carbon_rasters/Needleleaved_Decidous_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Needleleaved_Decidous_soil.tif Soil_carbon_rasters/Needleleaved_Decidous_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Needleleaved_Decidous_soil.tif Soil_carbon_rasters/Needleleaved_Decidous_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Needleleaved_Decidous_soil.tif Soil_carbon_rasters/Needleleaved_Decidous_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Needleleaved_Evergreen


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Needleleaved_Evergreen.tif --outfile=LC/Needleleaved_Evergreen_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Needleleaved_Evergreen_soil.tif Soil_carbon_rasters/Needleleaved_Evergreen_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Needleleaved_Evergreen_soil.tif Soil_carbon_rasters/Needleleaved_Evergreen_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Needleleaved_Evergreen_soil.tif Soil_carbon_rasters/Needleleaved_Evergreen_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Needleleaved_Evergreen_soil.tif Soil_carbon_rasters/Needleleaved_Evergreen_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Needleleaved_Evergreen_soil.tif Soil_carbon_rasters/Needleleaved_Evergreen_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Needleleaved_Evergreen_soil.tif Soil_carbon_rasters/Needleleaved_Evergreen_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Polar_Desert_Rock_Ice


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Polar_Desert_Rock_Ice.tif --outfile=LC/Polar_Desert_Rock_Ice_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Polar_Desert_Rock_Ice_soil.tif Soil_carbon_rasters/Polar_Desert_Rock_Ice_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Polar_Desert_Rock_Ice_soil.tif Soil_carbon_rasters/Polar_Desert_Rock_Ice_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Polar_Desert_Rock_Ice_soil.tif Soil_carbon_rasters/Polar_Desert_Rock_Ice_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Polar_Desert_Rock_Ice_soil.tif Soil_carbon_rasters/Polar_Desert_Rock_Ice_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Polar_Desert_Rock_Ice_soil.tif Soil_carbon_rasters/Polar_Desert_Rock_Ice_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Polar_Desert_Rock_Ice_soil.tif Soil_carbon_rasters/Polar_Desert_Rock_Ice_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Shrubland

#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Shrubland.tif --outfile=LC/Shrubland_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Shrubland_soil.tif Soil_carbon_rasters/Shrubland_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Shrubland_soil.tif Soil_carbon_rasters/Shrubland_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Shrubland_soil.tif Soil_carbon_rasters/Shrubland_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Shrubland_soil.tif Soil_carbon_rasters/Shrubland_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Shrubland_soil.tif Soil_carbon_rasters/Shrubland_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Shrubland_soil.tif Soil_carbon_rasters/Shrubland_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Sparse_Shrub


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Sparse_Shrub.tif --outfile=LC/Sparse_Shrub_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Sparse_Shrub_soil.tif Soil_carbon_rasters/Sparse_Shrub_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Sparse_Shrub_soil.tif Soil_carbon_rasters/Sparse_Shrub_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Sparse_Shrub_soil.tif Soil_carbon_rasters/Sparse_Shrub_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Sparse_Shrub_soil.tif Soil_carbon_rasters/Sparse_Shrub_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Sparse_Shrub_soil.tif Soil_carbon_rasters/Sparse_Shrub_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Sparse_Shrub_soil.tif Soil_carbon_rasters/Sparse_Shrub_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Sparse_Tree


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Sparse_Tree.tif --outfile=LC/Sparse_Tree_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Sparse_Tree_soil.tif Soil_carbon_rasters/Sparse_Tree_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Sparse_Tree_soil.tif Soil_carbon_rasters/Sparse_Tree_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Sparse_Tree_soil.tif Soil_carbon_rasters/Sparse_Tree_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Sparse_Tree_soil.tif Soil_carbon_rasters/Sparse_Tree_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Sparse_Tree_soil.tif Soil_carbon_rasters/Sparse_Tree_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Sparse_Tree_soil.tif Soil_carbon_rasters/Sparse_Tree_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Evergreen_Combined


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Evergreen_Combined.tif --outfile=LC/Evergreen_Combined_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Evergreen_Combined_soil.tif Soil_carbon_rasters/Evergreen_Combined_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Evergreen_Combined_soil.tif Soil_carbon_rasters/Evergreen_Combined_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Evergreen_Combined_soil.tif Soil_carbon_rasters/Evergreen_Combined_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Evergreen_Combined_soil.tif Soil_carbon_rasters/Evergreen_Combined_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Evergreen_Combined_soil.tif Soil_carbon_rasters/Evergreen_Combined_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Evergreen_Combined_soil.tif Soil_carbon_rasters/Evergreen_Combined_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Decidous_Combined


#Get carbon
gdal_calc.py -A soil_carbon_clipped.tif -B LC/Decidous_Combined.tif --outfile=LC/Decidous_Combined_soil.tif --calc="(A>-32767)*(A*B*0.1)" --co="TILED=YES" --co="COMPRESS=DEFLATE" --NoDataValue=0

#Start 
gdalwarp -ts 4320 2160 -r average LC/Decidous_Combined_soil.tif Soil_carbon_rasters/Decidous_Combined_soil_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Decidous_Combined_soil.tif Soil_carbon_rasters/Decidous_Combined_soil_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Decidous_Combined_soil.tif Soil_carbon_rasters/Decidous_Combined_soil_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Decidous_Combined_soil.tif Soil_carbon_rasters/Decidous_Combined_soil_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Decidous_Combined_soil.tif Soil_carbon_rasters/Decidous_Combined_soil_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Decidous_Combined_soil.tif Soil_carbon_rasters/Decidous_Combined_soil_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0






#First generate ESA tiled raster
gdal_translate NETCDF:"ESACCI-LC-L4-LCCS-Map-300m-P1Y-2010-v2.0.7cds.nc":lccs_class ESA_tiled.tif -co TILED=YES -co COMPRESS=DEFLATE -co BIGTIFF=YES

##Unknown_Herb
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Unknown_Herb.tif --calc="(1*(A==11))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Unknown_Tree
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Unknown_Tree.tif --calc="(1*(A==12))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Broadleaf_Decidous
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Broadleaf_Decidous.tif --calc="(1*(A==60))+(1*(A==61))+(1*(A==62))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Broadleaf_Evergreen
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Broadleaf_Evergreen.tif --calc="(1*(A==50))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Desert
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Desert.tif --calc="(1*(A==200))+(1*(A==201))+(1*(A==202))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Flood_Tree_Cover
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Flood_Tree_Cover.tif --calc="(1*(A==160))+(1*(A==170))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Flood_Shrub
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Flood_Shrub.tif --calc="(1*(A==180))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Grasslands
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Grasslands.tif --calc="(1*(A==130))+(1*(A==140))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Mixed_Forests
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Mixed_Forests.tif --calc="(1*(A==130))+(1*(A==140))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Mosaic_Herb
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Mosaic_Herb.tif --calc="(1*(A==110))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

##Mosaic_Tree
gdal_calc.py -A ESA_tiled.tif --outfile=LC/Mosaic_Tree.tif --calc="(1*(A==40))+(1*(A==100))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Mosaic_Tree.tif --outfile=LC/Mosaic_Tree_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Mosaic_Tree_AG.tif Carbon_rasters/Mosaic_Tree_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Mosaic_Tree_AG.tif Carbon_rasters/Mosaic_Tree_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Mosaic_Tree_AG.tif Carbon_rasters/Mosaic_Tree_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Mosaic_Tree_AG.tif Carbon_rasters/Mosaic_Tree_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Mosaic_Tree_AG.tif Carbon_rasters/Mosaic_Tree_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Mosaic_Tree_AG.tif Carbon_rasters/Mosaic_Tree_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Needleleaved_Decidous

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Needleleaved_Decidous.tif --calc="(1*(A==80))+(1*(A==81))+(1*(A==82))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Needleleaved_Decidous.tif --outfile=LC/Needleleaved_Decidous_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Needleleaved_Decidous_AG.tif Carbon_rasters/Needleleaved_Decidous_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Needleleaved_Decidous_AG.tif Carbon_rasters/Needleleaved_Decidous_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Needleleaved_Decidous_AG.tif Carbon_rasters/Needleleaved_Decidous_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Needleleaved_Decidous_AG.tif Carbon_rasters/Needleleaved_Decidous_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Needleleaved_Decidous_AG.tif Carbon_rasters/Needleleaved_Decidous_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Needleleaved_Decidous_AG.tif Carbon_rasters/Needleleaved_Decidous_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Needleleaved_Evergreen

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Needleleaved_Evergreen.tif --calc="(1*(A==70))+(1*(A==71))+(1*(A==72))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Needleleaved_Evergreen.tif --outfile=LC/Needleleaved_Evergreen_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Needleleaved_Evergreen_AG.tif Carbon_rasters/Needleleaved_Evergreen_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Needleleaved_Evergreen_AG.tif Carbon_rasters/Needleleaved_Evergreen_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Needleleaved_Evergreen_AG.tif Carbon_rasters/Needleleaved_Evergreen_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Needleleaved_Evergreen_AG.tif Carbon_rasters/Needleleaved_Evergreen_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Needleleaved_Evergreen_AG.tif Carbon_rasters/Needleleaved_Evergreen_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Needleleaved_Evergreen_AG.tif Carbon_rasters/Needleleaved_Evergreen_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Polar_Desert_Rock_Ice

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Polar_Desert_Rock_Ice.tif --calc="(1*(A==220))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Polar_Desert_Rock_Ice.tif --outfile=LC/Polar_Desert_Rock_Ice_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Polar_Desert_Rock_Ice_AG.tif Carbon_rasters/Polar_Desert_Rock_Ice_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Polar_Desert_Rock_Ice_AG.tif Carbon_rasters/Polar_Desert_Rock_Ice_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Polar_Desert_Rock_Ice_AG.tif Carbon_rasters/Polar_Desert_Rock_Ice_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Polar_Desert_Rock_Ice_AG.tif Carbon_rasters/Polar_Desert_Rock_Ice_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Polar_Desert_Rock_Ice_AG.tif Carbon_rasters/Polar_Desert_Rock_Ice_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Polar_Desert_Rock_Ice_AG.tif Carbon_rasters/Polar_Desert_Rock_Ice_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Shrubland

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Shrubland.tif --calc="(1*(A==120))+(1*(A==121))+(1*(A==122))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Shrubland.tif --outfile=LC/Shrubland_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Shrubland_AG.tif Carbon_rasters/Shrubland_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Shrubland_AG.tif Carbon_rasters/Shrubland_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Shrubland_AG.tif Carbon_rasters/Shrubland_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Shrubland_AG.tif Carbon_rasters/Shrubland_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Shrubland_AG.tif Carbon_rasters/Shrubland_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Shrubland_AG.tif Carbon_rasters/Shrubland_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0


##Sparse_Shrub

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Sparse_Shrub.tif --calc="(1*(A==152))+(1*(A==153))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Sparse_Shrub.tif --outfile=LC/Sparse_Shrub_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Sparse_Shrub_AG.tif Carbon_rasters/Sparse_Shrub_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Sparse_Shrub_AG.tif Carbon_rasters/Sparse_Shrub_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Sparse_Shrub_AG.tif Carbon_rasters/Sparse_Shrub_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Sparse_Shrub_AG.tif Carbon_rasters/Sparse_Shrub_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Sparse_Shrub_AG.tif Carbon_rasters/Sparse_Shrub_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Sparse_Shrub_AG.tif Carbon_rasters/Sparse_Shrub_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Sparse_Tree

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Sparse_Tree.tif --calc="(1*(A==152))+(1*(A==153))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Sparse_Tree.tif --outfile=LC/Sparse_Tree_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Sparse_Tree_AG.tif Carbon_rasters/Sparse_Tree_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0 
gdalwarp -ts 4320 2160 -r med  LC/Sparse_Tree_AG.tif Carbon_rasters/Sparse_Tree_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Sparse_Tree_AG.tif Carbon_rasters/Sparse_Tree_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Sparse_Tree_AG.tif Carbon_rasters/Sparse_Tree_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Sparse_Tree_AG.tif Carbon_rasters/Sparse_Tree_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Sparse_Tree_AG.tif Carbon_rasters/Sparse_Tree_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Evergreen_Combined

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Evergreen_Combined.tif --calc="(1*(A==50))+(1*(A==70))+(1*(A==71))+(1*(A==72))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Evergreen_Combined.tif --outfile=LC/Evergreen_Combined_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Evergreen_Combined_AG.tif Carbon_rasters/Evergreen_Combined_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Evergreen_Combined_AG.tif Carbon_rasters/Evergreen_Combined_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Evergreen_Combined_AG.tif Carbon_rasters/Evergreen_Combined_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Evergreen_Combined_AG.tif Carbon_rasters/Evergreen_Combined_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Evergreen_Combined_AG.tif Carbon_rasters/Evergreen_Combined_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Evergreen_Combined_AG.tif Carbon_rasters/Evergreen_Combined_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0

##Decidous_Combined

gdal_calc.py -A ESA_tiled.tif --outfile=LC/Decidous_Combined.tif --calc="(1*(A==60))+(1*(A==61))+(1*(A==62))+(1*(A==80))+(1*(A==81))+(1*(A==82))" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Get carbon
gdal_calc.py -A ag_carbon_clipped.tif -B LC/Decidous_Combined.tif --outfile=LC/Decidous_Combined_AG.tif --calc="(A!=65536)*(A*B)" --co="TILED=YES" --co="COMPRESS=DEFLATE"

#Start 
gdalwarp -ts 4320 2160 -r average LC/Decidous_Combined_AG.tif Carbon_rasters/Decidous_Combined_AG_weighted_average.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r med  LC/Decidous_Combined_AG.tif Carbon_rasters/Decidous_Combined_AG_median.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r min  LC/Decidous_Combined_AG.tif Carbon_rasters/Decidous_Combined_AG_min.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r max  LC/Decidous_Combined_AG.tif Carbon_rasters/Decidous_Combined_AG_max.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q1  LC/Decidous_Combined_AG.tif Carbon_rasters/Decidous_Combined_AG_q1.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0
gdalwarp -ts 4320 2160 -r q3 LC/Decidous_Combined_AG.tif Carbon_rasters/Decidous_Combined_AG_q3.bil -ot Float32 -of ENVI -t_srs "+proj=longlat +ellps=WGS84" -te -180 -90 180 90 -dstnodata -9999 -srcnodata 0






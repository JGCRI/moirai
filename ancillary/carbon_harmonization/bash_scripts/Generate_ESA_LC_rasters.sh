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


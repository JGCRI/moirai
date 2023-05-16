# Create, class, and reclass above ground carbon masks, then calculate carbon state statistics.

library(terra)
# library(parallel) # for detecting cores 

Sys.getenv('TEMP')
wd <- "D:/moiraiR/carbon_harmonization/"
setwd(wd)

## YMMV but may allow you to process rasters in-memory for enhanced speed if you have ~50-60gb free RAM
# terraOptions(memmax=57)
# terraOptions(memfrac=.9)


land_cover_raster <- rast("rasters/ESACCI-LC-L4-LCCS-Map-300m-P1Y-2010-v2.0.7.tif")
spawn_raster <- rast("rasters/aboveground_biomass_carbon_2010.tif")
old_res <- res(spawn_raster)
old_ext <- ext(spawn_raster)
new_ext <- ext(-180, 180, -61, 84) ## Removes the most southerly row 

######## Note crop() may result in raster having attribute of incorrect Max value, but unlikely to have actually changed the cell values.
spawn_to_mask <- crop(spawn_raster, new_ext, extend=FALSE, filename="rasters/spawn_cropped.tif", overwrite=TRUE, datatype='INT2U')
spawn_masked <- rast("rasters/spawn_cropped.tif")


# Align the extents of the rasters via cropping the larger one
LC_raster_cropped <- crop(land_cover_raster, spawn_masked)
cropped_output_name <- "rasters/ESA_LC_cropped_to_spawn.tif"


# LC raster only has values 0 to 220 so 8 bit unsigned works
writeRaster(LC_raster_cropped, cropped_output_name, overwrite=TRUE, gdal=c("COMPRESS=LZW"), datatype='INT1U')
written <- rast("rasters/ESA_LC_cropped_to_spawn.tif")
#### Origin, extent and cell sizes should now match

ESA_pixel_values <- c(10, 11, 12, 20, 30, 40, 50, 60, 61, 62, 70, 71, 72, 80, 81, 82, 90, 100, 110, 120, 121, 122, 130, 140, 150, 151, 152, 153, 160, 170, 180, 190, 200, 201, 202, 220)
value_count <- c(seq(1, 36))
ESA_classes <- c("cropland", "unknown_herb", "unknown_tree", "cropland", "mosaic_crop", "mosaic_tree", "broadleaf_evergreen", "broadleaf_deciduous", "broadleaf_deciduous", "broadleaf_deciduous", "needleleaved_evergreen", "needleleaved_evergreen", "needleleaved_evergreen", "needleleaved_deciduous", "needleleaved_deciduous", "needleleaved_deciduous", "mixed_forests", "mosaic_tree", "mosaic_herb", "shrubland", "shrubland", "shrubland", "grasslands", "grasslands", "sparse_tree", "sparse_tree", "sparse_shrub", "sparse_shrub", "flood_tree_cover", "flood_tree_cover", "flooded_shrub", "urbanland", "desert", "desert", "desert", "polar_desert_rock_ice")

rcl <- cbind(ESA_pixel_values, value_count)
rownames(rcl) <- ESA_classes

classified_ESA_LC <- "rasters/classified_ESA_LC.tif"
classify(written, rcl,others=0, filename=classified_ESA_LC, overwrite=TRUE)
classified_ESA_LC_written <- rast("rasters/classified_ESA_LC.tif")
# See if it worked:
plot(classified_ESA_LC_written)

cropland_mask_name <- "rasters/Cropland_AG.tif"
ukherb_mask_name <- "rasters/Unknown_Herb_AG.tif"
uktree_mask_name <- "rasters/Unknown_Tree_AG.tif"
mosaicC_mask_name <- "rasters/Mosaic_Crop_AG.tif"
mosaicT_mask_name <- "rasters/Mosaic_Tree_AG.tif"
BLEver_mask_name <- "rasters/Broadleaf_Evergreen_AG.tif"
BLDec_mask_name <- "rasters/Broadleaf_Decidous_AG.tif"
NLEver_mask_name <- "rasters/Needleleaved_Evergreen_AG.tif"
NLDec_mask_name <- "rasters/Needleleaved_Decidous_AG.tif"
mixed_forests_mask_name <- "rasters/Mixed_Forests_AG.tif"
mosaicH_mask_name <- "rasters/Mosaic_Herb_AG.tif"
shrubland_mask_name <- "rasters/Shrubland_AG.tif"
grasslands_mask_name <- "rasters/Grasslands_AG.tif"
sparseT_mask_name <- "rasters/Sparse_Tree_AG.tif"
sparseS_mask_name <- "rasters/Sparse_Shrub_AG.tif"
floodTC_mask_name <- "rasters/Flood_Tree_Cover_AG.tif"
floodSh_mask_name <- "rasters/Flood_Shrub_AG.tif"
urban_mask_name <- "rasters/Urban_AG.tif"
desert_mask_name <- "rasters/Desert_AG.tif"
polarDRI_mask_name <- "rasters/Polar_Desert_Rock_Ice_AG.tif"

### Reclass to moirai paper classes
start.time <- Sys.time()
start.time
ifel(classified_ESA_LC_written == 1 | classified_ESA_LC_written == 4, 1, 255, filename=cropland_mask_name, overwrite=TRUE, datatype="INT1U") # Mask where pixel values equal to 1 or 4 are set to 1 and all else to 0 (nodata)
ifel(classified_ESA_LC_written == 2, 2, 255, filename=ukherb_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 3, 3, 255, filename=uktree_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 5, 4, 255, filename=mosaicC_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 6 | classified_ESA_LC_written == 18, 5, 255, filename=mosaicT_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 7, 6, 255, filename=BLEver_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 8 | classified_ESA_LC_written == 9 | classified_ESA_LC_written == 10, 7, 255, filename=BLDec_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 11 | classified_ESA_LC_written == 12 | classified_ESA_LC_written == 13, 8, 255, filename=NLEver_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 14 | classified_ESA_LC_written == 15 | classified_ESA_LC_written == 16, 9, 255, filename=NLDec_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 17, 10, 255, filename=mixed_forests_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 19, 11, 255, filename=mosaicH_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 20 | classified_ESA_LC_written == 21 | classified_ESA_LC_written == 22, 12, 255, filename=shrubland_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 23 | classified_ESA_LC_written == 24, 13, 255, filename=grasslands_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 25 | classified_ESA_LC_written == 26, 14, 255, filename=sparseT_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 27 | classified_ESA_LC_written == 28, 15, 255, filename=sparseS_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 29 | classified_ESA_LC_written == 30, 16, 255, filename=floodTC_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 31, 17, 255, filename=floodSh_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 32, 18, 255, filename=urban_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 33 | classified_ESA_LC_written == 34 | classified_ESA_LC_written == 35, 19, 255, filename=desert_mask_name, overwrite=TRUE, datatype="INT1U")
ifel(classified_ESA_LC_written == 36, 20, 255, filename=polarDRI_mask_name, overwrite=TRUE, datatype="INT1U")
end.time <- Sys.time()
time.taken <- round(end.time - start.time)
time.taken # ~5.5 hours
# Do combined tree ones need to be included?

#### Make SpatRast objects of masks
cropland_mask_rastobj <- rast(cropland_mask_name)
ukherb_mask_rastobj <- rast(ukherb_mask_name)
uktree_mask_rastobj <- rast(uktree_mask_name)
mosaicC_mask_rastobj <- rast(mosaicC_mask_name)
mosaicT_mask_rastobj <- rast(mosaicT_mask_name)
BLEver_mask_rastobj <- rast(BLEver_mask_name)
BLDec_mask_rastobj <- rast(BLDec_mask_name)
NLEver_mask_rastobj <- rast(NLEver_mask_name)
NLDec_mask_rastobj <- rast(NLDec_mask_name)
mixed_forests_mask_rastobj <- rast(mixed_forests_mask_name)
mosaicH_mask_rastobj <- rast(mosaicH_mask_name)
shrubland_mask_rastobj <- rast(shrubland_mask_name)
grasslands_mask_rastobj <- rast(grasslands_mask_name)
sparseT_mask_rastobj <- rast(sparseT_mask_name)
sparseS_mask_rastobj <- rast(sparseS_mask_name)
floodTC_mask_rastobj <- rast(floodTC_mask_name)
floodSh_mask_rastobj <- rast(floodSh_mask_name)
urban_mask_rastobj <- rast(urban_mask_name)
desert_mask_rastobj <- rast(desert_mask_name)
polarDRI_mask_rastobj <- rast(polarDRI_mask_name)

#### Get carbon values per class mask for above ground 2010 carbon
start.time <- Sys.time()
start.time
cropland_AG_carbon <- crop(spawn_masked, cropland_mask_rastobj, mask=TRUE, extend=FALSE, overwrite=TRUE, datatype="INT2U", filename="rasters/Cropland_AG_carbon.tif")
ukherb_AG_carbone <- crop(spawn_masked, ukherb_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Unknown_Herb_AG_carbon.tif")
uktree_AG_carbon <- crop(spawn_masked, uktree_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Unknown_Tree_AG_carbon.tif")
mosaicC_AG_carbon <- crop(spawn_masked, mosaicC_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Mosaic_Crop_AG_carbon.tif")
mosaicT_AG_carbon <- crop(spawn_masked, mosaicT_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Mosaic_Tree_AG_carbon.tif")
BLEver_AG_carbon <- crop(spawn_masked, BLEver_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Broadleaf_Evergreen_AG_carbon.tif")
BLDec_AG_carbon <- crop(spawn_masked, BLDec_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Broadleaf_Decidous_AG_carbon.tif")
NLEver_AG_carbon <- crop(spawn_masked, NLEver_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Needleleaved_Evergreen_AG_carbon.tif")
NLDec_AG_carbon <- crop(spawn_masked, NLDec_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Needleleaved_Decidous_AG_carbon.tif")
mixed_forests_AG_carbon <- crop(spawn_masked, mixed_forests_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Mixed_Forests_AG_carbon.tif")
mosaicH_AG_carbon <- crop(spawn_masked, mosaicH_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Mosaic_Herb_AG_carbon.tif")
shrubland_AG_carbon <- crop(spawn_masked, shrubland_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Shrubland_AG_carbon.tif")
grasslands_AG_carbon <- crop(spawn_masked, grasslands_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Grasslands_AG_carbon.tif")
sparseT_AG_carbon <- crop(spawn_masked, sparseT_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Sparse_Tree_AG_carbon.tif")
sparseS_AG_carbon <- crop(spawn_masked, sparseS_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Sparse_Shrub_AG_carbon.tif")
floodTC_AG_carbon <- crop(spawn_masked, floodTC_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Flood_Tree_Cover_AG_carbon.tif")
floodSh_AG_carbon <- crop(spawn_masked, floodSh_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Flood_Shrub_AG_carbon.tif")
urban__AG_carbon <- crop(spawn_masked, urban_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Urban_AG_carbon.tif")
desert__AG_carbon <- crop(spawn_masked, desert_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Desert_AG_carbon.tif")
polarDRI_AG_carbon <- crop(spawn_masked, polarDRI_mask_rastobj, mask=TRUE, extend=FALSE, filename="rasters/Polar_Desert_Rock_Ice_AG_carbon.tif")
end.time <- Sys.time()
time.taken <- round(end.time - start.time)
time.taken # ~3 minutes each

#### Aggregate spawn raster by 6 states for each class
# library(parallel)
# detectCores()
#### how precise do you need??  Can't necessarily do average with integer without promoting bit depth, which can change default null value.
## 1. Mean
cropland_AG_avg <- aggregate(cropland_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/cropland_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
ukherb_AG_avg <- aggregate(ukherb_AG_carbon, fact=30, fun='mean', na.rm=TRUE, cores=19, filename="rasters/ukherb_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
uktree_AG_avg <- aggregate(uktree_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/uktree_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicC_AG_avg <- aggregate(mosaicC_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/mosaicC_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicT_AG_avg <- aggregate(mosaicT_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/mosaicT_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLEver_AG_avg <- aggregate(BLEver_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/BLEver_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLDec_AG_avg <- aggregate(BLDec_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/BLDec_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLEver_AG_avg <- aggregate(NLEver_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/NLEver_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLDec_AG_avg <- aggregate(NLDec_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/NLDec_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mixed_forests_AG_avg <- aggregate(mixed_forests_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/mixed_forests_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicH_AG_avg <- aggregate(mosaicH_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/mosaicH_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
shrubland_AG_avg <- aggregate(shrubland_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/shrubland_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
grasslands_AG_avg <- aggregate(grasslands_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/grasslands_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseT_AG_avg <- aggregate(sparseT_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/sparseT_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseS_AG_avg <- aggregate(sparseS_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/sparseS_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodTC_AG_avg <- aggregate(floodTC_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/floodTC_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodSh_AG_avg <- aggregate(floodSh_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/floodSh_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
urban_AG_avg <- aggregate(urban_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/urban_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
desert_AG_avg <- aggregate(desert_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/desert_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
polarDRI_AG_avg <- aggregate(polarDRI_AG_carbon, fact=30, fun="mean", na.rm=TRUE, cores=19, filename="rasters/polarDRI_AG_avg.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))

## 2. Median
cropland_AG_median <- aggregate(cropland_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/cropland_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
ukherb_AG_median <- aggregate(ukherb_AG_carbon, fact=30, fun='median', na.rm=TRUE, cores=19, filename="rasters/ukherb_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
uktree_AG_median <- aggregate(uktree_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/uktree_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicC_AG_median <- aggregate(mosaicC_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/mosaicC_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicT_AG_median <- aggregate(mosaicT_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/mosaicT_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLEver_AG_median <- aggregate(BLEver_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/BLEver_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLDec_AG_median <- aggregate(BLDec_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/BLDec_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLEver_AG_median <- aggregate(NLEver_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/NLEver_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLDec_AG_median <- aggregate(NLDec_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/NLDec_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mixed_forests_AG_median <- aggregate(mixed_forests_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/mixed_forests_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicH_AG_median <- aggregate(mosaicH_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/mosaicH_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
shrubland_AG_median <- aggregate(shrubland_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/shrubland_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
grasslands_AG_median <- aggregate(grasslands_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/grasslands_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseT_AG_median <- aggregate(sparseT_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/sparseT_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseS_AG_median <- aggregate(sparseS_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/sparseS_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodTC_AG_median <- aggregate(floodTC_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/floodTC_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodSh_AG_median <- aggregate(floodSh_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/floodSh_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
urban_AG_median <- aggregate(urban_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/urban_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
desert_AG_median <- aggregate(desert_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/desert_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
polarDRI_AG_median <- aggregate(polarDRI_AG_carbon, fact=30, fun="median", na.rm=TRUE, cores=19, filename="rasters/polarDRI_AG_median.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))

## 3. Min
cropland_AG_min <- aggregate(cropland_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/cropland_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
ukherb_AG_min <- aggregate(ukherb_AG_carbon, fact=30, fun='min', na.rm=TRUE, cores=19, filename="rasters/ukherb_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
uktree_AG_min <- aggregate(uktree_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/uktree_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicC_AG_min <- aggregate(mosaicC_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/mosaicC_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicT_AG_min <- aggregate(mosaicT_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/mosaicT_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLEver_AG_min <- aggregate(BLEver_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/BLEver_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLDec_AG_min <- aggregate(BLDec_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/BLDec_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLEver_AG_min <- aggregate(NLEver_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/NLEver_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLDec_AG_min <- aggregate(NLDec_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/NLDec_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mixed_forests_AG_min <- aggregate(mixed_forests_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/mixed_forests_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicH_AG_min <- aggregate(mosaicH_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/mosaicH_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
shrubland_AG_min <- aggregate(shrubland_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/shrubland_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
grasslands_AG_min <- aggregate(grasslands_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/grasslands_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseT_AG_min <- aggregate(sparseT_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/sparseT_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseS_AG_min <- aggregate(sparseS_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/sparseS_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodTC_AG_min <- aggregate(floodTC_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/floodTC_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodSh_AG_min <- aggregate(floodSh_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/floodSh_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
urban_AG_min <- aggregate(urban_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/urban_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
desert_AG_min <- aggregate(desert_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/desert_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
polarDRI_AG_min <- aggregate(polarDRI_AG_carbon, fact=30, fun="min", na.rm=TRUE, cores=19, filename="rasters/polarDRI_AG_min.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))

## 4. Max
cropland_AG_max <- aggregate(cropland_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/cropland_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
ukherb_AG_max <- aggregate(ukherb_AG_carbon, fact=30, fun='max', na.rm=TRUE, cores=19, filename="rasters/ukherb_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
uktree_AG_max <- aggregate(uktree_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/uktree_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicC_AG_max <- aggregate(mosaicC_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/mosaicC_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicT_AG_max <- aggregate(mosaicT_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/mosaicT_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLEver_AG_max <- aggregate(BLEver_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/BLEver_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLDec_AG_max <- aggregate(BLDec_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/BLDec_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLEver_AG_max <- aggregate(NLEver_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/NLEver_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLDec_AG_max <- aggregate(NLDec_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/NLDec_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mixed_forests_AG_max <- aggregate(mixed_forests_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/mixed_forests_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicH_AG_max <- aggregate(mosaicH_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/mosaicH_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
shrubland_AG_max <- aggregate(shrubland_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/shrubland_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
grasslands_AG_max <- aggregate(grasslands_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/grasslands_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseT_AG_max <- aggregate(sparseT_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/sparseT_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseS_AG_max <- aggregate(sparseS_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/sparseS_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodTC_AG_max <- aggregate(floodTC_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/floodTC_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodSh_AG_max <- aggregate(floodSh_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/floodSh_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
urban_AG_max <- aggregate(urban_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/urban_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
desert_AG_max <- aggregate(desert_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/desert_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
polarDRI_AG_max <- aggregate(polarDRI_AG_carbon, fact=30, fun="max", na.rm=TRUE, cores=19, filename="rasters/polarDRI_AG_max.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))

## 5.Q1
cropland_AG_Q1 <- aggregate(cropland_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/cropland_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
ukherb_AG_Q1 <- aggregate(ukherb_AG_carbon, fact=30, fun='q1', na.rm=TRUE, cores=19, filename="rasters/ukherb_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
uktree_AG_Q1 <- aggregate(uktree_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/uktree_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicC_AG_Q1 <- aggregate(mosaicC_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/mosaicC_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicT_AG_Q1 <- aggregate(mosaicT_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/mosaicT_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLEver_AG_Q1 <- aggregate(BLEver_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/BLEver_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLDec_AG_Q1 <- aggregate(BLDec_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/BLDec_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLEver_AG_Q1 <- aggregate(NLEver_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/NLEver_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLDec_AG_Q1 <- aggregate(NLDec_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/NLDec_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mixed_forests_AG_Q1 <- aggregate(mixed_forests_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/mixed_forests_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicH_AG_Q1 <- aggregate(mosaicH_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/mosaicH_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
shrubland_AG_Q1 <- aggregate(shrubland_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/shrubland_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
grasslands_AG_Q1 <- aggregate(grasslands_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/grasslands_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseT_AG_Q1 <- aggregate(sparseT_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/sparseT_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseS_AG_Q1 <- aggregate(sparseS_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/sparseS_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodTC_AG_Q1 <- aggregate(floodTC_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/floodTC_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodSh_AG_Q1 <- aggregate(floodSh_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/floodSh_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
urban_AG_Q1 <- aggregate(urban_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/urban_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
desert_AG_Q1 <- aggregate(desert_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/desert_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
polarDRI_AG_Q1 <- aggregate(polarDRI_AG_carbon, fact=30, fun="q1", na.rm=TRUE, cores=19, filename="rasters/polarDRI_AG_Q1.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))

## 6.Q3
cropland_AG_Q3 <- aggregate(cropland_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/cropland_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
ukherb_AG_Q3 <- aggregate(ukherb_AG_carbon, fact=30, fun='q3', na.rm=TRUE, cores=19, filename="rasters/ukherb_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
uktree_AG_Q3 <- aggregate(uktree_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/uktree_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicC_AG_Q3 <- aggregate(mosaicC_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/mosaicC_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicT_AG_Q3 <- aggregate(mosaicT_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/mosaicT_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLEver_AG_Q3 <- aggregate(BLEver_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/BLEver_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
BLDec_AG_Q3 <- aggregate(BLDec_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/BLDec_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLEver_AG_Q3 <- aggregate(NLEver_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/NLEver_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
NLDec_AG_Q3 <- aggregate(NLDec_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/NLDec_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mixed_forests_AG_Q3 <- aggregate(mixed_forests_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/mixed_forests_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
mosaicH_AG_Q3 <- aggregate(mosaicH_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/mosaicH_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
shrubland_AG_Q3 <- aggregate(shrubland_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/shrubland_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
grasslands_AG_Q3 <- aggregate(grasslands_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/grasslands_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseT_AG_Q3 <- aggregate(sparseT_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/sparseT_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
sparseS_AG_Q3 <- aggregate(sparseS_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/sparseS_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodTC_AG_Q3 <- aggregate(floodTC_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/floodTC_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
floodSh_AG_Q3 <- aggregate(floodSh_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/floodSh_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
urban_AG_Q3 <- aggregate(urban_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/urban_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
desert_AG_Q3 <- aggregate(desert_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/desert_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))
polarDRI_AG_Q3 <- aggregate(polarDRI_AG_carbon, fact=30, fun="q3", na.rm=TRUE, cores=19, filename="rasters/polarDRI_AG_Q3.tif", overwrite=TRUE,  wopt= list(verbose=TRUE))

 

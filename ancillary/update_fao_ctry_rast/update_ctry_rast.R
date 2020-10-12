library(dplyr)
library(tidyr)
library(sp)
library(sf)
library(raster)
library(data.table)

#Objective: The fao_ctry_rast.bil is used to define valid country boundaries in moirai. However, this file
# does not include boundaries for inland water bodies (For example- Lake victoria). To add in boundaries
# for water bodies, we update the original raster with an additional shape file (GCAM_32_w_Taiwan.shp).
# We map in the moirai fao codes to the ISO numbers in the shape file and then combine this with the original raster
# to fill in holes



#Get original data
fao_data_raster<-raster("D:/MOIRAI_clean/moirai/indata/fao_ctry_rast.bil")
fao_data_original<-as.data.table(rasterToPoints(fao_data_raster)) %>% 
                   filter(fao_ctry_rast != 55537)
#Get original rows
original_rows <- nrow(fao_data_original)

#Get a shapefile
shape_file<-shapefile("D:/input_files/GCAM_32_w_Taiwan.shp")
MOIRAI_mapping<-read.csv("D:/MOIRAI_3.1_Final/moirai/indata/FAO_ctry_GCAM_ctry87.csv")

#Get mapping data to map new data with moirai iso's. Renaming Romania ISO
shape_data<-as.data.frame(shape_file) %>%
            mutate(ISO_3DIGIT=if_else(ISO_3DIGIT=="ROU","ROM",ISO_3DIGIT)) %>%
            mutate(iso3_abbr=tolower(ISO_3DIGIT))

#we drop some rows here. Not a big deal for now since the idea behind this activity, 
#was filling in holes in fao_ctry_rast.bil. The dropped rows mainly belong to rows where there were no ISO's available.
shape_data %>% left_join(MOIRAI_mapping,by=c("iso3_abbr")) %>% na.omit()->Joined


#Now rasterize the shape data and align it to moirai's projection
moirai_projection <- "+proj=longlat +datum=WGS84 +no_defs +ellps=WGS84 +towgs84=0,0,0"

#Rasterize based on ISO numbers
shape_data_raster <- rasterize(shape_file,fao_data_raster,field="ISO_NUM")
crs(shape_data_raster) <- crs(fao_data_raster)
extent(shape_data_raster) <- extent(fao_data_raster)
shape_data_country <- as.data.table(rasterToPoints(shape_data_raster))
shape_data_country %>%
     rename(ISO_NUM=layer) %>%
     left_join(Joined %>% dplyr::select(ISO_NUM,fao_code,gcam_ctry87_id),by=c("ISO_NUM")) %>%
     na.omit() %>% filter(gcam_ctry87_id != -1) %>% distinct() %>%  dplyr::select(-ISO_NUM,gcam_ctry87_id)->shape_data_joined

shape_data_joined %>% rename(fao_ctry_rast=fao_code) %>% select(-gcam_ctry87_id)-> shape_data_joined



Final_data <-bind_rows(shape_data_joined,fao_data_original)
Final_data %>% distinct()->Final_data

new_rows <- nrow(Final_data)

print(paste0("New lines added for holes- ", (new_rows-original_rows)))
coordinates(Final_data)<-~x+y
gridded(Final_data)<-TRUE
final_raster <-raster(Final_data)

#Check that holes have been filled
plot(final_raster)

raster::projection(final_raster)<-"+proj=longlat +datum=WGS84 +no_defs +ellps=WGS84 +towgs84=0,0,0"
final_raster<-raster::extend(final_raster,fao_data_raster)
writeRaster(final_raster, filename="fao_ctry_rast.tif", format="GTiff", overwrite=TRUE)


#Now use the gdal commands in the file update_fao_rast_gdal.txt. These commands are also added here. 

#The first command will fill in nodata values with the moirai fao no_data value and also make sure that the data type is an integer.
#gdalwarp fao_ctry_rast.tif fao_ctry_rast_final.tif -dstnodata 55537 -ot UInt16

#The second command will translate the raster to a binary as required by moirai.
#gdal_translate -of ENVI fao_ctry_rast_final.tif fao_ctry_rast.bil


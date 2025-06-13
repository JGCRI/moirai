# Created by Kanishka Narayan and Alan Di Vittorio

# Moirai Land Data System (Moirai) Copyright (c) 2020, The
# Regents of the University of California, through Lawrence Berkeley National
# Laboratory (subject to receipt of any required approvals from the U.S.
# Dept. of Energy).  All rights reserved.

# If you have questions about your rights to use or distribute this software,
# please contact Berkeley Lab's Intellectual Property Office at
# IPO@lbl.gov.

# NOTICE.  This Software was developed under funding from the U.S. Department
# of Energy and the U.S. Government consequently retains certain rights.  As
# such, the U.S. Government has been granted for itself and others acting on
# its behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit other to do
# so.

# This file is part of Moirai.
 
# Moirai is free software: you can use it under the terms of the modified BSD-3 license (see …/moirai/license.txt)

library(dplyr)
library(tidyr)
library(sp)
library(sf)
library(data.table)
library(rasterVis)
library(terra)

# make sure that the working directory is: .../moirai/ancillary/update_country_raster_water !!!!!!!!

#Objective: The fao_ctry_rast.bil is used to define valid country boundaries in moirai. However, this file
# does not include boundaries for inland water bodies (For example- Lake victoria). To add in boundaries
# for water bodies, we update the original raster with an additional shape file (GCAM_32_w_Taiwan.shp).
# We map in the moirai fao codes to the ISO numbers in the shape file and then combine this with the original raster
# to fill in holes

moirai_projection <- "+proj=longlat +datum=WGS84 +no_defs +ellps=WGS84 +towgs84=0,0,0"

#Get original data
fao_data_raster<-rast("./fao_ctry_rast_nowater.bil")
crs(fao_data_raster) <- moirai_projection
ext(fao_data_raster)<-c(-180,180,-90,90)
#fao_data_original<-as.data.table(rasterToPoints(fao_data_raster))
                   
#Get original rows
#original_rows <- nrow(fao_data_original)

#Get a shapefile
shape_file<-st_read("./GCAM_32_w_Taiwan.shp")
MOIRAI_mapping<-read.csv("../../indata/FAO_ctry_GCAM_ctry87.csv")

#Get mapping data to map new data with moirai iso's. 
# Renaming Romania ISO to rom
# Renaming west bank iso to pse
shape_data<-as.data.frame(shape_file) %>%
            mutate(ISO_3DIGIT=if_else(ISO_3DIGIT=="ROU","ROM",ISO_3DIGIT)) %>%
            mutate(ISO_3DIGIT=if_else(FIPS_CNTRY=="WE","PSE",ISO_3DIGIT)) %>%
            mutate(iso3_abbr=tolower(ISO_3DIGIT))


# we drop some rows here. Not a big deal for now since the idea behind this activity, 
# was filling in holes in fao_ctry_rast_nowater.bil. The dropped rows mainly belong to rows where there were no ISO's available.
# these are islands without iso that are lost, once the west bank is updated with an iso above
shape_data %>% left_join(MOIRAI_mapping,by=c("iso3_abbr")) %>% na.omit()->Joined

# merge by the unique column and retain only features in the new shape file that have an fao code from the country raster file
shape_file_new = merge(shape_file, dplyr::select(Joined, FIPS_CNTRY, fao_code), by = "FIPS_CNTRY")
shape_file_fao = shape_file_new[!is.na(shape_file_new$fao_code),]

# rasterize the new shape files to the fao_code values
shape_data_raster <- rasterize(shape_file_fao,fao_data_raster,field="fao_code")

# determine where there are new pixels and add them to the original country raster
new_inds = which.lyr(is.na(fao_data_raster) & !is.na(shape_data_raster))
#new_inds = Which(is.na(fao_data_raster) & !is.na(shape_data_raster), cell=TRUE)
new_fao_raster = fao_data_raster
new_fao_raster[new_inds] = shape_data_raster[new_inds]

# fix the taiwan pixel that was added as china due to the china ring around taiwan in the shape file
new_fao_raster[3498493] = 303

print(paste0("New pixels added for water = ", length(which(as.vector(new_inds)==TRUE))))

#Check that holes have been filled
levelplot(new_fao_raster, margin=FALSE)

# write the final country raster file for Moirai
writeRaster(new_fao_raster, filename="fao_ctry_rast_test.bil", filetype="EHdr", datatype="INT2S", NAflag=-9999, overwrite=TRUE)


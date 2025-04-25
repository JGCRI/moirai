# Created by Kanishka Narayan, Alan Di Vittorio, and Evan Margiotta

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

#Purpose : This script harmonizes the 1026 rasters stored in the zip file Carbon_rasters.zip to the moirai refrence
#vegetation classes (currently in 2010). This script generates 18 final rasters for each  managed category that are used within the moirai code.  
#This script performs the harmonization for data for soil carbon from the soil grids (0-30 cms) and vegetation carbon
#from Spawn et al. for above and below ground vegetation.

#Load libraries
library(raster)
library(dplyr)
library(data.table)
library(FNN)
library(ggplot2)

#Scheme for plots
scheme_basic <- theme_bw() +
  theme(legend.text = element_text(size = 12)) +
  theme(legend.title = element_text(size = 12)) +
  theme(axis.text = element_text(size = 11)) +
  theme(axis.title = element_text(size = 11, face = "bold")) +
  theme(plot.title = element_text(size = 12, face = "bold", vjust = 1)) +
  theme(plot.subtitle = element_text(size = 9, face = "bold", vjust = 1))+ 
  theme(strip.text = element_text(size = 7))+
  theme(strip.text.x = element_text(size = 11, face = "bold"))+
  theme(strip.text.y = element_text(size = 11, face = "bold"))+
  theme(legend.position = "bottom")+
  theme(legend.text = element_text(size = 11))+
  theme(legend.title = element_text(size = 11,color = "black",face="bold"))+
  theme(axis.text.x= element_text(angle=60,hjust=1))+
  theme(legend.background = element_blank(),
        legend.box.background = element_rect(colour = "black"))+
  theme(legend.key.width=unit(4,"cm"))

#Set the working directory to moirai/ancillary/carbon_harmonization
path_to_working_directory <- "./"
setwd(path_to_working_directory)

#Create some directories
dir.create("outputs/",showWarnings = FALSE)
dir.create("diagnostics/",showWarnings = FALSE)

#moirai no data value (specified in moirai.h)
moirai_no_data_value <- -9999

#Scalers (Vegetation numbers need to be multiplied by 0.1 to get values, soil by 10)
veg_scaler <- 0.1
soil_scaler <- 10

# These are the SAGE HYDE land use classes. These need to be ignored since we are only interested in LC.
sage_hyde_land_use_codes <- 23:34

#Check if files have been unzipped. If not unzip them
if(!dir.exists("Carbon_rasters/")){
  
  unzip("Carbon_rasters.zip")
  
  
}

#Read in managed carbon rasters



#Set basin for debugging. Select basin of your choice from the input file 
#input_files/basin_to_country_mapping.csv 
basin_debug <- 110

#Helper function for nearest neighbor interpolation
find_nearest_reighbor <- function(sample_data){
  colnames(sample_data)<- final_col_names
  
  
  sample_data_zero <- sample_data %>% 
    filter(Carbon_AG == 0) %>% 
    left_join(nn_index, by = c("ID"))
  
  zero_cells <- c(sample_data_zero$ID)
  
  sample_data_non_zero <- sample_data %>% 
    filter(Carbon_AG != 0)
  
  sample_data_zero %>% tidyr::gather("Nearest_neighbor","nn_value","V1":"V40") %>% 
    filter(nn_value %in% c(sample_data$ID)) %>% 
    left_join(sample_data %>% dplyr::select(ID,Carbon_AG) %>% 
                rename(nn_value=ID, Carbon_nn = Carbon_AG),by = c("nn_value")) %>% 
    group_by(ID) %>% 
    mutate(Carbon_AG= Carbon_nn,
           Carbon_AG = max(Carbon_nn, na.rm = TRUE)) %>% 
    ungroup() %>% 
    dplyr::select(final_col_names) %>% 
    distinct() %>% 
    filter(Carbon_AG != 0)->t
  
  sample_data_zero %>% 
    dplyr::filter(!ID %in% c(t$ID)) %>% 
    dplyr::select(colnames(sample_data))->sample_data_zero
  
  bind_rows(sample_data_zero, sample_data_non_zero, t)->sample_data_transformed
  
  
  return(sample_data_transformed)
  
}

#Helper function for managed land nearest neighbor interpolation
find_nearest_reighbor_crop <- function(sample_data){

  
  colnames(sample_data)<- final_col_names
  
  
  sample_data_zero <- sample_data %>% 
    filter(Carbon_Crop == 0) %>% 
    left_join(nn_index, by = c("ID"))
  
  zero_cells <- c(sample_data_zero$ID)
  
  sample_data_non_zero <- sample_data %>% 
    filter(Carbon_Crop != 0)
  
  sample_data_zero %>% tidyr::gather("Nearest_neighbor","nn_value","V1":"V40") %>% 
    filter(nn_value %in% c(sample_data$ID)) %>% 
    left_join(sample_data %>% dplyr::select(ID,Carbon_Crop) %>% 
                rename(nn_value=ID, Carbon_nn = Carbon_Crop),by = c("nn_value")) %>% 
    group_by(ID) %>% 
    mutate(Carbon_Crop= Carbon_nn,
           Carbon_Crop = max(Carbon_nn, na.rm = TRUE)) %>% 
    ungroup() %>% 
    dplyr::select(final_col_names) %>% 
    distinct() %>% 
    filter(Carbon_Crop != 0)->t
  
  sample_data_zero %>% 
    dplyr::filter(!ID %in% c(t$ID)) %>% 
    dplyr::select(colnames(sample_data))->sample_data_zero
  
  bind_rows(sample_data_zero, sample_data_non_zero, t)->sample_data_transformed
  
  
  return(sample_data_transformed)
  
}

find_nearest_reighbor_pasture <- function(sample_data){
  
  
  colnames(sample_data)<- final_col_names
  
  
  sample_data_zero <- sample_data %>% 
    filter(Carbon_Pasture == 0) %>% 
    left_join(nn_index, by = c("ID"))
  
  zero_cells <- c(sample_data_zero$ID)
  
  sample_data_non_zero <- sample_data %>% 
    filter(Carbon_Pasture != 0)
  
  sample_data_zero %>% tidyr::gather("Nearest_neighbor","nn_value","V1":"V40") %>% 
    filter(nn_value %in% c(sample_data$ID)) %>% 
    left_join(sample_data %>% dplyr::select(ID,Carbon_Pasture) %>% 
                rename(nn_value=ID, Carbon_nn = Carbon_Pasture),by = c("nn_value")) %>% 
    group_by(ID) %>% 
    mutate(Carbon_Pasture= Carbon_nn,
           Carbon_Pasture = max(Carbon_nn, na.rm = TRUE)) %>% 
    ungroup() %>% 
    dplyr::select(final_col_names) %>% 
    distinct() %>% 
    filter(Carbon_Pasture != 0)->t
  
  sample_data_zero %>% 
    dplyr::filter(!ID %in% c(t$ID)) %>% 
    dplyr::select(colnames(sample_data))->sample_data_zero
  
  bind_rows(sample_data_zero, sample_data_non_zero, t)->sample_data_transformed
  
  
  return(sample_data_transformed)
  
}

find_nearest_reighbor_urban <- function(sample_data){
  
  
  colnames(sample_data)<- final_col_names
  
  
  sample_data_zero <- sample_data %>% 
    filter(Carbon_Urban == 0) %>% 
    left_join(nn_index, by = c("ID"))
  
  zero_cells <- c(sample_data_zero$ID)
  
  sample_data_non_zero <- sample_data %>% 
    filter(Carbon_Urban != 0)
  
  sample_data_zero %>% tidyr::gather("Nearest_neighbor","nn_value","V1":"V40") %>% 
    filter(nn_value %in% c(sample_data$ID)) %>% 
    left_join(sample_data %>% dplyr::select(ID,Carbon_Urban) %>% 
                rename(nn_value=ID, Carbon_nn = Carbon_Urban),by = c("nn_value")) %>% 
    group_by(ID) %>% 
    mutate(Carbon_Urban= Carbon_nn,
           Carbon_Urban = max(Carbon_nn, na.rm = TRUE)) %>% 
    ungroup() %>% 
    dplyr::select(final_col_names) %>% 
    distinct() %>% 
    filter(Carbon_Urban != 0)->t
  
  sample_data_zero %>% 
    dplyr::filter(!ID %in% c(t$ID)) %>% 
    dplyr::select(colnames(sample_data))->sample_data_zero
  
  bind_rows(sample_data_zero, sample_data_non_zero, t)->sample_data_transformed
  
  
  return(sample_data_transformed)
  
}


#Read in carbon thematic file for moirai. The below can be set to `AG` for above
#ground vegetation, `BG` for below ground vegetation and `soil` for soil. It is recommended that the user run 
#these one by one given the time it takes to generate the final rasters.  
veg_type <- "soil"

#This will print out diagnostics (.csv files) for the World (Global) and the Amazon basin
print_diagnostics <- FALSE
#This will generate maps for the World and the selected basin for the selected veg type
print_plots <- FALSE

#This is the current thematic map in moirai. Ensure the model has been run.
Carbon_thematic <- raster("input_files/refveg_carbon_thematic.bil")
Carbon_thematic_data <- as.data.frame(rasterToPoints(Carbon_thematic)) %>% 
  dplyr::filter(refveg_carbon_thematic != moirai_no_data_value)

#Current carbon area
Carbon_area <- as.data.frame(rasterToPoints(raster("input_files/refveg_area_carbon.bil"))) %>% 
  filter(refveg_area_carbon != moirai_no_data_value)

#Managed Carbon Areas
CropArea <- as.data.frame(rasterToPoints(raster("input_files/managed_carbon_areas/crop_area_carbon.bil")))%>% 
  filter(crop_area_carbon != moirai_no_data_value)
PastureArea <- as.data.frame(rasterToPoints(raster("input_files/managed_carbon_areas/pasture_area_carbon.bil")))%>% 
  filter(pasture_area_carbon != moirai_no_data_value)
UrbanArea <- as.data.frame(rasterToPoints(raster("input_files/managed_carbon_areas/urban_area_carbon.bil"))) %>% 
  filter(urban_area_carbon != moirai_no_data_value)


#GLU mapping
GLU_data <- as.data.frame(rasterToPoints(raster("input_files/gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin.tif")))


#Country Mapping
Country_data <- as.data.frame(rasterToPoints(raster("input_files/moirai_valid_boundaries_v31/moirai_valid_boundaries_v31/raster_files/gcam_country_boundaries_moirai_combined_3p1_0p5arcmin.tif")))
colnames(Country_data) <- c("x","y","Country_ID")

#Read in ESA land cover classes
ESA_classes <- read.csv("input_files/ESA_classes.csv", stringsAsFactors = FALSE)

#Can set states of carbon here.
#states_of_carbon <- c("q3")
states_of_carbon <- c("q3","median","q1","weighted_average","min","max")

for (i in states_of_carbon){
  Carbon_thematic_data_joined <- Carbon_thematic_data
    for (j in c(ESA_classes[1:nrow(ESA_classes),])){
      
      
    
      tmp_raster <- as.data.frame(rasterToPoints(raster(paste0("ESA_Soil_cover/Soil_carbon_rasters/",veg_type,"/",j,"_",veg_type,"_",i,".bil"))))
      
      
      Carbon_thematic_data_joined %>% left_join(tmp_raster, by= c("x","y"))->Carbon_thematic_data_joined
      
    }
  #join managed carbon raster area files
  Carbon_thematic_data_joined %>% left_join(CropArea, by= c("x","y"))->Carbon_thematic_data_joined
  Carbon_thematic_data_joined %>% left_join(PastureArea, by= c("x","y"))->Carbon_thematic_data_joined
  Carbon_thematic_data_joined %>% left_join(UrbanArea, by= c("x","y"))->Carbon_thematic_data_joined
  
  
  #Replace NA values with zeroes
  Carbon_thematic_data_joined[is.na(Carbon_thematic_data_joined)] <- 0
  
  #Set scaler here
  if(veg_type == "soil"){
    
    scaler = soil_scaler
    
  }else{
    
    scaler = veg_scaler
  }
  
  SAGE_names <- read.csv("../../indata/isam_2_sage_hyde_mapping.csv", stringsAsFactors = FALSE) %>%
    #Don't use land use types. Only use land covers
    dplyr::filter(!isam_code %in% sage_hyde_land_use_codes) %>% 
    dplyr::select(sage_hyde32_code, sage_hyde32_name) %>% 
    rename(refveg_carbon_thematic = sage_hyde32_code ) %>%
    mutate(sage_hyde32_name = gsub("_","",sage_hyde32_name)) %>% 
    distinct()
  
  Carbon_thematic_data_joined %>% left_join(SAGE_names, by = c("refveg_carbon_thematic")) %>% 
    mutate(Carbon_AG = 0)->Carbon_thematic_data_temp
  
  colnames(Carbon_thematic_data_temp)<- gsub(paste0("_",veg_type,"_",i),"",colnames(Carbon_thematic_data_temp))
  
  #Rule based mapping of carbon. These are specified in the file input_files/ESA_moirai_classes.csv
  Carbon_thematic_data_temp %>% mutate(Carbon_Crop = if_else(crop_area_carbon > 0, 
                                                             if_else(Cropland > 0, Cropland, 
                                                                     if_else(Mosaic_Crop > 0, Mosaic_Crop, 0)),0)) -> temp_data
                                                            
  temp_data %>% mutate(Carbon_Pasture = if_else(pasture_area_carbon > 0,
                                                                if_else(Grasslands > 0, Grasslands,
                                                                        if_else(Mosaic_Herb > 0, Mosaic_Herb,
                                                                                if_else(Unknown_Herb > 0, Unknown_Herb,
                                                                                        if_else(Sparse_Tree > 0, Sparse_Tree,
                                                                                                if_else(Sparse_Shrub>0, Sparse_Shrub,0))))),0)) -> temp_data
  temp_data %>% mutate(Carbon_Urban = if_else(urban_area_carbon > 0,
                                                              if_else(Urban > 0, Urban, 0), 0)) -> temp_data
  
  
  temp_data %>%  mutate(Carbon_AG = if_else(sage_hyde32_name=="TropicalEvergreenForest/Woodland",
                               if_else(Evergreen_Combined > 0, Evergreen_Combined,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="TropicalDeciduousForest/Woodland",
                               if_else(Decidous_Combined > 0, Decidous_Combined,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="TemperateBroadleafEvergreenForest/Woodland",
                               if_else(Broadleaf_Evergreen > 0, Broadleaf_Evergreen,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="TemperateNeedleleafEvergreenForest/Woodland",
                               if_else(Needleleaved_Evergreen > 0, Needleleaved_Evergreen,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="TemperateDeciduousForest/Woodland",
                               if_else(Decidous_Combined > 0, Decidous_Combined,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="BorealEvergreenForest/Woodland",
                               if_else(Evergreen_Combined > 0, Evergreen_Combined,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="BorealDeciduousForest/Woodland",
                               if_else(Decidous_Combined > 0, Decidous_Combined,
                                       if_else(Mixed_Forests > 0,Mixed_Forests,
                                               if_else(Mosaic_Tree > 0, Mosaic_Tree,
                                                       if_else(Flood_Tree_Cover> 0,Flood_Tree_Cover,
                                                               if_else(Unknown_Tree > 0, Unknown_Tree,Sparse_Tree))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="Savanna",
                               if_else(Mosaic_Herb > 0, Mosaic_Herb,
                                       if_else(Grasslands > 0,Grasslands,
                                               if_else(Unknown_Herb > 0, Unknown_Herb,
                                                       if_else(Flood_Shrub> 0,Flood_Shrub,0
                                                       )))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="Grassland/Steppe",
                               if_else(Grasslands > 0, Grasslands,
                                       if_else(Unknown_Herb > 0, Unknown_Herb, 0)),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="DenseShrubland",
                               if_else(Shrubland > 0, Shrubland,
                                       if_else(Unknown_Tree > 0,Unknown_Tree,
                                               if_else(Flood_Shrub > 0, Flood_Shrub,
                                                       if_else(Mosaic_Herb> 0,Mosaic_Herb,0
                                                       )))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="OpenShrubland",
                               if_else(Sparse_Shrub > 0, Sparse_Shrub,
                                       if_else(Mosaic_Herb > 0,Mosaic_Herb,
                                               if_else(Flood_Shrub > 0, Flood_Shrub,
                                                       if_else(Unknown_Herb> 0,Unknown_Herb,if_else(Shrubland > 0, Shrubland, 0)
                                                       )))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="Tundra",if_else(Polar_Desert_Rock_Ice > 0, Polar_Desert_Rock_Ice, 
                                                                  if_else(Sparse_Shrub >0,Sparse_Shrub,
                                                                          if_else(Mosaic_Herb>0,Mosaic_Herb,
                                                                                  if_else(Unknown_Herb>0,Unknown_Herb,
                                                                                          if_else(Unknown_Tree>0,Unknown_Tree,
                                                                                                  if_else(Sparse_Tree >0,Sparse_Tree,0,
                                                                                                          if_else(Shrubland>0,Shrubland,
                                                                                                                  if_else(Mosaic_Tree>0,Mosaic_Tree,0)))))))),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="Desert",if_else(Desert > 0, Desert, 0),Carbon_AG),
           Carbon_AG = if_else(sage_hyde32_name=="Polardesert/Rock/Ice",if_else(Polar_Desert_Rock_Ice > 0, Polar_Desert_Rock_Ice, 0),Carbon_AG))->temp_data
  
  #Replace NA values with zeroes
  Carbon_thematic_data[is.na(Carbon_thematic_data)] <- 0
  
  
  
  if(print_diagnostics){
  temp_data %>% mutate(temp_id =1) %>% group_by(sage_hyde32_name) %>% 
    mutate(total = sum(temp_id)) %>% 
    ungroup() %>% 
    filter(Carbon_AG== 0) %>% 
    group_by(sage_hyde32_name) %>% 
    mutate(total_zero = sum(temp_id)) %>%
    ungroup() %>% 
    dplyr::select(sage_hyde32_name, total, total_zero) %>% 
    distinct() %>% 
    mutate(perc = (total_zero/total)*100)->completeness_analytics
    
    #managed land diagnostic
    managed_diag <- as.data.frame(matrix(nrow = 3, ncol=4))
    colnames(managed_diag) <- colnames(completeness_analytics)
    
    managed_diag$sage_hyde32_name <- c("Cropland","Pasture","Urban")
    managed_diag$total[1] <- sum(temp_data$crop_area_carbon > 0)
    managed_diag$total[2] <- sum(temp_data$pasture_area_carbon > 0)
    managed_diag$total[3] <- sum(temp_data$urban_area_carbon > 0)
    
    managed_diag$total_zero[1] <- sum(temp_data$crop_area_carbon > 0) - sum(temp_data$Carbon_Crop > 0)
    managed_diag$total_zero[2] <- sum(temp_data$pasture_area_carbon > 0)- sum(temp_data$Carbon_Pasture > 0)
    managed_diag$total_zero[3] <- sum(temp_data$urban_area_carbon > 0)- sum(temp_data$Carbon_Urban > 0)
    
    managed_diag$perc[1] <- 100*(1- (sum(temp_data$Carbon_Crop > 0)/sum(temp_data$crop_area_carbon > 0))) 
    managed_diag$perc[2] <- 100*(1 - (sum(temp_data$Carbon_Pasture > 0)/sum(temp_data$pasture_area_carbon > 0)))
    managed_diag$perc[3] <- 100*(1 - (sum(temp_data$Carbon_Urban > 0)/sum(temp_data$urban_area_carbon > 0)))
    
    completeness_analytics <- rbind(completeness_analytics, managed_diag)
    
    write.csv(completeness_analytics, paste0("diagnostics/completeness_",veg_type,"_",i,".csv"))
    }
  
  #Read in carbon areas
  
  
  temp_data %>% inner_join(Carbon_area, by =c("x","y")) %>% 
    filter(refveg_area_carbon > 0)->temp_area
  
  temp_area %>% mutate(ID = 1:nrow(temp_area))->temp_area_w_IDS
  
  #Get nearest neighbors (40 nearest geographic neighbors)
  near_data <- get.knn(temp_area_w_IDS[, 1:2], k = 40)
  nn_index <- as.data.frame(near_data$nn.index) %>% 
    mutate(ID = 1:nrow(temp_area_w_IDS))
  
  final_col_names <- colnames(temp_area_w_IDS)
  #managed land nearest neighbors
  temp_area_boundaries_split <-split(temp_area_w_IDS, as.integer(temp_area_w_IDS$crop_area_carbon > 0))
  boundaries_final_crop <- find_nearest_reighbor_crop(temp_area_boundaries_split$`1`)
  boundaries_final_crop <- rbind(boundaries_final_crop, temp_area_boundaries_split$`0`)
  
  temp_area_boundaries_split <- split(boundaries_final_crop, as.integer(boundaries_final_crop$pasture_area_carbon > 0))
  boundaries_final_pasture <- find_nearest_reighbor_pasture(temp_area_boundaries_split$`1`)
  boundaries_final_pasture <- rbind(boundaries_final_pasture, temp_area_boundaries_split$`0`)
  
  temp_area_boundaries_split <- split(boundaries_final_pasture, as.integer(boundaries_final_pasture$urban_area_carbon > 0))
  boundaries_final_urban <- find_nearest_reighbor_urban(temp_area_boundaries_split$`1`)
  boundaries_final_urban <- rbind(boundaries_final_urban, temp_area_boundaries_split$`0`)
  
  #Unmanaged Land Nearest Neighbor Processing
  #Now split the data 
  temp_area_boundaries_split <-split(boundaries_final_urban, boundaries_final_urban$refveg_carbon_thematic) 
  boundaries_list <- lapply(temp_area_boundaries_split , find_nearest_reighbor)
  boundaries_final <- rbindlist(boundaries_list)
  
  if(print_diagnostics){
  boundaries_final %>% mutate(temp_id =1) %>% group_by(sage_hyde32_name) %>% 
    mutate(total = sum(temp_id)) %>% 
    ungroup() %>% 
    filter(Carbon_AG== 0) %>% 
    group_by(sage_hyde32_name) %>% 
    mutate(total_zero = sum(temp_id)) %>%
    ungroup() %>% 
    dplyr::select(sage_hyde32_name, total, total_zero) %>% 
    distinct() %>% 
    mutate(perc = (total_zero/total)*100)->completeness_analytics_interpolated
    
    #managed land diagnostic
    managed_diag <- as.data.frame(matrix(nrow = 3, ncol=4))
    colnames(managed_diag) <- colnames(completeness_analytics_interpolated)
    
    managed_diag$sage_hyde32_name <- c("Cropland","Pasture","Urban")
    managed_diag$total[1] <- sum(boundaries_final$crop_area_carbon > 0)
    managed_diag$total[2] <- sum(boundaries_final$pasture_area_carbon > 0)
    managed_diag$total[3] <- sum(boundaries_final$urban_area_carbon > 0)
    
    managed_diag$total_zero[1] <- sum(boundaries_final$crop_area_carbon > 0) - sum(boundaries_final$Carbon_Crop > 0)
    managed_diag$total_zero[2] <- sum(boundaries_final$pasture_area_carbon > 0)- sum(boundaries_final$Carbon_Pasture > 0)
    managed_diag$total_zero[3] <- sum(boundaries_final$urban_area_carbon > 0)- sum(boundaries_final$Carbon_Urban > 0)
    
    managed_diag$perc[1] <- 100*(1- (sum(boundaries_final$Carbon_Crop > 0)/sum(boundaries_final$crop_area_carbon > 0))) 
    managed_diag$perc[2] <- 100*(1 - (sum(boundaries_final$Carbon_Pasture > 0)/sum(boundaries_final$pasture_area_carbon > 0)))
    managed_diag$perc[3] <- 100*(1 - (sum(boundaries_final$Carbon_Urban > 0)/sum(boundaries_final$urban_area_carbon > 0)))
    
    completeness_analytics_interpolated <- rbind(completeness_analytics_interpolated, managed_diag)
    
    write.csv(completeness_analytics_interpolated, paste0("diagnostics/completeness_interpolated_",veg_type,"_",i,".csv"))
    }
  
  if(print_diagnostics){
    boundaries_final %>% left_join(GLU_data, by = c("x","y"))-> boundaries_glu
    boundaries_glu %>% left_join(Country_data, by = c("x","y"))-> boundaries_country
    
    boundaries_country %>% mutate(temp_id =1) %>% group_by(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total = sum(temp_id)) %>% 
      ungroup() %>% 
      filter(Carbon_AG== 0) %>% 
      group_by(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total_zero = sum(temp_id)) %>%
      ungroup() %>% 
      dplyr::select(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero) %>% 
      distinct() %>% 
      mutate(perc = (total_zero/total)*100)->completeness_analytics_interpolated_unmanaged
    
    boundaries_country %>% mutate(Crop_Binary = as.integer(crop_area_carbon > 0)) %>%
                        mutate(Pasture_Binary = as.integer(pasture_area_carbon > 0)) %>%
                        mutate(Urban_Binary = as.integer(urban_area_carbon > 0))-> boundaries_country_managed
    
    
    boundaries_country_managed %>% mutate(temp_id =1) %>% group_by(Crop_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total = sum(temp_id)) %>% 
      ungroup() %>% 
      filter(Carbon_AG== 0) %>% 
      group_by(Crop_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total_zero = sum(temp_id)) %>%
      ungroup() %>% 
      dplyr::select(Crop_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero) %>% 
      distinct() %>% 
      mutate(perc = (total_zero/total)*100) %>% 
      mutate(sage_hyde32_name = "Crop") %>% 
      dplyr::select(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero, perc) ->completeness_analytics_interpolated_crop
    
    boundaries_country_managed %>% mutate(temp_id =1) %>% group_by(Pasture_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total = sum(temp_id)) %>% 
      ungroup() %>% 
      filter(Carbon_AG== 0) %>% 
      group_by(Pasture_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total_zero = sum(temp_id)) %>%
      ungroup() %>% 
      dplyr::select(Pasture_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero) %>% 
      distinct() %>% 
      mutate(perc = (total_zero/total)*100) %>% 
      mutate(sage_hyde32_name = "Pasture") %>% 
      dplyr::select(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero, perc) ->completeness_analytics_interpolated_pasture
    
    boundaries_country_managed %>% mutate(temp_id =1) %>% group_by(Urban_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total = sum(temp_id)) %>% 
      ungroup() %>% 
      filter(Carbon_AG== 0) %>% 
      group_by(Urban_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID) %>% 
      mutate(total_zero = sum(temp_id)) %>%
      ungroup() %>% 
      dplyr::select(Urban_Binary, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero) %>% 
      distinct() %>% 
      mutate(perc = (total_zero/total)*100) %>% 
      mutate(sage_hyde32_name = "Urban") %>% 
      dplyr::select(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, Country_ID, total, total_zero, perc) ->completeness_analytics_interpolated_urban
    
      completeness_interpolated_basincountry <- rbind(completeness_analytics_interpolated_unmanaged,
                                                      completeness_analytics_interpolated_crop,
                                                      completeness_analytics_interpolated_pasture,
                                                      completeness_analytics_interpolated_urban)
      write.csv(completeness_interpolated_basincountry, paste0("diagnostics/completeness_interpolated_countrybasin_",veg_type,"_",i,".csv"))
      
    
  }
  
  
  if(print_diagnostics){
    
    boundaries_final %>% left_join(GLU_data, by = c("x","y"))-> boundaries_glu            
    
    boundaries_glu %>% filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin==basin_debug)->temp_area_Ganges
    
    temp_area_Amazon %>% mutate(Crop_Binary = as.integer(crop_area_carbon > 0)) %>%
      mutate(Pasture_Binary = as.integer(pasture_area_carbon > 0)) %>%
      mutate(Urban_Binary = as.integer(urban_area_carbon > 0))-> temp_area_Ganges
    
    #Unmanaged
    temp_area_Ganges %>% filter(Carbon_AG != 0) %>% 
      group_by(sage_hyde32_name) %>% 
      mutate(Carbon_AG = if_else(i =="q3",quantile(Carbon_AG*scaler,0.75),
                                 if_else(i =="q1",quantile(Carbon_AG*scaler,0.25),
                                         if_else(i == "median", median(Carbon_AG*scaler),
                                                 if_else(i == "min", min(Carbon_AG*scaler),
                                                         if_else(i == "max", max(Carbon_AG*scaler),
                                                                 sum(Carbon_AG*scaler*refveg_area_carbon)/sum(refveg_area_carbon)))))),
             sum_area = sum(refveg_area_carbon)) %>% 
      ungroup() %>% 
      dplyr::select(sage_hyde32_name, Carbon_AG, sum_area) %>% 
      distinct()->Ganges_carbon_summary
    
    
    #Crop
    temp_area_Ganges %>% filter(Carbon_Crop != 0) %>%
      group_by(Crop_Binary) %>% 
      mutate(Carbon_Crop = if_else(i =="q3",quantile(Carbon_Crop*scaler,0.75),
                                   if_else(i =="q1",quantile(Carbon_Crop*scaler,0.25),
                                           if_else(i == "median", median(Carbon_Crop*scaler),
                                                   if_else(i == "min", min(Carbon_Crop*scaler),
                                                           if_else(i == "max", max(Carbon_Crop*scaler),
                                                                   sum(Carbon_Crop*scaler*crop_area_carbon)/sum(crop_area_carbon)))))),
             sum_area = sum(crop_area_carbon)) %>% 
      ungroup() %>%
      mutate(sage_hyde32_name = "Cropland") %>%
      dplyr::select(sage_hyde32_name, Carbon_Crop, sum_area) %>% 
      distinct()->Ganges_carbon_summary_crop
    
    
      #Pasture
    temp_area_Ganges %>% filter(Carbon_Pasture != 0) %>%
      group_by(Pasture_Binary) %>% 
      mutate(Carbon_Pasture = if_else(i =="q3",quantile(Carbon_Pasture*scaler,0.75),
                                      if_else(i =="q1",quantile(Carbon_Pasture*scaler,0.25),
                                              if_else(i == "median", median(Carbon_Pasture*scaler),
                                                      if_else(i == "min", min(Carbon_Pasture*scaler),
                                                              if_else(i == "max", max(Carbon_Pasture*scaler),
                                                                      sum(Carbon_Pasture*scaler*pasture_area_carbon)/sum(pasture_area_carbon)))))),
             sum_area = sum(pasture_area_carbon)) %>% 
    ungroup() %>% 
      mutate(sage_hyde32_name = "Pasture") %>%
      dplyr::select(sage_hyde32_name, Carbon_Pasture, sum_area) %>% 
      distinct()->Ganges_carbon_summary_pasture
      
      #Urban
    temp_area_Ganges %>% filter(Carbon_Urban != 0) %>%
        group_by(Urban_Binary) %>% 
        mutate(Carbon_Urban = if_else(i =="q3",quantile(Carbon_Urban*scaler,0.75),
                                     if_else(i =="q1",quantile(Carbon_Urban*scaler,0.25),
                                             if_else(i == "median", median(Carbon_Urban*scaler),
                                                     if_else(i == "min", min(Carbon_Urban*scaler),
                                                             if_else(i == "max", max(Carbon_Urban*scaler),
                                                                     sum(Carbon_Urban*scaler*urban_area_carbon)/sum(urban_area_carbon)))))),
               sum_area = sum(urban_area_carbon)) %>% 
      ungroup() %>% 
      mutate(sage_hyde32_name = "Urban") %>%
        dplyr::select(sage_hyde32_name, Carbon_Urban, sum_area) %>% 
        distinct()->Ganges_carbon_summary_urban
    
    colnames(Ganges_carbon_summary) <- c('sage_hyde32_name','Carbon','sum_area')
    colnames(Ganges_carbon_summary_crop) <- c('sage_hyde32_name','Carbon','sum_area')
    colnames(Ganges_carbon_summary_pasture) <- c('sage_hyde32_name','Carbon','sum_area')
    colnames(Ganges_carbon_summary_urban) <- c('sage_hyde32_name','Carbon','sum_area')
    
    Ganges_carbon_summary <- rbind(Ganges_carbon_summary,
                                   Ganges_carbon_summary_crop,
                                   Ganges_carbon_summary_pasture,
                                   Ganges_carbon_summary_urban)
    
    rm(Amazon_carbon_summary_crop)
    rm(Amazon_carbon_summary_pasture)
    rm(Amazon_carbon_summary_urban)
    
    write.csv(Ganges_carbon_summary,paste0("diagnostics/Ganges_basin_",veg_type,i,".csv"))
    
    boundaries_glu ->temp_area_global
    
    temp_area_global %>% filter(Carbon_AG != 0) %>% 
      group_by(sage_hyde32_name) %>% 
      mutate(Carbon_AG = if_else(i =="q3",quantile(Carbon_AG*scaler,0.75),
                                 if_else(i =="q1",quantile(Carbon_AG*scaler,0.25),
                                         if_else(i == "median", median(Carbon_AG*scaler),
                                                 if_else(i == "min", min(Carbon_AG*scaler),
                                                         if_else(i == "max", max(Carbon_AG*scaler),
                                                                 sum(Carbon_AG*scaler*refveg_area_carbon)/sum(refveg_area_carbon)))))),
             sum_area = sum(refveg_area_carbon)) %>% 
      ungroup() %>% 
      dplyr::select(sage_hyde32_name, Carbon_AG, sum_area) %>% 
      distinct()->Global_carbon_summary
    
    write.csv(Global_carbon_summary,paste0("diagnostics/Global_carbon_",veg_type,i,".csv"))
  
  }
   #Remove some rasters for space and time
   rm(temp_data)
   rm(temp_area_w_IDS)
   rm(temp_area)
   rm(nn_index)
   rm(near_data)
   rm(temp_area_boundaries_split)
   rm(tmp_raster)
   rm(Carbon_thematic_data_temp)
   rm(Carbon_thematic_data_joined)
   rm(boundaries_final_crop)
   rm(boundaries_final_pasture)
   rm(boundaries_final_urban)
  
   
   
   if(print_plots){
     
     Crop_plot <- ggplot(data = boundaries_final %>% filter(Carbon_Crop != 0) %>% mutate(Carbon_Crop = Carbon_Crop*scaler), aes(x=x,y=y))+
       geom_tile(aes(fill=Carbon_Crop))+
       scale_fill_distiller(palette = "Spectral", direction = -1)+
       ggtitle("Distribution of carbon in MgC/ha for Cropland")
     Crop_plot + scheme_basic
     ggsave(paste0("diagnostics/Carbon_Crop_values",veg_type,i,".png"),width = 18, height = 10 )
     
     Pasture_plot <- ggplot(data = boundaries_final %>% filter(Carbon_Pasture != 0) %>% mutate(Carbon_Pasture = Carbon_Pasture*scaler), aes(x=x,y=y))+
       geom_tile(aes(fill=Carbon_Pasture))+
       scale_fill_distiller(palette = "Spectral", direction = -1)+
       ggtitle("Distribution of carbon in MgC/ha for Pastures")
     Pasture_plot + scheme_basic
     ggsave(paste0("diagnostics/Carbon_Pasture_values",veg_type,i,".png"),width = 18, height = 10 )
     
     Urban_plot <- ggplot(data = boundaries_final %>% filter(Carbon_Urban != 0) %>% mutate(Carbon_Urban = Carbon_Urban*scaler), aes(x=x,y=y))+
       geom_tile(aes(fill=Carbon_Urban))+
       scale_fill_distiller(palette = "Spectral", direction = -1)+
       ggtitle("Distribution of carbon in MgC/ha for Urban")
     Urban_plot + scheme_basic
     ggsave(paste0("diagnostics/Carbon_Urban_values",veg_type,i,".png"),width = 18, height = 10 )


     g <- ggplot(data= boundaries_final %>% filter(Carbon_AG != 0) %>% mutate(Carbon_AG = Carbon_AG*scaler), aes(x=x, y =y))+
       geom_tile(aes(fill=Carbon_AG))+
       facet_wrap(~sage_hyde32_name)+
       scale_fill_distiller(palette = "Spectral", direction = -1)+
       ggtitle("Distribution of carbon in MgC/ha for moirai land types")



     g + scheme_basic
     ggsave(paste0("diagnostics/Carbon_values_by_moirai_land_type",veg_type,i,".png"),width = 18, height = 10 )

     boundaries_final %>% left_join(GLU_data, by = c("x","y"))-> boundaries_glu
     boundaries_glu %>% filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin==basin_debug)->temp_area_Amazon

     g <- ggplot(data= temp_area_Amazon %>% filter(Carbon_AG != 0) %>% mutate(Carbon_AG=Carbon_AG*scaler ), aes(x=x, y =y))+
       geom_tile(aes(fill=Carbon_AG))+
       facet_wrap(~sage_hyde32_name)+
       scale_fill_distiller(palette = "Spectral", direction = -1)

     g + scheme_basic
     ggsave(paste0("diagnostics/basin_values",veg_type,i,".png"),width = 18, height = 10 )
   }
   
  #Final carbon_map --- Unmanaged Land
  boundaries_final %>%  dplyr::select(x, y, Carbon_AG) %>% 
    dplyr::filter(Carbon_AG != 0 )->raster_data_frame    
  
  Carbon_map <- as.data.frame(rasterToPoints(Carbon_thematic))
  if(veg_type== "soil"){
  Carbon_map %>% left_join(raster_data_frame, by = c("x","y")) %>% 
    mutate(Carbon_AG = if_else(is.na(Carbon_AG),moirai_no_data_value,Carbon_AG*scaler)) %>% 
    dplyr::select(x,y,Carbon_AG) %>% 
    distinct()->raster_data_joined}else{
      Carbon_map %>% left_join(raster_data_frame,by =c("x","y")) %>% 
        mutate(Carbon_AG = if_else(is.na(Carbon_AG),moirai_no_data_value,Carbon_AG)) %>% 
        dplyr::select(x,y,Carbon_AG) %>% 
        distinct()->raster_data_joined  
    }
      
  #Final carbon_map --- Crops
  boundaries_final %>%  dplyr::select(x, y, Carbon_Crop) %>% 
    dplyr::filter(Carbon_Crop != 0 )->raster_data_frame     
    
      Carbon_map <- as.data.frame(rasterToPoints(Carbon_thematic))
      if(veg_type== "soil"){
        Carbon_map %>% left_join(raster_data_frame, by = c("x","y")) %>% 
          mutate(Carbon_Crop = if_else(is.na(Carbon_Crop),moirai_no_data_value,Carbon_Crop*scaler)) %>% 
          dplyr::select(x,y,Carbon_Crop) %>% 
          distinct()->raster_data_joined_crop}else{
            Carbon_map %>% left_join(raster_data_frame,by =c("x","y")) %>% 
              mutate(Carbon_Crop = if_else(is.na(Carbon_Crop),moirai_no_data_value,Carbon_Crop)) %>% 
              dplyr::select(x,y,Carbon_Crop) %>% 
              distinct()->raster_data_joined_crop
          }
      
  #Final carbon_map --- Pasture
            
    boundaries_final %>%  dplyr::select(x, y, Carbon_Pasture) %>% 
        dplyr::filter(Carbon_Pasture != 0 )->raster_data_frame        
    Carbon_map <- as.data.frame(rasterToPoints(Carbon_thematic))
    if(veg_type== "soil"){
              Carbon_map %>% left_join(raster_data_frame, by = c("x","y")) %>% 
              mutate(Carbon_Pasture = if_else(is.na(Carbon_Pasture),moirai_no_data_value,Carbon_Pasture*scaler)) %>% 
              dplyr::select(x,y,Carbon_Pasture) %>% 
              distinct()->raster_data_joined_pasture}else{
                  Carbon_map %>% left_join(raster_data_frame,by =c("x","y")) %>% 
                  mutate(Carbon_Pasture = if_else(is.na(Carbon_Pasture),moirai_no_data_value,Carbon_Pasture)) %>% 
                  dplyr::select(x,y,Carbon_Pasture) %>% 
                  distinct()->raster_data_joined_pasture
                }
  #Final carbon_map --- Pasture
    boundaries_final %>%  dplyr::select(x, y, Carbon_Urban) %>% 
        dplyr::filter(Carbon_Urban != 0 )->raster_data_frame        
    Carbon_map <- as.data.frame(rasterToPoints(Carbon_thematic))
        if(veg_type== "soil"){
            Carbon_map %>% left_join(raster_data_frame, by = c("x","y")) %>% 
            mutate(Carbon_Urban = if_else(is.na(Carbon_Urban),moirai_no_data_value,Carbon_Urban*scaler)) %>% 
            dplyr::select(x,y,Carbon_Urban) %>% 
            distinct()->raster_data_joined_urban}else{
              Carbon_map %>% left_join(raster_data_frame,by =c("x","y")) %>% 
              mutate(Carbon_Urban = if_else(is.na(Carbon_Urban),moirai_no_data_value,Carbon_Urban)) %>% 
              dplyr::select(x,y,Carbon_Urban) %>% 
              distinct()->raster_data_joined_urban            
                
    }
  
    
    
  raster_print<- rasterize(raster_data_joined[,1:2],Carbon_thematic,raster_data_joined[,3])
  writeRaster(raster_print,paste0("outputs/",veg_type,"_carbon_",i), format= "ENVI",options="INTERLEAVE=BIL", overwrite = TRUE,bandorder="BIL")  
  
  raster_print<- rasterize(raster_data_joined_crop[,1:2],Carbon_thematic,raster_data_joined[,3])
  writeRaster(raster_print,paste0("outputs/",veg_type,"_carbon_crop_",i), format= "ENVI",options="INTERLEAVE=BIL", overwrite = TRUE,bandorder="BIL")
  
  raster_print<- rasterize(raster_data_joined_pasture[,1:2],Carbon_thematic,raster_data_joined[,3])
  writeRaster(raster_print,paste0("outputs/",veg_type,"_carbon_pasture_",i), format= "ENVI",options="INTERLEAVE=BIL", overwrite = TRUE,bandorder="BIL")
  
  raster_print<- rasterize(raster_data_joined_urban[,1:2],Carbon_thematic,raster_data_joined[,3])
  writeRaster(raster_print,paste0("outputs/",veg_type,"_carbon_urban_",i), format= "ENVI",options="INTERLEAVE=BIL", overwrite = TRUE,bandorder="BIL")
  
  rm(raster_print)
  rm(raster_data_joined_urban)
  rm(raster_data_joined_pasture)
  rm(raster_data_joined_crop)
  rm(raster_data_joined)
  rm(boundaries_final)
  rm(boundaries_list)
  
  
  gc()
  
}



#Additional Code to fix soil carbon NAs
# The weighted average values for soil carbon are different than those for other states.
# There are some NAs (approx 1158 values) that we replace with the medians. 
rdata_q1 <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_q1.envi"))) %>% filter(soil_carbon_q1 != moirai_no_data_value)
rdata_q3 <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_q3.envi"))) %>% filter(soil_carbon_q3 != moirai_no_data_value)
rdata_min <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_min.envi"))) %>% filter(soil_carbon_min != moirai_no_data_value)
rdata_max <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_max.envi"))) %>% filter(soil_carbon_max != moirai_no_data_value)
rdata_median <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_median.envi")))%>% filter(soil_carbon_median != moirai_no_data_value) 
rdata_wavg <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_weighted_average.envi"))) %>% filter(soil_carbon_weighted_average != moirai_no_data_value)

rdata_median %>% left_join(rdata_min) %>% left_join(rdata_max)%>% left_join(rdata_q1)%>% left_join(rdata_q3) %>% left_join(rdata_wavg)->j 

na_cells <- j[is.na(j$soil_carbon_weighted_average),]


na_cells$soil_carbon_weighted_average <- na_cells$soil_carbon_median

rdata_wavg <- as.data.frame(rasterToPoints(raster("outputs/soil_carbon_weighted_average.envi"))) %>% 
  left_join(na_cells %>% rename(replace_values = soil_carbon_weighted_average), by= c("x","y")) %>% 
  mutate(soil_carbon_weighted_average = if_else(is.na(replace_values),soil_carbon_weighted_average,replace_values)) %>% 
  dplyr::select(x,y,soil_carbon_weighted_average) %>% distinct() 

raster_print<- rasterize(rdata_wavg[,1:2],Carbon_thematic,rdata_wavg[,3])


writeRaster(raster_print,"outputs/soil_carbon_weighted_average", format= "ENVI",options="INTERLEAVE=BIL", overwrite = TRUE,bandorder="BIL")

#Additional Code for making assorted charts and graphs

boundaries_final %>% dplyr::select(x, y, Carbon_AG, Carbon_Crop, sage_hyde32_name) %>%
  dplyr::filter(Carbon_Crop > 0) %>%
  dplyr::filter(Carbon_AG > 0) %>%
  mutate(Carbon_AG = Carbon_AG*scaler) %>%
  mutate(Carbon_Crop = Carbon_Crop*scaler) %>%
  mutate(CropUnmanagedRatio = Carbon_Crop/Carbon_AG) -> ratio_df

ratio_plot <- ggplot(data = ratio_df %>% filter(CropUnmanagedRatio < 2), aes(x=x,y=y))+
  geom_tile(aes(fill=CropUnmanagedRatio))+
  scale_fill_distiller(palette = "Spectral", direction = -1)+
  ggtitle("Ratio of Crop to Unmanaged Land Carbon Values (MgC/ha)")
ratio_plot + scheme_basic
ggsave(paste0("diagnostics/CropUnmanagedRatio_Condensed_",veg_type,i,".png"),width = 18, height = 10 )


#Plots for basins
Urban_plot <- ggplot(data = temp_area_Ganges %>% filter(Carbon_Urban != 0) %>% mutate(Carbon_Urban = Carbon_Urban*scaler), aes(x=x,y=y))+
  geom_tile(aes(fill=Carbon_Urban))+
  scale_fill_distiller(palette = "Spectral", direction = -1)+
  ggtitle("Distribution of carbon in MgC/ha for Urban")
Urban_plot + scheme_basic
ggsave(paste0("diagnostics/Amazon_Carbon_Urban_values",veg_type,i,".png"),width = 18, height = 10 )

Pasture_plot <- ggplot(data = temp_area_Ganges %>% filter(Carbon_Pasture != 0) %>% mutate(Carbon_Pasture = Carbon_Pasture*scaler), aes(x=x,y=y))+
  geom_tile(aes(fill=Carbon_Pasture))+
  scale_fill_distiller(palette = "Spectral", direction = -1)+
  ggtitle("Distribution of carbon in MgC/ha for Pastures")
ggsave(paste0("diagnostics/Amazon_Carbon_Pasture_values",veg_type,i,".png"),width = 18, height = 10 )

Crop_plot <- ggplot(data = temp_area_Ganges %>% filter(Carbon_Crop != 0) %>% mutate(Carbon_Crop = Carbon_Crop*scaler), aes(x=x,y=y))+
  geom_tile(aes(fill=Carbon_Crop))+
  scale_fill_distiller(palette = "Spectral", direction = -1)+
  ggtitle("Distribution of carbon in MgC/ha for Cropland")
ggsave(paste0("diagnostics/Amazon_Carbon_Crop_values",veg_type,i,".png"),width = 18, height = 10 )

g <- ggplot(data= temp_area_Ganges %>% filter(Carbon_AG != 0) %>% mutate(Carbon_AG = Carbon_AG*scaler), aes(x=x, y =y))+
  geom_tile(aes(fill=Carbon_AG))+
  facet_wrap(~sage_hyde32_name)+
  scale_fill_distiller(palette = "Spectral", direction = -1)+
  ggtitle("Distribution of carbon in MgC/ha for moirai land types")
ggsave(paste0("diagnostics/Amazon_Carbon_values",veg_type,i,".png"),width = 18, height = 10 )

write.csv(ratio_df, "diagnostics/Crop_to_Unmanaged_ratio.csv")


#Plot Creation

CropPlotDF <- subset(boundaries_final, Carbon_Crop > 0)[, 'Carbon_Crop']
CropPlotDF$LandType <- 'Cropland'
colnames(CropPlotDF) <- c('Carbon', 'LandType')

PasturePlotDF <- subset(boundaries_final, Carbon_Pasture > 0)[, 'Carbon_Pasture']
PasturePlotDF$LandType <- 'Pasture'
colnames(PasturePlotDF) <- c('Carbon', 'LandType')

UrbanPlotDF <- subset(boundaries_final, Carbon_Urban > 0)[, 'Carbon_Urban']
UrbanPlotDF$LandType <- 'Urban'
colnames(UrbanPlotDF) <- c('Carbon', 'LandType')

ManagedPlotDF <- rbind(CropPlotDF, PasturePlotDF, UrbanPlotDF)
ManagedPlotDF$Carbon <- ManagedPlotDF$Carbon * 10

summary(CropPlotDF$Carbon)
summary(PasturePlotDF$Carbon)
summary(UrbanPlotDF$Carbon)


g <- ggplot(ManagedPlotDF, aes(x=Carbon, color=LandType, fill=LandType)) + geom_histogram( alpha=.5, binwidth=40) + xlim(0,1000) + ylim(0,150000) +
  xlab("Soil carbon at depth of 0-30 cms in MgC/ha")+ ylab("Count of 5 arcmin gridcells")   
  #geom_vline(data=ManagedPlotDF, aes(xintercept=mean(Carbon), color = "LandType"))
plot(g +scheme_basic)

ggsave(paste0("diagnostics/ManagedCarbonGlobalPlot_3.png"),width = 18, height = 10 )


PosterTable <- as.data.frame(matrix(nrow = 3, ncol = 4))
colnames(PosterTable) <- c("land_cover_type", "soil_carbon", "AG_carbon", "BG_carbon")
PosterTable$land_cover_type <- c("Cropland","Pasture","Urban")
PosterTable$soil_carbon[1] <- sum(boundaries_final$Carbon_Crop)
PosterTable$soil_carbon[2] <- sum(boundaries_final$Carbon_Pasture)
PosterTable$soil_carbon[3] <- sum(boundaries_final$Carbon_Urban)



#Ganges Plot
boundaries_final %>% left_join(GLU_data, by = c("x","y"))-> boundaries_glu            

boundaries_glu %>% filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin==basin_debug)->temp_area_Ganges

CropPlotDF <- subset(temp_area_Ganges, Carbon_Crop > 0)[, c('Carbon_Crop','x','y')]
CropPlotDF$LandType <- 'Cropland'
colnames(CropPlotDF) <- c('Carbon', 'x', 'y', 'LandType')

PasturePlotDF  <- subset(temp_area_Ganges, Carbon_Pasture > 0)[, c('Carbon_Pasture','x','y')]
PasturePlotDF$LandType <- 'Pasture'
colnames(PasturePlotDF) <- c('Carbon', 'x', 'y', 'LandType')

UnmanagedPlotDF <- subset(temp_area_Ganges, Carbon_AG > 0)[, c('Carbon_AG','x','y')]
UnmanagedPlotDF$LandType <- 'Unmanaged'
colnames(UnmanagedPlotDF) <- c('Carbon', 'x', 'y', 'LandType')

ManagedPlotDF <- rbind(CropPlotDF, PasturePlotDF, UnmanagedPlotDF)

g <- ggplot(data= ManagedPlotDF %>% filter(Carbon != 0) %>% mutate(Carbon = Carbon*scaler), aes(x=x, y =y))+
  geom_tile(aes(fill=Carbon))+
  facet_wrap(~LandType, ncol=1)+
  scale_fill_distiller(palette = "Spectral", direction = -1)+
  ggtitle("Distribution of carbon in MgC/ha for managed land types")
plot(g+scheme_basic)

ggsave(paste0("diagnostics/ManagedCarbonGangesPlotSoil2.png"),width = 12, height = 25 )




scheme_basic <- theme_bw() +
  theme(plot.title = element_text(size = 30)) +
  theme(legend.text = element_text(size = 15, hjust = 0.5)) +
  theme(legend.title = element_text(size = 15)) +
  theme(axis.text = element_text(size = 42)) +
  theme(axis.title = element_text(size = 35, face = "bold")) +
  theme(plot.title = element_text(size = 30, face = "bold", vjust = 1)) +
  theme(plot.subtitle = element_text(size = 22, face = "bold", vjust = 1))+ 
  theme(strip.text = element_text(size = 18))+
  theme(strip.text.x = element_text(size = 30, face = "bold"))+
  theme(strip.text.y = element_text(size = 15, face = "bold"))+
  theme(legend.position = "right")+
  theme(legend.text = element_text(size = 15))+
  theme(legend.title = element_text(size = 15,color = "black",face="bold"))+
  theme(axis.text.x= element_text(hjust=1))+
  theme(legend.background = element_blank())



g <- compare_carbon_distribution_ESA(carbon_type = c_type,
                                     basin_for_testing = "Amazon",
                                     moirai_LC = i,
                                     harmonized_carbon_raster_file_names = c("AG_carbon_q1.envi",
                                                                             "AG_carbon_q3.envi",
                                                                             "AG_carbon_median.envi",
                                                                             "AG_carbon_min.envi",
                                                                             "AG_carbon_max.envi",
                                                                             "AG_carbon_weighted_average.envi"),
                                     plot_lim = 8000,
                                     produce_ESA_distribution = FALSE)





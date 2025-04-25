# Created by Evan Margiotta, Kanishka Narayan and Alan Di Vittorio

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

# Moirai is free software: you can use it under the terms of the modified BSD-3 license (see ./moirai/license.txt)

#Purpose : This script harmonizes the 1026 rasters stored in the zip file FAO_carbon_rasters.zip to the moirai refrence
#vegetation classes (currently in 2000). This script generates 18 final rasters that are used within the moirai code.  
#This script performs the harmonization for data for soil carbon from the FAO HWSD (0-30cms or 0-100 cms) and vegetation carbon
#from Spawn et al. for above and below ground vegetation.

#Load libraries
library(raster)
library(dplyr)
library(data.table)
library(FNN)
library(ggplot2)
library(rgdal)
library(tidyr)
library(stringr)


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

#Set the working directory to moirai/ancillary/carbon_harmonization/
path_to_working_directory <- "./"
# 
setwd(path_to_working_directory)

#Create some directories
dir.create("outputs/",showWarnings = FALSE)
dir.create("diagnostics/",showWarnings = FALSE)

#moirai no data value (specified in moirai.h)
moirai_no_data_value <- -9999

#Scalers (Vegetation numbers need to be multiplied by 0.1 to get values, soil by 1000)
veg_scaler <- 0.1
soil_scaler <- 100 

# These are the SAGE HYDE land use classes. These need to be ignored since we are only interested in LC.
sage_hyde_land_use_codes <- 23:34

#Check if files have been unzipped. If not unzip them
if(!dir.exists("FAO_carbon_rasters/")){
  
  unzip("FAO_carbon_rasters.zip")
  
}

#Set basin for debugging. Select basin of your choice from the input file 
#input_files/basin_to_country_mapping.csv 
basin_debug <- 157

# User defined variables


#Read in carbon thematic file for moirai. The below can be set to `AG` for above
#ground vegetation, `BG` for below ground vegetation and `soil` for soil. It is recommended that the user run 
#these one by one given the time it takes to generate the final rasters.  
veg_type <- "soil" 

#decide whether to merge the two subsoil and topsoil or just run with topsoil
topsoil_only <- TRUE

#This will print out diagnostics (.csv files) for the World (Global) and the Amazon basin
print_diagnostics <- TRUE
#This will generate maps for the World and the selected basin for the selected veg type
print_plots <- FALSE

basin_diagnostics <- TRUE

#This is the current thematic map in moirai. Ensure the model has been run.
year <- "2000"
Carbon_thematic <- raster(paste0("input_files/refveg_carbon_thematic_",year,".bil"))
#Carbon_thematic <- raster("input_files/refveg_carbon_thematic.bil") # add 2010 file but make a variable so selecting the file can be dynamic
Carbon_thematic_data <- as.data.frame(rasterToPoints(Carbon_thematic)) %>% 
  dplyr::filter(refveg_carbon_thematic_2000 != moirai_no_data_value)

#Current carbon area
Carbon_area <- as.data.frame(rasterToPoints(raster(paste0("input_files/refveg_area_carbon_",year, ".bil")))) %>% 
  filter(paste0("refveg_area_carbon_", year) != moirai_no_data_value)

#GLU mapping
GLU_data <- as.data.frame(rasterToPoints(raster("input_files/gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin.tif")))
#Read in ESA land cover classes
IGBP_classes <- read.csv("input_files/IGBP_classes.csv", stringsAsFactors = FALSE)



#Can set states of carbon here.
states_of_carbon <- c("q3","min","max","q1","weighted_average","median")

num_neighbor <- 40 #can set this to a list (ie "c(1,10,20,40)") if want to run using multiple different search boxes

for(a in num_neighbor){
  # change line below for however many neighbors
  num_neighbor <- a
  #Helper function for nearest neighbor interpolation
  find_nearest_reighbor <- function(sample_data){
    colnames(sample_data)<- final_col_names
    
    
    sample_data_zero <- sample_data %>% 
      filter(Carbon_AG == 0) %>% 
      left_join(nn_index, by = c("ID"))
    
    zero_cells <- c(sample_data_zero$ID)
    
    sample_data_non_zero <- sample_data %>% 
      filter(Carbon_AG != 0)
    
    
    
    sample_data_zero %>% gather("Nearest_neighbor","nn_value","V1":(paste0("V",num_neighbor))) %>% 
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
  for (i in states_of_carbon){
    Carbon_thematic_data_joined <- Carbon_thematic_data
    for (j in c(IGBP_classes[1:nrow(IGBP_classes),])){
      
      if(topsoil_only){
        tmp_raster <- as.data.frame(rasterToPoints(raster(paste0("FAO_carbon_rasters/",veg_type,"/",j,"_topsoil_",i,".bil"))))
        colnames(tmp_raster) <- c("x","y",str_remove(colnames(tmp_raster)[3],paste0("_topsoil_",i)))
        Carbon_thematic_data_joined %>% left_join(tmp_raster, by= c("x","y"))->Carbon_thematic_data_joined
        rm(tmp_raster)
      }
      else{
        tmp_subsoil_raster <- as.data.frame(rasterToPoints(raster(paste0("FAO_carbon_rasters/",veg_type,"/",j,"_subsoil_",i,".bil"))))
        tmp_topsoil_raster <- as.data.frame(rasterToPoints(raster(paste0("FAO_carbon_rasters/",veg_type,"/",j,"_topsoil_",i,".bil"))))
        tmp_raster <- left_join(tmp_subsoil_raster, tmp_topsoil_raster, by = c("x", "y"))
        colnames(tmp_raster) <- c("x","y","subsoil","topsoil")
        tmp_raster$soilcombined <- tmp_raster$subsoil + tmp_raster$topsoil
        tmp_raster <- subset(tmp_raster, select = -c(subsoil, topsoil))
        colnames(tmp_raster) <- c("x","y", j)
        Carbon_thematic_data_joined %>% left_join(tmp_raster, by= c("x","y"))->Carbon_thematic_data_joined
        rm(tmp_raster)
      }
     
      
      
      
    }
    
    
    
    
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
      rename(refveg_carbon_thematic_2000 = sage_hyde32_code ) %>%
      mutate(sage_hyde32_name = gsub("_","",sage_hyde32_name)) %>% 
      distinct()
    
    Carbon_thematic_data_joined %>% left_join(SAGE_names, by = c(paste0("refveg_carbon_thematic_",year))) %>% 
      mutate(Carbon_AG = 0)->Carbon_thematic_data_temp
    
    #colnames(Carbon_thematic_data_temp)<- gsub(paste0("_",veg_type,"_",i),"",colnames(Carbon_thematic_data_temp))
    
    #Rule based mapping of carbon. These are specified in the file input_files/IGBP_moirai_classes.csv
    Carbon_thematic_data_temp %>% 
      mutate(Carbon_AG = if_else(sage_hyde32_name=="TropicalEvergreenForest/Woodland",
                                 if_else(EBF > 0, EBF,
                                         if_else(ENF > 0,ENF,
                                                 if_else(MixedForest > 0, MixedForest,
                                                         if_else(CombinedForest > 0, CombinedForest, 0
                                                         )))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="TropicalDeciduousForest/Woodland",
                                 if_else(DBF > 0, DBF,
                                         if_else(DNF > 0, DNF,
                                                 if_else(MixedForest > 0, MixedForest,
                                                         if_else(CombinedForest > 0, CombinedForest, 0
                                                         )))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="TemperateBroadleafEvergreenForest/Woodland",
                                 if_else(EBF > 0, EBF,
                                         if_else(ENF > 0, ENF,
                                                 if_else(MixedForest > 0,MixedForest,
                                                         if_else(CombinedForest > 0, CombinedForest, 0
                                                         )))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="TemperateNeedleleafEvergreenForest/Woodland",
                                 if_else(ENF > 0, ENF,
                                         if_else(EBF > 0, EBF,
                                                 if_else(MixedForest > 0,MixedForest,
                                                         if_else(CombinedForest > 0, CombinedForest, 0
                                                         )))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="TemperateDeciduousForest/Woodland",
                                 if_else(DBF > 0, DBF,
                                         if_else(DNF > 0,DNF,
                                                 if_else(MixedForest > 0, MixedForest,
                                                         if_else(CombinedForest > 0, CombinedForest, 0
                                                         )))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="BorealEvergreenForest/Woodland",
                                 if_else(ENF > 0, ENF,
                                         if_else(EBF > 0,EBF,
                                                 if_else(MixedForest > 0, MixedForest, 0
                                                 ))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="BorealDeciduousForest/Woodland",
                                 if_else(DNF > 0, DNF,
                                         if_else(DBF > 0, DBF,
                                                 if_else(MixedForest > 0, MixedForest,
                                                         if_else(CombinedForest > 0, CombinedForest, 0
                                                         )))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="Evergreen/DeciduousMixedForest/Woodland",
                                 if_else(MixedForest > 0, MixedForest,
                                         if_else(CombinedForest > 0, CombinedForest, 0
                                         )),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="Savanna",
                                 if_else(Savannas > 0, Savannas,
                                         if_else(WoodySavannas > 0,WoodySavannas,
                                                 if_else(Grasslands > 0, Grasslands, 0
                                                 ))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="Grassland/Steppe",
                                 if_else(Grasslands > 0, Grasslands,
                                         if_else(Savannas > 0, Savannas,
                                                 if_else(WoodySavannas > 0, WoodySavannas, 0
                                                 ))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="DenseShrubland",
                                 if_else(ClosedShrub > 0, ClosedShrub,
                                         if_else(OpenShrub > 0, OpenShrub, 0
                                         )),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="OpenShrubland",
                                 if_else(OpenShrub > 0, OpenShrub,
                                         if_else(ClosedShrub > 0,ClosedShrub, 0
                                         )),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="Tundra",
                                 if_else(BSV > 0, BSV,
                                         if_else(SnowIce > 0, SnowIce,
                                                 if_else(OpenShrub > 0, OpenShrub, 0
                                                 ))),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="Desert",
                                 if_else(BSV > 0, BSV, 0
                                 ),Carbon_AG),
             Carbon_AG = if_else(sage_hyde32_name=="Polardesert/rock/ice",
                                 if_else(SnowIce > 0, SnowIce,
                                          if_else(BSV > 0, BSV,
                                                  if_else(OpenShrub > 0, OpenShrub,
                                                         if_else(ClosedShrub>0, ClosedShrub,
                                                                 if_else(Savannas > 0, Savannas,
                                                                         if_else(WoodySavannas>0, WoodySavannas,
                                                                                 if_else(Grasslands>0, Grasslands,
                                                                                         if_else(CombinedForest>0, CombinedForest,
                                                                                                 if_else(MixedForest > 0, MixedForest,0
                                                                 ))))))))),
                                 Carbon_AG))->temp_data
    
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
      
      write.csv(completeness_analytics, paste0("diagnostics/completeness_",veg_type,"_",i,".csv"))
    }
    
    
    #Read in carbon areas
    
    
    temp_data %>% inner_join(Carbon_area, by =c("x","y")) %>% 
      filter(paste0("refveg_area_carbon_", year) > 0)->temp_area
    
    temp_area %>% mutate(ID = 1:nrow(temp_area))->temp_area_w_IDS
    
    #Get nearest neighbors (X nearest geographic neighbors)
    near_data <- get.knn(temp_area_w_IDS[, 1:2], k = num_neighbor)
    nn_index <- as.data.frame(near_data$nn.index) %>% 
      mutate(ID = 1:nrow(temp_area_w_IDS))
    
    #Now split the data 
    temp_area_boundaries_split <-split(temp_area_w_IDS,temp_area_w_IDS$refveg_carbon_thematic_2000) 
    
    #Save final column names
    final_col_names <- colnames(temp_area_w_IDS)
    
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
      
      write.csv(completeness_analytics_interpolated, paste0("diagnostics/completeness_interpolated_",veg_type,"_",i,".csv"))
    }
    
    #Check with Evan and Alan if we can take this out. It takes too much time and this diagnostic may not be required. 
    # if(basin_diagnostics){ # basin
    #   boundaries_final %>% left_join(GLU_data, by = c("x","y"))->boundaries_glu
    #   
    #    boundaries_glu %>% mutate(temp_id =1) %>% group_by(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin) %>% 
    #    mutate(total = sum(temp_id))%>% 
    #    ungroup() %>% 
    #    filter(Carbon_AG== 0) %>% 
    #    group_by(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin) %>% 
    #    mutate(total_zero = sum(temp_id)) %>% 
    #    ungroup() %>% 
    #    dplyr::select(sage_hyde32_name, gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin, total, total_zero) %>% 
    #    distinct() %>% 
    #    mutate(perc = (total_zero/total)*100)->completeness_analytics_interpolated_basin
    #   
    #   write.csv(completeness_analytics_interpolated_basin, paste0("diagnostics/completeness_interpolated_basin_",veg_type,"_",i,"_",num_neighbor,".csv"))
    # }
    
    if(print_diagnostics){
      
      boundaries_final %>% left_join(GLU_data, by = c("x","y"))->boundaries_glu            
      
      boundaries_glu %>% filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin==basin_debug)->temp_area_Amazon
      
      temp_area_Amazon %>% filter(Carbon_AG != 0) %>% 
        group_by(sage_hyde32_name) %>% 
        mutate(Carbon_AG = if_else(i =="q3",quantile(Carbon_AG*scaler,0.75),
                                   if_else(i =="q1",quantile(Carbon_AG*scaler,0.25),
                                           if_else(i == "median", median(Carbon_AG*scaler),
                                                   if_else(i == "min", min(Carbon_AG*scaler),
                                                           if_else(i == "max", max(Carbon_AG*scaler),
                                                                   sum(Carbon_AG*scaler*refveg_area_carbon_2000)/sum(refveg_area_carbon_2000)))))),
               sum_area = sum(refveg_area_carbon_2000)) %>% 
        ungroup() %>% 
        dplyr::select(sage_hyde32_name, Carbon_AG, sum_area) %>% 
        distinct()->Amazon_carbon_summary
      
      #write.csv(Amazon_carbon_summary,paste0("diagnostics/basin_",veg_type,i,".csv"))
      
      boundaries_glu ->temp_area_global
      
      temp_area_global %>% filter(Carbon_AG != 0) %>% 
        group_by(sage_hyde32_name) %>% 
        mutate(Carbon_AG = if_else(i =="q3",quantile(Carbon_AG*scaler,0.75),
                                   if_else(i =="q1",quantile(Carbon_AG*scaler,0.25),
                                           if_else(i == "median", median(Carbon_AG*scaler),
                                                   if_else(i == "min", min(Carbon_AG*scaler),
                                                           if_else(i == "max", max(Carbon_AG*scaler),
                                                                   sum(Carbon_AG*scaler*refveg_area_carbon_2000)/sum(refveg_area_carbon_2000)))))),
               sum_area = sum(refveg_area_carbon_2000)) %>% 
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
    
    if(print_plots){
      
      
      g <- ggplot(data= boundaries_final %>% filter(Carbon_AG != 0) %>% mutate(Carbon_AG = Carbon_AG*scaler), aes(x=x, y =y))+
        geom_tile(aes(fill=Carbon_AG))+
        facet_wrap(~sage_hyde32_name)+
        scale_fill_distiller(palette = "Spectral", direction = -1)+
        ggtitle("Distribution of carbon in MgC/ha for moirai land types")
      
      
      
      g + scheme_basic
      ggsave(paste0("diagnostics/Carbon_values_by_moirai_land_type",veg_type,i,".png"),width = 18, height = 10 )
      
      boundaries_glu %>% filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin==basin_debug)->temp_area_Amazon
      
      g <- ggplot(data= temp_area_Amazon %>% filter(Carbon_AG != 0) %>% mutate(Carbon_AG=Carbon_AG*scaler ), aes(x=x, y =y))+
        geom_tile(aes(fill=Carbon_AG))+
        facet_wrap(~sage_hyde32_name)+
        scale_fill_distiller(palette = "Spectral", direction = -1)
      
      g + scheme_basic
      ggsave(paste0("diagnostics/basin_values",veg_type,i,".png"),width = 18, height = 10 )
    }
    
    
    boundaries_final %>%  dplyr::select(x, y, Carbon_AG) %>% 
      dplyr::filter(Carbon_AG != 0 )->raster_data_frame    
    
    #Final carbon_map
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
    
    raster_print<- rasterize(raster_data_joined[,1:2],Carbon_thematic,raster_data_joined[,3])
    writeRaster(raster_print,paste0("outputs/",veg_type,"_carbonFAO_",i), format= "ENVI",options="INTERLEAVE=BIL", overwrite = TRUE,bandorder="BIL")  
    
    gc()
    
  }
}





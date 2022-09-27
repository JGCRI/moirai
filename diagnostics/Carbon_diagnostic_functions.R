# Created by Kanishka Narayana and Alan Di Vittorio
 
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

library(ggplot2)
library(dplyr)
library(raster)
library(data.table)
library(rgdal)
library(reldist)
library(ggsci)
library(scales)

# this is just a standalone helper function

get_raster_data_basin <- function(x,path_to_carbon_rasters="../indata/",
                                  print_raster_progress_log=TRUE,
                                  path_to_mapping = "./glu_mapping_basin235.csv",
                                  path_to_mapping_name = "../indata/Global235_CLM_5arcmin.csv",
                                  glu_name = "Missouri_River_Basin")
{
  
  carbon_data_raw<-NULL
  carbon_raster_name <- x
  mapping_data_basin <- read.csv(path_to_mapping)
  mapping_data_basin <- rename(mapping_data_basin,basin_id=glu_id)
  mapping_names_basin <- read.csv(path_to_mapping_name)
  
  carbon_data_raw <- raster(paste0(path_to_carbon_rasters,carbon_raster_name))
  carbon_data_raw <- as.data.table(rasterToPoints(carbon_data_raw))
  
  # need to distinguish between basin and aez input
	if(names(mapping_names_basin)[1] == "AEZ_ID") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data_basin, by=c("x","y")) %>% 
    		inner_join(mapping_names_basin %>% rename(basin_id= AEZ_ID,basin_nm= AEZ_NAME),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean 
	} else if (names(mapping_names_basin)[1] == "GCAM_ID_1") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data_basin, by=c("x","y")) %>% 
    		inner_join(mapping_names_basin %>% rename(basin_id= GCAM_ID_1,basin_nm= Basin_na_1),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean
	} else {
		stop(paste("Unrecognized set of GLU names in file:", gcam_glu_data_file))
	}
  
  if(print_raster_progress_log==TRUE){
    print(paste0("Done processing ", carbon_raster_name))
  }
  
  return(carbon_data_clean)
}



compare_carbon_distribution_ESA<-function(
  
  carbon_type = "above ground biomass",
  path_to_glu_data = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin.tif",
  path_to_carbon_outputs = "C:/Projects/moirai/moirai/example_outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",
  path_to_land_outputs = "C:/Projects/moirai/moirai/example_outputs/basins235/Land_type_area_ha.csv",
  path_to_lt_mapping = "C:/Projects/moirai/moirai/example_outputs/basins235/MOIRAI_land_types.csv",
  path_to_basin_mapping = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/basin_to_country_mapping.csv",
  path_to_carbon_rasters = "C:/Projects/moirai/moirai/indata/",
  path_to_ESA_rasters = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/Carbon_rasters",
  path_to_moirai_ESA = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/ESA_moirai_classes.csv",
  basin_for_testing = "Amazon",
  year_of_reference = 2016,
  moirai_LC = "TropicalEvergreenForest/Woodland",
  path_to_moirai_ref_veg_thematic = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/refveg_carbon_thematic.bil",
  path_to_sage_mapping = "C:/Projects/moirai/moirai/indata/SAGE_PVLT.csv",
  harmonized_carbon_raster_file_names = c("AG_carbon_q1.envi",
                                          "AG_carbon_q3.envi",
                                          "AG_carbon_median.envi",
                                          "AG_carbon_min.envi",
                                          "AG_carbon_max.envi",
                                          "AG_carbon_weighted_average.envi"),
  path_to_harmonized_rasters = "C:/Projects/moirai/moirai/indata/", 
  plot_lim = 25000,
  produce_ESA_distribution=FALSE
){
  #Step 1: Get the basin name
  basin_id <- read.csv(path_to_basin_mapping, skip = 7, stringsAsFactors = FALSE) %>% 
    filter(grepl(basin_for_testing, Basin_name))
  
  basin_id <- unique(basin_id$GCAM_basin_ID)
  
  managed_types <- c("Cropland","Urban","Pasture")
  #Step 1 : Get the carbon csv data
  LT_Mapping <- read.csv(path_to_lt_mapping, skip=4,stringsAsFactors = FALSE) %>%
    mutate(LT_SAGE = if_else(LT_HYDE %in% managed_types,LT_HYDE,LT_SAGE)) %>%
    dplyr::select(Category, LT_SAGE) %>% 
    rename(land_type = Category)
  Land_area <- read.csv(path_to_land_outputs, skip=5) %>% 
    filter(year == year_of_reference) %>% 
    dplyr::select(-year)
  
  Carbon_data_csv <- read.csv(path_to_carbon_outputs,stringsAsFactors = FALSE, skip = 5) %>% 
    filter(grepl(carbon_type, c_type)) %>%
    inner_join(Land_area, by = c("iso","glu_code","land_type")) %>% 
    filter(value != 0) %>% 
    mutate(value = value/1000) %>% 
    filter(glu_code == basin_id) %>% 
    left_join(LT_Mapping , by= c("land_type")) %>% 
    group_by(glu_code, LT_SAGE) %>% 
    mutate(min_value = min(min_value),
           max_value = max(max_value),
           q1_value = wtd.quantile(q1_value, q=0.25,weight = value),
           q3_value = wtd.quantile(q3_value, q=0.75,weight = value),
           median_value = wtd.quantile(median_value, q=0.5,weight = value),
           weighted_average = sum(weighted_average*value)/sum(value)) %>% 
    ungroup() %>% 
    filter(LT_SAGE %in% c(moirai_LC)) %>%
    dplyr::select(-value) %>% 
    gather("state","value", "weighted_average":"q3_value")
  
  #Step 2: Compile the ESA 5 arcmin data
  carbon_type_plot <- carbon_type
  
  if(carbon_type == "above ground biomass"){
    carbon_type <- "AG"}
  
  if(carbon_type == "below ground biomass"){
    carbon_type <- "BG"}
  
  if(carbon_type == "soil"){
    
    scaler = 10
    
  }else{
    
    scaler = 0.1
  }
  
  ESA_map <- read.csv(path_to_moirai_ESA, stringsAsFactors = FALSE) %>% 
    filter(sage_hyde32_name %in% c(moirai_LC))  
  
  ESA_vegetation_type <- unique(ESA_map$Primary)           
  
  if(!dir.exists(path_to_ESA_rasters)){
    
    unzip(paste0(path_to_ESA_rasters,".zip"))
    stop("Unzipping zip files. Please make sure these are placed within the carbon_harmonization folder in the ancillary folder. After this, relaunch")
  }
  
  get_carbon_data<- function(name){
    print(name)
    path <- paste0(path_to_ESA_rasters,"/",carbon_type,"/",name)  
    tmp <- as.data.frame(rasterToPoints(raster(path)))
    names(tmp)[3] <- "value"
    
    return(tmp)
  }
  
  
  GLU_Data <- as.data.frame(rasterToPoints(raster(path_to_glu_data))) %>% 
    filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin == basin_id)
  
  
  
  list.files(path= paste0(path_to_ESA_rasters,"/",carbon_type),pattern=paste0(ESA_vegetation_type,"_",carbon_type,'.*.bil'), recursive=TRUE)->l
  
  
  data_list <- lapply(l, get_carbon_data)
  
  data_list_bind <- rbindlist(data_list) %>% mutate(value=value*scaler)
  
  data_list_bind %>% inner_join(GLU_Data) ->t
  
  #Step 3: Get the harmonized carbon layer for moirai
  
  get_carbon_data_harmonized<- function(name){
    path <- paste0(path_to_harmonized_rasters,name)  
    tmp <- as.data.frame(rasterToPoints(raster(path)))
    names(tmp)[3] <- "value"
    
    return(tmp)
  }
  
  data_list_harmonized <- lapply(harmonized_carbon_raster_file_names, get_carbon_data_harmonized)
  data_harmonized <- rbindlist(data_list_harmonized)
  
  ref_veg_thematic <- as.data.frame(rasterToPoints(raster(path_to_moirai_ref_veg_thematic)))
  
  SAGE_mapping <- read.csv(path_to_sage_mapping, skip=3, stringsAsFactors = FALSE) %>% 
    mutate(LT_SAGE_NAME = gsub(" ","",LT_SAGE_NAME)) %>% 
    filter(LT_SAGE_NAME %in% moirai_LC)
  
  
  data_harmonized %>% 
    inner_join(ref_veg_thematic %>% 
                 filter(refveg_carbon_thematic %in% 
                          c(unique(SAGE_mapping$LT_SAGE_CODE))),by = c("x","y")) %>% 
    inner_join(GLU_Data, by = c("x","y")) %>% 
    filter(value != -9999) ->harmonized_data_filtered
  
  if(carbon_type != "soil"){
    
    harmonized_data_filtered %>% 
      mutate(value = value *scaler)->harmonized_data_filtered
    
  }
  
  cv <- round(sd(harmonized_data_filtered$value)/mean(harmonized_data_filtered$value),2)
  
  if(produce_ESA_distribution){
  g <- ggplot(data=t)+
    geom_vline(data=Carbon_data_csv %>% filter(state=="q3_value"), aes(xintercept=value, color=state), size = 1.2,linetype="dashed")+
    geom_text(aes(x=max(Carbon_data_csv$value),y=plot_lim),label=paste0("CV = ",cv))+
    geom_histogram(data= t, aes(x=t$value,fill="Distribution from ESA"),bins =100,alpha=0.1,color="black")+
    geom_histogram(data= harmonized_data_filtered, aes(x=harmonized_data_filtered$value,fill="Distribution from moirai"),bins =100,alpha=0.4,color="black")+
    ggtitle(paste0(moirai_LC," ",carbon_type_plot," carbon in Mgc/ha"))+
    xlab("MgC/ha")+scale_y_continuous(labels = unit_format(unit = "M", scale = 1e-6))+
    labs(subtitle = paste0("Basin name - ",basin_for_testing),
         caption = "All distributions are at a 5 arcmin resolution")+
    ylab("count of cells")+
    geom_point(data=Carbon_data_csv, aes(x=value,y=0, color=state),size=2)+
    scale_color_npg()
  }else{
    g <- ggplot(data=harmonized_data_filtered)+
      geom_vline(data=Carbon_data_csv %>% filter(state=="q3_value"), aes(xintercept=value, color=state), size = 1.2,linetype="dashed")+
      geom_text(aes(x=max(Carbon_data_csv$value)*1.2,y=6000),label=paste0("CV = ",cv))+
      geom_histogram(data= t, aes(x=t$value,fill="Distribution from ESA"),bins =100,alpha=0.1,color="white",fill="transparent")+
      geom_histogram(data= harmonized_data_filtered, aes(x=harmonized_data_filtered$value,fill="Distribution from moirai"),alpha=0.5,bins =100,color="black")+
      ggtitle(paste0(moirai_LC," ",carbon_type_plot," carbon in Mgc/ha"))+
      xlab("MgC/ha")+scale_y_continuous(labels = unit_format(unit = "M", scale = 1e-6))+
      labs(subtitle = paste0("Basin name - ",basin_for_testing),
           caption = "All distributions are at a 5 arcmin resolution")+
      ylab("count of cells")+
      geom_point(data=Carbon_data_csv, aes(x=value,y=0, color=state),size=2)+
      scale_color_npg()+
      scale_fill_manual(values=c("light blue"))
    
    
  }
  
  
  
  
  print("Completed current LT")
  return(g)
  
}




compare_carbon_distribution_HYDE<-function(
  
  carbon_type = "above ground biomass",
  path_to_glu_data = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin.tif",
  path_to_carbon_outputs = "C:/Projects/moirai/moirai/example_outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",
  path_to_land_outputs = "C:/Projects/moirai/moirai/example_outputs/basins235/Land_type_area_ha.csv",
  path_to_lt_mapping = "C:/Projects/moirai/moirai/example_outputs/basins235/MOIRAI_land_types.csv",
  path_to_basin_mapping = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/basin_to_country_mapping.csv",
  path_to_carbon_rasters = "C:/Projects/moirai/moirai/indata/",
  path_to_ESA_rasters = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/Carbon_rasters",
  path_to_moirai_ESA = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/ESA_moirai_classes.csv",
  basin_for_testing = "Amazon",
  year_of_reference = 2016,
  moirai_LC = "Cropland",
  path_to_moirai_ref_veg_thematic = "C:/Projects/moirai/moirai/ancillary/carbon_harmonization/input_files/refveg_carbon_thematic.bil",
  path_to_sage_mapping = "C:/Projects/moirai/moirai/indata/SAGE_PVLT.csv",
  harmonized_carbon_raster_file_names = c("AG_carbon_crop_q1.envi",
                                          "AG_carbon_crop_q3.envi",
                                          "AG_carbon_crop_median.envi",
                                          "AG_carbon_crop_min.envi",
                                          "AG_carbon_crop_max.envi",
                                          "AG_carbon_crop_average.envi"),
  path_to_harmonized_rasters = "C:/Projects/moirai/moirai/indata/", 
  plot_lim = 25000,
  produce_ESA_distribution=FALSE
){
  #Step 1: Get the basin name
  basin_id <- read.csv(path_to_basin_mapping, skip = 7, stringsAsFactors = FALSE) %>% 
    filter(grepl(basin_for_testing, Basin_name))
  
  basin_id <- unique(basin_id$GCAM_basin_ID)
  
  managed_types <- c("Cropland","Urban","Pasture")
  
  #Step 1 : Get the carbon csv data
  LT_Mapping <- read.csv(path_to_lt_mapping, skip=4,stringsAsFactors = FALSE) %>%
    mutate(LT_SAGE = if_else(LT_HYDE %in% managed_types,LT_HYDE,LT_SAGE)) %>% 
    dplyr::select(Category, LT_SAGE) %>% 
    rename(land_type = Category)
  
  Land_area <- read.csv(path_to_land_outputs, skip=5) %>% 
    filter(year == year_of_reference) %>% 
    dplyr::select(-year)
  
  
  Carbon_data_csv <- read.csv(path_to_carbon_outputs,stringsAsFactors = FALSE, skip = 5) %>% 
    filter(grepl(carbon_type, c_type)) %>%
    inner_join(Land_area, by = c("iso","glu_code","land_type")) %>% 
    filter(value != 0) %>% 
    mutate(value = value/1000) %>% 
    filter(glu_code == basin_id) %>% 
    left_join(LT_Mapping , by= c("land_type")) %>% 
    group_by(glu_code, LT_SAGE) %>% 
    mutate(min_value = min(min_value),
           max_value = max(max_value),
           q1_value = wtd.quantile(q1_value, q=0.25,weight = value),
           q3_value = wtd.quantile(q3_value, q=0.75,weight = value),
           median_value = wtd.quantile(median_value, q=0.5,weight = value),
           weighted_average = sum(weighted_average*value)/sum(value)) %>% 
    ungroup() %>% 
    filter(LT_SAGE %in% c(moirai_LC)) %>%
    dplyr::select(-value) %>% 
    gather("state","value", "weighted_average":"q3_value")
  
  #Step 2: Compile the ESA 5 arcmin data
  carbon_type_plot <- carbon_type
  
  if(carbon_type == "above ground biomass"){
    carbon_type <- "AG"}
  
  if(carbon_type == " below ground biomass"){
    carbon_type <- "BG"}
  
  if(carbon_type == "soil"){
    
    scaler = 10
    
  }else{
    
    scaler = 0.1
  }
  
  
  
  get_carbon_data<- function(name){
    path <- paste0(path_to_ESA_rasters,"/",carbon_type,"/",name)  
    tmp <- as.data.frame(rasterToPoints(raster(path)))
    names(tmp)[3] <- "value"
    
    return(tmp)
  }
  
  
  GLU_Data <- as.data.frame(rasterToPoints(raster(path_to_glu_data))) %>% 
    filter(gcam_glu_boundaries_moirai_land_cells_3p1_0p5arcmin == basin_id)
  
  
  
  
  
  
  
  #Step 3: Get the harmonized carbon layer for moirai
  
  get_carbon_data_harmonized<- function(name){
    path <- paste0(path_to_harmonized_rasters,name)  
    tmp <- as.data.frame(rasterToPoints(raster(path)))
    names(tmp)[3] <- "value"
    
    return(tmp)
  }
  
  data_list_harmonized <- lapply(harmonized_carbon_raster_file_names, get_carbon_data_harmonized)
  data_harmonized <- rbindlist(data_list_harmonized)
  
  
  
  
  
  
  data_harmonized %>% 
    inner_join(GLU_Data, by = c("x","y")) %>% 
    filter(value != -9999) ->harmonized_data_filtered
  
  if(carbon_type != "soil"){
    
    harmonized_data_filtered %>% 
      mutate(value = value *scaler)->harmonized_data_filtered
    
  }
  
  cv <- round(sd(harmonized_data_filtered$value)/mean(harmonized_data_filtered$value),2)
  
   
    g <- ggplot(data=harmonized_data_filtered)+
      geom_vline(data=Carbon_data_csv %>% filter(state=="q3_value"), aes(xintercept=value, color=state), size = 1.2,linetype="dashed")+
      geom_text(aes(x=max(Carbon_data_csv$value)*1.2,y=6000),label=paste0("CV = ",cv))+
      geom_histogram(data= harmonized_data_filtered, aes(x=harmonized_data_filtered$value,fill="Distribution from moirai"),alpha=0.5,bins =100,color="black")+
      ggtitle(paste0(moirai_LC," ",carbon_type_plot," carbon in Mgc/ha"))+
      xlab("MgC/ha")+scale_y_continuous(labels = unit_format(unit = "M", scale = 1e-6))+
      labs(subtitle = paste0("Basin name - ",basin_for_testing),
           caption = "All distributions are at a 5 arcmin resolution")+
      ylab("count of cells")+
      geom_point(data=Carbon_data_csv, aes(x=value,y=0, color=state),size=2)+
      scale_color_npg()+
      scale_fill_manual(values=c("light blue"))
    
    
  
  
  
  
  
  print("Completed current LT")
  return(g)
  
}

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

create_carbon_plots_soil<-function(fig_dir  = "carbon_plots/",
                              plot_lim = 1000, 
                              path_to_carbon_data = "../example_outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",
                              path_to_carbon_rasters = "../indata/",
                              path_to_mapping = "./glu_mapping_basin235.csv",
                              path_to_mapping_name = "../indata/Global235_CLM_5arcmin.csv",
                              path_to_iso_mapping = "../indata/FAO_iso_VMAP0_ctry.csv",
                              path_to_land_data = "../example_outputs/basins235/Land_type_area_ha.csv",
                              glu_name = "Missouri_River_Basin",
                              carbon_year = 2010,
                              carbon_input_src_name = "soil_grids_95pct",
                              print_raster_progress_log =FALSE)
{
	
  cat("started create_carbon_plots_soil() at ",date(), "\n")	
	
  if(substr(fig_dir,nchar(fig_dir), nchar(fig_dir)) != "/") { fig_dir = paste0(fig_dir, "/") }	
  dir.create(fig_dir, showWarnings = FALSE)
	
  #Declare scheme for plots
  scheme_basic <- theme_bw() +
  theme(legend.text = element_text(size = 10, vjust = 0.5)) +
  theme(legend.title = element_text(size = 10, vjust = 2)) +
  theme(axis.text = element_text(size = 10)) +
  theme(axis.title = element_text(size = 10, face = "bold")) +
  theme(plot.title = element_text(size = 10, face = "bold", vjust = 1)) +
  theme(plot.subtitle = element_text(size = 8, face = "bold", vjust = 1))+
  theme(strip.text = element_text(size = 7))+
  theme(strip.text.x = element_text(size = 8, face = "bold"))+
  #theme(legend.position = "bottom")+
  theme(legend.text = element_text(size = 10))+
  theme(legend.title = element_text(size = 10,color = "black",face="bold"))+
  theme(axis.text.x= element_text(angle=60,hjust=1))+
  theme(legend.background = element_blank(),
        legend.box.background = element_rect(colour = "black"))

  if (carbon_input_src_name == "FAO_HWS"){
  carbon_raster_file_names = c("FAO_HWS_q1.bil",
                               "FAO_HWS_q3.bil",
                               "FAO_HWS_median.bil",
                               "FAO_HWS_min.bil",
                               "FAO_HWS_max.bil",
                               "FAO_HWS_wavg.bil")}
  
  if (carbon_input_src_name == "soil_grids_mean"){
    carbon_raster_file_names = c("soil_carbon_q1.bil",
                                 "soil_carbon_q3.bil",
                                 "soil_carbon_median.bil",
                                 "soil_carbon_min.bil",
                                 "soil_carbon_max.bil",
                                 "soil_carbon_wavg.bil")}
  
  if (carbon_input_src_name == "soil_grids_95pct"){
    carbon_raster_file_names = c("soil_carbon_q1_95pct.bil",
                                 "soil_carbon_q3_95pct.bil",
                                 "soil_carbon_median_95pct.bil",
                                 "soil_carbon_min_95pct.bil",
                                 "soil_carbon_max_95pct.bil",
                                 "soil_carbon_weighted_average_95pct.bil")}



#Read in data
mapping_data <- read.csv(path_to_mapping,stringsAsFactors = FALSE) %>% rename(basin_id=glu_id)
mapping_names <- read.csv(path_to_mapping_name)
Carbon_data <- read.csv(path_to_carbon_data,skip=5,stringsAsFactors = FALSE)
Land_data <- read.csv(path_to_land_data,skip=5,stringsAsFactors = FALSE)

iso_data <- read.csv(path_to_iso_mapping,stringsAsFactors = FALSE) %>% 
  rename(ctry_id=fao_code,iso=iso3_abbr)

#Helper function for reading carbon data for specific basins
get_carbon_data_basin <- function(x){
  gc()
  carbon_data_raw<-NULL
  carbon_raster_name <- x
  
  carbon_data_raw <- raster(paste0(path_to_carbon_rasters,carbon_raster_name))
  carbon_data_raw <- as.data.table(rasterToPoints(carbon_data_raw))
  
  # need to distinguish between basin and aez input
	if(names(mapping_names)[1] == "AEZ_ID") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= AEZ_ID,basin_nm= AEZ_NAME),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean 
	} else if (names(mapping_names)[1] == "GCAM_ID_1") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= GCAM_ID_1,basin_nm= Basin_na_1),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean
	} else {
		stop(paste("Unrecognized set of GLU names in file:", gcam_glu_data_file))
	}      
  
  if(print_raster_progress_log==TRUE){
    print(paste0("Done processing ", carbon_raster_name))
    }
  
  return(carbon_data_clean)
  
} # end define get_carbon_data_basin

#Get carbon data for all combinations here using the function and bind together.
consolidated_carbon_data <- rbindlist(lapply(carbon_raster_file_names,get_carbon_data_basin))

#Get the soil carbon data and clean the same.
Carbon_data %>% 
  filter(c_type=="soil_c (0-30 cms)") %>%
  rename(wavg=weighted_average) %>% 
  distinct() %>% 
  inner_join(iso_data,by=c("iso")) %>% 
  filter(glu_code %in% unique(consolidated_carbon_data$basin_id)) %>% 
  inner_join(Land_data %>% rename(land_area=value) %>% filter(year==carbon_year),by=c("iso","glu_code","land_type"))->soil_carbon_data

#Aggregate carbon data across land types.  
soil_carbon_data %>% 
  dplyr::select(glu_code,land_type,wavg,median_value,min_value,max_value,q1_value,q3_value,land_area) %>% 
  group_by(glu_code) %>% 
  mutate(wavg=sum(as.numeric(wavg)*as.numeric(land_area))/sum(as.numeric(land_area)),
         median_value=median(median_value),
         min_value=min(min_value),
         max_value=max(max_value),
         q1_value=quantile(q1_value,0.25),
         q3_value=quantile(q3_value,0.75)) %>% 
  ungroup() %>% 
  dplyr::select(glu_code,wavg,median_value,min_value,max_value,q1_value,q3_value) %>% 
  distinct()->soil_carbon_agg

#Generate data for the plot
consolidated_carbon_data %>% left_join(soil_carbon_agg %>% rename(basin_id=glu_code), by=c("basin_id"))->data_for_plot

#Plot distributions
g<-ggplot(data=data_for_plot,aes(x=value)) +
  geom_histogram(color="black",fill="white",bins=35)+
  geom_point(data=data_for_plot,aes(x=median_value,y=plot_lim,color="median_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=min_value,y=plot_lim,color="min_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=max_value,y=plot_lim,color="max_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=q1_value,y=plot_lim,color="q1_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=q3_value,y=plot_lim,color="q3_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=wavg,y=plot_lim,color="weighted_average"),show.legend = TRUE,size=3,alpha=0.9)+
  ggtitle(paste0("Soil carbon distribution for ",toString(glu_name), " in MgC/ha (0-30 cm depth), distribution from ", toString(carbon_input_src_name)))+
  labs(subtitle = "Colors represent initialization values from moirai outputs for each category")

#Save distributions
ggsave( paste0(fig_dir, "Distribution_soil_c_", toString(glu_name), ".png"), width = 10, height = 6)

print("Completed processing succesfully. Plots stored in the outputs folder")

cat("finished create_carbon_plots_soil() at ",date(), "\n")

return(g)
}

create_carbon_plots_veg_carbon_above_ground<-function(fig_dir  = "carbon_plots/",
                                   plot_lim = 1000, 
                                   path_to_carbon_data = "../example_outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",
                                   path_to_carbon_rasters = "../indata/",
                                   path_to_mapping = "./glu_mapping_basin235.csv",
                                   path_to_mapping_name = "../indata/Global235_CLM_5arcmin.csv",
                                   path_to_iso_mapping = "../indata/FAO_iso_VMAP0_ctry.csv",
                                   path_to_land_data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                   glu_name = "Missouri_River_Basin",
                                   carbon_year = 2010,
                                   carbon_raster_file_names = c("veg_carbon_max.bil",
                                                                "veg_carbon_q3.bil",
                                                                "veg_carbon_median.bil",
                                                                "veg_carbon_min.bil",
                                                                "veg_carbon_q1.bil",
                                                                "veg_carbon_wavg.bil"),
                                   print_raster_progress_log =FALSE)                              
{
	
  cat("started create_carbon_plots_veg_carbon_above_ground() at ",date(), "\n")
	
  if(substr(fig_dir,nchar(fig_dir), nchar(fig_dir)) != "/") { fig_dir = paste0(fig_dir, "/") }
  dir.create(fig_dir, showWarnings = FALSE)
	
  scheme_basic <- theme_bw() +
    theme(legend.text = element_text(size = 10, vjust = 0.5)) +
    theme(legend.title = element_text(size = 10, vjust = 2)) +
    theme(axis.text = element_text(size = 10)) +
    theme(axis.title = element_text(size = 10, face = "bold")) +
    theme(plot.title = element_text(size = 10, face = "bold", vjust = 1)) +
    theme(plot.subtitle = element_text(size = 8, face = "bold", vjust = 1))+
    theme(strip.text = element_text(size = 7))+
    theme(strip.text.x = element_text(size = 8, face = "bold"))+
    #theme(legend.position = "bottom")+
    theme(legend.text = element_text(size = 10))+
    theme(legend.title = element_text(size = 10,color = "black",face="bold"))+
    theme(axis.text.x= element_text(angle=60,hjust=1))+
    theme(legend.background = element_blank(),
          legend.box.background = element_rect(colour = "black"))

  mapping_data <- read.csv(path_to_mapping,stringsAsFactors = FALSE) %>% rename(basin_id=glu_id)
  mapping_names <- read.csv(path_to_mapping_name)
  Carbon_data <- read.csv(path_to_carbon_data,skip=5,stringsAsFactors = FALSE)
  Land_data <- read.csv(path_to_land_data,skip=5,stringsAsFactors = FALSE)
  
  iso_data <- read.csv(path_to_iso_mapping,stringsAsFactors = FALSE) %>% 
    rename(ctry_id=fao_code,iso=iso3_abbr)
  
  get_carbon_data_basin <- function(x){
    gc()
    carbon_data_raw<-NULL
    carbon_raster_name <- x
    
    carbon_data_raw <- raster(paste0(path_to_carbon_rasters,carbon_raster_name))
    carbon_data_raw <- as.data.table(rasterToPoints(carbon_data_raw))
    
    # need to distinguish between basin and aez input
	if(names(mapping_names)[1] == "AEZ_ID") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= AEZ_ID,basin_nm= AEZ_NAME),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean 
	} else if (names(mapping_names)[1] == "GCAM_ID_1") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= GCAM_ID_1,basin_nm= Basin_na_1),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean
	} else {
		stop(paste("Unrecognized set of GLU names in file:", gcam_glu_data_file))
	}      
    
    if(print_raster_progress_log==TRUE){
      print(paste0("Done processing ", carbon_raster_name))
    }
    
    return(carbon_data_clean)
    
  } # end define get_carbon_data_basin
  
  consolidated_carbon_data <- rbindlist(lapply(carbon_raster_file_names,get_carbon_data_basin))
  
  Carbon_data %>% 
    filter(c_type=="veg_c (above ground biomass)") %>%
    rename(wavg=weighted_average) %>% 
    distinct() %>% 
    inner_join(iso_data,by=c("iso")) %>% 
    filter(glu_code %in% unique(consolidated_carbon_data$basin_id)) %>% 
    inner_join(Land_data %>% rename(land_area=value) %>% filter(year==carbon_year),by=c("iso","glu_code","land_type"))->vag_carbon_data
  
  vag_carbon_data %>% 
    dplyr::select(glu_code,land_type,wavg,median_value,min_value,max_value,q1_value,q3_value,land_area) %>% 
    group_by(glu_code) %>% 
    mutate(wavg=sum(as.numeric(wavg)*as.numeric(land_area))/sum(as.numeric(land_area)),
           median_value=median(median_value),
           min_value=min(min_value),
           max_value=max(max_value),
           q1_value=quantile(q1_value,0.25),
           q3_value=quantile(q3_value,0.75)) %>% 
    ungroup() %>% 
    dplyr::select(glu_code,wavg,median_value,min_value,max_value,q1_value,q3_value) %>% 
    distinct()->vag_carbon_agg
  
  
  consolidated_carbon_data %>%mutate(value=value*0.1) %>%  left_join(vag_carbon_agg %>% rename(basin_id=glu_code), by=c("basin_id"))->data_for_plot
  
  g<-ggplot(data=data_for_plot,aes(x=value)) +
    geom_histogram(color="black",fill="white",bins=35)+
    geom_point(data=data_for_plot,aes(x=median_value,y=plot_lim,color="median_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=min_value,y=plot_lim,color="min_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=max_value,y=plot_lim,color="max_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=q1_value,y=plot_lim,color="q1_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=q3_value,y=plot_lim,color="q3_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=wavg,y=plot_lim,color="weighted_average"),show.legend = TRUE,size=3,alpha=0.9)+
    ggtitle(paste0("Above ground biomass distribution for ",toString(glu_name), " in MgC/ha"))+
    labs(subtitle = "Colors represent initialization values for above ground biomass from moirai outputs for each category")
  
  plot(g)
  ggsave( paste0(fig_dir, "Distribution_veg_c_AG_", toString(glu_name), ".png"), width = 10, height = 6)
  
  print("Completed processing succesfully. Plots stored in the outputs folder")
  cat("finished create_carbon_plots_veg_carbon_above_ground() at ",date(), "\n")
  
  return(g)
}


create_carbon_plots_veg_carbon_below_ground<-function(fig_dir  = "carbon_plots/",
                                                      plot_lim = 1000, 
                                                      path_to_carbon_data = "../example_outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",
                                                      path_to_carbon_rasters = "../indata/",
                                                      path_to_mapping = "./glu_mapping_basin235.csv",
                                                      path_to_mapping_name = "../indata/Global235_CLM_5arcmin.csv",
                                                      path_to_iso_mapping = "../indata/FAO_iso_VMAP0_ctry.csv",
                                                      path_to_land_data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                                      glu_name = "Missouri_River_Basin",
                                                      carbon_year = 2010,
                                                      carbon_raster_file_names = c("veg_BG_carbon_weighted_average.bil",
                                                                                   "veg_BG_carbon_q1.bil",
                                                                                   "veg_BG_carbon_median.bil",
                                                                                   "veg_BG_carbon_min.bil",
                                                                                   "veg_BG_carbon_q3.bil",
                                                                                   "veg_BG_carbon_max.bil"),
                                                      print_raster_progress_log =FALSE)                                                 
{
	
  cat("started create_carbon_plots_veg_carbon_below_ground() at ",date(), "\n")
	
  if(substr(fig_dir,nchar(fig_dir), nchar(fig_dir)) != "/") { fig_dir = paste0(fig_dir, "/") }
  dir.create(fig_dir, showWarnings = FALSE)
	
  scheme_basic <- theme_bw() +
    theme(legend.text = element_text(size = 10, vjust = 0.5)) +
    theme(legend.title = element_text(size = 10, vjust = 2)) +
    theme(axis.text = element_text(size = 10)) +
    theme(axis.title = element_text(size = 10, face = "bold")) +
    theme(plot.title = element_text(size = 10, face = "bold", vjust = 1)) +
    theme(plot.subtitle = element_text(size = 8, face = "bold", vjust = 1))+
    theme(strip.text = element_text(size = 7))+
    theme(strip.text.x = element_text(size = 8, face = "bold"))+
    #theme(legend.position = "bottom")+
    theme(legend.text = element_text(size = 10))+
    theme(legend.title = element_text(size = 10,color = "black",face="bold"))+
    theme(axis.text.x= element_text(angle=60,hjust=1))+
    theme(legend.background = element_blank(),
          legend.box.background = element_rect(colour = "black"))
  
  mapping_data <- read.csv(path_to_mapping,stringsAsFactors = FALSE) %>% rename(basin_id=glu_id)
  mapping_names <- read.csv(path_to_mapping_name)
  Carbon_data <- read.csv(path_to_carbon_data,skip=5,stringsAsFactors = FALSE)
  Land_data <- read.csv(path_to_land_data,skip=5,stringsAsFactors = FALSE)
  
  iso_data <- read.csv(path_to_iso_mapping,stringsAsFactors = FALSE) %>% 
    rename(ctry_id=fao_code,iso=iso3_abbr)
  
  get_carbon_data_basin <- function(x){
    gc()
    carbon_data_raw<-NULL
    carbon_raster_name <- x
    
    carbon_data_raw <- raster(paste0(path_to_carbon_rasters,carbon_raster_name))
    carbon_data_raw <- as.data.table(rasterToPoints(carbon_data_raw))
    
    # need to distinguish between basin and aez input
	if(names(mapping_names)[1] == "AEZ_ID") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= AEZ_ID,basin_nm= AEZ_NAME),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean 
	} else if (names(mapping_names)[1] == "GCAM_ID_1") {
		carbon_data_raw %>% rename(value=gsub(".bil","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= GCAM_ID_1,basin_nm= Basin_na_1),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean
	} else {
		stop(paste("Unrecognized set of GLU names in file:", gcam_glu_data_file))
	}     
    
    if(print_raster_progress_log==TRUE){
      print(paste0("Done processing ", carbon_raster_name))
    }
    
    return(carbon_data_clean)
    
  } # end define get_carbon_data_basin
  
  consolidated_carbon_data <- rbindlist(lapply(carbon_raster_file_names,get_carbon_data_basin))
  
  Carbon_data %>% 
    filter(c_type=="veg_c (below ground biomass)") %>%
    rename(wavg=weighted_average) %>% 
    distinct() %>% 
    inner_join(iso_data,by=c("iso")) %>% 
    filter(glu_code %in% unique(consolidated_carbon_data$basin_id)) %>% 
    inner_join(Land_data %>% rename(land_area=value) %>% filter(year==carbon_year),by=c("iso","glu_code","land_type"))->vbg_carbon_data
  
  vbg_carbon_data %>% 
    dplyr::select(glu_code,land_type,wavg,median_value,min_value,max_value,q1_value,q3_value,land_area) %>% 
    group_by(glu_code) %>% 
    mutate(wavg=sum(as.numeric(wavg)*as.numeric(land_area))/sum(as.numeric(land_area)),
           median_value=median(median_value),
           min_value=min(min_value),
           max_value=max(max_value),
           q1_value=quantile(q1_value,0.25),
           q3_value=quantile(q3_value,0.75)) %>% 
    ungroup() %>% 
    dplyr::select(glu_code,wavg,median_value,min_value,max_value,q1_value,q3_value) %>% 
    distinct()->vbg_carbon_agg
  
  
  consolidated_carbon_data %>%mutate(value=value*0.1) %>%  left_join(vbg_carbon_agg %>% rename(basin_id=glu_code), by=c("basin_id"))->data_for_plot
  
  g<-ggplot(data=data_for_plot,aes(x=value)) +
    geom_histogram(color="black",fill="white",bins=35)+
    geom_point(data=data_for_plot,aes(x=median_value,y=plot_lim,color="median_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=min_value,y=plot_lim,color="min_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=max_value,y=plot_lim,color="max_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=q1_value,y=plot_lim,color="q1_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=q3_value,y=plot_lim,color="q3_value"),show.legend = TRUE,size=3,alpha=0.9)+
    geom_point(data=data_for_plot,aes(x=wavg,y=plot_lim,color="weighted_average"),show.legend = TRUE,size=3,alpha=0.9)+
    ggtitle(paste0("Below ground biomass distribution for ",toString(glu_name), " in MgC/ha"))+
    labs(subtitle = "Colors represent initialization values for below ground biomass from moirai outputs for each category")
  
  plot(g)
  ggsave( paste0(fig_dir, "Distribution_veg_c_BG_", toString(glu_name), ".png"), width = 10, height = 6)
  
  print("Completed processing succesfully. Plots stored in the outputs folder")
  
  cat("finished create_carbon_plots_veg_carbon_below_ground() at ",date(), "\n")
  
  return(g)
}




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


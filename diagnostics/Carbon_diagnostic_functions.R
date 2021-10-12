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

create_carbon_plots_soil<-function(fig_dir  = "carbon_plots/",
                              plot_lim = 1000, 
                              path_to_carbon_data = "../example_outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",
                              path_to_carbon_rasters = "../indata/",
                              path_to_mapping = "./glu_mapping_basin235.csv",
                              path_to_mapping_name = "../indata/Global235_CLM_5arcmin.csv",
                              path_to_iso_mapping = "../indata/FAO_iso_VMAP0_ctry.csv",
                              path_to_land_data = "../example_outputs/basins235/Land_type_area_ha.csv",
                              path_to_LT_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                              glu_name = "Missouri_River_Basin",
                              carbon_year = 2010,
                              carbon_input_src_name = "soil_grids",
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
  
  if (carbon_input_src_name == "soil_grids"){
    carbon_raster_file_names = c("soil_carbon_q1.envi",
                                 "soil_carbon_q3.envi",
                                 "soil_carbon_median.envi",
                                 "soil_carbon_min.envi",
                                 "soil_carbon_max.envi",
                                 "soil_carbon_weighted_average.envi")}



#Read in data
mapping_data <- read.csv(path_to_mapping,stringsAsFactors = FALSE) %>% rename(basin_id=glu_id)
mapping_names <- read.csv(path_to_mapping_name)
Carbon_data <- read.csv(path_to_carbon_data,skip=5,stringsAsFactors = FALSE)
Land_data <- read.csv(path_to_land_data,skip=5,stringsAsFactors = FALSE)
LT_mapping <- read.csv(path_to_LT_mapping, skip=4, stringsAsFactors = FALSE) %>% select()

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
		carbon_data_raw %>% rename(value=gsub(".envi","",carbon_raster_name,)) %>%
    		filter(value>0) %>% 
    		inner_join(mapping_data, by=c("x","y")) %>% 
    		inner_join(mapping_names %>% rename(basin_id= AEZ_ID,basin_nm= AEZ_NAME),by=c("basin_id")) %>% 
    		filter(basin_nm==glu_name)->carbon_data_clean 
	} else if (names(mapping_names)[1] == "GCAM_ID_1") {
		carbon_data_raw %>% rename(value=gsub(".envi","",carbon_raster_name,)) %>%
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
  filter(c_type=="soil_c (0-100 cms)") %>%
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
if(print_raster_progress_log == TRUE){
  write.csv(soil_carbon_agg, "soil_carbon_agg.csv")
  
}


#Generate data for the plot
consolidated_carbon_data %>% left_join(soil_carbon_agg %>% rename(basin_id=glu_code), by=c("basin_id"))->data_for_plot
if(print_raster_progress_log == TRUE){
  write.csv(data_for_plot, "data_for_plot.csv")
  
}
#Plot distributions
g<-ggplot(data=data_for_plot,aes(x=value)) +
  geom_histogram(color="black",fill="white",bins=35)+
  geom_point(data=data_for_plot,aes(x=median_value,y=plot_lim,color="median_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=min_value,y=plot_lim,color="min_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=max_value,y=plot_lim,color="max_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=q1_value,y=plot_lim,color="q1_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=q3_value,y=plot_lim,color="q3_value"),show.legend = TRUE,size=3,alpha=0.9)+
  geom_point(data=data_for_plot,aes(x=wavg,y=plot_lim,color="weighted_average"),show.legend = TRUE,size=3,alpha=0.9)+
  ggtitle(paste0("Soil carbon distribution for ",toString(glu_name), " in MgC/ha (0-100 cm depth), distribution from ", toString(carbon_input_src_name)))+
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



compare_carbon_distribution<-function(
  
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
  plot_lim = 25000
){
  #Step 1: Get the basin name
  basin_id <- read.csv(path_to_basin_mapping, skip = 7, stringsAsFactors = FALSE) %>% 
    filter(grepl(basin_for_testing, Basin_name))
  
  basin_id <- unique(basin_id$GCAM_basin_ID)
  
  
  #Step 1 : Get the carbon csv data
  LT_Mapping <- read.csv(path_to_lt_mapping, skip=4,stringsAsFactors = FALSE) %>% 
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
  
  g <- ggplot(data=t)+
    geom_histogram(data= t, aes(x=t$value,fill="Distribution from ESA"),bins =100,alpha=0.1,color="black")+
    geom_histogram(data= harmonized_data_filtered, aes(x=harmonized_data_filtered$value,fill="Distribution from moirai"),bins =100,alpha=0.5,color="black")+
    geom_point(data=Carbon_data_csv, aes(x=value, y =plot_lim, color=state), size = 3)+
    geom_text(aes(x=max(Carbon_data_csv$value), y= plot_lim*1.2),label=paste0("CV = ",cv))+
    ggtitle(paste0(moirai_LC," ",carbon_type_plot," carbon in Mgc/ha"))+
    xlab("MgC/ha")+scale_y_continuous(labels = unit_format(unit = "M", scale = 1e-6))+
    labs(subtitle = paste0("Basin name - ",basin_for_testing),
         caption = "All distributions are at a 5 arcmin resolution")+
    scale_color_npg()
  
  
  
  
  print("Completed current LT")
  return(g)
  
}

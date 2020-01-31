context("Compare raster data to LDS outputs")
library(dplyr)
library(tibble)
library(raster)
library(testthat)
library(rgdal)
library(data.table)

#kbn 2020-01-30 Adding system.time

start.time <- Sys.time()
test_that("Compare raster outputs with LDS outputs at basin level", {
  #Get land data from raster files for each basin intersection (32 regions, 235 basins)
  #Get output data from LDS and compare with the above
  #Allow an error tolerance of 2% to account for rounding. Note that this works fine with an error tolerance of 1%
  
  #1. Get LDS data and iso data
  LDS_data <- read.csv("outputs/Land_type_area_ha.csv",skip = 9, header = T,stringsAsFactors = FALSE)
  ISO <- read.csv('outputs/iso_GCAM_regID.csv',skip = 6, header=T,stringsAsFactors = FALSE)
  LDS_data %>% 
  left_join(ISO %>% dplyr::select(iso,GCAM_region_ID),by="iso") ->LDS_data
  
  #2. Process LDS data
  #2a .Get subtotals in LDS data
  LDS_data_w_totals <- LDS_data %>%
    #Filter for the latest year
    filter(year==2016) %>% 
    #Compute GLU values
    group_by(glu_code) %>% 
    mutate(glu_total_LDS=sum(value)) %>% 
    ungroup() %>% 
    #Compute values for regions
    group_by(GCAM_region_ID) %>% 
    mutate(REG_total_LDS=sum(value)) %>% 
    ungroup()
  
  #2. Get raster data
  #Unzip data files
  unzip(zipfile = "data/ValidBoundaries.zip", 
        exdir = 'data')
  #Read Raster file
  Basin_raster <- raster('data/moirai_valid_region32_water_basin235.bsq')#This is the raster file which maps in basins
  Land_Data_Binary <- raster('data/moirai_valid_land_area.bsq')#This is the raster file with actual land area 
  
  
  
  #Convert raster files to data frames
  Land_Data_Actual <- (as.data.table(rasterToPoints(Land_Data_Binary))) 
  Basin_Data_Raster <- (as.data.table(rasterToPoints(Basin_raster)))
  
  
  #Process raster data
  Joined_Raster_Data <- Basin_Data_Raster %>% 
    #Join in valid land area 
    left_join(Land_Data_Actual,by=c("x","y")) %>% 
    #Drop na's
    na.omit() %>% 
    #Group by intersection
    group_by(moirai_valid_region32_water_basin235) %>% 
    #Aggregate totals
    mutate(Total_Raster_LandArea = sum(moirai_valid_land_area)) %>% 
    ungroup() %>% 
    #un-select irrelevant cols and get unique values 
    dplyr::select(-x,-y,-moirai_valid_land_area) %>% 
    distinct() 
  
  #3. Get LDS data for intersections
  LDS_data_intersections <- LDS_data_w_totals %>% 
    mutate(moirai_valid_region32_water_basin235 = (GCAM_region_ID*10000)+glu_code) %>% 
    group_by(moirai_valid_region32_water_basin235) %>% 
    mutate(Total_Intersection_Area_LDS = sum(value)) %>% 
    ungroup() %>% 
    dplyr::select(moirai_valid_region32_water_basin235 , Total_Intersection_Area_LDS) %>% 
    distinct()
  
  #4. Create Reco_File
  Reco <- Joined_Raster_Data %>%  
    mutate(Total_Raster_LandArea = Total_Raster_LandArea * 100) %>% 
    left_join(LDS_data_intersections, by=c("moirai_valid_region32_water_basin235")) 
    
  
  expect_equal(Reco$Total_Raster_LandArea, Reco$Total_Intersection_Area_LDS, tolerance=0.02,info= paste("data in raster does not match land data outputs"))
  
})

end.time <- Sys.time()

print(paste0("Test - Compare raster outputs with LDS outputs at basin level  completing in ",start.time - end.time, " seconds" ))


start.time <- Sys.time()
test_that("Compare raster outputs with LDS outputs at country level", {
  #Get land data from raster files for each region (32 regions)
  #Get output data from LDS and compare with the above
  #Allow an error tolerance of 2% to account for rounding. Note that this works fine with an error tolerance of 1%
  
  #1. Get LDS data and iso data
  LDS_data <- read.csv("outputs/Land_type_area_ha.csv",skip = 9, header = T,stringsAsFactors = FALSE)
  ISO <- read.csv('outputs/iso_GCAM_regID.csv',skip = 6, header=T,stringsAsFactors = FALSE)
  LDS_data %>% 
  left_join(ISO %>% dplyr::select(iso,GCAM_region_ID),by="iso") -> LDS_data
  
  #2. Process LDS data
  #2a .Get subtotals in LDS data
  LDS_data_w_totals <- LDS_data %>%
    #Filter for the latest year
    filter(year==2016) %>% 
    #Compute GLU values
    group_by(glu_code) %>% 
    mutate(glu_total_LDS = sum(value)) %>% 
    ungroup() %>% 
    #Compute values for regions
    group_by(GCAM_region_ID) %>% 
    mutate(REG_total_LDS = sum(value)) %>% 
    ungroup()
  
  
  
  #2. Get raster data
  #Unzip data files
  unzip(zipfile = "data/ValidBoundaries.zip", 
        exdir = 'data')
  #Read Raster file
  Land_Data_Binary <- raster('data/moirai_valid_land_area.bsq')#This is the raster file with actual land area 
  Country_raster <- raster('data/moirai_valid_reigon32.bsq')#This is the raster file which maps countries
  
  
  #Convert raster files to data frames
  Land_Data_Actual <- (as.data.table(rasterToPoints(Land_Data_Binary))) 
  Country_Data_raster <- (as.data.table(rasterToPoints(Country_raster)))
  
  #Process raster data
  Joined_Raster_Data <- Country_Data_raster %>% 
    #Join in valid land area 
    left_join(Land_Data_Actual,by=c("x","y")) %>% 
    #Drop na's
    na.omit() %>% 
    #Group by Country
    #Note- Ask Alan to fix typo
    group_by(moirai_valid_reigon32) %>% 
    #Aggregate totals
    mutate(Total_Raster_LandArea = sum(moirai_valid_land_area)) %>% 
    ungroup() %>% 
    #un-select irrelevant cols and get unique values 
    dplyr::select(-x,-y,-moirai_valid_land_area) %>% 
    distinct() %>% 
    rename(moirai_valid_region32 = moirai_valid_reigon32)
  
  #3. Get LDS data for Country
  LDS_data_Country <- LDS_data_w_totals %>% 
    rename(moirai_valid_region32 = GCAM_region_ID)%>% 
    group_by(moirai_valid_region32) %>% 
    mutate(Total_Country_Area_LDS = sum(value)) %>% 
    ungroup() %>% 
    dplyr::select(moirai_valid_region32 , Total_Country_Area_LDS) %>% 
    distinct()
  
  Reco <- Joined_Raster_Data %>%  
    mutate(Total_Raster_LandArea = Total_Raster_LandArea *100) %>% 
    left_join(LDS_data_Country ,by=c("moirai_valid_region32")) 
  
  
  expect_equal(Reco$Total_Raster_LandArea, Reco$Total_Country_Area_LDS, tolerance=0.02,info= paste("data in raster does not match land data outputs"))
  
})
end.time <- Sys.time()
print(paste0("Test - Compare raster outputs with LDS outputs at country level completing in ",start.time - end.time, " seconds" ))
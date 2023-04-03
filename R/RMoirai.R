# Marcus Perry PNNL 3/22/23 
# R Moirai 
# Combines other draft R scripts.


library(raster)
library(rgdal)
library(sf)
# setwd('')

values <- suppressWarnings(readLines(file.choose()))  
values <- gsub("#.*", "", values)   
values <- gsub("\\t", "", values) 
values <- gsub("\\s+$", "", values)  

input <- values[values != ""]      
input

shp_name <- '~/example/path/example_shapefile.shp'  
## shp_name <- input[#index_of_shp] #read in from input

input_shapefile <- st_read(shp_name)

# Define the raster extents and resolution
output_raster <- raster(extent(input_shapefile), res = 0.0833333) # Change 'res' value to desired raster resolution

out_raster_file <- "~/yourpath/name_the_output_raster.tif" # change extension as appropriate for desired output raster format
## out_raster_file <- input[#index_for_output_Raster_name] #read in from input

head(input_shapefile)  
    
raster_value = "Shape_Area"  # Add additional line to end of input text file for this? Otherwise user modifies inline or potentially selects

final_raster <- rasterize(input_shapefile, output_raster, field = raster_value) 
writeRaster(final_raster, out_raster_file, format = "GTiff", overwrite = TRUE) # Can change raster format to .bil or other if preferred but match extension in out_raster_file

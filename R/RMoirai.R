# Marcus Perry PNNL 3/22/23 
# R Moirai 
# Combines other draft R scripts.


library(raster)
library(sf)
# setwd('')

# read_input
read_input <- function(input_txt_file) {
  values <- suppressWarnings(readLines(input_txt_file))  
  values <- gsub("#.*", "", values)   
  values <- gsub("\\t", "", values) 
  values <- gsub("\\s+$", "", values)  
  
  input <- values[values != ""]      
  return(input)
}

input <- read_input(input_txt_file)


shp_to_raster <- function(input_shp, crs, value_field, out_raster_name) {
    read_shp = st_read(input_shp, crs = crs)
    input_res = res(read_shp)
    output_specs <- raster(extent(read_shp), res = 0.0833333) # may need to change output extent
    raster_out <- rasterize(input_shp, output_specs, field = value_field)
    rasterized_shp <- writeRaster(raster_out, out_raster_name, format = "GTiff", overwrite = TRUE) 
    # Can create a separate file for each layer with a vector of filenames provided with "bylayer = TRUE"
    # Can output CRS to a .prj file with "prj = TRUE"
    # Can store min/max cell values (per format) with "setStatistics = TRUE"
    return(rasterized_shp)
}

rasterized_shp <- shp_to_raster(input_shp, crs, value_field, out_raster_name)

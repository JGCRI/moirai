# Marcus Perry, PNNL, 3/28/23
# Pre-input-- user specifies shapefile which is then rasterized for use in standardized Moirai input file

# install.packages("raster")
# install.packages("rgdal")
# install.packages("sf")

library(raster)
library(rgdal)
library(sf)




#### shp_name <- file.choose()
shp_name <- '~/example/path/example_shapefile.shp'
input_shapefile <- st_read(shp_name)

# Define the raster extents and resolution
output_raster <- raster(extent(input_shapefile), res = 0.0833333) # Change 'res' value to desired raster resolution.  0.083333 degrees = 5 arcmin)

##############
#out_raster_prompt <- function(){    # "~/example/path/example_output_raster.tif"
#  out_raster_file <- readline(prompt = "File name for output raster: ")
#}
#out_raster_prompt()
##############

out_raster_file <- "~/example/path/example_output_raster.tif"

###############
#field_prompt <- function(){
#  raster_value <- readline(prompt = "Field name for input raster value: ")
#}
#field_prompt()
################  
  
# Define the value field name
raster_value = "Shape_Area" # desired value field from input shapefile

final_raster <- rasterize(input_shapefile, output_raster, field = raster_value) 
writeRaster(final_raster, out_raster_file, format = "GTiff", overwrite = TRUE) # Can change raster format to .bil or other if preferred as long as it matches out_raster_file extension

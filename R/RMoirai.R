# Marcus Perry PNNL 3/22/23 
# R Moirai 
# Combines other draft R scripts.


library(raster)
library(sf)
library(stringr)
### Make sure you set working directory from which new directories will be created
# setwd('')

input_txt_file <- file.choose()

#### read_input
read_input <- function(input_txt_file) {
  values <- suppressWarnings(readLines(input_txt_file))  
  values <- gsub("#.*", "", values)   
  values <- gsub("\\t", "", values) 
  values <- gsub("\\s+$", "", values)  
  
  input <- values[values != ""]      
  return(input)
}
input <- read_input(input_txt_file)


#### Quick reference for input index number and info, retrieve values and variable names for later
input_index <- function(input_txt_file) {
  values <- suppressWarnings(readLines(input_txt_file))  
  values <- values[!grepl("^#", values)] 
  values <- gsub("\\t", "", values) 
  values <- gsub("\\s+$", "", values)  
  
  input <- values[values != ""]      
  return(input)
}
input_info <- input_index(input_txt_file)


#### Assign input filenames, paths, years, and values to their corresponding variables

process_input <- function(input_info) {
  # Initialize an empty list to store the results
  result <- list()
  
  # Iterate through the input vector
  #for (i in seq(1,130)) {                   # Change index values as appropriate for file names or for all use seq_along(input_info)) {
  for (i in seq_along(input_info)) {
    # Extract the file name (with extension) and the variable name
    file_name <- str_extract(input_info[i], "^[^#\\s]+")
    variable_name <- str_extract(input_info[i], "(?<=#)[^:]+")
    
    # If both file name and variable name are found, store them in the result list
    if (!is.na(file_name) && !is.na(variable_name)) {
      result[[variable_name]] <- file_name
    }
  }
  # Assign the file names to their corresponding variables in the global environment
  list2env(result, envir = .GlobalEnv) 
  
  
  return(result)
}
result <- process_input(input_info)

#### Create variables from indexed input values with which to create directories if they don't exist  
## Note ensure working directory is set e.g. setwd('')

create_directories <- function(result) {
  # Iterate through the result list from process_input()
  for (value in result) {
    # Check if the value is a path via starts with "./" and ends with "/"
    if (startsWith(value, "./") && endsWith(value, "/")) {
      # Create the directory if it doesn't exist
      if (!dir.exists(value)) {
        dir.create(value, recursive = TRUE)
        cat("Created directory:", value, "\n")
      }
    }
  }
}
# Call with the result list from process_input()
create_directories(result)


$$$$ Rasterize shapefile

shp_to_raster <- function(input_shp, crs, value_field, out_raster_name) {
    read_shp <- st_read(input_shp, crs = crs)
    input_res <- res(read_shp)
    output_specs <- raster(extent(read_shp), res = 0.0833333) # may need to change output extent
    raster_out <- rasterize(input_shp, output_specs, field = value_field)
    rasterized_shp <- writeRaster(raster_out, out_raster_name, format = "GTiff", overwrite = TRUE) 
    # Can create a separate file for each layer with a vector of filenames provided with "bylayer = TRUE"
    # Can output CRS to a .prj file with "prj = TRUE"
    # Can store min/max cell values (per format) with "setStatistics = TRUE"
    return(rasterized_shp)
}
rasterized_shp <- shp_to_raster(input_shp, crs, value_field, out_raster_name)

/**********
 calc_potveg_area.c
 
 the main purpose of this function is to normalize the year 2000 land area info
  and to calculate the area of potential vegetation (used only for forest area for land rent) in year 2000 as:
    the remaining land area after cropland, pasture, and urban has been subtracted
  and to store the pasture area for calculating animal sector land rent
 
 the base working grid will be based on hyde land area to be consistent with the historical data processing
 the cropland, pasture, and urban data read here are the year 2000 data from the bsq files used in proc_land_type_area()
 
 SAGE crop data processing for harvested area, irrigated harvested area, and production will still use sage land cells
    land rent is then calculated from the output production and harvested area arrays
    with some additional pasture area info, which is independent, so can be from hyde
    with some additional forest area info, which is independent, and is calculated from this pot veg area

 the working area arrays should have been initialized to NODATA in get_land_cells()

 arguments:
 args_struct in_args:	input argument structure
 rinfo_struct raster_info: information about input raster data

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 15 May 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified oct 2015 by Alan Di Vittorio
    changed a diagnostic comparison to sage (calculated) cell area vs hyde cell area
 
 Updated jan 2016 to use hyde land area as the working grid
 
 **********/

#include "lds.h"

int calc_potveg_area(args_struct in_args, rinfo_struct *raster_info) {
	
	// use this function to convert the sage and hyde data to the working grid area
	// do this all in one function so that there is only one loop
	
	// this requires the SAGE land area file and the HYDE land area file already available
	// also requires that the land cells have been identified
	// convert sage fraction of land area to area using the SAGE file
	// convert hyde urban values to fraction of land area then to sage working grid area
	// calculate the area of potential vegetation
	
	// sage 2000 cropland and pasture input units are fraction of land area
	// sage potential vegetation input units are classes 1 - 15
	// hyde urban input units are in km^2
	// all these data are on the same grid already
	// working units are km^2, based on the sage land area data
	
	int i;
	int err = OK;			// store error code from the write function
	int grid_index = 0;		// the whole grid index of current land cell for processing
	
    // hyde land use raster info
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int nodata = -9999;             // nodata value
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary
    
    float cur_land_area = -1;
    float cur_crop_area = -1;
    float cur_pasture_area = -1;
    float cur_urban_area = -1;
    
    // first read in the appropriate hyde land use area data
    if((err = read_lu_hyde(in_args, HYDE_YEAR, cropland_area, pasture_area, urban_area)) != OK)
    {
        fprintf(fplog, "Failed to read lu hude data for input: calc_potveg_area()\n");
        return err;
    }
    
    // now set the raster data info in case it is used
    raster_info->cropland_nrows = nrows;
    raster_info->cropland_ncols = ncols;
    raster_info->cropland_ncells = ncells;
    raster_info->cropland_nodata = nodata;
    raster_info->cropland_res = res;
    raster_info->cropland_xmin = xmin;
    raster_info->cropland_xmax = xmax;
    raster_info->cropland_ymin = ymin;
    raster_info->cropland_ymax = ymax;
    
    raster_info->pasture_nrows = nrows;
    raster_info->pasture_ncols = ncols;
    raster_info->pasture_ncells = ncells;
    raster_info->pasture_nodata = nodata;
    raster_info->pasture_res = res;
    raster_info->pasture_xmin = xmin;
    raster_info->pasture_xmax = xmax;
    raster_info->pasture_ymin = ymin;
    raster_info->pasture_ymax = ymax;
    
    raster_info->urban_nrows = nrows;
    raster_info->urban_ncols = ncols;
    raster_info->urban_ncells = ncells;
    raster_info->urban_nodata = nodata;
    raster_info->urban_res = res;
    raster_info->urban_xmin = xmin;
    raster_info->urban_xmax = xmax;
    raster_info->urban_ymin = ymin;
    raster_info->urban_ymax = ymax;
    
	// loop over the hyde land cells
	for (i = 0; i < num_land_cells_hyde; i++) {
		grid_index = land_cells_hyde[i];
		// convert cropland and pasture to area and urban to sage area
		if (land_area_hyde[grid_index] == 0) {
			// there may be some zero area cells, so make the land use areas consistent
			// zero land so set land types to 0 area
			cropland_area[grid_index] = 0;
			pasture_area[grid_index] = 0;
			urban_area[grid_index] = 0;
			potveg_area[grid_index] = 0;
		} else if (land_area_hyde[grid_index] != raster_info->land_area_hyde_nodata) {
            // check for nodata values, and set them to zero if valid land cell
			if (cropland_area[grid_index] == raster_info->cropland_nodata) {
                cropland_area[grid_index] = 0;
			}
			if (pasture_area[grid_index] == raster_info->pasture_nodata) {
                pasture_area[grid_index] = 0;
			}
            if (urban_area[grid_index] == raster_info->urban_nodata) {
				urban_area[grid_index] = 0;
			}

            cur_land_area = land_area_hyde[grid_index];
            cur_crop_area = cropland_area[grid_index];
            cur_pasture_area = pasture_area[grid_index];
            cur_urban_area = urban_area[grid_index];
            
			// calculate potential veg area
			potveg_area[grid_index] = land_area_hyde[grid_index] -
				cropland_area[grid_index] - pasture_area[grid_index] - urban_area[grid_index];
			// check for negative values
			if (potveg_area[grid_index] < 0) {
				// adjust urban area if not enough land
				urban_area[grid_index] = urban_area[grid_index] + potveg_area[grid_index];
				potveg_area[grid_index] = 0;
			}
			// double-check for enough land and adjust pasture
			if (urban_area[grid_index] < 0) {
				pasture_area[grid_index] = pasture_area[grid_index] + urban_area[grid_index];
				urban_area[grid_index] = 0;
			}
			// final check for enough land and adjust crops
			if (pasture_area[grid_index] < 0) {
				cropland_area[grid_index] = cropland_area[grid_index] + pasture_area[grid_index];
				pasture_area[grid_index] = 0;
			}
			// this shouldn't happen, but check anyway
			if (cropland_area[grid_index] < 0) {
				cropland_area[grid_index] = 0;
                fprintf(fplog, "Error: negative land area at grid_index %i: calc_potveg_area()\n", grid_index);
                return ERROR_CALC;
			}
		} else {
			// this should never be reached because the loop is over valid hyde land cells
			// just in case, hyde land area = nodata sets all land type areas to zero for this land cell
			land_area_hyde[grid_index] = 0;
			cropland_area[grid_index] = 0;
			pasture_area[grid_index] = 0;
			urban_area[grid_index] = 0;
			potveg_area[grid_index] = 0;
            fprintf(fplog, "Warning, nodata found in grid_index %i: calc_potveg_area()\n", grid_index);
		} // end if valid hyde land area else not
	}	// end for i loop over land cells
	
	if (in_args.diagnostics) {
		// cropland area
		if ((err = write_raster_float(cropland_area, NUM_CELLS, "cropland_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_potveg_area()\n", "cropland_area.bil");
			return err;
		}
		// pasture area
		if ((err = write_raster_float(pasture_area, NUM_CELLS, "pasture_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_potveg_area()\n", "pasture_area.bil");
			return err;
		}
		// urban area
		if ((err = write_raster_float(urban_area, NUM_CELLS, "urban_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_potveg_area()\n", "urban_area.bil");
			return err;
		}
		// potential vegetation area
		if ((err = write_raster_float(potveg_area, NUM_CELLS, "potveg_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_potveg_area()\n", "potveg_area.bil");
			return err;
		}
	}	// end if output diagnostics
	
	return OK;
}
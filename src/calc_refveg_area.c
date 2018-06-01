/**********
 calc_refveg_area.c
 
 the main purpose of this function is to normalize the year 2000 land area info
  and to calculate the area of reference vegetation (used only for forest area for land rent) in year 2000 as:
    a combintion of the hyde land use data, the lulc data, and the sage potential veg data
  and to store the pasture area for calculating animal sector land rent
 
 the base working grid will be based on hyde land area to be consistent with the historical data processing
 the cropland, pasture, and urban data read here are the year 2000 data from hyde32
 
 SAGE crop data processing for harvested area, irrigated harvested area, and production will still use sage land cells
    land rent is then calculated from the output production and harvested area arrays
    with some additional pasture area info, which is independent, so can be from hyde
    with some additional forest area info, which is independent, and is calculated from this pot veg area

 the working area arrays should have been initialized to NODATA in get_land_cells()

 also store the forest cells based on the reference veg
 
 this function does not check for valid country/glu
 
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

#include "moirai.h"

int calc_refveg_area(args_struct in_args, rinfo_struct *raster_info) {
	
	// use this function to call the lulc disaggregation function
	//  the hyde land use and land area are the base
	//  the lulc land cover data are distributed into the available non-land use area
	
	// this requires HYDE land area file to be already available
	// need the lulc data, disaggregated to working grid
	// also requires that the land cells have been identified
	// each half degree lulc cell is disaggregated to the hyde base
	
	// sage potential vegetation input units are classes 1 - 15
	// hyde urban input units are in km^2
	// all these data are on the same grid already
	// working units are km^2, based on the sage land area data
	
	int i, j, m, n;
	int err = OK;			// store error code from the write function
	int count = 0;			// counting the working grid cells
	
	// should probably retrieve these from the info arrays
	int urban_ind = 0;		// index in lu_area of urban values
	int crop_ind = 1;		// index in lu_area of cropland values; may need to find these from an array
	int pasture_ind = 2;	// index in lu_area of pasture values
	
    // hyde land use raster info
    int ncols;				// num hyde lons
	
	// lulc raster info
	int ncols_lulc;		// num lulc input lons
	int ncells_lulc;	// number of lulc input cells
	
	// used to determine working grid cell indices
	int num_split = 0;		// number of working grid cells in one dimension of one lulc cell
	int num_lu_cells = 0;	// number of working grid cells in one lulc cell
	int grid_y_ul;				// row for ul corner working grid cell in lulc cell
	int grid_x_ul;				// col for ul corner working grid cell in lulc cell
	double rem_dbl;				// used to get the remainder of a decimal number
	double int_dbl;				// used to get the integer of a decimal number
	
	float *lulc_area;		// array for the lulc areas per type for a single lulc cell
	float **lu_area;		// array for the lu areas determined for each lulc cell; dim1=num_lu_cells, dim2 = NUM_HYDE_TYPES
	int *lu_indices;		// array for the working grid indices for the lu cells for a single lulc cell
	float *refveg_area_out;		// array for the reference veg areas in each working grid cell, for a single lulc cell
	int *refveg_them;		// array for the reference veg tyep values in each working grid cell, for a single lulc cell
	
	// first read in the appropriate hyde land use area data
	// this is needed to get num_lu_cells
	if((err = read_hyde32(in_args, raster_info, REF_YEAR, cropland_area, pasture_area, urban_area, lu_detail_area)) != OK)
	{
		fprintf(fplog, "Failed to read lu hyde data for reference year: calc_refveg_area()\n");
		return err;
	}
	
	// read the lulc data
	if((err = read_lulc_isam(in_args, REF_YEAR, lulc_input_grid)) != OK)
	{
		fprintf(fplog, "Failed to read lulc data for reference year: calc_refveg_area()\n");
		return err;
	}
	
	// determine how many base lu cells are in one lulc cell
	// assume perfect fit of working grid into lulc data
	// assume symmetric cells
	ncols = raster_info->lu_ncols;
	ncols_lulc = raster_info->lulc_input_ncols;
	ncells_lulc = raster_info->lulc_input_ncells;
	
	num_split = ncols / ncols_lulc;
	num_lu_cells = num_split * num_split;
	
	// allocate some arrays
	lulc_area = calloc(NUM_LULC_TYPES, sizeof(float));
	if(lulc_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_area: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	lu_area = calloc(num_lu_cells, sizeof(float*));
	if(lu_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_area: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < num_lu_cells; i++) {
		lu_area[i] = calloc(NUM_HYDE_TYPES, sizeof(float));
		if(lu_area[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_area[%i]: calc_refveg_area()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
	lu_indices = calloc(num_lu_cells, sizeof(int));
	if(lu_indices == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_indices: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_area_out = calloc(num_lu_cells, sizeof(float));
	if(refveg_area_out == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_area_out: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_them = calloc(num_lu_cells, sizeof(float));
	if(refveg_them == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_them: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	
	// loop over the coarse lulc data
	for (i = 0; i < ncells_lulc; i++) {
		
		//if (in_args.diagnostics) {
		//	fprintf(fplog, "\nLULC cell %i: calc_refveg_area()\n", i);
		//}
		
		// get lulc areas for this cell
		for (j = 0; j < NUM_LULC_TYPES; j++) {
			lulc_area[j] = lulc_input_grid[j][i];
		}
		
		// determine the working grid 1d indices of the lu cells in this lulc cell
		// first get the upper left corner pixel in terms of rows and columns
		rem_dbl = fmod((double) i, (double) ncols_lulc);
		modf((double) (i / ncols_lulc), &int_dbl);
		grid_y_ul = (int) int_dbl * (num_split);
		grid_x_ul = (int) rem_dbl * (num_split);
		// now loop over the working grid cells to store the 1d indices and input areas, and initialize the ref veg values
		count = 0;
		for (m = grid_y_ul; m < grid_y_ul + num_split; m++) {
			for (n = grid_x_ul; n < grid_x_ul + num_split; n++) {
				lu_indices[count] = m * NUM_LON + n;
				lu_area[count][urban_ind] = urban_area[lu_indices[count]];
				lu_area[count][crop_ind] = cropland_area[lu_indices[count]];
				lu_area[count][pasture_ind] = pasture_area[lu_indices[count]];
				for (j = NUM_HYDE_TYPES_MAIN; j < NUM_HYDE_TYPES; j++) {
					lu_area[count][j] = lu_detail_area[j-NUM_HYDE_TYPES_MAIN][lu_indices[count]];
				}
				refveg_area_out[count] = 0;
				refveg_them[count] = 0;
				count++;
			} // end for n loop over the columns to set
		} // end for m loop over the rows to set
		if (count != num_lu_cells) {
			fprintf(fplog, "Failed to get working grid indices for lulc cell %i for reference year: calc_refveg_area()\n", i);
			return err;
		}
		
		// calculate the areas for this lulc cell
		// this keeps the hyde land use (but checks it for land consistency), and disaggregates the lc data to the non-lu cell area
		if ((err = proc_lulc_area(in_args, *raster_info, lulc_area, lu_indices, lu_area, refveg_area_out, refveg_them, num_lu_cells)) != OK)
		{
			fprintf(fplog, "Failed to process lulc cell %i for reference year: calc_refveg_area()\n", i);
			return err;
		}
		
		// store the areas in the appropriate places
		// set cell to nodata if it is not a land cell
		for (j = 0; j < num_lu_cells; j++) {
			if (land_area_hyde[lu_indices[j]] != raster_info->land_area_hyde_nodata) {
				cropland_area[lu_indices[j]] = lu_area[j][crop_ind];
				pasture_area[lu_indices[j]] = lu_area[j][pasture_ind];
				urban_area[lu_indices[j]] = lu_area[j][urban_ind];
				for (m = NUM_HYDE_TYPES_MAIN; m < NUM_HYDE_TYPES; m++) {
					lu_detail_area[m-NUM_HYDE_TYPES_MAIN][lu_indices[j]] = lu_area[j][m];
				}
				refveg_area[lu_indices[j]] = refveg_area_out[j];
				refveg_thematic[lu_indices[j]] = refveg_them[j];
				
				// if ref veg, then add cell index to land_mask_refveg and forest cells as appropriate
				if (refveg_thematic[lu_indices[j]] != raster_info->potveg_nodata) {
					land_mask_refveg[lu_indices[j]] = 1;
					// store the indices of the forest cells
					if (refveg_thematic[lu_indices[j]] <= MAX_SAGE_FOREST_CODE && refveg_thematic[lu_indices[j]] >= MIN_SAGE_FOREST_CODE) {
						forest_cells[num_forest_cells++] = lu_indices[j];
						land_mask_forest[lu_indices[j]] = 1;
					}
				} // end if valid ref veg and land area; forest will be checked in calc_rent_frs_use_aez for valid country/glu
				
			} else {
				cropland_area[lu_indices[j]] = NODATA;
				pasture_area[lu_indices[j]] = NODATA;
				urban_area[lu_indices[j]] = NODATA;
				for (m = NUM_HYDE_TYPES_MAIN; m < NUM_HYDE_TYPES; m++) {
					lu_detail_area[m-NUM_HYDE_TYPES_MAIN][lu_indices[j]] = NODATA;
				}
				refveg_area[lu_indices[j]] = NODATA;
				refveg_thematic[lu_indices[j]] = raster_info->potveg_nodata;
			}
		} // end for j loop over the lu cells to store
	} // end for i loop over the lulc cells
	
	 
	if (in_args.diagnostics) {
		// cropland area
		if ((err = write_raster_float(cropland_area, NUM_CELLS, "cropland_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "cropland_area.bil");
			return err;
		}
		// pasture area
		if ((err = write_raster_float(pasture_area, NUM_CELLS, "pasture_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "pasture_area.bil");
			return err;
		}
		// urban area
		if ((err = write_raster_float(urban_area, NUM_CELLS, "urban_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "urban_area.bil");
			return err;
		}
		// reference vegetation area
		if ((err = write_raster_float(refveg_area, NUM_CELLS, "refveg_area.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "refveg_area.bil");
			return err;
		}
		// reference vegetation types
		if ((err = write_raster_int(refveg_thematic, NUM_CELLS, "refveg_thematic.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "refveg_thematic.bil");
			return err;
		}
	}	// end if output diagnostics
	
	free(lulc_area);
	free(refveg_area_out);
	free(refveg_them);
	free(lu_indices);
	for (i = 0; i < num_lu_cells; i++) {
		free(lu_area[i]);
	}
	free(lu_area);
	
	
	return OK;
}
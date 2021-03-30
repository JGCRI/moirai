/**********
 calc_refcarbon_area.c
 
 the main purpose of this function is to normalize the year 2010 land area info
  and to calculate the area of reference vegetation (used only for soil and vvegetation carbon) in year 2010 as:
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
 
 this function also sets the cell order for distribution of ref veg within coarse lulc cells,
 because this is the first time the hyde and lulc data are read.
 
 arguments:
 args_struct in_args:	input argument structure
 rinfo_struct raster_info: information about input raster data

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 15 May 2013
 
 Moirai Land Data System (Moirai) Copyright (c) 2019, The
 Regents of the University of California, through Lawrence Berkeley National
 Laboratory (subject to receipt of any required approvals from the U.S.
 Dept. of Energy).  All rights reserved.
 
 If you have questions about your rights to use or distribute this software,
 please contact Berkeley Lab's Intellectual Property Office at
 IPO@lbl.gov.
 
 NOTICE.  This Software was developed under funding from the U.S. Department
 of Energy and the U.S. Government consequently retains certain rights.  As
 such, the U.S. Government has been granted for itself and others acting on
 its behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
 Software to reproduce, distribute copies to the public, prepare derivative
 works, and perform publicly and display publicly, and to permit other to do
 so.
 
 This file is part of Moirai.
 
 Moirai is free software: you can use it under the terms of the modified BSD-3 license (see …/moirai/license.txt)
 
 Modified oct 2015 by Alan Di Vittorio
    changed a diagnostic comparison to sage (calculated) cell area vs hyde cell area
 
 Updated jan 2016 to use hyde land area as the working grid
 
 **********/

#include "moirai.h"

int calc_refcarbon_area(args_struct in_args, rinfo_struct raster_info) {
	
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
    int ncols = raster_info.lu_ncols;				// num hyde lons
	
	// lulc raster info
	int ncols_lulc = raster_info.lulc_input_ncols;		// num lulc input lons
	int ncells_lulc = raster_info.lulc_input_ncells;	// number of lulc input cells
	
	// used to determine working grid cell indices
	int temp_int;			// for setting the random order
	int num_split = 0;		// number of working grid cells in one dimension of one lulc cell
	int grid_y_ul;				// row for ul corner working grid cell in lulc cell
	int grid_x_ul;				// col for ul corner working grid cell in lulc cell
	double rem_dbl;				// used to get the remainder of a decimal number
	double int_dbl;				// used to get the integer of a decimal number
	

    float *crop_grid;  // 1d array to store current crop data; start up left corner, row by row; lon varies faster
    float *pasture_grid;  // 1d array to store current pasture data; start up left corner, row by row; lon varies faster
    float *urban_grid;  // 1d array to store current urban data; start up left corner, row by row; lon varies faster
	float **lu_detail_grid;		// for the rest of the hyde types; dim1=hyde types, dim2=cells
	float **lulc_temp_grid;		// lulc input area (km^2); dim 1 = land types; dim 2 = grid cells

	double *lulc_area;		// array for the lulc areas per type for a single lulc cell
	double **lu_area;		// array for the lu areas determined for each lulc cell; dim1=NUM_LU_CELLS, dim2 = NUM_HYDE_TYPES
	int *lu_indices;		// array for the working grid indices for the lu cells for a single lulc cell
	double *refveg_area_out;		// array for the reference veg areas in each working grid cell, for a single lulc cell
	int *refveg_them;		// array for the reference veg tyep values in each working grid cell, for a single lulc cell
	
	double rfarea_check;
	double luarea_check;
	
	
	
	
	
	num_split = ncols / ncols_lulc;
	NUM_LU_CELLS = num_split * num_split;
	
	
	// allocate some arrays
	lulc_area = calloc(NUM_LULC_TYPES, sizeof(double));
	if(lulc_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_area: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	lu_area = calloc(NUM_LU_CELLS, sizeof(double*));
	if(lu_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_area: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_LU_CELLS; i++) {
		lu_area[i] = calloc(NUM_HYDE_TYPES, sizeof(double));
		if(lu_area[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_area[%i]: calc_refveg_area()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
	lu_indices = calloc(NUM_LU_CELLS, sizeof(int));
	if(lu_indices == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_indices: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_area_out = calloc(NUM_LU_CELLS, sizeof(double));
	if(refveg_area_out == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_area_out: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_them = calloc(NUM_LU_CELLS, sizeof(float));
	if(refveg_them == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_them: calc_refveg_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}

	crop_grid = calloc(NUM_CELLS, sizeof(float));
    if(crop_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for crop_grid: proc_land_type_area()\n");
        return ERROR_MEM;
    }
    pasture_grid = calloc(NUM_CELLS, sizeof(float));
    if(pasture_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for pasture_grid: proc_land_type_area()\n");
        return ERROR_MEM;
    }
    urban_grid = calloc(NUM_CELLS, sizeof(float));
    if(urban_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for urban_grid: proc_land_type_area()\n");
        return ERROR_MEM;
    }
	
	lu_detail_grid = calloc(NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN, sizeof(float*));
	if(lu_detail_grid == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_detail_grid: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		lu_detail_grid[i] = calloc(NUM_CELLS, sizeof(float));
		if(lu_detail_grid[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_detail_grid[%i]: proc_land_type_area()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
	
	lulc_temp_grid = calloc(NUM_LULC_TYPES, sizeof(float*));
	if(lulc_temp_grid == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_temp_grid: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		lulc_temp_grid[i] = calloc(NUM_CELLS_LULC, sizeof(float));
		if(lulc_temp_grid[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_temp_grid[%i]: proc_land_type_area()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}


	// first read in the appropriate hyde land use area data
	if((err = read_hyde32(in_args, &raster_info, REF_CARBON_YEAR, crop_grid, pasture_grid, urban_grid, lu_detail_grid)) != OK)
	{
		fprintf(fplog, "Failed to read lu hyde data for reference year: calc_refveg_area()\n");
		return err;
	}
	
	// read the lulc data
	if((err = read_lulc_isam(in_args, REF_CARBON_YEAR, lulc_temp_grid)) != OK)
	{
		fprintf(fplog, "Failed to read lulc data for reference year: calc_refveg_area()\n");
		return err;
	}
	
	// loop over the coarse lulc data
	for (i = 0; i < ncells_lulc; i++) {
		
		//if (in_args.diagnostics) {
		//	fprintf(fplog, "\nLULC cell %i: calc_refveg_area()\n", i);
		//}
		
		// get lulc areas for this cell
		for (j = 0; j < NUM_LULC_TYPES; j++) {
			lulc_area[j] = (double) lulc_temp_grid[j][i];
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
				lu_area[count][urban_ind] = (double) urban_grid[lu_indices[count]];
				lu_area[count][crop_ind] = (double) crop_grid[lu_indices[count]];
				lu_area[count][pasture_ind] = (double) pasture_grid[lu_indices[count]];
				for (j = NUM_HYDE_TYPES_MAIN; j < NUM_HYDE_TYPES; j++) {
					lu_area[count][j] = (double) lu_detail_grid[j-NUM_HYDE_TYPES_MAIN][lu_indices[count]];
				}
				refveg_area_out[count] = 0;
				refveg_them[count] = 0;
				count++;
			} // end for n loop over the columns to set
		} // end for m loop over the rows to set
		if (count != NUM_LU_CELLS) {
			fprintf(fplog, "Failed to get working grid indices for lulc cell %i for reference year: calc_refveg_area()\n", i);
			return err;
		}
		
		// calculate the areas for this lulc cell
		// this keeps the hyde land use (but checks it for land consistency), and disaggregates the lc data to the non-lu cell area
		if ((err = proc_lulc_area(in_args, raster_info, lulc_area, lu_indices, lu_area, refveg_area_out, refveg_them, NUM_LU_CELLS, i)) != OK)
		{
			fprintf(fplog, "Failed to process lulc cell %i for reference year: calc_refveg_area()\n", i);
			return err;
		}
		
		// store the areas in the appropriate places
		// set cell to nodata if it is not a land cell
		rfarea_check = 0;
		luarea_check = 0;
		for (j = 0; j < NUM_LU_CELLS; j++) {
			if (land_area_hyde[lu_indices[j]] != raster_info.land_area_hyde_nodata) {
				crop_grid[lu_indices[j]] = (float) lu_area[j][crop_ind];
				pasture_grid[lu_indices[j]] = (float) lu_area[j][pasture_ind];
				urban_grid[lu_indices[j]] = (float) lu_area[j][urban_ind];
				for (m = NUM_HYDE_TYPES_MAIN; m < NUM_HYDE_TYPES; m++) {
					lu_detail_grid[m-NUM_HYDE_TYPES_MAIN][lu_indices[j]] = (float) lu_area[j][m];
				}
				refcarbon_area[lu_indices[j]] = (float) refveg_area_out[j];
				refvegcarbon_thematic[lu_indices[j]] = refveg_them[j];
				
				rfarea_check = rfarea_check + refveg_area_out[j];
				luarea_check = luarea_check + lu_area[j][crop_ind] + lu_area[j][pasture_ind] +lu_area[j][urban_ind];
				
				
			} else {
				crop_grid[lu_indices[j]] = NODATA;
				pasture_grid[lu_indices[j]] = NODATA;
				urban_grid[lu_indices[j]] = NODATA;
				for (m = NUM_HYDE_TYPES_MAIN; m < NUM_HYDE_TYPES; m++) {
					lu_detail_grid[m-NUM_HYDE_TYPES_MAIN][lu_indices[j]] = NODATA;
				}
				refcarbon_area[lu_indices[j]] = NODATA;
				refvegcarbon_thematic[lu_indices[j]] = raster_info.potveg_nodata;
			}
		} // end for j loop over the lu cells to store
		
		//if(rfarea_check != 0 || luarea_check != 0){
		//	fprintf(fplog, "Check: year 2000 lulc cell %i refveg area %lf lu area %lf: calc_refveg_area()\n", i, rfarea_check, luarea_check);
		//	fprintf(debug_file, "calc_refveg_area,2000,%i,%lf,%lf,%lf\n", i, rfarea_check, luarea_check, rfarea_check + luarea_check);
		//}
		
	} // end for i loop over the lulc cells
	
	 
	if (in_args.diagnostics) {
		// reference vegetation area
		if ((err = write_raster_float(refcarbon_area, NUM_CELLS, "refveg_area_carbon.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "refveg_area_carbon.bil");
			return err;
		}
		// reference vegetation types
		if ((err = write_raster_int(refvegcarbon_thematic, NUM_CELLS, "refveg_carbon_thematic.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_refveg_area()\n", "refveg_carbon_thematic.bil");
			return err;
		}
	}	// end if output diagnostics
	
	free(lulc_area);
	free(refveg_area_out);
	free(refveg_them);
	free(lu_indices);
	for (i = 0; i < NUM_LU_CELLS; i++) {
		free(lu_area[i]);
	}
	free(lu_area);
	free(crop_grid);
    free(pasture_grid);
    free(urban_grid);
	for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		free(lu_detail_grid[i]);
	}
	free(lu_detail_grid);
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		free(lulc_temp_grid[i]);
	}
	free(lulc_temp_grid);
	
	
	return OK;}
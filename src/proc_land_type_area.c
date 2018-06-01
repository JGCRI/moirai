/***********
 proc_land_type_area.c
 
 generate csv table of land type area (ha) by country and glu and land type and year
 output table has 4 label columns and one value column
 iso, glu, land type (see LDS_land_types.csv), and land type area (ha)
 no zero value records
 order follows harvest area output, with iso alphabetically, then glu# in order, then land type in order, then year in order
 only countries with glus are written
 only countries with valid economic (ctry87) mapping are processed
 
 the land type categories are in LDS_land_types.csv and are generated as follows:
    sage potential vegetation code * 100 + lu_code + proctected_code
    sage potential vegetation values are 0=unknown, and 1-15 in order of SAGE_PVLT.csv
    lu_code is 0 for non-crop, non-pasture, non-urban (i.e. non hyde)
    land use codes are set in moirai.h: crop = 10, pasture = 20, urban = 30
    protected_code is 1 for protectected and 2 for unprotected
 
 source data are the
 hyde32 urban, crop, pasture data - the nodata values are NODATA
 lulc data
 reference vegetation as determined by the lulc data and sage pot veg data
 the protected image file
 the hyde land area data
 the fao country data
 the glu raster data
 country iso mapping
 ctry87 mapping
 
 the hyde lu input files are arc ascii raster files
 47 years available: 1700 - 2000 every 10 years, 2001-2016 each year
 the input file names are determined from the hyde input type file
 the first 3 files are the total crop, total pasture, and total urban area
 the remaining 9 files are the lu detail
 
 the lulc data start at 1800 and are half degree
 	use the 1800 lulc data for the previous hyde years
 	or process only 1800 forward?
 
 use proc_lulc_area() to determine lu and reference veg areas
 	this disaggregates the lulc land cover types to the hyde land area and maintains hyde land use area
 	using the sage potential vegetation categories
 
 input units are km^2
 output units are in ha - rounded to the nearest integer
 
 process only valid hyde land cells, as these are the source for land type area
 
 serbia and montenegro data are merged
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Alan Di Vittorio, 13 jan 2016
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified by Alan Di VIittorio, jan 2018
 	use lulc data to calc the reference veg area instead of just the potential veg area
 
 ***********/

#include "moirai.h"

int proc_land_type_area(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the hyde land area data set determine the land cells to process
    
    int i, j, k, m, n = 0;
    int year_ind;               // the index for looping over the years
    int grid_ind;               // the index within the 1d grid of the current land cell
    int rv_ind;                 // the index of the current reference veg land type
    int err = OK;				// store error code from the read/write functions
	int count = 0;				// counting the working grid cells
	
	// should probably retrieve these from the info arrays
	int urban_ind = 0;		// index in lu_area of urban values
	int crop_ind = 1;		// index in lu_area of cropland values; may need to find these from an array
	int pasture_ind = 2;	// index in lu_area of pasture values
	
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
	
	// hyde land use raster info
	int ncols = raster_info.lu_ncols;				// num hyde lons
	
	// lulc raster info
	int ncols_lulc = raster_info.lulc_input_ncols;		// num lulc input lons
	int ncells_lulc = raster_info.lulc_input_ncells;	// number of lulc input cells
	
    float *crop_grid;  // 1d array to store current crop data; start up left corner, row by row; lon varies faster
    float *pasture_grid;  // 1d array to store current pasture data; start up left corner, row by row; lon varies faster
    float *urban_grid;  // 1d array to store current urban data; start up left corner, row by row; lon varies faster
	float **lu_detail_grid;		// for the rest of the hyde types; dim1=hyde types, dim2=cells
	float **lulc_temp_grid;		// lulc input area (km^2); dim 1 = land types; dim 2 = grid cells
	
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
    
    float ****area_out;		// output table as 4-d array
    float outval;           // the integer value to output
    int rv_value;           // the reference veg value for the current land type category
	
	int lulc_year;			// current lulc year to read
    int aez_val;            // current aez value
    int ctry_code;          // current fao country code
    int aez_ind;            // current aez index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    int cur_lt_cat;         // current land type category
    int cur_lt_cat_ind;     // current land type category index
    int nrecords = 0;       // count # of records written
	
	float *global_lulc_in;	// for tracking global area in
	float *global_lt_out;	// for tracking global area out
	float global_area_out;	// total land area out
	float global_area_in;	// total land area in
	float temp_flt;
	
    int hyde_years[NUM_HYDE_YEARS]; // the years in the hyde historical lu files
   
    char fname[MAXCHAR];        // current file name to write
    FILE *fpout;                // out file pointer
    
    double tmp_dbl;
    
    // create the array of available years
    hyde_years[0] = HYDE_START_YEAR;
    for (i = 1; i < (NUM_HYDE_YEARS - NUM_HYDE_POST2000_YEARS); i++) {
        hyde_years[i] = hyde_years[i-1] + 10;
    }
	for (i = (NUM_HYDE_YEARS - NUM_HYDE_POST2000_YEARS); i < NUM_HYDE_YEARS; i++) {
		hyde_years[i] = hyde_years[i-1] + 1;
	}
	
	// determine how many base lu cells are in one lulc cell
	// assume perfect fit of working grid into lulc data
	// assume symmetric cells
	num_split = ncols / ncols_lulc;
	num_lu_cells = num_split * num_split;
	
    // allocate arrays
    
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
	
	// for proc_lulc_area
	lulc_area = calloc(NUM_LULC_TYPES, sizeof(float));
	if(lulc_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_area: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	lu_area = calloc(num_lu_cells, sizeof(float*));
	if(lu_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_area: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < num_lu_cells; i++) {
		lu_area[i] = calloc(NUM_HYDE_TYPES, sizeof(float));
		if(lu_area[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_area[%i]: proc_land_type_area()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
	lu_indices = calloc(num_lu_cells, sizeof(int));
	if(lu_indices == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_indices: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_area_out = calloc(num_lu_cells, sizeof(float));
	if(refveg_area_out == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_area_out: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_them = calloc(num_lu_cells, sizeof(float));
	if(refveg_them == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_them: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	
	// output
    area_out = calloc(NUM_FAO_CTRY, sizeof(float***));
    if(area_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for area_out: proc_land_type_area()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        area_out[i] = calloc(ctry_aez_num[i], sizeof(float**));
        if(area_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for area_out[%i]: proc_land_type_area()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            area_out[i][j] = calloc(num_lt_cats, sizeof(float*));
            if(area_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for area_out[%i][%i]: proc_land_type_area()\n", i, j);
                return ERROR_MEM;
            }
            for (k = 0; k < num_lt_cats; k++) {
                area_out[i][j][k] = calloc(NUM_HYDE_YEARS, sizeof(float));
                if(area_out[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for area_out[%i][%i][%i]: proc_land_type_area()\n", i, j, k);
                    return ERROR_MEM;
                }
            } // end for k loop over land type categories
        } // end for j loop over aezs
    } // end for i loop over fao country
	
	// for tracking global area
	global_lt_out = calloc(NUM_SAGE_PVLT + 1 + NUM_HYDE_TYPES, sizeof(float));
	if(global_lt_out == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for global_lt_out: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	global_lulc_in = calloc(NUM_SAGE_PVLT + 1 + NUM_HYDE_TYPES, sizeof(float));
	if(global_lulc_in == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for global_lulc_in: proc_land_type_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	
    // process each year
    for (year_ind = 0; year_ind < NUM_HYDE_YEARS; year_ind++) {

		if (in_args.diagnostics) {
			fprintf(fplog, "\nYear %i: proc_land_type_area()\n", hyde_years[year_ind]);
		}
		
		// first read in the appropriate hyde land use area data
		if((err = read_hyde32(in_args, &raster_info, hyde_years[year_ind], crop_grid, pasture_grid, urban_grid, lu_detail_grid)) != OK)
		{
			fprintf(fplog, "Failed to read lu hyde data for year %i: proc_land_type_area()\n", hyde_years[year_ind]);
			return err;
		}
		
		// read the appropriate lulc data
		if (hyde_years[year_ind] < LULC_START_YEAR) {
			lulc_year = LULC_START_YEAR;
		} else {
			lulc_year = hyde_years[year_ind];
		}
		if((err = read_lulc_isam(in_args, lulc_year, lulc_temp_grid)) != OK)
		{
			fprintf(fplog, "Failed to read lulc data for year %i: proc_land_type_area()\n", lulc_year);
			return err;
		}
		
		// initialize the diagnostic tracking arrays
		for (j = 0; j < NUM_SAGE_PVLT + 1 + NUM_HYDE_TYPES; j++) {
			global_lt_out[j] = 0;
			global_lulc_in[j] = 0;
		}
		
		// loop over the coarse lulc data
		for (i = 0; i < ncells_lulc; i++) {
			
			//if (in_args.diagnostics) {
			//	fprintf(fplog, "\nLULC cell %i: proc_land_type_area()\n", i);
			//}
			
			if (i == 58776) {
    			;
			}
			
			// get lulc areas for this cell
			for (j = 0; j < NUM_LULC_TYPES; j++) {
				lulc_area[j] = lulc_temp_grid[j][i];
			}
			
			// aggregate the lulc land cover type areas to pot veg types for global area
			// the sage pvlt values are the indices here, because of the zero unknown value
			// so sage pvlt data are first, then hyde data
			for (j = 0; j < NUM_LULC_LC_TYPES; j++) {
				if (lulc_area[j] != raster_info.lulc_input_nodata && lulc2sagecodes[j] != -1) {
					global_lulc_in[lulc2sagecodes[j]] = global_lulc_in[lulc2sagecodes[j]] + lulc_area[j];
				}
			}
			for (j = NUM_LULC_LC_TYPES; j < NUM_LULC_TYPES; j++) {
				if (lulc_area[j] != raster_info.lulc_input_nodata && lulc2hydecodes[j] != -1) {
					global_lulc_in[NUM_SAGE_PVLT + lulc2hydecodes[j]] = global_lulc_in[NUM_SAGE_PVLT + lulc2hydecodes[j]] + lulc_area[j];
				}
			}
			
			// determine the working grid 1d indices of the lu cells in this lulc cell
			// first get the upper left corner pixel in terms of rows and columns
			rem_dbl = fmod((double) i, (double) ncols_lulc);
			modf((double) (i / ncols_lulc), &int_dbl);
			grid_y_ul = (int) int_dbl * (num_split);
			grid_x_ul = (int) rem_dbl * (num_split);
			// now loop over the working grid cells to store the 1d indices and input areas, and initialize ref veg values
			count = 0;
			for (m = grid_y_ul; m < grid_y_ul + num_split; m++) {
				for (n = grid_x_ul; n < grid_x_ul + num_split; n++) {
					if (m == 490 && n == 2740) {
						;
					}
					lu_indices[count] = m * NUM_LON + n;
					temp_flt = urban_grid[lu_indices[count]];
					lu_area[count][urban_ind] = urban_grid[lu_indices[count]];
					temp_flt = crop_grid[lu_indices[count]];
					lu_area[count][crop_ind] = crop_grid[lu_indices[count]];
					temp_flt = pasture_grid[lu_indices[count]];
					lu_area[count][pasture_ind] = pasture_grid[lu_indices[count]];
					for (j = NUM_HYDE_TYPES_MAIN; j < NUM_HYDE_TYPES; j++) {
						temp_flt = lu_detail_grid[j-NUM_HYDE_TYPES_MAIN][lu_indices[count]];
						lu_area[count][j] = lu_detail_grid[j-NUM_HYDE_TYPES_MAIN][lu_indices[count]];
					}
					refveg_area_out[count] = 0;
					refveg_them[count] = 0;
					count++;
				} // end for n loop over the columns to set
			} // end for m loop over the rows to set
			if (count != num_lu_cells) {
				fprintf(fplog, "Failed to get working grid indices for lulc cell %i for reference year: proc_land_type_area()\n", i);
				return err;
			}
			
			// calculate the areas for this lulc cell
			// this keeps the hyde land use (but checks it for land consistency), and disaggregates the lc data to the non-lu cell area
			if ((err = proc_lulc_area(in_args, raster_info, lulc_area, lu_indices, lu_area, refveg_area_out, refveg_them, num_lu_cells)) != OK)
			{
				fprintf(fplog, "Failed to process lulc cell %i for reference year: proc_land_type_area()\n", i);
				return err;
			}
			
			// add data to output array as appropriate
			// don't need to store the updated grid data at all in the read in grids
			for (j = 0; j < num_lu_cells; j++) {
				grid_ind = lu_indices[j];
				// process only if there is land area
				// also skip if not a valid economic country
				if (land_area_hyde[grid_ind] != raster_info.land_area_hyde_nodata && land_area_hyde[grid_ind] != 0) {
					
					aez_val = aez_bounds_new[grid_ind];
					ctry_code = country_fao[grid_ind];
					
					if (aez_val != raster_info.aez_new_nodata) {
						// get the fao country index
						ctry_ind = NOMATCH;
						for (m = 0; m < NUM_FAO_CTRY; m++) {
							if (countrycodes_fao[m] == ctry_code) {
								ctry_ind = m;
								break;
							}
						} // end for m loop to get ctry index
						
						// merge serbia and montenegro for scg record
						if (ctry_code == mne_code || ctry_code == srb_code) {
							ctry_code = scg_code;
							ctry_ind = NOMATCH;
							for (m = 0; m < NUM_FAO_CTRY; m++) {
								if (countrycodes_fao[m] == ctry_code) {
									ctry_ind = m;
									break;
								}
							}
							if (ctry_ind == NOMATCH) {
								// this should never happen
								fprintf(fplog, "Error finding scg ctry index: proc_land_type_area()\n");
								return ERROR_IND;
							}
						} // end if serbia or montenegro
						
						// skip if not a valid economic country
						if (ctry_ind == NOMATCH || ctry2ctry87codes_gtap[ctry_ind] == NOMATCH) {
							continue;
						}
						
						// get the glu index within the country aez list
						aez_ind = NOMATCH;
						for (m = 0; m < ctry_aez_num[ctry_ind]; m++) {
							if (ctry_aez_list[ctry_ind][m] == aez_val) {
								aez_ind = m;
								break;
							}
						} // end for m loop to get aez index
						
						// this shouldn't happen because the countryXglu list has been made already
						if (aez_ind == NOMATCH) {
							fprintf(fplog, "Failed to match glu %i to country %i: proc_land_type_area()\n",aez_val,ctry_code);
							return ERROR_IND;
						}
						
						// generate the land type category and add/store the area
						
						// get index of ref veg to make sure it is valid
						rv_ind = NOMATCH;
						for (m = 0; m < NUM_SAGE_PVLT; m++) {
							if (refveg_them[j] == landtypecodes_sage[m]) {
								rv_ind = m;
								break;
							}
						}
						
						// if no ref veg cat, then use the unknown value of 0, otherwise set it to the grid value
						if (rv_ind == NOMATCH) {
							rv_value = 0;
						} else {
							rv_value = refveg_them[j];
						}
						
						// reference veg; i.e. non-crop, non-pasture, non-urban
						cur_lt_cat = rv_value * SCALE_POTVEG + protected_thematic[grid_ind];
						cur_lt_cat_ind = NOMATCH;
						for (m = 0; m < num_lt_cats; m++) {
							if (lt_cats[m] == cur_lt_cat) {
								cur_lt_cat_ind = m;
								break;
							}
						}
						if (cur_lt_cat_ind == NOMATCH) {
							fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
							return ERROR_IND;
						}
						if (refveg_area_out[j] != NODATA) { // don't add if NODATA
							temp_flt = refveg_area_out[j];
							area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] +	refveg_area_out[j];
							// sum the global out land type area
							// use the rv values as the index to capture the unknown value of zero
							global_lt_out[rv_value] = global_lt_out[rv_value] + refveg_area_out[j];
						}
						
						// crop
						cur_lt_cat = rv_value * SCALE_POTVEG + CROP_LT_CODE + protected_thematic[grid_ind];
						cur_lt_cat_ind = NOMATCH;
						for (m = 0; m < num_lt_cats; m++) {
							if (lt_cats[m] == cur_lt_cat) {
								cur_lt_cat_ind = m;
								break;
							}
						}
						if (cur_lt_cat_ind == NOMATCH) {
							fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
							return ERROR_IND;
						}
						if (lu_area[j][crop_ind] != raster_info.lu_nodata) { // don't add if nodata
							temp_flt = lu_area[j][crop_ind];
							area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] + lu_area[j][crop_ind];
							// sum the global out land type area
							// sage types plus one are first, then hyde types
							global_lt_out[crop_ind + NUM_SAGE_PVLT + 1] = global_lt_out[crop_ind + NUM_SAGE_PVLT + 1] + lu_area[j][crop_ind];
						}
						
						// pasture
						cur_lt_cat = rv_value * SCALE_POTVEG + PASTURE_LT_CODE + protected_thematic[grid_ind];
						cur_lt_cat_ind = NOMATCH;
						for (m = 0; m < num_lt_cats; m++) {
							if (lt_cats[m] == cur_lt_cat) {
								cur_lt_cat_ind = m;
								break;
							}
						}
						if (cur_lt_cat_ind == NOMATCH) {
							fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
							return ERROR_IND;
						}
						if (lu_area[j][pasture_ind] != raster_info.lu_nodata) { // don't add if nodata
							temp_flt = lu_area[j][pasture_ind];
							area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] + lu_area[j][pasture_ind];
							// sum the global out land type area
							// sage types plus one are first, then hyde types
							global_lt_out[pasture_ind + NUM_SAGE_PVLT + 1] = global_lt_out[pasture_ind + NUM_SAGE_PVLT + 1] + lu_area[j][pasture_ind];
						}
						
						// urban
						cur_lt_cat = rv_value * SCALE_POTVEG + URBAN_LT_CODE + protected_thematic[grid_ind];
						cur_lt_cat_ind = NOMATCH;
						for (m = 0; m < num_lt_cats; m++) {
							if (lt_cats[m] == cur_lt_cat) {
								cur_lt_cat_ind = m;
								break;
							}
						}
						if (cur_lt_cat_ind == NOMATCH) {
							fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
							return ERROR_IND;
						}
						if (lu_area[j][urban_ind] != raster_info.lu_nodata) { // don't add if nodata
							temp_flt = lu_area[j][urban_ind];
							area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] + lu_area[j][urban_ind];
							// sum the global out land type area
							// sage types plus one are first, then hyde types
							global_lt_out[urban_ind + NUM_SAGE_PVLT + 1] = global_lt_out[urban_ind + NUM_SAGE_PVLT + 1] + lu_area[j][urban_ind];
						}
						
						// sum the detailed lu categories also
						for (m = NUM_HYDE_TYPES_MAIN; m < NUM_HYDE_TYPES; m++) {
							temp_flt = lu_area[j][m];
							if (lu_area[j][m] != raster_info.lu_nodata) { // don't add if nodata
								global_lt_out[m + NUM_SAGE_PVLT + 1] = global_lt_out[m + NUM_SAGE_PVLT + 1] + lu_area[j][m];
							}
						}
						
					} // end if valid glu cell
					
				} // end if valid land area
				
			} // end for j loop over the lu cells to store
			
		} // end for i loop over the lulc cells
		
		if (in_args.diagnostics) {
			// write the global area check to the log file
			fprintf(fplog, "\nGlobal lulc area check for year %i: proc_land_type_area()\n", hyde_years[year_ind]);
			fprintf(fplog, "Unknown: out =\t%f\n", global_lt_out[0]);
			global_area_out = global_lt_out[0];
			global_area_in = 0;
			temp_flt = global_lt_out[0];
			for (j = 1; j <= NUM_SAGE_PVLT; j++) {
				fprintf(fplog, "%s: out =\t%f;\tin =\t%f\n", landtypenames_sage[j-1], global_lt_out[j], global_lulc_in[j]);
				global_area_out = global_area_out + global_lt_out[j];
				temp_flt = global_lt_out[j];
				global_area_in = global_area_in + global_lulc_in[j];
				temp_flt = global_lulc_in[j];
			}
			for (j = NUM_SAGE_PVLT + 1; j < NUM_SAGE_PVLT + 1 + NUM_HYDE_TYPES; j++) {
				fprintf(fplog, "%s: out =\t%f;\tin =\t%f\n", lutypenames_hyde[j - NUM_SAGE_PVLT - 1], global_lt_out[j], global_lulc_in[j]);
				if (j < NUM_SAGE_PVLT + 1 + NUM_HYDE_TYPES_MAIN) {
					global_area_out = global_area_out + global_lt_out[j];
					temp_flt = global_lt_out[j];
					global_area_in = global_area_in + global_lulc_in[j];
					temp_flt = global_lulc_in[j];
				}
			}
			fprintf(fplog, "Global land area: out =\t%f;\tin =\t%f\n", global_area_out, global_area_in);
		}
		
    } // end for year_ind loop over the years
    
    // write the output file
    
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.land_type_area_fname);
    fpout = fopen(fname,"w"); //float
    if(fpout == NULL)
    {
        fprintf(fplog,"Failed to open file  %s for write:  proc_land_type_area()\n", fname);
        return ERROR_FILE;
    }
    // write header lines
    fprintf(fpout,"# File: %s\n", fname);
    fprintf(fpout,"# Author: %s\n", CODENAME);
    fprintf(fpout,"# Description: area (ha) for sage land cells in country X glu X land type X protected category X year\n");
    fprintf(fpout,"# Original source: hyde areas; reference veg; country raster; new glu raster; hyde land area\n");
    fprintf(fpout,"# ----------\n");
    fprintf(fpout,"iso,glu_code,land_type,year,value");
    
    // write the records (convert to ha and round to nearest integer)
    for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
        for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
            for (cur_lt_cat_ind = 0; cur_lt_cat_ind < num_lt_cats; cur_lt_cat_ind++) {
                for (year_ind = 0; year_ind < NUM_HYDE_YEARS; year_ind++) {
                    tmp_dbl = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind];
                    outval = (float) floor((double) 0.5 + area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] * KMSQ2HA);
                    // output only positive values
                    if (outval > 0) {
                        fprintf(fpout,"\n%s,%i,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
                                lt_cats[cur_lt_cat_ind], hyde_years[year_ind], outval);
                        nrecords++;
                    } // end if value is positive
                } // end for year loop
            } // end for land type loop
        } // end for aez loop
    } // end for country loop
    
    fclose(fpout);
    
    fprintf(fplog, "Wrote file %s: proc_land_type_area(); records written=%i\n", fname, nrecords);
	
    free(crop_grid);
    free(pasture_grid);
    free(urban_grid);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            for (k = 0; k < num_lt_cats; k++) {
                free(area_out[i][j][k]);
            }
            free(area_out[i][j]);
        }
        free(area_out[i]);
    }
    free(area_out);
	for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		free(lu_detail_grid[i]);
	}
	free(lu_detail_grid);
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		free(lulc_temp_grid[i]);
	}
	free(lulc_temp_grid);
	free(lulc_area);
	free(refveg_area_out);
	free(refveg_them);
	free(lu_indices);
	for (i = 0; i < num_lu_cells; i++) {
		free(lu_area[i]);
	}
	free(lu_area);
	free(global_lt_out);
	free(global_lulc_in);
	
    return OK;

}

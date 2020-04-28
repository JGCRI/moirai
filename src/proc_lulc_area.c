/**********
 proc_lulc_area.c
 
 this function calculates the lulc area by:
 	using a self-consistent lulc dataset to adjust the land cover area
 	the hyde land use area (and land area) is maintained as the base
 	the lulc non-land-use area is applied to the reference veg area (hyde non-crop/grazing/urban)
 
 it disaggregates coarser resolution lulc data if necessary
 it does not currently deal with lulc data at finer resolution than hyde
 
 only one non-land-use land cover type is currently allowed in each working grid cell
 currently aggregate to sage potential veg types, because that is what gcam data system currently uses
 
 the randomized order for wg cells within each lulc cell is set once for each cell in calc_refveg_area
 
 arguments:
 args_struct in_args:		input argument structure
 rinfo_struct raster_info: 	information about input raster data
 double *lulc_area:			array of area values for each lulc land type; length is NUM_LULC_TYPES
 int *lu_indices:			array of lu cell indices for lu_area etc., from main lu raster arrays
 double **lu_area:			2-d array of area values for each lu cell and land type; d1=num_lu_cells (upper left start), d2=NUM_HYDE_TYPES
 double *refveg_area_out:	array of ref veg area values for each out lu cell
 int *refveg_them:			array of refveg thematic out values for each lu cell
 int num_lu_cells:			number of lu cells in lulc cell
 int lulc_index:			index of the current lulc cell

 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 18 Jan 2018
 
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
 
 **********/

#include "moirai.h"

int proc_lulc_area(args_struct in_args, rinfo_struct raster_info, double *lulc_area, int *lu_indices, double **lu_area, double *refveg_area_out, int *refveg_them, int num_lu_cells, int lulc_index) {
	
	int i, j, x, y, m;
	int potveg_ind;			// the index of current cell potential vegeation; for refveg_type_area_sum and lc_agg_area
	int potveg_val;				// the value of current pot veg; can be 0 (unknown)
	// should probably retrieve these from the info arrays
	int urban_ind = 0;		// index in lu_area of urban values
	int crop_ind = 1;		// index in lu_area of cropland values; may need to find these from an array
	int pasture_ind = 2;	// index in lu_area of pasture values
	int intense_ind = 3;	// intense pasture
	int range_ind = 4;		// range pasture
	int ir_norice_ind = 5;	// irrigated non-rice
	int rf_norice_ind = 6;	// rainfed non-rice
	int ir_rice_ind = 7;	// irrigated rice
	int rf_rice_ind = 8;	// rainfed rice
	int tot_irr_ind = 9;	// total irrigated
	int tot_rain_ind = 10;	// total rainfed
	int tot_rice_ind = 11;	// total rice
	
	double lulc_scalar = 1;			// the ratio of reference veg area to input lulc non-land-use area
	double sum_lulc_veg_area = 0;	// the total input lulc non-land-use area in this lulc cell
	double sum_refveg_area = 0;	// the total reference veg area in this lulc cell
	double sum_lu_land_area = 0;	// the total lu land area in this lulc cell
	double *sum_lu_area;			// the total lu area in this lulc cell; NUM_HYDE_TYPES
	double *lc_agg_area;		// the input lc and lu area in this lulc cell by type, aggregated to sage pot veg
	double *refveg_type_area_sum;	// sum of each output ref veg type within this lulc cell
	
	int temp_int;					// for swapping
	int sub_type;					// corresponding substitute ref veg type (index)
	int find_other = 0;				// flag to look for a substitute ref veg type
	int other_ind;					// index of the other substitute ref veg
	int max_resid_ind;				// index of the max residual area
	int num_leftover_cells = 0;		// number of output cells not assigned a ref veg in the first pass
	int *leftover_cell_inds;		// the indices of the output cells not assigned a ref veg in the first pass
	double sum_area_diff;			// difference between lulc area for a given type and the ref veg area for a given type within the lulc cell
	double max_sum_area_diff;		// the maximum sum_area_diff across types
	double *type_area_resid;			// array of residual areas (lulc - assigned refveg within lulc cell) for the types after the first pass
	double max_resid_area;			// for finding the max resid area
	double temp_rvt_area;			// for checking
	double temp_dbl2;				// for checking
	double land_area_in;				// input land area for this lulc cell
	
	// for searching the lu cells
	int ncols;
	int nrows;
	int current_val;
	double temp_dbl;
	double integer_dbl;
	int irow;
	int icol;
	int count;
	int toprow;
	int botrow;
	int leftcol;
	int rightcol;
	
	// allocate some arrays
	sum_lu_area = calloc(NUM_HYDE_TYPES, sizeof(double));
	if(sum_lu_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for sum_lu_area: proc_lulc_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	lc_agg_area = calloc(NUM_LULC_TYPES, sizeof(double));
	if(lc_agg_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lc_agg_area: proc_lulc_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	leftover_cell_inds = calloc(num_lu_cells, sizeof(int));
	if(leftover_cell_inds == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for leftover_cell_inds: proc_lulc_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	refveg_type_area_sum = calloc(NUM_SAGE_PVLT, sizeof(double));
	if(refveg_type_area_sum == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_type_area_sum: proc_lulc_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	type_area_resid = calloc(NUM_SAGE_PVLT, sizeof(double));
	if(type_area_resid == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for type_area_resid: proc_lulc_area()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	
	// determine the reference veg area then sum the land use and ref veg area in this lulc input cell and
	for (i = 0; i < num_lu_cells; i++) {
		
		temp_dbl = land_area_hyde[lu_indices[i]];
			// determine reference veg area
			if (land_area_hyde[lu_indices[i]] == 0) {
				// there may be some zero area cells, so make the land use areas consistent
				// zero land so set land types to 0 area
				for (j = 0; j < NUM_HYDE_TYPES; j++) {
					lu_area[i][j] = 0;
					temp_dbl = lu_area[i][j];
				}
				refveg_area_out[i] = 0;
			} else if (land_area_hyde[lu_indices[i]] != raster_info.land_area_hyde_nodata) {
				// check for nodata values, and set them to zero if valid land cell
				for (j = 0; j < NUM_HYDE_TYPES; j++) {
					if (lu_area[i][j] == raster_info.lu_nodata) {
						lu_area[i][j] = 0;
					}
					temp_dbl = lu_area[i][j];
				}
				refveg_area_out[i] = 0;
				// calculate reference veg area
				refveg_area_out[i] = land_area_hyde[lu_indices[i]] - lu_area[i][crop_ind] -
					lu_area[i][pasture_ind] - lu_area[i][urban_ind];
				temp_dbl = refveg_area_out[i];
				// check for negative values
				if (refveg_area_out[i] < 0) {
					// adjust urban area if not enough land
					lu_area[i][urban_ind] = lu_area[i][urban_ind] + refveg_area_out[i];
					refveg_area_out[i] = 0;
				}
				temp_dbl = lu_area[i][urban_ind];
				// double-check for enough land and adjust pasture
				if (lu_area[i][urban_ind] < 0) {
					lu_area[i][pasture_ind] = lu_area[i][pasture_ind] + lu_area[i][urban_ind];
					if (lu_area[i][pasture_ind] != 0) {
						lu_area[i][intense_ind] = lu_area[i][intense_ind] + lu_area[i][urban_ind] * lu_area[i][intense_ind] / lu_area[i][pasture_ind];
						lu_area[i][range_ind] = lu_area[i][range_ind] + lu_area[i][urban_ind] * lu_area[i][range_ind] / lu_area[i][pasture_ind];
					} else {
						lu_area[i][intense_ind] = 0;
						lu_area[i][range_ind] = 0;
					}
					lu_area[i][urban_ind] = 0;
				}
				temp_dbl = lu_area[i][pasture_ind];
				// final check for enough land and adjust crops
				if (lu_area[i][pasture_ind] < 0) {
					lu_area[i][crop_ind] = lu_area[i][crop_ind] + lu_area[i][pasture_ind];
					if (lu_area[i][crop_ind] != 0) {
						lu_area[i][ir_norice_ind] = lu_area[i][ir_norice_ind] + lu_area[i][pasture_ind] * lu_area[i][ir_norice_ind] / lu_area[i][crop_ind];
						lu_area[i][rf_norice_ind] = lu_area[i][rf_norice_ind] + lu_area[i][pasture_ind] * lu_area[i][rf_norice_ind] / lu_area[i][crop_ind];
						lu_area[i][ir_rice_ind] = lu_area[i][ir_rice_ind] + lu_area[i][pasture_ind] * lu_area[i][ir_rice_ind] / lu_area[i][crop_ind];
						lu_area[i][rf_rice_ind] = lu_area[i][rf_rice_ind] + lu_area[i][pasture_ind] * lu_area[i][rf_rice_ind] / lu_area[i][crop_ind];
						lu_area[i][tot_irr_ind] = lu_area[i][tot_irr_ind] + lu_area[i][pasture_ind] * lu_area[i][tot_irr_ind] / lu_area[i][crop_ind];
						lu_area[i][tot_rain_ind] = lu_area[i][tot_rain_ind] + lu_area[i][pasture_ind] * lu_area[i][tot_rain_ind] / lu_area[i][crop_ind];
						lu_area[i][tot_rice_ind] = lu_area[i][tot_rice_ind] + lu_area[i][pasture_ind] * lu_area[i][tot_rice_ind] / lu_area[i][crop_ind];
					} else {
						lu_area[i][ir_norice_ind] = 0;
						lu_area[i][rf_norice_ind] = 0;
						lu_area[i][ir_rice_ind] = 0;
						lu_area[i][rf_rice_ind] = 0;
						lu_area[i][tot_irr_ind] = 0;
						lu_area[i][tot_rain_ind] = 0;
						lu_area[i][tot_rice_ind] = 0;
					}
					lu_area[i][pasture_ind] = 0;
					lu_area[i][intense_ind] = 0;
					lu_area[i][range_ind] = 0;
				}
				temp_dbl = lu_area[i][crop_ind];
				// this shouldn't happen, but check anyway
				if (lu_area[i][crop_ind] < -ZERO_THRESH) {
					lu_area[i][crop_ind] = 0;
					lu_area[i][ir_norice_ind] = 0;
					lu_area[i][rf_norice_ind] = 0;
					lu_area[i][ir_rice_ind] = 0;
					lu_area[i][rf_rice_ind] = 0;
					lu_area[i][tot_irr_ind] = 0;
					lu_area[i][tot_rain_ind] = 0;
					lu_area[i][tot_rice_ind] = 0;
					fprintf(fplog, "Warning: negative crop area %lf at i %i: proc_lulc_area()\n", temp_dbl, i);
					// some small (effectively zero) negative values appear here from time to time
					return ERROR_CALC;
				}
			} else {
				// this could be reached due to coarse resolution lulc data
				// here, hyde land area == nodata sets all land type areas to zero for this lu cell
				// calc_refveg_area and proc_land_type_area store nodata for the types for hyde land area == nodata
				for (j = 0; j < NUM_HYDE_TYPES; j++) {
					lu_area[i][j] = 0;
				}
				refveg_area_out[i] = 0;
				if (in_args.diagnostics) {
					// this will happen a lot because all lulc cells are passed to proc_lulc_area()
					//fprintf(fplog, "Warning, hyde land area nodata found in i %i: proc_lulc_area()\n", i);
				}
			} // end if valid hyde land area else not

		// now sum the final lu areas
		// also sum the physical lu land areas
		temp_dbl2 = 0;
		for (j = 0; j < NUM_HYDE_TYPES; j++) {
			if (lu_area[i][j] != raster_info.lu_nodata) {
				sum_lu_area[j] = sum_lu_area[j] + lu_area[i][j];
			}
			if (j < NUM_HYDE_TYPES_MAIN) {
    			temp_dbl2 = temp_dbl2 + lu_area[i][j];
			}
		}
		
		// sum the ref veg area
		sum_refveg_area = sum_refveg_area + refveg_area_out[i];
		
		// sum the lu land area within this lulc cell
		if (land_area_hyde[lu_indices[i]] != raster_info.land_area_hyde_nodata) {
			sum_lu_land_area = sum_lu_land_area + land_area_hyde[lu_indices[i]];
			
			// check that these sum to land area for each lu cell, within tolerance
			// the max precision error here is 0.000015
			temp_dbl = land_area_hyde[lu_indices[i]] - (temp_dbl2 + refveg_area_out[i]);
			if (in_args.diagnostics) {
				if ((temp_dbl2 + refveg_area_out[i]) < (land_area_hyde[lu_indices[i]] - ROUND_TOLERANCE) || (temp_dbl2 + refveg_area_out[i]) > 	(land_area_hyde[lu_indices[i]] + ROUND_TOLERANCE)) {
					fprintf(fplog, "Warning: cell %i lu area %lf + refveg area %lf != land area %f, diff = %lf: proc_lulc_area()\n", lu_indices[i], temp_dbl2, refveg_area_out[i], land_area_hyde[lu_indices[i]], temp_dbl);
					//return ERROR_CALC;
				}
			} // end if diagnostics output
		} // end sum the lu area within this cell

		//if (lulc_index == 63120) {
		//	fprintf(cell_file, "proc_lulc_area,%i,%i,%i,%f,%lf,%lf,%lf,%lf,%lf\n", i, lu_indices[i], lulc_index, land_area_hyde[lu_indices[i]], refveg_area_out[i], temp_dbl2, lu_area[i][0], lu_area[i][1], lu_area[i][2]);
		//}
		
	} // end i loop over the lu cells to determine total areas
	
	// aggregate the lulc land cover type areas to pot veg types
	for (i = 0; i < NUM_LULC_LC_TYPES; i++) {
		if (lulc_area[i] != raster_info.lulc_input_nodata && lulc2sagecodes[i] != -1) {
			lc_agg_area[lulc2sagecodes[i]-1] = lc_agg_area[lulc2sagecodes[i]-1] + lulc_area[i];
		}
	}
	for (j = NUM_LULC_LC_TYPES; j < NUM_LULC_TYPES; j++) {
		if (lulc_area[j] != raster_info.lulc_input_nodata && lulc2hydecodes[j] != -1) {
			lc_agg_area[NUM_SAGE_PVLT + lulc2hydecodes[j]] = lc_agg_area[NUM_SAGE_PVLT + lulc2hydecodes[j]] + lulc_area[j];
		}
	}
	
	// now sum the non-land-use veg types for input lulc
	for (j = 0; j < NUM_SAGE_PVLT; j++) {
		sum_lulc_veg_area = sum_lulc_veg_area + lc_agg_area[j];
	}
	
	// sum the land use types for input lulc to get in land area
	land_area_in = sum_lulc_veg_area;
	temp_dbl2 = 0;
	for (j = NUM_SAGE_PVLT + 1; j < NUM_SAGE_PVLT + 1 + NUM_HYDE_TYPES_MAIN; j++) {
		land_area_in = land_area_in + lc_agg_area[j];
		temp_dbl2 = temp_dbl2 + sum_lu_area[j-(NUM_SAGE_PVLT + 1)];
	}
	
	// print the refveg area and lu area
	//if (sum_refveg_area != 0 || temp_dbl2 != 0) {
	//	fprintf(fplog, "Check: lulc cell %i refveg area %lf lu area %lf: proc_lulc_area()\n", lulc_index, sum_refveg_area, temp_dbl2);
	//	fprintf(debug_file, "proc_lulc_area,,%i,%lf,%lf,%lf\n", lulc_index, sum_refveg_area, temp_dbl2, sum_refveg_area + temp_dbl2);
	//}
	
	// check that lu land sum equals lulc land sum for this lulc cell
	// this unequal situation happens a fair amount due to different estimates of land along coastlines
	// this is why a scalar is calculated
	//if ((temp_dbl2 + sum_refveg_area) != land_area_in) {
	//	fprintf(fplog, "Error: lulc cell %i lu area %lf + refveg area %lf != lulc land area %lf: proc_lulc_area()\n", lulc_index, sum_lu_land_area, sum_refveg_area, land_area_in);
	//}
	
	// scale the input land cover to the available ref veg area
	if (sum_lulc_veg_area > 0) {
		lulc_scalar = sum_refveg_area / sum_lulc_veg_area;
	} else {
		lulc_scalar = 0;
	}
	
	// distribute the land cover based on the potential vegetation and adjust to lulc as necessary, but not over lulc limts
	// no lu land sets ref veg to nodata
	// zero ref veg area sets ref veg to pot veg
	for (m = 0; m < num_lu_cells; m++) {
		// get the randomized cell index
		i = rand_order[lulc_index][m];
		
		// do this only for cells with land area
		if (land_area_hyde[lu_indices[i]] != raster_info.land_area_hyde_nodata) {
			
			// set the reference veg and sum the ref veg area per land cover type
			// if the lulc limit is reached then move area to different cover type
			// unassigned cells are 0 in refveg_them
			
			// use potential veg if available
			if (potveg_thematic[lu_indices[i]] != raster_info.potveg_nodata) {
				potveg_ind = potveg_thematic[lu_indices[i]] - 1;
				potveg_val = potveg_thematic[lu_indices[i]];
			} else {
				// find a nearby pot veg type
				// assume symmetric cell size right now
				ncols = (int) round(sqrt((double)num_lu_cells));
				nrows = ncols;
				potveg_val = raster_info.potveg_nodata;
				temp_dbl = i / ncols;
				modf(temp_dbl, &integer_dbl);
				irow = (int) integer_dbl;
				icol = i - irow * ncols;
				// search vicinity for nearest valid pot veg value
				count = 1;
				while (potveg_val == raster_info.potveg_nodata) {
					// determine rows and cols to search
					toprow = irow - count;
					if (toprow < 0) {
						toprow = 0;
					}
					botrow = irow + count;
					if (botrow >= nrows) {
						botrow = nrows - 1 ;
					}
					leftcol = icol - count;
					if (leftcol < 0) {
						leftcol = 0;
					}
					rightcol = icol + count;
					if (rightcol >= ncols) {
						rightcol = ncols - 1;
					}
					// loop over rows
					for (x = toprow; x <= botrow; x++) {
						if (x == toprow || x == botrow) {
							// loop over columns if at top or bottom of search ring
							for (y = leftcol; y <= rightcol; y++) {
								current_val = potveg_thematic[x * nrows + y];
								// grab the first found value
								if (current_val != raster_info.potveg_nodata) {
									potveg_val = current_val;
									//fprintf(fplog, "Found potveg value for index %i at index %i\n", i, x * nrows + y);
									break; // don't need to search this row anymore
								}
							}	// end for y loop over columns
						} else {	// end if top or bottom of search ring
							current_val = potveg_thematic[x * nrows + leftcol];
							// grab the first found value
							if (current_val != raster_info.potveg_nodata) {
								potveg_val = current_val;
								//fprintf(fplog, "Found potveg value for index %i at index %i\n", i, x * nrows + leftcol);
								break; // don't need to search this row anymore
							}
							current_val = potveg_thematic[x * nrows + rightcol];
							// grab the first found value
							if (current_val != raster_info.potveg_nodata) {
								potveg_val = current_val;
								//fprintf(fplog, "Found potveg value for index %i at index %i\n", i, x * nrows + rightcol);
								break; // don't need to search this row anymore
							}
						}	// end else not top or bottom of search ring
					}	// end for x loop over rows
					count++;
					if (count == ncols) {
						potveg_val = 0; // set it to unknown to end the search
					}
				}	// end while loop over search rings
				potveg_ind = potveg_val - 1;
			} // end else find nearby pot veg type
			
			find_other = 0;
			if (potveg_val != 8 && potveg_val != 0) {
    			sum_area_diff = lulc_scalar * lc_agg_area[potveg_ind] - refveg_type_area_sum[potveg_ind];
			} else {
				sum_area_diff = -1;
			}
			
			if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0 && potveg_val != 8 && potveg_val != 0) {
				refveg_them[i] = potveg_val;
				refveg_type_area_sum[potveg_ind] = refveg_type_area_sum[potveg_ind] + refveg_area_out[i];
			} else if (lulc_scalar == 0 && potveg_val != 0) {
				if (lc_agg_area[potveg_ind] > 0) {
					// store the type, but don't add any area
					refveg_them[i] = potveg_val;
				}
			} else {
				// find an appropriate subsitute land cover
				switch (potveg_val) {
					case 0: // unknown
						find_other = 1;
						break;
					case 1: // trop evergreen
						// sub trop decid
						sub_type = 2;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0) {
							// store the type, but don't add any area
							refveg_them[i] = sub_type;
						} else {
							find_other = 1;
						}
						break;
					case 2: // trop decid
						// sub trop evergreen
						sub_type = 1;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0) {
							// store the type, but don't add any area
							refveg_them[i] = sub_type;
						} else {
							find_other = 1;
						}
						break;
					case 3: // temp broad ever
						// sub temp needle ever
						sub_type = 4;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else { // sub temp decid
							sub_type = 5;
							sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
							if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
								refveg_them[i] = sub_type;
								refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type;
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-2] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type-1;
							} else {
								find_other = 1;
							}
						}
						break;
					case 4: // temp needle ever
						// sub temp broad ever
						sub_type = 3;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else { // sub temp decid
							sub_type = 5;
							sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
							if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
								refveg_them[i] = sub_type;
								refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type;
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-2] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type-1;
							} else {
								find_other = 1;
							}
						}
						break;
					case 5: // temp decid
						// sub temp broad ever
						sub_type = 3;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else { // sub temp needle ever
							sub_type = 4;
							sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
							if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
								refveg_them[i] = sub_type;
								refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type;
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-2] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type-1;
							} else {
								find_other = 1;
							}
						}
						break;
					case 6: // boreal ever
						// sub boreal decid
						sub_type = 7;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
							// store the type, but don't add any area
							refveg_them[i] = sub_type;
						} else {
							find_other = 1;
						}
						break;
					case 7: // boreal decid
						// sub boreal ever
						sub_type = 6;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
							// store the type, but don't add any area
							refveg_them[i] = sub_type;
						} else {
							find_other = 1;
						}
						break;
					case 8:
						// Evergreen/Deciduous Mixed Forest/Woodland -
						// this type does not exist in the lulc data, and it appears mainly in the boreal region
						// the current carbon data show that this type has only slightly more carbon than the boreal types
						//	labelled as: Cold deciduous forest with evergreen
						// so first assign this cell to boreal deciduous, then to boreal evergreen
						
						// sub boreal decid
						sub_type = 7;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else { // sub boreal ever
							sub_type = 6;
							sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
							if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
								refveg_them[i] = sub_type;
								refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type;
							} else if (lulc_scalar == 0 && lc_agg_area[sub_type] > 0 ) {
								// store the type, but don't add any area
								refveg_them[i] = sub_type+1;
							} else {
								find_other = 1;
							}
						}
						break;
					case 9: // savanna
						find_other = 1;
						break;
					case 10: // grassland/steppe
						find_other = 1;
						break;
					case 11: // dense shrubland
						// sub open shrubland
						sub_type = 12;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
							// store the type, but don't add any area
							refveg_them[i] = sub_type;
						} else {
							find_other = 1;
						}
						break;
					case 12: // open shrubland
						// sub dense shrubland
						sub_type = 11;
						sum_area_diff = lulc_scalar * lc_agg_area[sub_type-1] - refveg_type_area_sum[sub_type-1];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0) {
							refveg_them[i] = sub_type;
							refveg_type_area_sum[sub_type-1] = refveg_type_area_sum[sub_type-1] + refveg_area_out[i];
						} else if (lulc_scalar == 0 && lc_agg_area[sub_type-1] > 0 ) {
							// store the type, but don't add any area
							refveg_them[i] = sub_type;
						} else {
							find_other = 1;
						}
						break;
					case 13: // tundra
						find_other = 1;
						break;
					case 14: // desert
						find_other = 1;
						break;
					case 15: // polar desert/rock/ice
						find_other = 1;
						break;
					default:
						fprintf(fplog, "Error: invalid potential veg value %i in lu cell %i proc_lulc_area()\n", potveg_val, i);
						return ERROR_CALC;
						break;
				} // end switch
				
				if (find_other == 1) {
					// search all the types and select the one with the most available area that fits
					// if nothting matches send it to leftovers
					other_ind = NOMATCH;
					max_sum_area_diff = 0;
					for (j = 0; j < NUM_SAGE_PVLT; j++) {
						sum_area_diff = lulc_scalar * lc_agg_area[j] - refveg_type_area_sum[j];
						// check zero area thresh here also
						temp_rvt_area = refveg_type_area_sum[j];
						temp_dbl = lc_agg_area[j];
						if (sum_area_diff >= refveg_area_out[i] && sum_area_diff > 0 && lc_agg_area[j] > ZERO_THRESH) {
							if (sum_area_diff > max_sum_area_diff) {
								other_ind = j;
								max_sum_area_diff = sum_area_diff;
							}
						} else if (lulc_scalar == 0 && lc_agg_area[j] > max_sum_area_diff ) { // end if this type fits
							// store a non-zero area type with the most area, but don't add any area
							// this is in case NOMATCH is found due to lulc_scalar = 0, in which case the prior if will never be true
							// check for rounding errors and/or essentially zero values
							//  it turns out that several lulc agg cells have type 1 as a very small non-zero number
							if (lc_agg_area[j] > ZERO_THRESH) {
								other_ind = j;
								max_sum_area_diff = lc_agg_area[j];
							}
						}
					} // end for j loop over pot veg types
					if (other_ind == NOMATCH && refveg_them[i] == 0) { // set it to potveg if no type is found
						refveg_them[i] = 0;
						if (potveg_val == 8) {
							potveg_val = potveg_val - 1;
							refveg_them[i] = potveg_val;
						}
						refveg_them[i] = potveg_val;
						// add the area if appropriate
						if (lulc_scalar != 0 && potveg_val != 0) {
							refveg_type_area_sum[potveg_val - 1] = refveg_type_area_sum[potveg_val - 1] + refveg_area_out[i];
						}
					} else if (refveg_them[i] == 0) { // set it only if it has not been set
						refveg_them[i] = landtypecodes_sage[other_ind];
						refveg_type_area_sum[other_ind] = refveg_type_area_sum[other_ind] + refveg_area_out[i];
					}
				} // end if find_other == 1
				
			} // end else find substitute cover type

		} else { // end if lu land area is not nodata
				refveg_them[i] = NODATA;
		} // end else cell not processed
		
		// track unassigned cells
		if (refveg_them[i] == 0) {
			leftover_cell_inds[num_leftover_cells] = i;
			num_leftover_cells++;
		}
		
	} // end for m loop over lu cells to distribute land cover without going over lulc limits
	
	// calc lulc and ref veg area discrepancies
	for (j = 0; j < NUM_SAGE_PVLT; j++) {
		type_area_resid[j] = lulc_scalar * lc_agg_area[j] - refveg_type_area_sum[j];
		if (type_area_resid[j] < 0 && in_args.diagnostics) {
			//fprintf(fplog, "Warning, resid area for pot veg type %i is < 0 (%lf): proc_lulc_area()\n", j+1, type_area_resid[j]);
		}
	} // end for j loop over pot veg types

	// loop over unassigned cells to deal with remaining unassigned cells and area
	//  rank the refveg area on the fly so that only one loop is necessary
	// assign the largest lulc resid area to the largest refveg area, and adjust lulc resid area and iterate
	for (i = 0; i < num_leftover_cells; i++) {
		// get the largest refveg area and reorder the indices to rank
		for	(j = i+1; j < num_leftover_cells; j++) {
			if (refveg_area_out[leftover_cell_inds[j]] > refveg_area_out[leftover_cell_inds[i]]) {
				temp_int = leftover_cell_inds[i];
				leftover_cell_inds[i] = leftover_cell_inds[j];
				leftover_cell_inds[j] = temp_int;
			}
		} // end for j loop to get the largest refveg area
	
		// assign the largest lulc residual to the largest refveg area
		max_resid_ind = NOMATCH;
		max_resid_area = 0;
		for (j = 0; j < NUM_SAGE_PVLT; j++) {
			if (type_area_resid[j] > max_resid_area) {
				max_resid_ind = j;
				max_resid_area = type_area_resid[j];
			}
		} // end for j loop over pot veg types for getting largest resid area
		
		// assign the veg type and reduce the corresponding residual area
		// if there is no max resid area found, then assign an unkown type, or check for potential veg value to reduce unknown area
		if (max_resid_ind != NOMATCH) {
			refveg_them[leftover_cell_inds[i]] = landtypecodes_sage[max_resid_ind];
			type_area_resid[max_resid_ind] = type_area_resid[max_resid_ind] - refveg_type_area_sum[max_resid_ind];
			if (in_args.diagnostics && type_area_resid[max_resid_ind] < 0) {
				//fprintf(fplog, "Extra lulc cell area for ref veg type %i is %lf: proc_lulc_area()\n", max_resid_ind+1, type_area_resid[max_resid_ind]);
			}
		} else {
			if (potveg_thematic[lu_indices[leftover_cell_inds[i]]] != raster_info.potveg_nodata) {
    			refveg_them[leftover_cell_inds[i]] = potveg_thematic[lu_indices[leftover_cell_inds[i]]];
			} else {
				refveg_them[leftover_cell_inds[i]] = 0;
			}
		}
		
	} // end for i loop over cells to deal with unassigned cells and residual area
	
	//if (in_args.diagnostics) {
	//	for (j = 0; j < NUM_SAGE_PVLT; j++) {
	//		fprintf(fplog,"pvlt %i:\tlc_agg_area = %lf;\trv_sum = %lf\n", j+1, lulc_scalar * lc_agg_area[j], refveg_type_area_sum[j]);
	//	}
	//}
	
	free(sum_lu_area);
	free(lc_agg_area);
	free(leftover_cell_inds);
	free(refveg_type_area_sum);
	free(type_area_resid);
	
	return OK;
}
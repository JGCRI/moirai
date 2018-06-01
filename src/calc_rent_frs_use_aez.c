/**********
 calc_rent_frs_use_aez.c
 
 calculate forest land rent values for the new aezs (million USD)
 
 arguments:
 args_struct in_args:	input argument structure
 rinfo_struct raster_info: information about input raster data
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 As I was unable to obtain the appropriate DGTM value data and forest type and biome data,
	this algorithm simply redistributes forest land rent to new aez boundaries based on forest area
 
 Created by Alan Di Vittorio on 26 June 2014
 Completed Aug 2014 by Alan Di Vittorio
 Copyright 2014 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
  Modified fall 2015 by Alan Di Vittorio
 
 to do: check lrout for gridded new aez input
 
 **********/

#include "moirai.h"

int calc_rent_frs_use_aez(args_struct in_args, rinfo_struct raster_info) {
	
	int i, j;
	int aez_ind_orig, aez_ind_reglr, use_ind, fa_ind, roa_ind, reglr_ind, out_index, all_aez_index;	// loop and placement indices
	int forest_cell_ind;	// index for looping over forest_cells
	
	int aez_val;			// the aez number for current cell
	int frs_sect = 13;		// the use index for the forest sector
	int err = OK;
	
	float *forest_area;         // forest area per original aez per land rent region (aez vaeries faster) (km^2)
	float *rent_orig_per_area;	// original rent per forest area per aez per land rent region (aez vaeries faster) (million USD/km^2)
	int *temp_indices;			// temp array for storing the forest cell indices when lengthening the forest_indices dim2
	int **forest_indices;		// the forest cell indices per original aez per land rent region (aez vaeries faster); cell indices in dim2
	int *num_forest_indices;	// the number of forest cell indices per original aez per land rent region (aez vaeries faster)
	
	float *newvorigrent87;		// store the new forest rent summed across aezs in USD (i.e. per ctry87, first dim is new, second dim is orig)
	float *lrout;				// for diagnostic output in USD
	
	char out_name[] = "rent_use_aez_all.csv";			// diagnostic output csv file name for entire table
	char out_comp_name[] = "newvorigrent87.csv";		// diagnostic output csv file name for ctry87 forest rent comparison
	char out_fa_name[] = "forest_area.csv";				// diagnostic output csv file name for ctry87 sage forest area comparison
	
	// these need to be initialized with zeroes, so use pointers and calloc
	forest_area = calloc(NUM_GTAP_CTRY87 * NUM_ORIG_AEZ, sizeof(float));
	if(forest_area == NULL) {
		fprintf(fplog,"Failed to allocate memory for forest_area:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	rent_orig_per_area = calloc(NUM_GTAP_CTRY87 * NUM_ORIG_AEZ, sizeof(float));
	if(rent_orig_per_area == NULL) {
		fprintf(fplog,"Failed to allocate memory for rent_orig_per_area:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	num_forest_indices = calloc(NUM_GTAP_CTRY87 * NUM_ORIG_AEZ, sizeof(int));
	if(num_forest_indices == NULL) {
		fprintf(fplog,"Failed to allocate memory for num_forest_indices:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	temp_indices = calloc(NUM_CELLS, sizeof(int));
	if(temp_indices == NULL) {
		fprintf(fplog,"Failed to allocate memory for temp_indices:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	forest_indices = calloc(NUM_GTAP_CTRY87 * NUM_ORIG_AEZ, sizeof(int *));
	if(forest_indices == NULL) {
		fprintf(fplog,"Failed to allocate memory for dim1 of forest_indices:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	// the second dimension will be dynamically reallocated for each dim1 index as needed
	// but put initial allocation here
	for (i = 0; i < NUM_GTAP_CTRY87 * NUM_ORIG_AEZ; i++) {
		forest_indices[i] = calloc(1, sizeof(int));
		if(forest_indices[i] == NULL) {
			fprintf(fplog,"Failed to allocate initial dummy memory for forest_indices[i]; i=%i:  calc_rent_frs_use_aez()\n", i);
			return ERROR_MEM;
		}
	}
	// allocate memory for the diagnostic output
	newvorigrent87 = calloc(NUM_GTAP_CTRY87 * 2, sizeof(float));
	if(newvorigrent87 == NULL) {
		fprintf(fplog,"Failed to allocate memory for newvorigrent87:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	lrout = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE * NUM_NEW_AEZ, sizeof(float));
	if(lrout == NULL) {
		fprintf(fplog,"Failed to allocate memory for lrout:  calc_rent_frs_use_aez()\n");
		return ERROR_MEM;
	}
	
	// loop over forest_cells to calculate forest area per cell and to assign forest cells to reglrxorigaez
	for (forest_cell_ind = 0; forest_cell_ind < num_forest_cells; forest_cell_ind++) {
		
		// get the orig aez id; this function retrieves the nodata value if no associated aez is found
		// do not use this cell data if there is no associated aez
		if ((err = get_aez_val(aez_bounds_orig, forest_cells[forest_cell_ind], raster_info.aez_orig_nrows,
							   raster_info.aez_orig_ncols, raster_info.aez_orig_nodata, &aez_val))) {
			fprintf(fplog, "Failed to get orig aez_val for grid cell %i: calc_rent_frs_use_aez()\n", forest_cells[forest_cell_ind]);
			return err;
		}
		
		// process this cell only if there is a valid aez id and a valid land rent region code
		if (aez_val != raster_info.aez_orig_nodata && country87_gtap[forest_cells[forest_cell_ind]] != NODATA) {
			
			// get reglr index of this cell
			reglr_ind = NOMATCH;
			for (i = 0; i < NUM_GTAP_CTRY87; i++) {
				if (country87codes_gtap[i] == country87_gtap[forest_cells[forest_cell_ind]]) {
					reglr_ind = i;
					break;
				}
			}	// end for i loop to get reglr_ind
			if(reglr_ind == NOMATCH) {	// now this should not happen
				fprintf(fplog,"Failed to find land rent region index:  calc_rent_frs_use_aez()\n");
				return ERROR_IND;
			}
			
            // get the original aez index for this cell
            // which is simply the value - 1
            aez_ind_orig = aez_val - 1;
            
			fa_ind = reglr_ind * NUM_ORIG_AEZ + aez_ind_orig; // index of the 2d forest_area array
			forest_area[fa_ind] = forest_area[fa_ind] + refveg_area[forest_cells[forest_cell_ind]];
			
			// grow the dim2 as needed, per orig aez and ctry87
			for (i = 0; i < num_forest_indices[fa_ind]; i++) {
				temp_indices[i] = forest_indices[fa_ind][i];
			}
			free(forest_indices[fa_ind]);
			forest_indices[fa_ind] = calloc(num_forest_indices[fa_ind] + 1, sizeof(int));
			if(forest_indices[fa_ind] == NULL) {
				fprintf(fplog,"Failed to allocate memory for forest_indices[i]; i=%i:  calc_rent_frs_use_aez()\n", i);
				return ERROR_MEM;
			}
			for (i = 0; i < num_forest_indices[fa_ind]; i++) {
				forest_indices[fa_ind][i] = temp_indices[i];
			}
			forest_indices[fa_ind][num_forest_indices[fa_ind]++] = forest_cells[forest_cell_ind];
		
		}	// end if valid aez cell
	} // end for forest_cell_ind loop over forest cells
	
	// loop over reglrxorigaez to calculate the land rent per unit of forest area for reglrxorigaez:
	//	rent_orig_per_area[reglrxorig_aez]=rent_orig_aez[reglrxusexorig_aez] / forest_area[reglrxorig_aez]
	// also loop over the forest indices to calc rent_use_aez[reglr][newaez][use]:
	//  rent_use_aez[reglr][newaez][use] =
	//   rent_use_aez[reglr][newaez][use] + rent_orig_per_area[reglrxorig_aez] * forest_area[forest_indices[fa_ind][i]]
	for (reglr_ind = 0; reglr_ind <  NUM_GTAP_CTRY87; reglr_ind++) {
		for (aez_ind_orig = 0; aez_ind_orig < NUM_ORIG_AEZ; aez_ind_orig++) {
			// get the use sector index for forest sector
			use_ind = NOMATCH;
			for (i = 0; i < NUM_GTAP_USE; i++) {
				if (frs_sect == usecodes_gtap[i]) {
					use_ind = i;
					break;
				}
			}
			if (use_ind == NOMATCH) {
				fprintf(fplog,"Failed to find use index for sector %i:  calc_rent_frs_use_aez()\n", frs_sect);
				return ERROR_IND;
			}
			fa_ind = reglr_ind * NUM_ORIG_AEZ + aez_ind_orig;
			roa_ind = reglr_ind * NUM_GTAP_USE * NUM_ORIG_AEZ + use_ind * NUM_ORIG_AEZ + aez_ind_orig;
			
			if (forest_area[fa_ind] == 0) {
				rent_orig_per_area[fa_ind] = 0;
			} else {
				rent_orig_per_area[fa_ind] = rent_orig_aez[roa_ind] / forest_area[fa_ind];
			}
			
			for (i = 0; i < num_forest_indices[fa_ind]; i++) {
				// get the new aez id; this function retrieves the nodata value if no associated aez is found
				// do not use this cell data if there is no associated new aez
				if ((err = get_aez_val(aez_bounds_new, forest_indices[fa_ind][i], raster_info.aez_new_nrows,
									   raster_info.aez_new_ncols, raster_info.aez_new_nodata, &aez_val))) {
					fprintf(fplog, "Failed to get new aez_val for forest_indices[%i][%i]: calc_rent_frs_use_aez()\n", fa_ind, i);
					return err;
				}
				if (aez_val != raster_info.aez_new_nodata) {
                    // get the new aez index in this land rent region for this cell
                    aez_ind_reglr = NOMATCH;
                    for (j = 0; j < reglr_aez_num[reglr_ind]; j++) {
                        if (reglr_aez_list[reglr_ind][j] == aez_val) {
                            aez_ind_reglr = j;
                            break;
                        }
                    }	// end for i loop to get reglr_ind
                    if(aez_ind_reglr == NOMATCH) {	// now this should not happen
                        fprintf(fplog,"Failed to find aez index for land rent region index %i:  calc_rent_frs_use_aez()\n",
                                reglr_ind);
                        return ERROR_IND;
                    }

                    rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] = rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] +
						rent_orig_per_area[fa_ind] * refveg_area[forest_indices[fa_ind][i]];
                    
					// for diagnostic output in USD, new in the first dim
					newvorigrent87[reglr_ind * 2] = newvorigrent87[reglr_ind * 2] +
						MIL2ONE * rent_orig_per_area[fa_ind] * refveg_area[forest_indices[fa_ind][i]];
				} // end if valid new aez value
			}	// end for i loop over forest_indices to calc rent_use_aez
			
			// for diagnostic output in USD, orig in the second dim
			newvorigrent87[reglr_ind * 2 + 1] = newvorigrent87[reglr_ind * 2 + 1] + MIL2ONE * rent_orig_aez[roa_ind];
			
			// log if there is original forest land rent but no forest area in this ctry87 and aez
			if (forest_area[fa_ind] == 0 && rent_orig_aez[roa_ind] > 0) {
				if (in_args.diagnostics){
					fprintf(fplog,"Warning: No forest area but orig rent for reglr_ind %i, aez_ind %i:  calc_rent_frs_use_aez()\n",
                        reglr_ind, aez_ind_orig);
				}
			}

		}	// end for aez_ind_orig loop to calc rent_orig_per_area
        
        // convert rent_use_aez to USD for diagnostic output; this land rent region is done
        
        // loop over the new aezs in this land rent region
        for (aez_ind_reglr = 0; aez_ind_reglr < reglr_aez_num[reglr_ind]; aez_ind_reglr++) {
            
            // get the new aez index in the complete aez list
            all_aez_index = NOMATCH;
            for (i = 0; i < NUM_NEW_AEZ; i++) {
                if (aez_codes_new[i] == reglr_aez_list[reglr_ind][aez_ind_reglr]) {
                    all_aez_index = i;
                    break;
                }
            }
            if (all_aez_index == NOMATCH) {
                // this shouldn't happen
                fprintf(fplog,"Error finding all aez index: calc_rent_frs_use_aez(); land rent region=%i aez=%i\n",
                        country87codes_gtap[reglr_ind], reglr_aez_list[reglr_ind][aez_ind_reglr]);
                return ERROR_IND;
            }
            
            for (i = 0; i < NUM_GTAP_USE; i++) {
                out_index = reglr_ind * NUM_GTAP_USE * NUM_NEW_AEZ + i * NUM_NEW_AEZ + all_aez_index;
                lrout[out_index] = MIL2ONE * rent_use_aez[reglr_ind][aez_ind_reglr][i];
            }
            
        } // end for loop over the new aezs in this land rent region to fill the diagnostic array
        
	}	// end for reglr_ind loop to calc rent_orig_per_area


	if (in_args.diagnostics) {
		if ((err = write_csv_float3d(lrout, country87codes_gtap, usecodes_gtap,
									 NUM_GTAP_CTRY87, NUM_GTAP_USE, NUM_NEW_AEZ, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_rent_frs_use_aez()\n", out_name);
			return err;
		}
		if ((err = write_csv_float2d(newvorigrent87, country87codes_gtap,
									 NUM_GTAP_CTRY87, 2, out_comp_name, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_rent_frs_use_aez()\n", out_comp_name);
			return err;
		}
		if ((err = write_csv_float2d(forest_area, country87codes_gtap,
									 NUM_GTAP_CTRY87, NUM_ORIG_AEZ, out_fa_name, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_rent_frs_use_aez()\n", out_fa_name);
			return err;
		}
	}
	
	free(newvorigrent87);
	free(forest_area);
	for (i = 0; i < NUM_GTAP_CTRY87 * NUM_ORIG_AEZ; i++) {
		free(forest_indices[i]);
	}
	free(forest_indices);
	free(temp_indices);
	free(num_forest_indices);
	free(lrout);
    free(rent_orig_per_area);
	
	return OK;
}
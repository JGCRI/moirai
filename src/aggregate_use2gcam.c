/**********
 aggregate_use2gcam.c

 aggregate the land rent region data to the gcam land units
 one land rent region can include multiple gcam regions
 
 write these data as a diagnostic output
 
 arguments:
 args_struct in_args:	the input argument structure
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 25 Nov 2013
 Copyright 2018 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 This file is part of Moirai.
 
 Moirai is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Moirai is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Moirai (/moirai/docs/COPYING.txt). If not, see <https://www.gnu.org/licenses/>.
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "moirai.h"

int aggregate_use2gcam(args_struct in_args) {
	
	// define one record as the set of aez values for a single country87 and use
	// the records need to be aggregated from countries to regions
	
	int i,j, k;
    int reggcam_code = 0;               // the current gcam region code
    int reglr_index = NOMATCH;          // land rent region index
    int reggcam_index[NUM_FAO_CTRY];    // gcam region indices for current reglr
    int num_reggcam_index;              // the number of gcam regions for current reglr
    int reggcam_out_ind;                // index in reggcam_index that is used for output
    int reglr_aez_index = NOMATCH;      // land rent region aez index
    int reggcam_aez_index = NOMATCH;    // gcam region aez index
    int use_index = NOMATCH;            // gtap index of use
    int all_aez_index = NOMATCH;        // aez index in the list of all new aezs
    int diag_index = NOMATCH;           // index of old-format 1d array for diagnostic output
	int err = OK;			// error code for called functions
	
	float ***rent_use_aez_gcam;			// array to output diagnostics in USD
    
    // for now, use the old-format 1d array for diagnostic output
    // aez varies fastest, then use, then gcam region
    // to do: check for gridded new aez input
    float *diag_rent_use_aez_gcam;
	    
	// allocate memory for the diagnostic output
	rent_use_aez_gcam = calloc(NUM_GCAM_RGN, sizeof(float**));
	if(rent_use_aez_gcam == NULL) {
		fprintf(fplog,"Failed to allocate memory for rent_use_aez_gcam:  aggregate_use2gcam()\n");
		return ERROR_MEM;
	}
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        rent_use_aez_gcam[i] = calloc(reggcam_aez_num[i], sizeof(float*));
        if(rent_use_aez_gcam[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for rent_use_aez_gcam[%i]:  aggregate_use2gcam()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < reggcam_aez_num[i]; j++) {
            rent_use_aez_gcam[i][j] = calloc(NUM_GTAP_USE, sizeof(float));
            if(rent_use_aez_gcam[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for rent_use_aez_gcam[%i][%i]:  aggregate_use2gcam()\n", i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
	
    // allocate the 1d diagnostic array
    diag_rent_use_aez_gcam = calloc(NUM_GCAM_RGN * NUM_GTAP_USE * NUM_NEW_AEZ, sizeof(float));
    if(diag_rent_use_aez_gcam == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_rent_use_aez_gcam:  aggregate_use2gcam()\n");
        return ERROR_MEM;
    }
    
	// loop over the land rent regions
	for (reglr_index = 0; reglr_index < NUM_GTAP_CTRY87; reglr_index++) {

        // one land rent region can map to multiple countries and gcam regions
        num_reggcam_index = 0;
        for (j = 0; j < NUM_FAO_CTRY; j++) {
            // skip fao countries without economic region - they are not going to find matches with ctry87
            if (ctry2regioncodes_gcam[j] == NOMATCH) {
                continue;
            }else if (country87codes_gtap[reglr_index] == ctry2ctry87codes_gtap[j]) {
                reggcam_code = ctry2regioncodes_gcam[j];
                for (k = 0; k < NUM_GCAM_RGN; k++) {
                    if (regioncodes_gcam[k] == reggcam_code) {
                        reggcam_index[num_reggcam_index++] = k;
                    }
                } // end for k loop to get current region index
            } // end if-else fao country found so get region code and index
        } // end for j loop to find the gcam region indices for this land rent region
        if (num_reggcam_index == 0) {
            // this shouldn't happen because already skipping countries above
            fprintf(fplog,"Error finding reggcam_index for reglr_index = %i:  aggregate_use2gcam()\n", reglr_index);
            return ERROR_IND;
        }
        
        // loop over the land rent region aezs
        for (reglr_aez_index = 0; reglr_aez_index < reglr_aez_num[reglr_index]; reglr_aez_index++) {
            // determine the gcam region aez index
            // just select the first one that matches
            reggcam_aez_index = NOMATCH;
            for (j = 0; j < num_reggcam_index; j++) {
                for (i = 0; i < reggcam_aez_num[reggcam_index[j]]; i++) {
                    if (reglr_aez_list[reglr_index][reglr_aez_index] == reggcam_aez_list[reggcam_index[j]][i]) {
                        reggcam_aez_index = i;
                        break;
                    }
                }
                if (reggcam_aez_index != NOMATCH) {
                    reggcam_out_ind = j;
                    break;
                }
            } // end j loop over gcam regions for this land rent region to find gcam aez index
            
            if (reggcam_aez_index == NOMATCH) {
                // this shouldn't happen because the gcam region list was made from the country list (see write_glu_mapping())
                fprintf(fplog,"Error finding gcam region aez index: aggregate_use2gcam(); lr region=%i lr reg aez=%i\n",
                        country87codes_gtap[reglr_index], reglr_aez_list[reglr_index][reglr_aez_index]);
                return ERROR_FILE;
            }
            
            // loop over the the use sectors
            for (use_index = 0; use_index < NUM_GTAP_USE; use_index++) {
                
                // convert to USD for diagnostic output
                rent_use_aez_gcam[reggcam_index[reggcam_out_ind]][reggcam_aez_index][use_index] =
                rent_use_aez_gcam[reggcam_index[reggcam_out_ind]][reggcam_aez_index][use_index] +
                rent_use_aez[reglr_index][reglr_aez_index][use_index] * MIL2ONE;
                
                // get the aez index in the complete aez list
                all_aez_index = NOMATCH;
                for (i = 0; i < NUM_NEW_AEZ; i++) {
                    if (aez_codes_new[i] == reggcam_aez_list[reggcam_index[reggcam_out_ind]][reggcam_aez_index]) {
                        all_aez_index = i;
                        break;
                    }
                }
                if (all_aez_index == NOMATCH) {
                    // this shouldn't happen
                    fprintf(fplog,"Error finding all aez index: aggregate_use2gcam(); land rent region=%i gcam region=%i aez=%i\n",
                            country87codes_gtap[reglr_index], regioncodes_gcam[reggcam_index[reggcam_out_ind]],
                            reggcam_aez_list[reggcam_index[reggcam_out_ind]][reggcam_aez_index]);
                    return ERROR_FILE;
                }
                // fill the 1d arrays
                diag_index =
                    reggcam_index[reggcam_out_ind] * NUM_GTAP_USE * NUM_NEW_AEZ + use_index * NUM_NEW_AEZ + all_aez_index;
                diag_rent_use_aez_gcam[diag_index] =
                diag_rent_use_aez_gcam[diag_index] +
                rent_use_aez[reglr_index][reglr_aez_index][use_index] * MIL2ONE;
                
            } // end for loop over the use sectors
		} // end for loop over the reglr aezs
	} // end for loop over the land rent regions

	if (in_args.diagnostics) {
		// land rent
		if ((err = write_csv_float3d(diag_rent_use_aez_gcam, regioncodes_gcam, usecodes_gtap, NUM_GCAM_RGN,
									 NUM_GTAP_USE, NUM_NEW_AEZ, "land_rent_aez_gcam.csv", in_args))) {
			fprintf(fplog, "Error writing file %s: aggregate_use2gam()\n", "land_rent_aez_gcam.csv");
			return err;
		}
	}
	
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        for (j = 0; j < reggcam_aez_num[i]; j++) {
            free(rent_use_aez_gcam[i][j]);
        }
        free(rent_use_aez_gcam[i]);
    }
	free(rent_use_aez_gcam);
    
    free(diag_rent_use_aez_gcam);
	
	return OK;
}
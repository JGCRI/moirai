/**********
 aggregate_crop2gcam.c
 
 aggregate the output fao country production and harvest area data to the gcam land units
 
 write these data as a diagnostic output
 
 arguments:
 args_struct in_args:	the input argument structure
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 13 Sep 2013
 
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
 
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "moirai.h"

int aggregate_crop2gcam(args_struct in_args) {
	
	// define one record as the set of aez values for a single country and crop
	// the records need to be aggregated from countries to regions
	
	int i,j;
	int reg_index = NOMATCH;		// region index
	int crop_index = NOMATCH;		// sage index of crop
	int ctry_index = NOMATCH;		// fao ctry index (to get region code)
    int aez_index = NOMATCH;        // aez index for current ctry index
    int reg_aez_index = NOMATCH;    // aez index for current reg index
    int all_aez_index = NOMATCH;    // aez index in the list of all new aezs
    int diag_index = NOMATCH;       // index of old-format 1d array for diagnostic output
	int err = OK;			// error code for called functions
	
    float ***harvestarea_crop_aez_gcam;			// array to output aggregated harvested area in ha
    float ***production_crop_aez_gcam;          // array to output aggregated produciton in metric tonnes
    
    // to do: deal with this for gridded new aez input
    // for now, fill an old-format 1d array for diagnostic output
    // aez varies fastest, then crop, then gcam region
    float *diag_harvestarea_crop_aez_gcam;          // array to output aggregated harvested area in ha
    float *diag_production_crop_aez_gcam;          // array to output aggregated produciton in metric tonnes
    
    // allocate memory for the diagnostic output
    harvestarea_crop_aez_gcam = calloc(NUM_GCAM_RGN, sizeof(float**));
    if(harvestarea_crop_aez_gcam == NULL) {
        fprintf(fplog,"Failed to allocate memory for harvestarea_crop_aez_gcam:  aggregate_crop2gcam()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        harvestarea_crop_aez_gcam[i] = calloc(reggcam_aez_num[i], sizeof(float*));
        if(harvestarea_crop_aez_gcam[i] == NULL) {
            fprintf(fplog,"\nFailed to allocate memory for harvestarea_crop_aez_gcam[%i]:  aggregate_crop2gcam()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < reggcam_aez_num[i]; j++) {
            harvestarea_crop_aez_gcam[i][j] = calloc(NUM_SAGE_CROP, sizeof(float));
            if(harvestarea_crop_aez_gcam[i][j] == NULL) {
                fprintf(fplog,"\nFailed to allocate memory for harvestarea_crop_aez_gcam[%i][%i]:  aggregate_crop2gcam()\n", i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
    
    production_crop_aez_gcam = calloc(NUM_GCAM_RGN, sizeof(float**));
    if(production_crop_aez_gcam == NULL) {
        fprintf(fplog,"Failed to allocate memory for production_crop_aez_gcam:  aggregate_crop2gcam()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        production_crop_aez_gcam[i] = calloc(reggcam_aez_num[i], sizeof(float*));
        if(production_crop_aez_gcam[i] == NULL) {
            fprintf(fplog,"\nFailed to allocate memory for production_crop_aez_gcam[%i]:  aggregate_crop2gcam()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < reggcam_aez_num[i]; j++) {
            production_crop_aez_gcam[i][j] = calloc(NUM_SAGE_CROP, sizeof(float));
            if(production_crop_aez_gcam[i][j] == NULL) {
                fprintf(fplog,"\nFailed to allocate memory for production_crop_aez_gcam[%i][%i]:  aggregate_crop2gcam()\n", i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country

    // allocate the 1d arrays
    diag_harvestarea_crop_aez_gcam = calloc(NUM_GCAM_RGN * NUM_SAGE_CROP * NUM_NEW_AEZ, sizeof(float));
    if(diag_harvestarea_crop_aez_gcam == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_harvestarea_crop_aez_gcam:  aggregate_crop2gcam()\n");
        return ERROR_MEM;
    }
    diag_production_crop_aez_gcam = calloc(NUM_GCAM_RGN * NUM_SAGE_CROP * NUM_NEW_AEZ, sizeof(float));
    if(diag_production_crop_aez_gcam == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_production_crop_aez_gcam:  aggregate_crop2gcam()\n");
        return ERROR_MEM;
    }
    
	// loop over the countries
    // skip fao countries that do not have an economic region
	for (ctry_index = 0; ctry_index < NUM_FAO_CTRY; ctry_index++) {
		
        if (ctry2regioncodes_gcam[ctry_index] == NOMATCH) {
			if (in_args.diagnostics){
            	fprintf(fplog,"Warning: FAO country %i not in economic region; aggregate_crop2gcam()\n", countrycodes_fao[ctry_index]);
			}
            continue;
        }else {
            // get gcam region index; have already checked for NOMATCH
            reg_index = NOMATCH;
            for (i = 0; i < NUM_GCAM_RGN; i++) {
                if (regioncodes_gcam[i] == ctry2regioncodes_gcam[ctry_index]) {
                    reg_index = i;
                    break;
                }
            }
            // loop over the country aezs
            for (aez_index = 0; aez_index < ctry_aez_num[ctry_index]; aez_index++) {
                // determine the region aez index
                reg_aez_index = NOMATCH;
                for (i = 0; i < reggcam_aez_num[reg_index]; i++) {
                    if (ctry_aez_list[ctry_index][aez_index] == reggcam_aez_list[reg_index][i]) {
                        reg_aez_index = i;
                        break;
                    }
                }
                if (reg_aez_index == NOMATCH) {
                    // this shouldn't happen because the gcam region list was made from the country list (see write_glu_mapping())
                    fprintf(fplog,"Error finding gcam region index: aggregate_crop2gcam(); country=%i region=%i aez=%i\n",
                            countrycodes_fao[ctry_index], regioncodes_gcam[reg_index], ctry_aez_list[ctry_index][aez_index]);
                    return ERROR_FILE;
                }
                // loop over the crops
                for (crop_index = 0; crop_index < NUM_SAGE_CROP; crop_index++) {
                    production_crop_aez_gcam[reg_index][reg_aez_index][crop_index] =
                        production_crop_aez_gcam[reg_index][reg_aez_index][crop_index] +
                        production_crop_aez[ctry_index][aez_index][crop_index];
                    harvestarea_crop_aez_gcam[reg_index][reg_aez_index][crop_index] =
                        harvestarea_crop_aez_gcam[reg_index][reg_aez_index][crop_index] +
                        harvestarea_crop_aez[ctry_index][aez_index][crop_index];
                    
                    // get the aez index in the complete aez list
                    all_aez_index = NOMATCH;
                    for (i = 0; i < NUM_NEW_AEZ; i++) {
                        if (aez_codes_new[i] == reggcam_aez_list[reg_index][reg_aez_index]) {
                            all_aez_index = i;
                            break;
                        }
                    }
                    if (all_aez_index == NOMATCH) {
                        // this shouldn't happen
                        fprintf(fplog,"Error finding all aez index: aggregate_crop2gcam(); country=%i region=%i aez=%i\n",
                                countrycodes_fao[ctry_index], regioncodes_gcam[reg_index], reggcam_aez_list[reg_index][reg_aez_index]);
                        return ERROR_FILE;
                    }
                    // fill the 1d arrays
                    diag_index = reg_index * NUM_SAGE_CROP * NUM_NEW_AEZ + crop_index * NUM_NEW_AEZ + all_aez_index;
                    diag_harvestarea_crop_aez_gcam[diag_index] =
                        diag_harvestarea_crop_aez_gcam[diag_index] +
                        harvestarea_crop_aez[ctry_index][aez_index][crop_index];
                    diag_production_crop_aez_gcam[diag_index] =
                        diag_production_crop_aez_gcam[diag_index] +
                        production_crop_aez[ctry_index][aez_index][crop_index];
                    
                } // end for crop loop
            } // end for country aez loop
        } // end else process this country because it is assigned to a gcam region
	} // end loop over fao country
	
	if (in_args.diagnostics) {
		// production
		if ((err = write_csv_float3d(diag_production_crop_aez_gcam, regioncodes_gcam, cropcodes_sage, NUM_GCAM_RGN,
									 NUM_SAGE_CROP, NUM_NEW_AEZ, "production_crop_aez_gcam.csv", in_args))) {
			fprintf(fplog, "Error writing file %s: aggregate_crop2gam()\n", "production_crop_aez_gcam.csv");
			return err;
		}
		// harvested area
		if ((err = write_csv_float3d(diag_harvestarea_crop_aez_gcam, regioncodes_gcam, cropcodes_sage, NUM_GCAM_RGN,
									 NUM_SAGE_CROP,NUM_NEW_AEZ, "harvestarea_crop_aez_gcam.csv", in_args))) {
			fprintf(fplog, "Error writing file %s: aggregate_crop2gam()\n", "harvestarea_crop_aez_gcam.csv");
			return err;
		}
	}
	
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        for (j = 0; j < reggcam_aez_num[i]; j++) {
            free(harvestarea_crop_aez_gcam[i][j]);
            free(production_crop_aez_gcam[i][j]);
        }
        free(harvestarea_crop_aez_gcam[i]);
        free(production_crop_aez_gcam[i]);
    }
    free(harvestarea_crop_aez_gcam);
    free(production_crop_aez_gcam);
    
    free(diag_harvestarea_crop_aez_gcam);
    free(diag_production_crop_aez_gcam);
    
	return OK;}

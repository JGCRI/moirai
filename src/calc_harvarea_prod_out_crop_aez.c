/**********
 calc_harvarea_prod_out_crop_aez.c
 
 read in sage yield and harvested area by crop and store output production and harvested area in arrays
    only cells with both area and yield positive are used
 
 also aggregate pasture area to fao ctry and aez
 
 calibrate yields to a different reference year if desired (calibrate to fao production and harv area)
 the recalibration year is determined by the available fao data and must be consistent with prodprice_fao
  (see read_yield_fao(), read_harvestarea_fao(), read_production_fao(), and read_prodprice_fao())
 
 The diagnostics do show that the 2003-2007 avg fao data are slightly farther from the gtap data than the 1997-2003 fao data
 To figure this out the prod_val_fao and harvest_val_fao need to be calculated once per countryXcrop and stored, and they should be checked in conjunction with each other for consistency
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct raster_info: info about input raster files

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 3 Sep 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "moirai.h"

int calc_harvarea_prod_out_crop_aez(args_struct in_args, rinfo_struct raster_info) {
	
	float country_prod[NUM_FAO_CTRY * NUM_SAGE_CROP];			// aggregated values per fao country x crop (metric tonnes)
	float country_harvarea[NUM_FAO_CTRY * NUM_SAGE_CROP];		// aggregated values per fao country x crop (km^2)
	
	int i,j,k;							// looping index
	int ctry_index;					// fao country index (output fao country index)
    int in_ctry_index;              // input fao country index in case countries have to be merged (for recalibration)
    int aez_index;                  // aez index for current aez_val
    int recal_index;				// the fao_country x sage_crop index for recalibration
	int prod_index;					// the index to get the fao production value
    int temp_index;                 // temporary index for storing the pre-merged ctry_index (for recalibration)
	int cellind;					// index for looping over grid cells
	int cropind;					// index for looping over crops
	int aez_val;					// the glu number for current cell
	int land_cell;					// the current land cell
	char fname[MAXCHAR];			// file name to open
	
    int all_aez_index;              // for the 1d old-format diagnostic output arrays
    int diag_index;                 // for the 1d old-format diagnostic output arrays
    
	int serbia_code = 272;			// for merging serbia (272, srb) into serbia and montenegro (186, scg)
	int montenegro_code = 273;		// for merging montenegro (273, mne) into serbia and montenegro (186, scg)
    int scg_code = 186;             // the fao code for accessing and storing merged serbia and montenegro (iso3 = scg)
    int scg_lastyear_index = 8;     // this is the index for year 2005 (fao data are years 1997 - 2007; index starts at 0)
	
	// this needs to match with the if statement lines at 117 and 283 in read_prodprice_fao()
	//int recal_year_ind = 6;			// hardcoded to 2005 avg; this is the starting index for averaging
	//int recal_num_years = 5;		// hardcoded to 2005 avg; this is the number of years to average
	//int recal_year_ind = 0;		// hardcoded to orig 2000 avg; this is the starting index for averaging
	//int recal_num_years = 7;		// hardcoded to orig 2000 avg; this is the number of years to average
	int num_yrs;					// the actual number of years with data for averaging
	float prod_val_fao;					// the fao production value for recalibration
	float harvest_val_fao;				// the fao harvest value for recalibration
	float temp_flt = 0;				// float variable for read in
	double temp_dbl = 0;			// variable for gettin integer part of quotient
	int start_recalib_year = 0;			// the first year of recalibration average
	int fao_start_year_index;		// the fao year index of the starting year for averaging
	
	int err = OK;								// store error code from the write functions
	int ncells = NUM_CELLS;						// the number of cells in the aez mask array
	char out_name[] = "missing_aez_mask.bil";	// diagnositic output raster file name
	char bildir[] = "sage/";					// the sage bil subdirectory of the outptus directory
	char yieldtag[] = "_yield.bil";				// the rest of the output yield file name
	char harvtag[] = "_harvarea.bil";			// the rest of the output yield file name
	char out_name_prod[] = "production_crop_aez.csv";	// diagnostic output name for production
	char out_name_harv[] = "harvestarea_crop_aez.csv";	// diagnostic output name for harvested area
	char out_name_past[] = "pasturearea_aez.csv";	// diagnostic output name for pasture area
	
    float lost_harvested_area[NUM_SAGE_CROP];     // harvested area not included due to no country match
    float mismatched_harvested_area[NUM_SAGE_CROP];     // when yield=0
    float mismatched_yield[NUM_SAGE_CROP];              // when harvested area=0
    int mismatched_yield_count[NUM_SAGE_CROP];          // to calc the avg mismatched yield
    
	float *yield_recalib;				// the recalibrated yield for a single crop, if needed
	float *area_recalib;				// the recalibrated area for a single crop, if needed
	
    // for now, use the old-format 1d arrays for the diagnostic outputs
    // glu variest fastest, then crop, then country
    // will this work if the glus are individual cells?
    float *diag_harvestarea_crop_aez;            // harvested area output (ha), output to nearest integer
    float *diag_production_crop_aez;             // production output (metric tonnes), output to nearest integer
    // glu varies faster, then country
    float *diag_pasturearea_aez;                 // pasture area (ha)
    
    
	// initialize some local arrays for recalibration
	for (i = 0; i < NUM_FAO_CTRY * NUM_SAGE_CROP; i++) {
		country_prod[i] = 0;
		country_harvarea[i] = 0;
	}
    
    // allocate 1d arrays for diagnostic output
    diag_harvestarea_crop_aez = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_NEW_AEZ, sizeof(float));
    if(diag_harvestarea_crop_aez == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_harvestarea_crop_aez:  calc_harvarea_prod_out_aez()\n");
        return ERROR_MEM;
    }
    diag_production_crop_aez = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_NEW_AEZ, sizeof(float));
    if(diag_production_crop_aez == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_production_crop_aez:  calc_harvarea_prod_out_aez()\n");
        return ERROR_MEM;
    }
    diag_pasturearea_aez = calloc(NUM_FAO_CTRY * NUM_NEW_AEZ, sizeof(float));
    if(diag_pasturearea_aez == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_pasturearea_aez:  calc_harvarea_prod_out_aez()\n");
        return ERROR_MEM;
    }
	
	// loop over SAGE crops
	for (cropind = 0; cropind < NUM_SAGE_CROP; cropind++) {
		
		// read in yield and harvest area
		// file units are converted from t/ha to t/km^2 and from fraction of land area to km^2
		// this function ensures that valid yield and area values exist for sage land cells
		strcpy(fname, in_args.sagepath);
		strcat(fname, &cropfilebase_sage[cropind][0]); // the read function will determine whether the file is zipped or not
		if ((err = read_sage_crop(fname, in_args.sagepath, &cropfilebase_sage[cropind][0], raster_info))) {
			fprintf(fplog, "Failed to read yield and area for crop %s: calc_harvarea_prod_out_aez()\n", fname);
			return err;
		}
		
		// deprecated diagnostic: write the unit-converted data as bil files
		// these file are written into a subdirectory of the outputs directory
        // and currently are not written
		//if (in_args.diagnostics) {
		if (0) {
			strcpy(fname, bildir);
			strcat(fname, &cropfilebase_sage[cropind][0]);
			strcat(fname, yieldtag);
			if ((err = write_raster_float(yield_in, NUM_CELLS, fname, in_args))) {
				fprintf(fplog, "Failed to write yield raster for crop %s: calc_harvarea_prod_out_aez()\n", fname);
				return err;
			}
			strcpy(fname, bildir);
			strcat(fname, &cropfilebase_sage[cropind][0]);
			strcat(fname, harvtag);
			if ((err = write_raster_float(harvestarea_in, NUM_CELLS, fname, in_args))) {
				fprintf(fplog, "Failed to write harvest area raster for crop %s: calc_harvarea_prod_out_aez()\n", fname);
				return err;
			}
		}
		
        // initialize some arrays
        lost_harvested_area[cropind] = 0;
        mismatched_harvested_area[cropind] = 0;
        mismatched_yield[cropind] = 0;
        mismatched_yield_count[cropind] = 0;
        
		// loop over sage land cells
		// determine fao country, skip if fao country not found
		// aggregate to fao country for optional calibration
		// aggregate to land unit (aez within each fao country)
		for (cellind = 0; cellind < num_land_cells_sage; cellind++) {
			land_cell = land_cells_sage[cellind];
			// fao country index
			if ((int) country_fao[land_cell] != raster_info.country_fao_nodata) {
				ctry_index = NOMATCH;
				for (i = 0; i < NUM_FAO_CTRY; i++) {
					if (countrycodes_fao[i] == (int) country_fao[land_cell]) {
						ctry_index = i;
						break;
					}
				}	// end for i loop over fao ctry to find fao index
			} else {
				//fprintf(fplog, "No fao country exists for this cell: calc_harvarea_prod_out_aez(); cellind = %i\n", cellind);
                lost_harvested_area[cropind] = lost_harvested_area[cropind] + harvestarea_in[land_cell];
				continue;	// no country associated with these data so don't use this cell and go to the next one
			}	// end if fao country else no country
	
			if (ctry_index == NOMATCH) {
				fprintf(fplog, "Error determining fao country index: calc_harvarea_prod_out_aez(); cellind = %i\n", cellind);
				return ERROR_IND;
            } else {
                // aggregate to fao country and aez
                
				// get the glu number; this function retrieves the nodata value if no associated glu is found
				// do not use this cell data if there is no associated aez
				if ((err = get_aez_val(aez_bounds_new, land_cell, raster_info.aez_new_nrows,
									  raster_info.aez_new_ncols, raster_info.aez_new_nodata, &aez_val))) {
					fprintf(fplog, "Failed to get aez_val for crop %s: calc_harvarea_prod_out_aez()\n", fname);
					return err;
				}
				
				// store the output values, and aggregate area to fao for recalib
				if (aez_val != raster_info.aez_new_nodata) {
                    
                    // data for serbia and montenegro need to be merged for processing
                    if (countrycodes_fao[ctry_index] == serbia_code || countrycodes_fao[ctry_index] == montenegro_code) {
                        for (i = 0; i < NUM_FAO_CTRY; i++) {
                            if (countrycodes_fao[i] == scg_code) {
                                ctry_index = i;
                                break;
                            }
                        }
                    }
                    
                    // get the current glu index in the complete glu list
                    all_aez_index = NOMATCH;
                    for (i = 0; i < NUM_NEW_AEZ ; i++) {
                        if (aez_codes_new[i] == aez_val) {
                            all_aez_index = i;
                            break;
                        }
                    }
                    if (all_aez_index == NOMATCH) {
                        fprintf(fplog, "Failed to get all_aez_index for crop %s in cellind = %i: calc_harvarea_prod_out_aez()\n",
                                fname, cellind);
                        return err;
                    }
                    
                    // get the current glu index in the country list
                    aez_index = NOMATCH;
                    for (i = 0; i < ctry_aez_num[ctry_index]; i++) {
                        if (ctry_aez_list[ctry_index][i] == aez_val) {
                            aez_index = i;
                            break;
                        }
                    }
                    if (aez_index == NOMATCH) {
                        fprintf(fplog, "Failed to get aez_index for crop %s in cellind = %i: calc_harvarea_prod_out_aez()\n",
                                fname, cellind);
                        return err;
                    }
                    
                    // both values for this cell are set to zero if either area or yield are not non-zero, positive values
                    if (harvestarea_in[land_cell] > 0 && yield_in[land_cell] > 0) {
                        harvestarea_crop_aez[ctry_index][aez_index][cropind] =
                            harvestarea_crop_aez[ctry_index][aez_index][cropind] +
                            KMSQ2HA * harvestarea_in[land_cell];
                        production_crop_aez[ctry_index][aez_index][cropind] =
                            production_crop_aez[ctry_index][aez_index][cropind] +
                            harvestarea_in[land_cell] * yield_in[land_cell];
                        
                        // fill the 1d arrays
                        diag_index = ctry_index * NUM_SAGE_CROP * NUM_NEW_AEZ + cropind * NUM_NEW_AEZ + all_aez_index;
                        diag_harvestarea_crop_aez[diag_index] = diag_harvestarea_crop_aez[diag_index] +
                            KMSQ2HA * harvestarea_in[land_cell];
                        diag_production_crop_aez[diag_index] = diag_production_crop_aez[diag_index] +
                            harvestarea_in[land_cell] * yield_in[land_cell];
                        
                        // aggregate to fao countries by sage crop, for recalibration; only area is needed here
                        // do this only for data that will be included in the ctryXglu pixel output
                        // and only if both area and yield values are non-zero and positive
                        // all fao indices have valid codes
                        recal_index = ctry_index * NUM_SAGE_CROP + cropind;
                        // to do: left hand operand of + is garbage value???
                        country_harvarea[recal_index] = country_harvarea[recal_index] + harvestarea_in[land_cell];
                        //if (recal_index == 24328) {
                        //    i=-1;
                        //}
                        
                    }else { // end if adding non-zero values from this cell to the total
                        mismatched_harvested_area[cropind] = mismatched_harvested_area[cropind] + harvestarea_in[land_cell];
                        if (yield_in[land_cell] > 0) {
                            mismatched_yield[cropind] = mismatched_yield[cropind] + yield_in[land_cell];
                            mismatched_yield_count[cropind] = mismatched_yield_count[cropind] + 1;
                        }
                    }
					
					// these conditions are never true for the current sage data
                    // even before the new test for valid area and yield values above
					/*
					if (harvestarea_crop_aez[ctry_index][aez_index][cropind] < 0 ||
                        harvestarea_crop_aez[ctry_index][aez_index][cropind] > 30000000) {
						fprintf(fplog, "Bad harvestarea_crop_aez = %f output at ctry_index = %i and aez_index = %i and cropind = %i: calc_harvarea_prod_out_aez()\n",
								harvestarea_crop_aez[ctry_index][aez_index][cropind], ctry_index, aez_index, cropind);
					}
					if (production_crop_aez[ctry_index][aez_index][cropind] < 0 ||
                        production_crop_aez[ctry_index][aez_index][cropind] > 200000000) {
						fprintf(fplog, "Bad production_crop_aez = %f output at ctry_index = %i and aez_index = %i and cropind = %i: calc_harvarea_prod_out_aez()\n",
								production_crop_aez[ctry_index][aez_index][cropind], ctry_index, aez_index, cropind);
					}
					 */
					
					// only do these once, and if valid pasture are
					if(cropind == 0 && pasture_area[land_cell] != NODATA) {
						// pasture
						pasturearea_aez[ctry_index][aez_index] = pasturearea_aez[ctry_index][aez_index] +
							KMSQ2HA * pasture_area[land_cell];
                        
                        // fill the 1d array
                        diag_index = ctry_index * NUM_NEW_AEZ + all_aez_index;
                        diag_pasturearea_aez[diag_index] = diag_pasturearea_aez[diag_index] +
                            KMSQ2HA * pasture_area[land_cell];
						
						// store the output countryXaez land mask
						land_mask_ctryaez[land_cell] = 1;
					}
				}	// end if valid aez cell
			}	// end if aggregating to fao country values
			
		}	// end for cellind loop over sage land cells
		
	}	// end for cropind loop over sage crops
    
	// most efficient way to recalibrate area and yield is to now loop over the crops,
	//  then the land cells twice within the crop loop
	//   first to recalibrate area and calculate a new production sum
	//   second to recalibrate the yield and calculate the output production
	
	// recalibration is done at the pixel level, but only for output ctryXglu pixels
	// area and yield values will be zero if no fao country and glu are associated with the cell
	if (in_args.out_year_prod_ha_lr != 0) {

		temp_flt = (float) modf(RECALIB_AVG_PERIOD / 2, &temp_dbl);
		start_recalib_year = in_args.out_year_prod_ha_lr - (int) temp_dbl;
		
		// match the fao data year to the start recalib data year
		// to get the fao year index for the first averaging year
		fao_start_year_index = NOMATCH;
		for (i = 0; i < NUM_FAO_YRS; i++) {
			if ((FAO_START_YEAR + i) == start_recalib_year) {
				fao_start_year_index = i;
				break;
			}
		} // end for i loop over fao years
		if (fao_start_year_index == NOMATCH) {
			fprintf(fplog,"Recalibrate: Failed to find start FAO data for year %i:  calc_harvarea_prod_out_aez()\n", start_recalib_year);
			return ERROR_IND;
		}
		
		// allocate recalib area and yield arrays
		area_recalib = calloc(NUM_CELLS, sizeof(float));
		if(area_recalib == NULL) {
			fprintf(fplog,"Recalibrate: Failed to allocate memory for area_recalib:  calc_harvarea_prod_out_aez()\n");
			return ERROR_MEM;
		}
		yield_recalib = calloc(NUM_CELLS, sizeof(float));
		if(yield_recalib == NULL) {
			fprintf(fplog,"Recalibrate: Failed to allocate memory for yield_recalib:  calc_harvarea_prod_out_aez()\n");
			return ERROR_MEM;
		}
		
		// need to zero the output production and harvest area arrays
		for (i = 0; i < NUM_FAO_CTRY; i++) {
			for (j = 0; j < ctry_aez_num[i]; j++) {
    			for (k = 0; k < NUM_SAGE_CROP; k++) {
					production_crop_aez[i][j][k] = 0;
					harvestarea_crop_aez[i][j][k] = 0;
				}
			}
		}
		
		// need to zero the diagnostic production and harvest area arrays
		for (i = 0; i < NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_NEW_AEZ; i++) {
			diag_production_crop_aez[i] = 0;
			diag_harvestarea_crop_aez[i] = 0;
		}
		
		// loop over crops, then cells, so that only two raster loops are needed per crop
		// to do: write the recalibrated area and yield data for each crop
		for (cropind = 0; cropind < NUM_SAGE_CROP; cropind++) {
			
			// read in yield and harvest area, again
			// file units are converted from t/ha to t/km^2 and from fraction of land area to km^2
			// this function ensures that valid yield and area values exist for sage land cells
			strcpy(fname, in_args.sagepath);
			strcat(fname, &cropfilebase_sage[cropind][0]); // the read function will determine whether the file is zipped or not
			if ((err = read_sage_crop(fname, in_args.sagepath, &cropfilebase_sage[cropind][0], raster_info))) {
				fprintf(fplog, "Failed to read yield and area for crop %s: calc_harvarea_prod_out_aez()\n", fname);
				return err;
			}
			
			// area recalibration loop
			for (cellind = 0; cellind < num_land_cells_sage; cellind++) {
				land_cell = land_cells_sage[cellind];
				area_recalib[land_cell] = 0;
				// fao country index
				if ((int) country_fao[land_cell] != raster_info.country_fao_nodata) {
					ctry_index = NOMATCH;
					for (i = 0; i < NUM_FAO_CTRY; i++) {
						if (countrycodes_fao[i] == (int) country_fao[land_cell]) {
							ctry_index = i;
							break;
						}
					}	// end for i loop over fao ctry to find fao index
				} else {
					//fprintf(fplog, "No country exists for this cell: calc_harvarea_prod_out_aez(); cellind = %i\n", cellind);
					continue;	// no country associated with these data so don't use this cell and go to the next one
				}	// end if fao country else no country
				
				if (ctry_index == NOMATCH) {
					fprintf(fplog, "Recalibrate: Error determining fao country index: calc_harvarea_prod_out_aez(); cellind = %i\n", cellind);
					return ERROR_CALC;
                } else {
                    // aggregate to country and aez
                    
					// get the glu number; this function retrieves the nodata value if no associated glu is found
					// do not use this cell data if there is no associated glu
					if ((err = get_aez_val(aez_bounds_new, land_cell, raster_info.aez_new_nrows,
										   raster_info.aez_new_ncols, raster_info.aez_new_nodata, &aez_val))) {
						fprintf(fplog, "Recalibrate: Failed to get aez_val for crop %s: calc_harvarea_prod_out_aez()\n", fname);
						return err;
					}
					
					if (aez_val != raster_info.aez_new_nodata) {
                        
                        // use only cells with positve values for area and yield
                        if (harvestarea_in[land_cell] > 0 && yield_in[land_cell] > 0) {
							
							if (cropind == 5 && country_fao[land_cell] == 231) {
								// maize in usa
								;
							}
							
                            // the fao input data may require a different index than the output data
                            // for example, merging serbia and montenegro
                            temp_index = ctry_index;
                            
                            // data for serbia and montenegro need to be merged for processing
                            // the fao data is separate for these for years > 2005
                            if (countrycodes_fao[ctry_index] == serbia_code || countrycodes_fao[ctry_index] == montenegro_code) {
                                for (i = 0; i < NUM_FAO_CTRY; i++) {
                                    if (countrycodes_fao[i] == scg_code) {
                                        ctry_index = i;
                                        break;
                                    }
                                }
                            }
                            
                            // this average over years inefficient
                            // first get the fao area values; average over years if desired
                            // production is not weighted by area
                            // keep track of the number of years where there are values
                            // if there are no fao values, then clear the harvestarea_in value
                            recal_index = ctry_index * NUM_SAGE_CROP + cropind;
                            harvest_val_fao = 0;
                            num_yrs = 0;

                            for (i = 0; i < RECALIB_AVG_PERIOD; i++) {
                                // serbia and montenegro need to be merged for processing
                                // the fao data is separate for these for years > 2005
                                if (countrycodes_fao[ctry_index] == serbia_code || countrycodes_fao[ctry_index] == montenegro_code) {
                                    if (i <= scg_lastyear_index) {
                                        // read the merged fao data
                                        in_ctry_index = ctry_index;
                                    } else {
                                        // read the separate fao data
                                        in_ctry_index = temp_index;
                                    }
								} else {
									in_ctry_index = ctry_index;
								}
                                prod_index = in_ctry_index * NUM_SAGE_CROP * NUM_FAO_YRS + cropind * NUM_FAO_YRS + i + fao_start_year_index;
                                if (harvestarea_fao[prod_index] != 0) {
                                    harvest_val_fao = harvest_val_fao + harvestarea_fao[prod_index];
                                    num_yrs = num_yrs + 1;
                                }
                            } // end for i loop over average period
							
                            if (num_yrs != 0) {
                                harvest_val_fao = harvest_val_fao / num_yrs;
                            }
                            
                            // now recalibrate the harvest area and recalculate production
                            // but first check the denominator for abnormally low values (< 100 m^2)
                            if (country_harvarea[recal_index] != 0) {
                                if (country_harvarea[recal_index] < 0.0001) {
                                    fprintf(fplog, "Recalibrate: Bad country_harvarea[%i] = %e value at ctry_index = %i and cropind = %i: calc_harvarea_prod_out_aez()\n",
                                            recal_index, country_harvarea[recal_index], ctry_index, cropind);
                                    area_recalib[land_cell] = 0;
                                } else {
                                    area_recalib[land_cell] = harvestarea_in[land_cell] * harvest_val_fao / country_harvarea[recal_index];
                                    country_prod[recal_index] = country_prod[recal_index] +
                                    area_recalib[land_cell] * yield_in[land_cell];
                                }
                            } else {
                                area_recalib[land_cell] = 0;
                            }
                            
                            // get the current aez index in the complete aez list
                            all_aez_index = NOMATCH;
                            for (i = 0; i < NUM_NEW_AEZ ; i++) {
                                if (aez_codes_new[i] == aez_val) {
                                    all_aez_index = i;
                                    break;
                                }
                            }
                            if (all_aez_index == NOMATCH) {
                                fprintf(fplog, "Failed to get all_aez_index for crop %s for area recalib: calc_harvarea_prod_out_aez()\n", fname);
                                return err;
                            }
                            
                            // get the current aez index in the country aez list
                            aez_index = NOMATCH;
                            for (i = 0; i < ctry_aez_num[ctry_index]; i++) {
                                if (ctry_aez_list[ctry_index][i] == aez_val) {
                                    aez_index = i;
                                    break;
                                }
                            }
                            if (aez_index == NOMATCH) {
                                fprintf(fplog, "Failed to get aez_index for crop %s for area recalib: calc_harvarea_prod_out_aez()\n", fname);
                                return err;
                            }
                            
                            // now recalculate the output harvest area
                            // aggregate to fao country and aez
                            harvestarea_crop_aez[ctry_index][aez_index][cropind] =
                                harvestarea_crop_aez[ctry_index][aez_index][cropind] +
                                KMSQ2HA * area_recalib[land_cell];
                            
                            if (harvestarea_crop_aez[ctry_index][aez_index][cropind] < 0 ||
                                harvestarea_crop_aez[ctry_index][aez_index][cropind] > 30000000) {
                                fprintf(fplog, "Recalibrate: Bad harvestarea_crop_aez = %f output at ctry_index = %i and aez_index = %i and cropind = %i: calc_harvarea_prod_out_aez()\n",
                                        harvestarea_crop_aez[ctry_index][aez_index][cropind], ctry_index, aez_index, cropind);
                            }
                            
                            // fill the 1d array
                            diag_index = ctry_index * NUM_SAGE_CROP * NUM_NEW_AEZ + cropind * NUM_NEW_AEZ + all_aez_index;
                            diag_harvestarea_crop_aez[diag_index] = diag_harvestarea_crop_aez[diag_index] +
                            KMSQ2HA * area_recalib[land_cell];
                            
                        } // end if area and yield are both positve values for this cell
						
					}	// end if valid aez cell for area
					
				}	// end if fao country for recalibration of area
				
			}	// end for cellind loop to recalibrate area
			
			// now loop again to recalibrate the yields and calculate the output production
			for (cellind = 0; cellind < num_land_cells_sage; cellind++) {
				land_cell = land_cells_sage[cellind];
				yield_recalib[land_cell] = 0;
				// fao country index
				if ((int) country_fao[land_cell] != raster_info.country_fao_nodata) {
					ctry_index = NOMATCH;
					for (i = 0; i < NUM_FAO_CTRY; i++) {
						if (countrycodes_fao[i] == (int) country_fao[land_cell]) {
							ctry_index = i;
							break;
						}
					}	// end for i loop over fao ctry to find fao index
				} else {
					//fprintf(fplog, "No country exists for this cell: calc_harvarea_prod_out_aez(); cellind = %i\n", cellind);
					continue;	// no country associated with these data so don't use this cell and go to the next one
				}	// end if fao country else no country
				
				if (ctry_index == NOMATCH) {
					fprintf(fplog, "Recalibrate: Error determining fao country index: calc_harvarea_prod_out_aez(); cellind = %i\n", cellind);
					return ERROR_CALC;
                } else {
					// get the aez number; this function retrieves the nodata value if no associated aez is found
					// do not use this cell data if there is no associated aez
					if ((err = get_aez_val(aez_bounds_new, land_cell, raster_info.aez_new_nrows,
										   raster_info.aez_new_ncols, raster_info.aez_new_nodata, &aez_val))) {
						fprintf(fplog, "Recalibrate: Failed to get aez_val for crop %s: calc_harvarea_prod_out_aez()\n", fname);
						return err;
					}
					if (aez_val != raster_info.aez_new_nodata) {
                        
                        // do this only if the area and yield are positive for this cell
                        if (area_recalib[land_cell] > 0 && yield_in[land_cell] > 0) {
							
							if (cropind == 5 && country_fao[land_cell] == 231) {
								// maize in usa
								;
							}
							
							// the fao input data may require a different index than the output data
							// for example, merging serbia and montenegro
							temp_index = ctry_index;
							
							// data for serbia and montenegro need to be merged for processing
							// the fao data is separate for these for years > 2005
							if (countrycodes_fao[ctry_index] == serbia_code || countrycodes_fao[ctry_index] == montenegro_code) {
								for (i = 0; i < NUM_FAO_CTRY; i++) {
									if (countrycodes_fao[i] == scg_code) {
										ctry_index = i;
										break;
									}
								}
							}
							
							// this average over years inefficient
							// first get the fao area values; average over years if desired
							// production is not weighted by area
							// keep track of the number of years where there are values
							// if there are no fao values, then clear the production_in value
							recal_index = ctry_index * NUM_SAGE_CROP + cropind;
							prod_val_fao = 0;
							num_yrs = 0;
							
							for (i = 0; i < RECALIB_AVG_PERIOD; i++) {
								// serbia and montenegro need to be merged for processing
								// the fao data is separate for these for years > 2005
								if (countrycodes_fao[ctry_index] == serbia_code || countrycodes_fao[ctry_index] == montenegro_code) {
									if (i <= scg_lastyear_index) {
										// read the merged fao data
										in_ctry_index = ctry_index;
									} else {
										// read the separate fao data
										in_ctry_index = temp_index;
									}
								} else {
									in_ctry_index = ctry_index;
								}
								prod_index = in_ctry_index * NUM_SAGE_CROP * NUM_FAO_YRS + cropind * NUM_FAO_YRS + i + fao_start_year_index;
								if (production_fao[prod_index] != 0) {
									prod_val_fao = prod_val_fao + production_fao[prod_index];
									num_yrs = num_yrs + 1;
								}
							}
							if (num_yrs != 0) {
								prod_val_fao = prod_val_fao / num_yrs;
							}
							
							// now recalibrate the yield
							// but first check for abnormally low values in the denominator
							// this treshold is based on 0.1 t / km^2, or 0.001 t / ha, (min fao value is ~0.02 t / ha)
							// so it is 0.1 t / km^2 * 1 km^2 (which is the ~ size of one grid cell at 89deglat) = 0.1 t
							if (country_prod[recal_index] != 0) {
								if (country_prod[recal_index] < 0.1) {
									fprintf(fplog, "Recalibrate: Bad country_prod[recal_index][%i] = %e value at ctry_index = %i and cropind = %i: calc_harvarea_prod_out_aez()\n",
											recal_index, country_prod[recal_index], ctry_index, cropind);
									yield_recalib[land_cell] = 0;
								} else {
									yield_recalib[land_cell] = yield_in[land_cell] * prod_val_fao / country_prod[recal_index];
								}
							} else {
								yield_recalib[land_cell] = 0;
							}
							
							// get the current aez index in the complete aez list
							all_aez_index = NOMATCH;
							for (i = 0; i < NUM_NEW_AEZ ; i++) {
								if (aez_codes_new[i] == aez_val) {
									all_aez_index = i;
									break;
								}
							}
							if (all_aez_index == NOMATCH) {
								fprintf(fplog, "Failed to get all_aez_index for crop %s for area recalib: calc_harvarea_prod_out_aez()\n", fname);
								return err;
							}
							
							// get the current aez index in the country aez list
							aez_index = NOMATCH;
							for (i = 0; i < ctry_aez_num[ctry_index]; i++) {
								if (ctry_aez_list[ctry_index][i] == aez_val) {
									aez_index = i;
									break;
								}
							}
							if (aez_index == NOMATCH) {
								fprintf(fplog, "Failed to get aez_index for crop %s for area recalib: calc_harvarea_prod_out_aez()\n", fname);
								return err;
							}
							
							// now recalculate the output production
							// aggregate to fao country and aez
							
							production_crop_aez[ctry_index][aez_index][cropind] =
							production_crop_aez[ctry_index][aez_index][cropind] +
							area_recalib[land_cell] * yield_recalib[land_cell];
							
							// this condition is not hit with the calibration to 2003-2007 avg annual values
							// even without the preceding filter
							if (production_crop_aez[ctry_index][aez_index][cropind] < 0 ||
								production_crop_aez[ctry_index][aez_index][cropind] > 200000000) {
								fprintf(fplog, "Recalibrate: Bad production_crop_aez = %f output at ctry_index = %i and aez_index = %i and cropind = %i: calc_harvarea_prod_out_aez()\n",
										production_crop_aez[ctry_index][aez_index][cropind], ctry_index, aez_index, cropind);
							}
							
							// fill the 1d array
							diag_index = ctry_index * NUM_SAGE_CROP * NUM_NEW_AEZ + cropind * NUM_NEW_AEZ + all_aez_index;
							diag_production_crop_aez[diag_index] = diag_production_crop_aez[diag_index] +
							area_recalib[land_cell] * yield_recalib[land_cell];
							
						} // end if area and yield are both positive for this cell
						
					}	// end if valid aez cell for production/yield
					
				}	// end if fao country for recalibration of production/yield
				
			}	// end for cellind loop to recalibrate production/yield
			
			
			// to do: this is where each recalibrated crop harvested area and yield can be written
			if (cropind == 5) {
				// maize
				;
			}
		}	// end for cropind for area and production recalibration
		
		free(area_recalib);
		free(yield_recalib);
	}	// end if recalibrate
	
    // write the lost info to the log file
    fprintf(fplog, "Discarded data (sqkm and t/sqkm): calc_harvarea_prod_out_crop_aez()\n");
    for (i = 0; i < NUM_SAGE_CROP; i++) {
        fprintf(fplog, "%s: lost_harvested_area=%f; mismatched_harvested_area=%f; mismatched_yield=%f\n",
                cropnames_gtap[i], lost_harvested_area[i], mismatched_harvested_area[i],
                mismatched_yield[i] / mismatched_yield_count[i]);
    }
	
	if (in_args.diagnostics) {
		// this is the diagnostic output for the missing aez mask
		if ((err = write_raster_int(missing_aez_mask, ncells, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_harvarea_prod_out_crop_aez()\n", out_name);
			return err;
		}
		
		// write the sage production and harvest area and pasture area by fao country, crop, and aez
		if ((err = write_csv_float3d(diag_production_crop_aez, countrycodes_fao, cropcodes_sage,
									 NUM_FAO_CTRY, NUM_SAGE_CROP, NUM_NEW_AEZ, out_name_prod, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_harvarea_prod_out_crop_aez()\n", out_name_prod);
			return err;
		}
		if ((err = write_csv_float3d(diag_harvestarea_crop_aez, countrycodes_fao, cropcodes_sage,
									 NUM_FAO_CTRY, NUM_SAGE_CROP, NUM_NEW_AEZ, out_name_harv, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_harvarea_prod_out_crop_aez()\n", out_name_harv);
			return err;
		}
		if ((err = write_csv_float2d(diag_pasturearea_aez, countrycodes_fao,
									 NUM_FAO_CTRY, NUM_NEW_AEZ, out_name_past, in_args))) {
			fprintf(fplog, "Error writing file %s: calc_harvarea_prod_out_crop_aez()\n", out_name_past);
			return err;
		}
		
		// ctryXaez output land mask
		if ((err = write_raster_int(land_mask_ctryaez, NUM_CELLS, "land_mask_ctryaez.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: calc_harvarea_prod_out_crop_aez()\n", "land_mask_ctryaez.bil");
			return err;
		}
	}
	
    free(diag_production_crop_aez);
    free(diag_harvestarea_crop_aez);
    free(diag_pasturearea_aez);
    
	return OK;
}

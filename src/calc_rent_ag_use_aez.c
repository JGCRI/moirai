/**********
 calc_rent_ag_use_aez.c
 
 calculate the agricultural (including livestock) land rent values for the new aezs (million USD)
 
 arguments:
 args_struct in_args:	input argument structure
 rinfo_struct raster_info: information about input raster data
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 note: the Lee 2005 gtap report describes adjustments to Belgium, Luxembourg, and Japan land rents
	that are not described in the 2007 chapter, and do not have to be addressed here because
 1) Belgium and Luxembourg now have crop land rents
 2) Japan's pfb sector land rent is zero in the input gtap original land rent file
 
 note: hong kong and taiwan are still treated specially (they use vietnam shares) because that was the original method
 hong kong needs to be treated specially because it does not have production values and harvested area values
 actually, check this now
	there are taiwan production values and harvested area values, so taiwan could be calculated on its own
 
 13 march 2015
 fixed the livestock land rent calc
 the only changes in the genaez outputs were to the 3 livestock sector land rents
	it improved the reproduction of GTAP data a bit
 this did not change the presented GCAM outputs except for small changes in global luc emissions on the order of 1/100 of a percent in 2100 and slightly larger changes in regional emissions (for 14 and 32 regions)
 
 Created by Alan Di Vittorio on 21 Nov 2013
 Completed May/June 2014 by Alan Di Vittorio
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "moirai.h"

int calc_rent_ag_use_aez(args_struct in_args, rinfo_struct raster_info) {
    
    int i,j,k,m;
    int aez_ind, aez_ind_reglr, crop_ind, use_ind, sum_index, ctry_ind, reglr_ind, out_index, all_aez_index;	// loop and placement indices
    // find these based on the codes below
    int vnm_ind = NOMATCH;		// vietnam land rent region index
    int hkg_ind = NOMATCH;		// hong kong land rent region index
    int twn_ind = NOMATCH;		// taiwan land rent region index
    
    // these are the land rent region codes for the alphabetical iso/abbr list
    int vnm_code = 66;		// vietnam ctry87 index (code minus 1)
    int hkg_code = 25;		// hong kong ctry87 index  (code minus 1)
    int twn_code = 60;		// taiwan ctry87 index  (code minus 1)
    
    int vnm_sum_ind;        // for holding the vietnam indices
    int diag_index;         // for a couple of 1d output diagnostic arrays
    int gro_sect = 3;		// the use code for the grain sector
    int ctl_sect = 9;		// the use code for the cattle, sheep, etc sector
    int rmk_sect = 11;		// the use code for the dairy sector
    int wol_sect = 12;		// the use code for the animal fiber sector
    int err = OK;
    
    float temp_float;		// temporary storage
    float *value_sum;		// store the land values per ctry87, per use sector
    float *origrent87;		// store the original rent summed across aezs (i.e. per ctry87, per use sector)
    float *newrent87;		// store the new rent summed across aezs (i.e. per ctry87, per use sector)
    float **harvestsum;		// sum for averaging yield and price to gro sector and land rent region per aez (ha)
    float **pasture87_aez;	// pasture area per aez per land rent region (ha)
    float *lrout;			// for diagnostic output in USD
    float *orout;			// for diagnostic output in USD
    float *nrout;			// for diagnostic output in USD
    
    // for now, use old-format 1d arrays for these diagnostic outputs
    // aez varies faster, then land rent region
    // to do: check for gridded new aez input
    float *diag_harvestsum;		// sum for averaging yield and price to gro sector and land rent region per aez (ha)
    float *diag_pasture87_aez;	// pasture area per aez per land rent region (ha)
    
    char out_name[] = "rent_use_aez_ag.csv";				// diagnostic output csv file name
    char out_name_past[] = "pasturearea87_aez.csv";		// diagnostic output for the aggregated pasture area
    
    
    // these need to be initialized with zeroes, so use pointers and calloc
    value_sum = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE, sizeof(float));
    if(value_sum == NULL) {
        fprintf(fplog,"Failed to allocate memory for value_sum:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    origrent87 = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE, sizeof(float));
    if(origrent87 == NULL) {
        fprintf(fplog,"Failed to allocate memory for origrent87:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    newrent87 = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE, sizeof(float));
    if(newrent87 == NULL) {
        fprintf(fplog,"Failed to allocate memory for newrent87:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    harvestsum = calloc(NUM_GTAP_CTRY87, sizeof(float*));
    if(harvestsum == NULL) {
        fprintf(fplog,"Failed to allocate memory for harvestsum:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    pasture87_aez = calloc(NUM_GTAP_CTRY87, sizeof(float*));
    if(pasture87_aez == NULL) {
        fprintf(fplog,"Failed to allocate memory for pasture87_aez:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        harvestsum[i] = calloc(reglr_aez_num[i], sizeof(float));
        if(harvestsum[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for harvestsum[%i]:  calc_rent_ag_use_aez()\n", i);
            return ERROR_MEM;
        }
        pasture87_aez[i] = calloc(reglr_aez_num[i], sizeof(float));
        if(pasture87_aez[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for pasture87_aez[%i]:  calc_rent_ag_use_aez()\n", i);
            return ERROR_MEM;
        }
    }
    // allocate memory for the diagnostic output
    lrout = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE * NUM_NEW_AEZ, sizeof(float));
    if(lrout == NULL) {
        fprintf(fplog,"Failed to allocate memory for lrout:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    orout = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE, sizeof(float));
    if(orout == NULL) {
        fprintf(fplog,"Failed to allocate memory for orout:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    nrout = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE, sizeof(float));
    if(nrout == NULL) {
        fprintf(fplog,"Failed to allocate memory for nrout:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    diag_harvestsum = calloc(NUM_GTAP_CTRY87 * NUM_NEW_AEZ, sizeof(float));
    if(diag_harvestsum == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_harvestsum:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    diag_pasture87_aez = calloc(NUM_GTAP_CTRY87 * NUM_NEW_AEZ, sizeof(float));
    if(diag_pasture87_aez == NULL) {
        fprintf(fplog,"Failed to allocate memory for diag_pasture87_aez:  calc_rent_ag_use_aez()\n");
        return ERROR_MEM;
    }
    
    // get the indices of vietnam, hong kong, and taiwan
    // there shouldn't be a failure here, unless the codes above are wrong
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        if (country87codes_gtap[i] == hkg_code) {
            hkg_ind = i;
        } else if (country87codes_gtap[i] == twn_code) {
            twn_ind = i;
        } else if (country87codes_gtap[i] == vnm_code) {
            vnm_ind = i;
        }
    }
    if (hkg_ind == NOMATCH || twn_ind == NOMATCH || vnm_ind == NOMATCH) {
        fprintf(fplog,"Error finding an index: hkg_ind = %i, twn_ind = %i, vnm_ind = %i:  calc_rent_ag_use_aez()\n",
                hkg_ind, twn_ind, vnm_ind);
        return ERROR_IND;
    }
    
    // loop over sage crops
    for (crop_ind = 0; crop_ind < NUM_SAGE_CROP; crop_ind++) {
        // get the gtap use index for this crop
        use_ind = NOMATCH;
        for (i = 0; i < NUM_GTAP_USE; i++) {
            if (crop_sage2gtap_use[crop_ind] == usecodes_gtap[i]) {
                use_ind = i;
                break;
            }
        }
        if (use_ind == NOMATCH) {
            // this should not happen
            fprintf(fplog,"Failed to find use_ind for crop %i:  calc_rent_ag_use_aez()\n", cropcodes_sage[crop_ind]);
            return ERROR_IND;
        }
        // loop over the fao countries to calculate appropriate land values per ctry87 and use sector
        for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY; ctry_ind++) {
            
            // get the land rent region index
            reglr_ind = NOMATCH;
            for (i = 0; i < NUM_GTAP_CTRY87; i++) {
                if (country87codes_gtap[i] == ctry2ctry87codes_gtap[ctry_ind]) {
                    reglr_ind = i;
                    break;
                }
            }	// end for i loop to get reglr_ind
            
            // skip this fao country because it is not part of an economic region and thus not processed for output
            if(reglr_ind == NOMATCH) {
				if (in_args.diagnostics){
                	fprintf(fplog,"Warning: Failed to find land rent region index for fao country %i:  calc_rent_ag_use_aez()\n", countrycodes_fao[ctry_ind]);
				}
                continue;
            }
            
            // loop over the country aezs
            for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
                
                // get the index for this aez in the land rent region
                aez_ind_reglr = NOMATCH;
                for (i = 0; i < reglr_aez_num[reglr_ind]; i++) {
                    if (ctry_aez_list[ctry_ind][aez_ind] == reglr_aez_list[reglr_ind][i]) {
                        aez_ind_reglr = i;
                        break;
                    }
                }	// end for i loop to get aez_ind_reglr
                
                // this should not happen
                if(aez_ind_reglr == NOMATCH) {
					fprintf(fplog,"Error: Failed to find aez index for land rent region %i, country %i:  calc_rent_ag_use_aez()\n", country87codes_gtap[reglr_ind], countrycodes_fao[ctry_ind]);
                    return ERROR_IND;
                }
                
                // sum index for reglr_ind and use sector use_ind
                sum_index = reglr_ind * NUM_GTAP_USE + use_ind;
                
                // calc the index of the output production data and calculate the land value sums
                k = reglr_ind * NUM_SAGE_CROP + crop_ind;
                temp_float = production_crop_aez[ctry_ind][aez_ind][crop_ind] * prodprice_fao_reglr[k];
                rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] = rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] + temp_float;
                value_sum[sum_index] = value_sum[sum_index] + temp_float;
                
                /* some debugging stuff
                 if (country87codes_gtap[reglr_ind] == 64 && cropcodes_sage[crop_ind] == 16 && production_crop_aez[j] != 0) {
                 fprintf(fplog,"found usa wheat aez:  calc_rent_ag_use_aez()\n");
                 }
                 
                 if(reglr_ind == 0 && use_ind == 0) {
                 fprintf(fplog, "Checking ctry87 1 and use 6; ctry_ind=%i, crop_ind=%i, aez_ind=%i: calc_rent_ag_use_aez()\n",
                 ctry_ind, crop_ind, aez_ind);
                 fprintf(fplog, "\tproduction_crop_aez[%i]=%.2f\n\tprodpricegtap87[%i]=%.2f\n\ttemp_float=%.2f\n\tvalue_sum[%i]=%.2f\n",j, production_crop_aez[j], k, prodprice_fao_reglr[k], temp_float, sum_index, value_sum[sum_index]);
                 }
                 
                 if (isnan(value_sum[sum_index])) {
                 fprintf(fplog, "Error: nan in value_sum[%i]; ctry_ind=%i, crop_ind=%i, aez_ind=%i: calc_rent_ag_use_aez()\n",
                 sum_index, ctry_ind, crop_ind, aez_ind);
                 }
                 */
                
                // get the aez index in the complete aez list
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
                    return ERROR_FILE;
                }
                
                // ruminant sectors (ctl, rmk, wol)
                // get the average grain sector (gro) yield from production / harvested area - area weighted!
                // get the avearge grain sector (gro) prodprice - production weighted!
                //  these averages turn the final calc into the temporary rent_use_aez weighted by ha-pasture/ha-raez
                if (gro_sect == usecodes_gtap[use_ind]) {
                    // use only crops that have data for both price and yield
                    if(prodprice_fao_reglr[k] != 0 && production_crop_aez[ctry_ind][aez_ind][crop_ind] != 0) {
                        harvestsum[reglr_ind][aez_ind_reglr] =
                        harvestsum[reglr_ind][aez_ind_reglr] + harvestarea_crop_aez[ctry_ind][aez_ind][crop_ind];
                        
                        // fill the 1d array
                        diag_index = reglr_ind * NUM_NEW_AEZ + all_aez_index;
                        diag_harvestsum[diag_index] = diag_harvestsum[diag_index] +
                            harvestarea_crop_aez[ctry_ind][aez_ind][crop_ind];
                    }
                } // end if gro sector
                
                // aggregate pasture area to ctry87; do this for only one crop index
                if (crop_ind == 0) {
                    pasture87_aez[reglr_ind][aez_ind_reglr] =
                    pasture87_aez[reglr_ind][aez_ind_reglr] + pasturearea_aez[ctry_ind][aez_ind];
                    
                    // fill the 1d array
                    diag_index = reglr_ind * NUM_NEW_AEZ + all_aez_index;
                    diag_pasture87_aez[diag_index] = diag_pasture87_aez[diag_index] +
                        pasturearea_aez[ctry_ind][aez_ind];
                }
                
            }	// end for loop over country aezs
            
        }	// end for loop over fao ctry_ind
        
    }	// end for loop over sage crop_ind
    
    // get the use_ind for the grain sector
    m = -1;
    for (i = 0; i < NUM_GTAP_USE; i++) {
        if (gro_sect == usecodes_gtap[i]) {
            m = i;
            break;
        }
    }	// end for i loop to get grain sector use_ind
    
    // now loop over the use sectors and land rent regions to aggregate the original rent across aezs
    // and also to finish preparing the livestock sectors for calculating the final land rents
    for (use_ind = 0; use_ind < NUM_GTAP_USE; use_ind++) {
        for (reglr_ind = 0; reglr_ind < NUM_GTAP_CTRY87; reglr_ind++) {
            // first loop over the original AEZs
            for (i = 0; i < NUM_ORIG_AEZ; i++) {
                j = reglr_ind * NUM_GTAP_USE * NUM_ORIG_AEZ + use_ind * NUM_ORIG_AEZ + i;
                out_index = reglr_ind * NUM_GTAP_USE + use_ind;
                origrent87[out_index] = origrent87[out_index] + rent_orig_aez[j];
            }
            
            // now loop over the new aezs
            for (aez_ind_reglr = 0; aez_ind_reglr < reglr_aez_num[reglr_ind]; aez_ind_reglr++) {
                
                // cattle, dairy, wool sectors
                if (ctl_sect == usecodes_gtap[use_ind] || rmk_sect == usecodes_gtap[use_ind] ||
                    wol_sect == usecodes_gtap[use_ind]) {
                    // the pasture index does not vary across sector, and neither does the gro source data
                    if (harvestsum[reglr_ind][aez_ind_reglr] == 0) {
                        temp_float = 0;
                    }else {
                        // adjust the gro sector value by the pasture to grain area ratio
                        temp_float = rent_use_aez[reglr_ind][aez_ind_reglr][m] *
                        pasture87_aez[reglr_ind][aez_ind_reglr] / harvestsum[reglr_ind][aez_ind_reglr];
                    }
                    
                    // get the indices and calculate the values
                    sum_index = reglr_ind * NUM_GTAP_USE + use_ind;
                    rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] = temp_float;
                    value_sum[sum_index] = value_sum[sum_index] + temp_float;
                }
                
            } // end for loop over aez_ind_reglr for more livestock sector prep
        }	// end for loop over gtap reglr_ind for spatial aggregation of orig rent and livestock prep
    }	// end for loop over gtap use_ind for spatial aggregation of orig rent and livestock prep
    
    
    // now calculate the final land rent values (see eq.1, section 2.2.3.2 of the lee et al 2005 gtap lu document)
    // or the Di Vittorio et al 2016 paper on gcam land resource projection uncertainty
    // no need to convert rent_use_aez and value_sum to million USD because the conversion cancels out
    for (reglr_ind = 0; reglr_ind < NUM_GTAP_CTRY87; reglr_ind++) {
        for (use_ind = 0; use_ind < NUM_GTAP_USE; use_ind++) {
            for (aez_ind_reglr = 0; aez_ind_reglr < reglr_aez_num[reglr_ind]; aez_ind_reglr++) {
                
                // if hong kong or taiwan, use vietnam rent_use_aez and value_sum, but respective origrent87 values
                // if production is zero elsewhere, the numerator will be zero, so the land rent will be zero
                //	but need to check the denominator
                // throw warnings in the log file to see where this happens
                
                j = reglr_ind * NUM_GTAP_USE + use_ind;
                if (reglr_ind == hkg_ind || reglr_ind == twn_ind) {
                    //vnm_rent_ind = vnm_ind * NUM_GTAP_USE * NUM_NEW_AEZ + use_ind * NUM_NEW_AEZ + aez_ind_reglr;
                    vnm_sum_ind = vnm_ind * NUM_GTAP_USE + use_ind;
                    // here the original for the country is split based on vietnam shares
                    if (value_sum[vnm_sum_ind] == 0) {
                        rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] = 0;
                    }else {
                        rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] =
                        rent_use_aez[vnm_ind][aez_ind_reglr][use_ind] * origrent87[j] / value_sum[vnm_sum_ind];
                    }
                }else {
                    if (value_sum[j] == 0) {
                        rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] = 0;
                    }else {
                        rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] =
                        rent_use_aez[reglr_ind][aez_ind_reglr][use_ind] * origrent87[j] / value_sum[j];
                    }
                }
                
                // add up the new rent across aez to land rent region for diagnostics
                newrent87[j] = newrent87[j] + rent_use_aez[reglr_ind][aez_ind_reglr][use_ind];
                
                // get the aez index in the complete aez list
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
                    return ERROR_FILE;
                }
                
                // convert origrent87, newrent87, and rent_use_aez to USD for diagnostic output
                out_index = reglr_ind * NUM_GTAP_USE * NUM_NEW_AEZ + use_ind * NUM_NEW_AEZ + all_aez_index;
                lrout[out_index] = MIL2ONE * rent_use_aez[reglr_ind][aez_ind_reglr][use_ind];
                nrout[j] = MIL2ONE * newrent87[j];
                orout[j] = MIL2ONE * origrent87[j];
                
            }	// end for aez_ind_reglr loop to calculate final land rent values
        }	// end for use_ind loop to calculate final land rent values
    }	// end for reglr_ind loop to calculate final land rent values
    
    if (in_args.diagnostics) {
        if ((err = write_csv_float3d(lrout, country87codes_gtap, usecodes_gtap,
                                     NUM_GTAP_CTRY87, NUM_GTAP_USE, NUM_NEW_AEZ, out_name, in_args))) {
            fprintf(fplog, "Error writing file %s: calc_rent_ag_use_aez()\n", out_name);
            return err;
        }
        if ((err = write_csv_float2d(diag_pasture87_aez, country87codes_gtap,
                                     NUM_GTAP_CTRY87, NUM_NEW_AEZ, out_name_past, in_args))) {
            fprintf(fplog, "Error writing file %s: calc_rent_ag_use_aez()\n", out_name_past);
            return err;
        }
        if ((err = write_csv_float2d(orout, country87codes_gtap,
                                     NUM_GTAP_CTRY87, NUM_GTAP_USE, "origrent87.csv", in_args))) {
            fprintf(fplog, "Error writing file %s: calc_rent_ag_use_aez()\n", "origrent87.csv");
            return err;
        }
        if ((err = write_csv_float2d(nrout, country87codes_gtap,
                                     NUM_GTAP_CTRY87, NUM_GTAP_USE, "newrent87.csv", in_args))) {
            fprintf(fplog, "Error writing file %s: calc_rent_ag_use_aez()\n", "newrent87.csv");
            return err;
        }
        if ((err = write_csv_float2d(value_sum, country87codes_gtap,
                                     NUM_GTAP_CTRY87, NUM_GTAP_USE, "value_sum.csv", in_args))) {
            fprintf(fplog, "Error writing file %s: calc_rent_ag_use_aez()\n", "value_sum.csv");
            return err;
        }
        if ((err = write_csv_float2d(diag_harvestsum, country87codes_gtap,
                                     NUM_GTAP_CTRY87, NUM_NEW_AEZ, "harvestsum.csv", in_args))) {
            fprintf(fplog, "Error writing file %s: calc_rent_ag_use_aez()\n", "harvestsum.csv");
            return err;
        }
    }
    
    free(value_sum);
    free(origrent87);
    free(newrent87);
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        free(harvestsum[i]);
        free(pasture87_aez[i]);
    }
    free(harvestsum);
    free(pasture87_aez);
    free(lrout);
    free(orout);
    free(nrout);
    
    free(diag_harvestsum);
    free(diag_pasture87_aez);
    
    return OK;
}
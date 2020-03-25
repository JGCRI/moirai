/***********
 proc_refveg_carbon.c
 
 generate csv table of total reference vegetation carbon density (MgC/ha), based on HYDE_YEAR pot veg area
    by country and glu and land type category (pot veg only, but uses codes from LDS_land_types.csv)
 output table has 4 label columns and on value column
 iso, glu, land type category, carbon type (soil, or veg(includes roots)), value (MgC/ha)
 no zero value records
 rounded to integer
 order follows harvest area output, with iso alphabetically, then glu# in order, then land type category in order
 only countries with glus are written
 only countries mapped to ctry87 will have non-zero outputs
 
 source data are the
 the soil carbon for potential veg text file - ref veg has the same types as pot veg
    these are literature estimates of equilibrium c density for the SAGE pot veg cagtegories
 the potential veg carbon text file (shoots and roots)
    these are literature estimates of equilibrium c density for the SAGE pot veg cagtegories
 the protected image file
 the hyde land area data
 orginally hyde land area data were used
 the fao country data
 the glu raster data
 country iso mapping
 
 input units are kg/m^2 for carbon and km^2 for area
 output units are MgC/ha - rounded to nearest integer
 file names are built here and passed to the read functions
  
 diagnostic total global carbon values output to the log file are for ref veg area
 
 process only valid hyde land cells, as these are the source of land type area
 
 serbia and montenegro data are merged
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Alan Di Vittorio, 14 jan 2016
 
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
 
 ***********/

#include "moirai.h"

int proc_refveg_carbon(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the hyde land area data set determine the land cells to process
    
    int i, j, k = 0;
    int grid_ind;               // the index for looping over the raster grid
    int rv_ind;                 // the index of the current sage reference veg land type
    int err = OK;				// store error code from the read/write functions
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    //float* soil_carbon_grid;    // 1d array to store the soil carbon data; start up left corner, row by row; lon varies faster
    float *soil_carbon_sage;     // array to store the veg c values for the 15 sage pot veg types and 3 land use types
    float *veg_carbon_sage;     // array to store the veg c values for the 15 sage pot veg types and 3 land use types
    float global_soilc = 0;         // total pot veg soil carbon
    float global_vegc = 0;          // total pot veg veg carbon
    float outval_soilc;                 // for the output values
    float outval_vegc;                 // for the output values
    float temp_float;           // temporary float
    
    // output table as 4-d array
    float ****refveg_carbon_out;		// the potveg carbon out table; 4th dim is the two carbon density values and the area
    int soilc_ind = 0;                  // index in output array
    int vegc_ind = 1;                   // index in output array
    int area_ind = 2;                   // index in output array, but used only for averaging

    int rv_value;           // current ref veg value
    int aez_val;            // current glu value
    int ctry_code;          // current fao country code
    int aez_ind;            // current glu index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    int cur_lt_cat;             // current land type category
    int cur_lt_cat_ind;             // current land type category index
    int num_out_vals = 3;   // the number of values to output (soil c den, veg c den, area for averaging)
    int nrecords = 0;       // count # of records written
    
    char fname[MAXCHAR];        // current file name to write
    FILE *fpout;                // out file pointer
    float temp_frac;           //Create temporary fraction for protected areas
    
    // allocate arrays
    
    soil_carbon_sage = calloc(NUM_SAGE_PVLT, sizeof(float));
    if(soil_carbon_sage == NULL) {
        fprintf(fplog,"Failed to allocate memory for soil_carbon_sage: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    veg_carbon_sage = calloc(NUM_SAGE_PVLT, sizeof(float));
    if(veg_carbon_sage == NULL) {
        fprintf(fplog,"Failed to allocate memory for veg_carbon_sage: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    
    refveg_carbon_out = calloc(NUM_FAO_CTRY, sizeof(float***));
    if(refveg_carbon_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for refveg_carbon_out: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        refveg_carbon_out[i] = calloc(ctry_aez_num[i], sizeof(float**));
        if(refveg_carbon_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            refveg_carbon_out[i][j] = calloc(num_lt_cats, sizeof(float*));
            if(refveg_carbon_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }
            for (k = 0; k < num_lt_cats; k++) {
                refveg_carbon_out[i][j][k] = calloc(num_out_vals, sizeof(float));
                if(refveg_carbon_out[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k);
                    return ERROR_MEM;
                } // end for k loop over output values
            } // end for j loop over aezs
        } // end for j loop over glus
    } // end for i loop over fao country
    
    // read the soil c file
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soilc_csv_fname);
    if((err = read_soil_carbon(fname, soil_carbon_sage, in_args)) != OK)
    {
        fprintf(fplog, "Failed to read file %s for input: proc_refveg_carbon()\n",fname);
        return err;
    }
    // read the veg c file
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.vegc_csv_fname);
    if((err = read_veg_carbon(fname, veg_carbon_sage)) != OK)
    {
        fprintf(fplog, "Failed to read file %s for input: proc_refveg_carbon()\n",fname);
        return err;
    }
    
    // loop over the valid hyde land cells
    //  and skip it if no valid glu value or country value (country has to be mapped to ctry87)
    for (j = 0; j < num_land_cells_hyde; j++) {
        

        grid_ind = land_cells_hyde[j];
        

        aez_val = aez_bounds_new[grid_ind];
        ctry_code = country_fao[grid_ind];
        
        if (aez_val != raster_info.aez_new_nodata) {
            // get the fao country index
            ctry_ind = NOMATCH;
            for (i = 0; i < NUM_FAO_CTRY; i++) {
                if (countrycodes_fao[i] == ctry_code) {
                    ctry_ind = i;
                    break;
                }
            } // end for i loop to get ctry index
            
            // merge serbia and montenegro for scg record
            if (ctry_code == mne_code || ctry_code == srb_code) {
                ctry_code = scg_code;
                ctry_ind = NOMATCH;
                for (i = 0; i < NUM_FAO_CTRY; i++) {
                    if (countrycodes_fao[i] == ctry_code) {
                        ctry_ind = i;
                        break;
                    }
                }
                if (ctry_ind == NOMATCH) {
                    // this should never happen
                    fprintf(fplog, "Error finding scg ctry index: proc_refveg_carbon()\n");
                    return ERROR_IND;
                }
            } // end if serbia or montenegro
			
			if (ctry_ind == NOMATCH || ctry2ctry87codes_gtap[ctry_ind] == NOMATCH) {
				continue;
			}
			
            // get the glu index within the country glu list
            aez_ind = NOMATCH;
            for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                if (ctry_aez_list[ctry_ind][i] == aez_val) {
                    aez_ind = i;
                    break;
                }
            } // end for i loop to get aez index
            
            // this shouldn't happen because the countryXglu list has been made already
            if (aez_ind == NOMATCH) {
                fprintf(fplog, "Failed to match aez %i to country %i: proc_refveg_carbon()\n",aez_val,ctry_code);
                return ERROR_IND;
            }
            
            // nodata land area cells have already been removed, and it is ok if the land area is zero
            
            // generate the output land type category and add/store the values
            // only unmanaged potential vegetation area is used here
            
            // get index of sage pot veg; set value to 0 if unknown
            rv_ind = NOMATCH;
            for (i = 0; i < NUM_SAGE_PVLT; i++) {
                if (potveg_thematic[grid_ind] == landtypecodes_sage[i]) {
                    rv_ind = i;
                    break;
                }
            }
            if (rv_ind == NOMATCH) {
                rv_value = 0;
            } else {
                rv_value = refveg_thematic[grid_ind];
            }
            
            
            //kbn 2020 Add code for protected areas
        for (k=0; k< NUM_EPA_PROTECTED; k++){
        //temporary fractions for protected areas
        temp_frac = protected_EPA[k][grid_ind];    
            
            // get index of land category
            cur_lt_cat = rv_value * SCALE_POTVEG + k;
            cur_lt_cat_ind = NOMATCH;
            for (i = 0; i < num_lt_cats; i++) {
                if (lt_cats[i] == cur_lt_cat) {
                    cur_lt_cat_ind = i;
                    break;
                }
            }
            if (cur_lt_cat_ind == NOMATCH) {
                fprintf(fplog, "Failed to match lt_cat %i: proc_refveg_carbon()\n", cur_lt_cat);
                return ERROR_IND;
            }
            
            // calculate an area weighted average based on ref veg area for HYDE_YEAR
            // the unit conversion cancels out when the average is calculated, so don't do it here
            //kbn 2020 Updating below for protected area fractions
            // soil c
            refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind] =
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind] +
                soil_carbon_sage[rv_ind] * refveg_area[grid_ind]*temp_frac;
            
            // veg c
            refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind] =
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind] +
                veg_carbon_sage[rv_ind] * refveg_area[grid_ind]*temp_frac;
            
            // area
            refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][area_ind] =
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][area_ind] +
                refveg_area[grid_ind]*temp_frac;
            
        }	// end if valid aez cell
        }   //end loop for protected areas
    }	// end for j loop over valid hyde land cells
    
    // write the output file
    
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.refveg_carbon_fname);
    fpout = fopen(fname,"w"); //float
    if(fpout == NULL)
    {
        fprintf(fplog,"Failed to open file  %s for write:  proc_refveg_carbon()\n", fname);
        return ERROR_FILE;
    }
    // write header lines
    fprintf(fpout,"# File: %s\n", fname);
    fprintf(fpout,"# Author: %s\n", CODENAME);
    fprintf(fpout,"# Description: ref veg soil and veg carbon density (Mg/ha) for hyde land cells in country X glu X land type\n");
    fprintf(fpout,"# Original source: soil c for sage pot veg; veg c for sage pot veg; reference veg; country raster; new glu raster; hyde land area\n");
    fprintf(fpout,"# ----------\n");
    fprintf(fpout,"iso,glu_code,land_type,c_type,value");
    
    // write the records (rounded to integer)
    for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
        for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
            for (cur_lt_cat_ind = 0; cur_lt_cat_ind < num_lt_cats; cur_lt_cat_ind++) {
                if (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][area_ind] > 0) {
                    for (i = 0; i < 2; i++) {
                        // output only the carbon values - soil is the first index
                        // average, convert to Mg/ha, and round at the end
                        if (i == soilc_ind) {
                            // soil carbon
                            temp_float = KGMSQ2MGHA * refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind] /
                            refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][area_ind];
                            outval_soilc = (float) floor((double) 0.5 + temp_float);
                            // sum the total
                            global_soilc = global_soilc + refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind];
                            // write the value
                            if (outval_soilc > 0) {
                                fprintf(fpout,"\n%s,%i,%i,%s", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
                                        lt_cats[cur_lt_cat_ind], "soil_c");
                                fprintf(fpout,",%.0f", outval_soilc);
                                nrecords++;
                            }
                        } else if (i == vegc_ind) {
                            // veg carbon
                            temp_float = KGMSQ2MGHA * refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind] /
                            refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][area_ind];
                            outval_vegc = (float) floor((double) 0.5 + temp_float);
                            // sum the total
                            global_vegc = global_vegc + refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind];
                            // write the value
                            if (outval_vegc > 0) {
                                fprintf(fpout,"\n%s,%i,%i,%s", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
                                        lt_cats[cur_lt_cat_ind], "veg_c");
                                fprintf(fpout,",%.0f", outval_vegc);
                                nrecords++;
                            }
                        } // end if soil else veg
                    } // end if positive area value
                } // end for outval loop
            } // end for land type loop
        } // end for glu loop
    } // end for country loop
    
    fclose(fpout);
    
    fprintf(fplog, "Wrote file %s: proc_refveg_carbon(); records written=%i\n", fname, nrecords);
    
    // also write the total global carbon values to the log file
    // in Gg because the conversions cancel out
    fprintf(fplog, "\nGlobal reference vegetation carbon values, in Gg: proc_refveg_carbon()\n");
    fprintf(fplog, "Soil C = %f\n", global_soilc);
    fprintf(fplog, "Veg C = %f\n\n", global_vegc);
    
    free(soil_carbon_sage);
    free(veg_carbon_sage);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            for (k = 0; k < num_lt_cats; k++) {
                free(refveg_carbon_out[i][j][k]);
            }
            free(refveg_carbon_out[i][j]);
        }
        free(refveg_carbon_out[i]);
    }
    free(refveg_carbon_out);
    
    return OK;
    
}
/***********
 proc_nfert.c
 
 deprecated and removed from lds target
    the original processing ported below was wrong
    it used the total area instead of the harvested area
    and it is unclear how the n rate data were averaged across crops into one file
 
 generate csv table of total nitrogen application (kg) by country and glu and protected category, circa 2000
 output table has 3 label columns and one value column
 iso, glu, protected category, and total N application (kg)
 no zero value records
 truncated to 3 decimal places
 order follows harvest area output, with iso alphabetically, then glu# in order, then protected category in order
 only countries with aezs are written
 
 source data are the
 nfert image file - from Potter et al 2010, averaged somehow
 the protected image file
 the land area data - sage area is used because that is where the crop data are from
    orginally hyde land area data were used - but the harvested area should be used instead of land area, see below
 the fao country data
 the aez raster data
 country iso mapping
 
 input units are kg/ha
 output units are kg - rounded to nearest integer
 nfert file name is built here and passed to read_nfert()

 process only valid sage land cells, as that is where the crop data comes from
 
 serbia and montenegro data are merged
 
 NOTE: this output is not used by the gcam data processing system, except as a diagnostic
    but the original code has a bug, which is included here, and not fixed beause it is not actually used
        the rates are applied by total area, rather than harvested area, which does not match the input data
            the input data are based on harvested area
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Alan Di Vittorio, 12 jan 2016
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 ***********/

#include "lds.h"

int proc_nfert(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the sage land area data set determine the land cells to process
    
    int i, j = 0;
    int pc_ind;                 // the index for looping over mirca crops
    int err = OK;				// store error code from the read/write functions
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    float *nfert_grid;  // 1d array to store the nfert data; start up left corner, row by row; lon varies faster
    
    // output table as 3-d array
    float ***nfert_out;		// the nfert out table
    float outval;             // for output value
    
    int aez_val;            // current aez value
    int ctry_code;          // current fao country code
    int aez_ind;            // current aez index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    int nrecords = 0;       // count # of records written
    
    char fname[MAXCHAR];        // current file name to read/write

    FILE *fpout;                // out file pointer
    
    // allocate arrays
    
    nfert_grid = calloc(NUM_CELLS, sizeof(float));
    if(nfert_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for nfert_grid: proc_nfert()\n");
        return ERROR_MEM;
    }
    
    nfert_out = calloc(NUM_FAO_CTRY, sizeof(float**));
    if(nfert_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for nfert_out: proc_nfert()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        nfert_out[i] = calloc(ctry_aez_num[i], sizeof(float*));
        if(nfert_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for nfert_out[%i]: proc_nfert()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            nfert_out[i][j] = calloc(NUM_PROTECTED, sizeof(float));
            if(nfert_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for nfert_out[%i][%i]: proc_nfert()\n", i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
    
    // read the nfert file
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.nfert_rast_fname);
    if((err = read_nfert(fname, nfert_grid, in_args)) != OK)
    {
        fprintf(fplog, "Failed to read file %s for input: proc_nfert()\n",fname);
        return err;
    }
    
    // loop over the valid sage land cells
    //  and skip it if no valid aez value or country value
    for (j = 0; j < num_land_cells_sage; j++) {
        aez_val = aez_bounds_new[land_cells_sage[j]];
        ctry_code = country_fao[land_cells_sage[j]];
        
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
                    fprintf(fplog, "Error finding scg ctry index: proc_nfert()\n");
                    return ERROR_IND;
                }
            } // end if serbia or montenegro
			
			if (ctry_ind == NOMATCH || ctry2ctry87codes_gtap[ctry_ind] == NOMATCH) {
				continue;
			}
			
            // get the aez index within the country aez list
            aez_ind = NOMATCH;
            for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                if (ctry_aez_list[ctry_ind][i] == aez_val) {
                    aez_ind = i;
                    break;
                }
            } // end for i loop to get aez index
            
            // this shouldn't happen because the countryXaez list has been made already
            if (aez_ind == NOMATCH) {
                fprintf(fplog, "Failed to match aez %i to country %i: proc_nfert()\n",aez_val,ctry_code);
                return ERROR_IND;
            }
            
            // get the protected index
            if (protected_thematic[land_cells_sage[j]] == PROTECTED) {
                i = PROTECTED - 1;
            } else {
                i = UNPROTECTED - 1;
            }
            // multiply the rate by the area (also convert km^2 area to ha)
            nfert_out[ctry_ind][aez_ind][i] =
                nfert_out[ctry_ind][aez_ind][i] +
                nfert_grid[land_cells_sage[j]] * KMSQ2HA * land_area_sage[land_cells_sage[j]];
            
        }	// end if valid aez cell
    }	// end for j loop over valid sage land cells
    
    // write the output file
    
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.nfert_fname);
    fpout = fopen(fname,"w"); //float
    if(fpout == NULL)
    {
        fprintf(fplog,"Failed to open file  %s for write:  proc_nfert()\n", fname);
        return ERROR_FILE;
    }
    // write header lines
    fprintf(fpout,"# File: %s\n", fname);
    fprintf(fpout,"# Author: %s\n", CODENAME);
    fprintf(fpout,"# Description: nitrogen application (kg) for sage land cells in country X glu X protected category\n");
    fprintf(fpout,"# Original source: Nfert raster; country raster; new glu raster; sage land area\n");
    fprintf(fpout,"# ----------\n");
    fprintf(fpout,"iso,glu_code,protected_category,value");
    
    // write the records (truncated to 3 decimal places)
    for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
        for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
            for (pc_ind = 0; pc_ind < NUM_PROTECTED; pc_ind++) {
                outval = (float) floor((double) 0.5 + nfert_out[ctry_ind][aez_ind][pc_ind]);
                // output only positive values
                if (outval > 0) {
                    fprintf(fpout,"\n%s,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
                            pc_ind+1, outval);
                    nrecords++;
                } // end if value is positive
            } // end for protected loop
        } // end for aez loop
    } // end for country loop
    
    fclose(fpout);
    
    fprintf(fplog, "Wrote file %s: proc_nfert(); records written=%i\n", fname, nrecords);
    
    free(nfert_grid);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            free(nfert_out[i][j]);
        }
        free(nfert_out[i]);
    }
    free(nfert_out);
    
    return OK;
}

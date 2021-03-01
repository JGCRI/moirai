/***********
 proc_mirca.c
 
 generate csv tables of rainfed and irrigated crop areas by country and glu, circa 2000
    output tables have 3 label columns and one value column
        iso, glu, crop, and either irrigated or rainfed area
    no zero value records
    rounded to the nearest integer
    order follows harvest area output, with iso alphabetically, then glu# in order, then crop# in order
    only countries with glus and in ctry87 are processed
 
 source data are the
    MIRCA2000 envi ascii grid files
    the fao country data
    the glu raster data
    country iso mapping
 
 units are hectares - outputs are rounded to hectares
 file names are built here and passed to read_mirca()
 the crop # has 1 digit for #<10, and 2 digits for #>=10
 
 process only valid sage land cells, as that is where the crop data comes from
 
 serbia and montenegro data are merged
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Alan Di Vittorio, 11 jan 2016
 
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

int proc_mirca(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the sage land area data set determine the land cells to process
    
    int i, j = 0;
    int crop_index;             // the index for looping over mirca crops
    int err = OK;				// store error code from the write functions
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    float *irr_grid;  // 1d array to store current mirca raster file; start up left corner, row by row; lon varies faster
    float *rfd_grid;  // 1d array to store current mirca raster file; start up left corner, row by row; lon varies faster

    
    // output tables as 3-d arrays; ctry, glu, crop; crop varies fastest
    float ***irr_out;		// the irrigated crop area in ha
    float ***rfd_out;		// the rainfed crop area in ha
    
    int aez_val;            // current glu value
    int ctry_code;          // current fao country code
    int aez_ind;            // current glu index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    float outval;             // rounded value to write
    int nrecords_irr = 0;           // count # of irrigation records written
    int nrecords_rfd = 0;           // count # of rainfed records written
    
    char fname[MAXCHAR];        // current file name to read, or write irrigation
    char fname2[MAXCHAR];       // file name to write rainfed
    char tmp_str[MAXCHAR];		// stores a temporary string
    
    FILE *fpout;                // out file pointer for irrigation
    FILE *fpout2;               // out file pointer for rainfed

    // mirca file names
    const char irr_base[] = "ANNUAL_AREA_HARVESTED_IRC_CROP";   // mirca irrigated file base; 5 arcmin
    const char rfd_base[] = "ANNUAL_AREA_HARVESTED_RFC_CROP";   // mirca rainfed file base; 5 arcmin
    const char mirca_tag[] = "_HA.ASC";                         // mirca file end; 5 arcmin
    
    // allocate arrays
    
    irr_grid = calloc(NUM_CELLS, sizeof(float));
    if(irr_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for irr_grid: proc_mirca()\n");
        return ERROR_MEM;
    }
    
    rfd_grid = calloc(NUM_CELLS, sizeof(float));
    if(rfd_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for rfd_grid: proc_mirca()\n");
        return ERROR_MEM;
    }
    
    irr_out = calloc(NUM_FAO_CTRY, sizeof(float**));
    if(irr_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for irr_out: proc_mirca()\n");
        return ERROR_MEM;
    }
    rfd_out = calloc(NUM_FAO_CTRY, sizeof(float**));
    if(rfd_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for rfd_out: proc_mirca()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        irr_out[i] = calloc(ctry_aez_num[i], sizeof(float*));
        if(irr_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for irr_out[%i]: proc_mirca()\n", i);
            return ERROR_MEM;
        }
        rfd_out[i] = calloc(ctry_aez_num[i], sizeof(float*));
        if(rfd_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for rfd_out[%i]: proc_mirca()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            irr_out[i][j] = calloc(NUM_MIRCA_CROPS, sizeof(float));
            if(irr_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for irr_out[%i][%i]: proc_mirca()\n", i, j);
                return ERROR_MEM;
            }
            rfd_out[i][j] = calloc(NUM_MIRCA_CROPS, sizeof(float));
            if(rfd_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for rfd_out[%i][%i]: proc_mirca()\n", i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
    
    // loop over the MIRCA crops
    for (crop_index = 0; crop_index < NUM_MIRCA_CROPS; crop_index++) {
        
        // read the irrigated crop file
        strcpy(fname, in_args.mircapath);
        strcat(fname, irr_base);
        sprintf(tmp_str, "%i%s", (crop_index+1), mirca_tag);
        strcat(fname, tmp_str);
        if((err = read_mirca(fname, irr_grid)) != OK)
        {
            fprintf(fplog, "Failed to read file %s for input: proc_mirca()\n",fname);
            return err;
        }
        
        // read the rainfed crop file
        strcpy(fname, in_args.mircapath);
        strcat(fname, rfd_base);
        sprintf(tmp_str, "%i%s", (crop_index+1), mirca_tag);
        strcat(fname, tmp_str);
        if((err = read_mirca(fname, rfd_grid)) != OK)
        {
            fprintf(fplog, "Failed to read file %s for input: proc_mirca()\n",fname);
            return err;
        }
        
        // loop over the valid sage land cells
        //  and skip it if no valid glu value or country value
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
                        fprintf(fplog, "Error finding scg ctry index: proc_mirca()\n");
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
                
                // this shouldn't happen because the countryXglu list has been made already
                if (aez_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match aez %i to country %i: proc_mirca()\n",aez_val,ctry_code);
                    return ERROR_IND;
                }

                irr_out[ctry_ind][aez_ind][crop_index] = irr_out[ctry_ind][aez_ind][crop_index] + irr_grid[land_cells_sage[j]];
                rfd_out[ctry_ind][aez_ind][crop_index] = rfd_out[ctry_ind][aez_ind][crop_index] + rfd_grid[land_cells_sage[j]];
                
            }	// end if valid aez cell
        }	// end for j loop over valid sage land cells
    }   // end for loop over the mirca crops
    
    // write the output files
    
    // irrigated
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.mirca_irr_fname);
    fpout = fopen(fname,"w"); //float
    if(fpout == NULL)
    {
        fprintf(fplog,"Failed to open file  %s for write:  proc_mirca()\n", fname);
        return ERROR_FILE;
    }
    // write header lines
    fprintf(fpout,"# File: %s\n", fname);
    fprintf(fpout,"# Author: %s\n", CODENAME);
    fprintf(fpout,"# Description: mirca irrigated harvested area (ha) for sage land cells in country X glu\n");
    fprintf(fpout,"# Original source: MIRCA2000; country raster; new glu raster\n");
    fprintf(fpout,"# ----------\n");
    fprintf(fpout,"iso,glu_code,mirca_crop,value");
    
    // rainfed
    strcpy(fname2, in_args.outpath);
    strcat(fname2, in_args.mirca_rfd_fname);
    fpout2 = fopen(fname2,"w"); //float
    if(fpout2 == NULL)
    {
        fprintf(fplog,"Failed to open file  %s for write:  proc_mirca()\n", fname2);
        return ERROR_FILE;
    }
    // write header lines
    fprintf(fpout2,"# File: %s\n", fname2);
    fprintf(fpout2,"# Author: %s\n", CODENAME);
    fprintf(fpout2,"# Description: mirca rainfed havested area (ha) for sage land cells in country X glu\n");
    fprintf(fpout2,"# Original source: MIRCA2000; country raster; new glu raster\n");
    fprintf(fpout2,"# ----------\n");
    fprintf(fpout2,"iso,glu_code,mirca_crop,value");
    
    // write the records (rounded to nearest integer)
    for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
        for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
            for (crop_index = 0; crop_index < NUM_MIRCA_CROPS; crop_index++) {
                // irrigated
                outval = (float) floor((double) 0.5 + irr_out[ctry_ind][aez_ind][crop_index]);
                // output only positive values
                if (outval > 0) {
                    fprintf(fpout,"\n%s,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
                            crop_index+1, outval);
                    nrecords_irr++;
                } // end if value is positive
                // rainfed
                outval = (float) floor((double) 0.5 + rfd_out[ctry_ind][aez_ind][crop_index]);
                // output only positive values
                if (outval > 0) {
                    fprintf(fpout2,"\n%s,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
                            crop_index+1, outval);
                    nrecords_rfd++;
                } // end if value is positive
            } // end for crop loop
        } // end for aez loop
    } // end for country loop
    
    fclose(fpout);
    fclose(fpout2);
    
    fprintf(fplog, "Wrote file %s: proc_mirca(); records written=%i\n", fname, nrecords_irr);
    fprintf(fplog, "Wrote file %s: proc_mirca(); records written=%i\n", fname2, nrecords_rfd);
    
    free(irr_grid);
    free(rfd_grid);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            free(irr_out[i][j]);
            free(rfd_out[i][j]);
        }
        free(irr_out[i]);
        free(rfd_out[i]);
    }
    free(irr_out);
    free(rfd_out);
    
    return OK;}

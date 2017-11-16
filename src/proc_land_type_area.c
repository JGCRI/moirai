/***********
 proc_land_type_area.c
 
 generate csv table of land type area (ha) by country and glu and land type and year
 output table has 4 label columns and one value column
 iso, glu, land type (see LDS_land_types.csv), and land type area (ha)
 no zero value records
 order follows harvest area output, with iso alphabetically, then glu# in order, then land type in order, then year in order
 only countries with aezs are written
 
 the land type categories are in LDS_land_types.csv and are generated as follows:
    sage potential vegetation code * 100 + lu_code + proctected_code
    sage potential vegetation values are 0=unknown, and 1-15 in order of SAGE_PVLT.csv
    lu_code is 0 for non-crop, non-pasture, non-urban (i.e. non hyde)
    land use codes are set in lds.h: crop = 10, pasture = 20, urban = 30
    protected_code is 1 for protectected and 2 for unprotected
 
 source data are the
 hyde crop, pasture, urban bsq files - the nodata values are NODATA
 sage potential vegetation raster file
 the protected image file
 the hyde land area data
 the fao country data
 the glu raster data
 country iso mapping
 
 the hyde lu input files are bsq files that have been made by concatenating the individual year files
 33 years: 1700 - 2000 every 10 years, 2005, 2010
 the three file names are in the input file
 
 use the hyde land area to determine the non land use area
    the non land use is from sage potential vegetation
 
 input units are km^2
 output units are in ha - rounded to the nearest integer
 
 process only valid hyde land cells, as these are the source for land type area
 
 serbia and montenegro data are merged
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Alan Di Vittorio, 13 jan 2016
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 ***********/

#include "lds.h"

int proc_land_type_area(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the sage land area data set determine the land cells to process
    
    int i, j, k = 0;
    int year_ind;               // the index for looping over the years
    int grid_ind;               // the index within the 1d grid of the current land cell
    int pv_ind;                 // the index of the current sage pot veg land type
    int err = OK;				// store error code from the read/write functions
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    float *crop_grid;  // 1d array to store current crop data; start up left corner, row by row; lon varies faster
    float *pasture_grid;  // 1d array to store current pasture data; start up left corner, row by row; lon varies faster
    float *urban_grid;  // 1d array to store current urban data; start up left corner, row by row; lon varies faster
    
    float cur_land_area;        // current index land area
    float cur_crop_area;        // current index crop area value
    float cur_pasture_area;     // current index pasture area value
    float cur_urban_area;       // current index urban area value
    float cur_potveg_area;      // current index potential vegetation area value
    
    float ****area_out;		// output table as 4-d array
    float outval;             // the integer value to output
    int pv_value;           // the potential veg value for the current land type category
        
    int aez_val;            // current aez value
    int ctry_code;          // current fao country code
    int aez_ind;            // current aez index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    int cur_lt_cat;         // current land type category
    int cur_lt_cat_ind;     // current land type category index
    int nrecords = 0;       // count # of records written
    
    int hyde_years[NUM_HYDE_YEARS]; // the years in the hyde historical lu files
   
    char fname[MAXCHAR];        // current file name to write
    FILE *fpout;                // out file pointer
    
    double tmp_dbl;
    
    // create the array of available years
    hyde_years[0] = HYDE_START_YEAR;
    for (i = 1; i < (NUM_HYDE_YEARS - 2); i++) {
        hyde_years[i] = hyde_years[i-1] + 10;
    }
    hyde_years[NUM_HYDE_YEARS - 2] = 2005;
    hyde_years[NUM_HYDE_YEARS - 1] = 2010;
    
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
    
    // process each year
    for (year_ind = 0; year_ind < NUM_HYDE_YEARS; year_ind++) {
        // read the lu hyde data
        if((err = read_lu_hyde(in_args, hyde_years[year_ind], crop_grid, pasture_grid, urban_grid)) != OK)
        {
            fprintf(fplog, "Failed to read lu hude data for input: proc_land_type_area()\n");
            return err;
        }
        
        // loop over the valid hyde land cells
        //  and skip it if no valid glu value or country value
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
                
                if (ctry_ind == NOMATCH) {
                    continue;
                }
                
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
                        fprintf(fplog, "Error finding scg ctry index: proc_land_type_area()\n");
                        return ERROR_IND;
                    }
                } // end if serbia or montenegro
                
                // get the glu index within the country aez list
                aez_ind = NOMATCH;
                for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                    if (ctry_aez_list[ctry_ind][i] == aez_val) {
                        aez_ind = i;
                        break;
                    }
                } // end for i loop to get aez index
                
                // this shouldn't happen because the countryXglu list has been made already
                if (aez_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match aez %i to country %i: proc_land_type_area()\n",aez_val,ctry_code);
                    return ERROR_IND;
                }

                // nodata land area cells have already been removed
                // there may be some zero area cells
                // if so, continue to next cell
                if (land_area_hyde[grid_ind] == 0) {
                    continue;
                }
                
                // check for valid current land use area
                cur_land_area = land_area_hyde[grid_ind];
                if (crop_grid[grid_ind] != NODATA) {
                    cur_crop_area = crop_grid[grid_ind];
                } else { cur_crop_area = 0; }
                if (pasture_grid[grid_ind] != NODATA) {
                    cur_pasture_area = pasture_grid[grid_ind];
                } else { cur_pasture_area = 0; }
                if (urban_grid[grid_ind] != NODATA) {
                    cur_urban_area = urban_grid[grid_ind];
                } else { cur_urban_area = 0; }
                
                // calculate potential veg area
                cur_potveg_area = land_area_hyde[grid_ind] -
                cur_crop_area - cur_pasture_area - cur_urban_area;
                // check for negative values
                if (cur_potveg_area < 0) {
                    // adjust urban area if not enough land
                    cur_urban_area = cur_urban_area + cur_potveg_area;
                    cur_potveg_area = 0;
                }
                // double-check for enough land and adjust pasture
                if (cur_urban_area < 0) {
                    cur_pasture_area = cur_pasture_area + cur_urban_area;
                    cur_urban_area = 0;
                }
                // final check for enough land and adjust crops
                if (cur_pasture_area < 0) {
                    cur_crop_area = cur_crop_area + cur_pasture_area;
                    cur_pasture_area = 0;
                }
                // this shouldn't happen, but check anyway
                if (cur_crop_area < 0) {
                    cur_crop_area = 0;
                    fprintf(fplog, "Error: negative land area at grid_ind %i: proc_land_type_area()\n", grid_ind);
                    return ERROR_CALC;
                }
                
                // generate the land type category and add/store the area
                
                // get index of sage pot veg to make sure it is valid
                pv_ind = NOMATCH;
                for (i = 0; i < NUM_SAGE_PVLT; i++) {
                    if (potveg_thematic[grid_ind] == landtypecodes_sage[i]) {
                        pv_ind = i;
                        break;
                    }
                }
                
                // if no pot veg cat, then use the unknown value of 0, otherwise set it to the grid value
                if (pv_ind == NOMATCH) {
                    pv_value = 0;
                } else {
                    pv_value = potveg_thematic[grid_ind];
                }
                
                // potential veg; i.e. non-crop, non-pasture, non-urban
                cur_lt_cat = pv_value * SCALE_POTVEG + protected_thematic[grid_ind];
                cur_lt_cat_ind = NOMATCH;
                for (i = 0; i < num_lt_cats; i++) {
                    if (lt_cats[i] == cur_lt_cat) {
                        cur_lt_cat_ind = i;
                        break;
                    }
                }
                if (cur_lt_cat_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
                    return ERROR_IND;
                }
                area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] +
                cur_potveg_area;
                
                // crop
                cur_lt_cat = pv_value * SCALE_POTVEG + CROP_LT_CODE + protected_thematic[grid_ind];
                cur_lt_cat_ind = NOMATCH;
                for (i = 0; i < num_lt_cats; i++) {
                    if (lt_cats[i] == cur_lt_cat) {
                        cur_lt_cat_ind = i;
                        break;
                    }
                }
                if (cur_lt_cat_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
                    return ERROR_IND;
                }
                area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] +
                cur_crop_area;
                
                // pasture
                cur_lt_cat = pv_value * SCALE_POTVEG + PASTURE_LT_CODE + protected_thematic[grid_ind];
                cur_lt_cat_ind = NOMATCH;
                for (i = 0; i < num_lt_cats; i++) {
                    if (lt_cats[i] == cur_lt_cat) {
                        cur_lt_cat_ind = i;
                        break;
                    }
                }
                if (cur_lt_cat_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
                    return ERROR_IND;
                }
                area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] +
                cur_pasture_area;
                
                // urban
                cur_lt_cat = pv_value * SCALE_POTVEG + URBAN_LT_CODE + protected_thematic[grid_ind];
                cur_lt_cat_ind = NOMATCH;
                for (i = 0; i < num_lt_cats; i++) {
                    if (lt_cats[i] == cur_lt_cat) {
                        cur_lt_cat_ind = i;
                        break;
                    }
                }
                if (cur_lt_cat_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match lt_cat %i: proc_land_type_area()\n", cur_lt_cat);
                    return ERROR_IND;
                }
                area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] = area_out[ctry_ind][aez_ind][cur_lt_cat_ind][year_ind] +
                cur_urban_area;
                
            }	// end if valid glu cell
        }	// end for j loop over valid hyde land cells
        
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
    fprintf(fpout,"# Original source: hyde areas; sage potential veg; country raster; new glu raster; hyde land area\n");
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
    
    return OK;

}

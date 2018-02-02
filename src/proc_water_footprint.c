/***********
 proc_water_footprint.c
 
 generate csv tables of water footprint by country and glu and crop, 1996-2005 annual average volume within grid cell
 output tables have 3 label columns and five value columns
 iso, glu_code, SAGE_crop, water_type (blue, green, gray, total), value (water volume in m^3)
 no zero value records
 rounded to nearest integer
 order follows harvest area output, with iso alphabetically, then glu# in order, then crop# in order
 only countries with glus are written
 
 source data are the
 water footprint binary files generated from the arc grid files by an r script
 the fao country data
 the glu raster data
 country iso mapping
 
 water footprint files:
    4320 columns, 2160 rows, 5 arcmin res
    geographic projection of wgs84
    the original data have been resampled to the full grid with upper left corner at -180, +90
    float data type
    the nodata value is -9999
    units are average annual mm for entire grid cell area (1996-2005)
    the .gri files are the binary raster data
    the .grd files are the text header info
 
 process only valid sage land cells, as that is where the crop data comes from
    so use the calculated cell area to get water volume, rather than the hyde cell area
 
 serbia and montenegro data are merged
 
 file names are constructed here, and passed to read_water_footprint()
 
 the diagnostic outputs are simple binary files of the input data
 these are hardcoded to not output because they require a subdirectory and a fair amount of space
 they were output only for testing against the original AEZs
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Alan Di Vittorio, 25 feb 2016
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 ***********/

#include "lds.h"

int proc_water_footprint(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the sage land area data set determine the land cells to process
    
    int i, j, k = 0;
    int crop_index;             // the index for looping over wf crops
    int err = OK;				// store error code from the write functions
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    float *bl_grid;  // 1d array to store current blue raster file; start up left corner, row by row; lon varies faster
    float *gn_grid;  // 1d array to store current green raster file; start up left corner, row by row; lon varies faster
    float *gy_grid;  // 1d array to store current gray raster file; start up left corner, row by row; lon varies faster
    float *tot_grid;  // 1d array to store current total raster file; start up left corner, row by row; lon varies faster
    
    // output table as 4-d array; ctry, glu, crop, water type; water type varies fastest
    float ****wf_out;		// the water volume data, in m^3, dim order: blue, green gray, total
    
    int glu_val;            // current glu value
    int ctry_code;          // current fao country code
    int glu_ind;            // current glu index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    float outval;             // rounded value to write
    int nrecords_wf = 0;           // count # of irrigation records written
    
    char fname[MAXCHAR];        // current file name to read, or write
    char tmp_str[MAXCHAR];		// stores a temporary string
    char diag_name[MAXCHAR];	// for diagnostic output names
    
    FILE *fpout;                // out file pointer
    
    float wf_nodata = NODATA;  // wf binary file nodata value
    float CONV2M3 = 1000;            // mm * 1km/1000000mm * km2 * 1000000000m3/1km3 so conversion is *1000
    double tmp_dbl = 0;
    
    // wf file names
    const char bl_base[] = "/wfbl_mmyr.gri";   // blue base; 5 arcmin
    const char gn_base[] = "/wfgn_mmyr.gri";   // green base; 5 arcmin
    const char gy_base[] = "/wfgy_mmyr.gri";   // gray base; 5 arcmin
    const char tot_base[] = "/wftot_mmyr.gri";   // total base; 5 arcmin
    
    // wf crops (these are the data directory names)
    const char *crop_names[NUM_WF_CROPS] = {"Barley", "Cassava", "Coconuts", "Coffee", "Cotton", "Groundnut", "Maize", "Millet", "Oilpalm", "Olives", "Potatoes", "Rapeseed", "Rice", "Sorghum", "Soybean", "Sugarcane", "Sunflower", "Wheat"};
    
    // wf water types
    const char *wftype_names[NUM_WF_TYPES] = {"blue", "green", "gray", "total"};
    
    // allocate arrays
    
    bl_grid = calloc(NUM_CELLS, sizeof(float));
    if(bl_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for bl_grid: proc_water_footprint()\n");
        return ERROR_MEM;
    }
    
    gn_grid = calloc(NUM_CELLS, sizeof(float));
    if(gn_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for gn_grid: proc_water_footprint()\n");
        return ERROR_MEM;
    }
    
    gy_grid = calloc(NUM_CELLS, sizeof(float));
    if(gy_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for gy_grid: proc_water_footprint()\n");
        return ERROR_MEM;
    }
    
    tot_grid = calloc(NUM_CELLS, sizeof(float));
    if(tot_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for tot_grid: proc_water_footprint()\n");
        return ERROR_MEM;
    }
    
    wf_out = calloc(NUM_FAO_CTRY, sizeof(float***));
    if(wf_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for wf_out: proc_water_footprint()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        wf_out[i] = calloc(ctry_aez_num[i], sizeof(float**));
        if(wf_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for wf_out[%i]: proc_water_footprint()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            wf_out[i][j] = calloc(NUM_WF_CROPS, sizeof(float*));
            if(wf_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for wf_out[%i][%i]: proc_water_footprint()\n", i, j);
                return ERROR_MEM;
            }
            for (k = 0; k < NUM_WF_CROPS; k++) {
                wf_out[i][j][k] = calloc(NUM_WF_TYPES, sizeof(float));
                if(wf_out[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for wf_out[%i][%i][%i]: proc_water_footprint()\n", i, j, k);
                    return ERROR_MEM;
                }
            } // end for k loop over crops
        } // end for j loop over glus
    } // end for i loop over fao country
    
    // loop over the wf crops
    for (crop_index = 0; crop_index < NUM_WF_CROPS; crop_index++) {
        
        // read the blue water file
        strcpy(fname, in_args.wfpath);
        strcat(fname, crop_names[crop_index]);
        strcat(fname, bl_base);
        if((err = read_water_footprint(fname, bl_grid)) != OK)
        {
            fprintf(fplog, "Failed to read file %s for input: proc_water_footprint()\n",fname);
            return err;
        }
        
        // read the green water file
        strcpy(fname, in_args.wfpath);
        strcat(fname, crop_names[crop_index]);
        strcat(fname, gn_base);
        if((err = read_water_footprint(fname, gn_grid)) != OK)
        {
            fprintf(fplog, "Failed to read file %s for input: proc_water_footprint()\n",fname);
            return err;
        }
        
        // read the gray water file
        strcpy(fname, in_args.wfpath);
        strcat(fname, crop_names[crop_index]);
        strcat(fname, gy_base);
        if((err = read_water_footprint(fname, gy_grid)) != OK)
        {
            fprintf(fplog, "Failed to read file %s for input: proc_water_footprint()\n",fname);
            return err;
        }
        
        // read the total water file
        strcpy(fname, in_args.wfpath);
        strcat(fname, crop_names[crop_index]);
        strcat(fname, tot_base);
        if((err = read_water_footprint(fname, tot_grid)) != OK)
        {
            fprintf(fplog, "Failed to read file %s for input: proc_water_footprint()\n",fname);
            return err;
        }
        
        // loop over the valid sage land cells
        //  and skip it if no valid glu value or country value
        for (j = 0; j < num_land_cells_sage; j++) {
            glu_val = aez_bounds_new[land_cells_sage[j]];
            ctry_code = country_fao[land_cells_sage[j]];
            
            if (glu_val != raster_info.aez_new_nodata) {
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
                        fprintf(fplog, "Error finding scg ctry index: proc_water_footprint()\n");
                        return ERROR_IND;
                    }
                } // end if serbia or montenegro
				
				if (ctry_ind == NOMATCH || ctry2ctry87codes_gtap[ctry_ind] == NOMATCH) {
					continue;
				}
				
                // get the glu index within the country glu list
                glu_ind = NOMATCH;
                for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                    if (ctry_aez_list[ctry_ind][i] == glu_val) {
                        glu_ind = i;
                        break;
                    }
                } // end for i loop to get glu index
                
                // this shouldn't happen because the countryXglu list has been made already
                if (glu_ind == NOMATCH) {
                    fprintf(fplog, "Failed to match glu %i to country %i: proc_water_footprint()\n",glu_val,ctry_code);
                    return ERROR_IND;
                }
                
                //if(ctry_code == 103 && glu_val == 84) {
                if(0) {
                    printf("grid_ind = %i; x = %f, y = %i\n", land_cells_sage[j], modf(land_cells_sage[j]/NUM_LON, &tmp_dbl) + 1, land_cells_sage[j]/NUM_LON + 1);
                    printf("blue = %f\nwf_out_blu = %f\n", bl_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][0]);
                    printf("green = %f\nwf_out_grn = %f\n", gn_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][1]);
                    printf("gray = %f\nwf_out_gry = %f\n", gy_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][2]);
                    printf("tot = %f\nwf_out_tot = %f\n", tot_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][3]);
                }
                
                // multiply the mm depth by the km^2 grid cell area and add it to the total for this country/glu/crop/wftype
                // check for valid values first
                
                if (bl_grid[land_cells_sage[j]] != wf_nodata) {
                    wf_out[ctry_ind][glu_ind][crop_index][0] = wf_out[ctry_ind][glu_ind][crop_index][0] +
                        CONV2M3 * bl_grid[land_cells_sage[j]] * cell_area[land_cells_sage[j]];
                }
                if (gn_grid[land_cells_sage[j]] != wf_nodata) {
                    wf_out[ctry_ind][glu_ind][crop_index][1] = wf_out[ctry_ind][glu_ind][crop_index][1] +
                        CONV2M3 * gn_grid[land_cells_sage[j]] * cell_area[land_cells_sage[j]];
                }
                if (gy_grid[land_cells_sage[j]] != wf_nodata) {
                    wf_out[ctry_ind][glu_ind][crop_index][2] = wf_out[ctry_ind][glu_ind][crop_index][2] +
                        CONV2M3 * gy_grid[land_cells_sage[j]] * cell_area[land_cells_sage[j]];
                }
                if (tot_grid[land_cells_sage[j]] != wf_nodata) {
                    wf_out[ctry_ind][glu_ind][crop_index][3] = wf_out[ctry_ind][glu_ind][crop_index][3] +
                        CONV2M3 * tot_grid[land_cells_sage[j]] * cell_area[land_cells_sage[j]];
                }
                
                //if(ctry_code == 103 && glu_val == 84) {
                if(0) {
                    printf("blue = %f\nwf_out_blu = %f\n", bl_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][0]);
                    printf("green = %f\nwf_out_grn = %f\n", gn_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][1]);
                    printf("gray = %f\nwf_out_gry = %f\n", gy_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][2]);
                    printf("tot = %f\nwf_out_tot = %f\n", tot_grid[land_cells_sage[j]], wf_out[ctry_ind][glu_ind][crop_index][3]);
                }
                
            }	// end if valid glu cell
        }	// end for j loop over valid sage land cells
        
        if (0) {
            // blue
            sprintf(tmp_str, "%s%s_%s%s", "wf_grids/", crop_names[crop_index], wftype_names[0], ".bil");
            strcpy(diag_name, tmp_str);
            if ((err = write_raster_float(bl_grid, NUM_CELLS, diag_name, in_args))) {
                fprintf(fplog, "Error writing file %s: proc_water_footprint()\n", diag_name);
                return err;
            }
            // green
            sprintf(tmp_str, "%s%s_%s%s", "wf_grids/", crop_names[crop_index], wftype_names[1], ".bil");
            strcpy(diag_name, tmp_str);
            if ((err = write_raster_float(gn_grid, NUM_CELLS, diag_name, in_args))) {
                fprintf(fplog, "Error writing file %s: proc_water_footprint()\n", diag_name);
                return err;
            }
            // gray
            sprintf(tmp_str, "%s%s_%s%s", "wf_grids/", crop_names[crop_index], wftype_names[2], ".bil");
            strcpy(diag_name, tmp_str);
            if ((err = write_raster_float(gy_grid, NUM_CELLS, diag_name, in_args))) {
                fprintf(fplog, "Error writing file %s: proc_water_footprint()\n", diag_name);
                return err;
            }
            // total
            sprintf(tmp_str, "%s%s_%s%s", "wf_grids/", crop_names[crop_index], wftype_names[3], ".bil");
            strcpy(diag_name, tmp_str);
            if ((err = write_raster_float(tot_grid, NUM_CELLS, diag_name, in_args))) {
                fprintf(fplog, "Error writing file %s: proc_water_footprint()\n", diag_name);
                return err;
            }
        }
        
    }   // end for loop over the wf crops
    
    // write the output file
    
    // water footprint
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.wf_fname);
    fpout = fopen(fname,"w"); //float
    if(fpout == NULL)
    {
        fprintf(fplog,"Failed to open file  %s for write:  proc_water_footprint()\n", fname);
        return ERROR_FILE;
    }
    // write header lines
    fprintf(fpout,"# File: %s\n", fname);
    fprintf(fpout,"# Author: %s\n", CODENAME);
    fprintf(fpout,"# Description: crop average annual water volume consumed (m^3) for sage land cells in country X glu\n");
    fprintf(fpout,"# Original source: water footprint network; country raster; new glu raster\n");
    fprintf(fpout,"# ----------\n");
    fprintf(fpout,"iso,glu_code,SAGE_crop,water_type,value");
    
    // write the records (rounded to nearest integer)
    for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
        for (glu_ind = 0; glu_ind < ctry_aez_num[ctry_ind]; glu_ind++) {
            for (crop_index = 0; crop_index < NUM_WF_CROPS; crop_index++) {
                for (i = 0; i < NUM_WF_TYPES; i++) {
                    // round to integer
                    outval = (float) floor((double) 0.5 + wf_out[ctry_ind][glu_ind][crop_index][i]);
                    // output only positive values
                    if (outval > 0) {
                        fprintf(fpout,"\n%s,%i,%s,%s,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][glu_ind],
                                crop_names[crop_index], wftype_names[i], outval);
                        nrecords_wf++;
                    } // end if value is positive
                } // end for water type loop
            } // end for crop loop
        } // end for glu loop
    } // end for country loop
    
    fclose(fpout);
    
    fprintf(fplog, "Wrote file %s: proc_water_footprint(); records written=%i\n", fname, nrecords_wf);
    
    free(bl_grid);
    free(gn_grid);
    free(gy_grid);
    free(tot_grid);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            for (k = 0; k < NUM_WF_CROPS; k++) {
                free(wf_out[i][j][k]);
            }
            free(wf_out[i][j]);
        }
        free(wf_out[i]);
    }
    free(wf_out);
    
    return OK;
}
/***********
 proc_mapspam.c
 
 generate csv tables of rainfed and irrigated crop areas by country and glu, circa 2020
    output tables have 3 label columns and one value column
        iso, glu, crop, and either irrigated or rainfed area
    no zero value records
    rounded to the nearest integer
    order follows harvest area output, with iso alphabetically, then glu# in order, then crop# in order
    only countries with glus and in ctry87 are processed
 
 source data are the
    mapspam2020 envi ascii grid files
    the fao country data
    the glu raster data
    country iso mapping
 
 units are hectares - outputs are rounded to hectares
 file names are built here and passed to read_mapspam()
 the crop # has 1 digit for #<10, and 2 digits for #>=10
 
 process only valid sage land cells, as that is where the crop data comes from
 
 serbia and montenegro data are merged
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 created by Roan Chadsey, 28 Jan 2016
 
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

int proc_mapspam(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the sage land area data set determine the land cells to process
    
    int i, j, k = 0;
    int crop_index;             // the index for looping over mapspam crops
    int crop_ind;				// second crop index for harmonizing with HYDE
    char crop_names[NUM_MAPSPAM_CROPS][5]	= {"WHEA","RICE","MAIZ","BARL","MILL","PMIL","SORG","OCER","POTA","SWPO","YAMS","CASS","ORTS","BEAN","CHIC","COWP","PIGE","LENT","OPUL","SOYB","GROU","CNUT","OILP","SUNF","RAPE","SESA","OOIL","SUGC","SUGB","COTT","OFIB","COFF","RCOF","COCO","TEAS","TOBA","BANA","PLNT","CITR","TROF","TEMF","TOMA","ONIO","VEGE","RUBB","REST"}; // the mapspam crop names
    int var_index;				// the index for looping over mapspam variables
    char var_names[NUM_MAPSPAM_VARS][3] = {"_H","_A","_P","_Y"}; // mapspam variables (harvested area, physical area, production, yield)
    char csv_tags[NUM_MAPSPAM_VARS][8] = {"_ha.csv", "_ha.csv", "_Mt.csv", "_Mt.csv"};
    char var_long_names[NUM_MAPSPAM_VARS][20] = {"harvested area (ha)", "physical area (ha)", "production (mt)", "yield (mt/ha)"};
    int err = OK;				// store error code from the write functions
    int HYDE_violation = 0;		// counter for HYDE-mapSPAM cropland violations
    int add_crops_rfd = 0;		// counter for HYDE-mapSPAM cropland violations
	int add_crops_irr = 0;		// counter for HYDE-mapSPAM cropland violations
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    float *irr_grid;  // 1d array to store current mapspam raster file; start up left corner, row by row; lon varies faster
    float *rfd_grid;  // 1d array to store current mapspam raster file; start up left corner, row by row; lon varies faster
    float cropland_irr;   // value to harmonize HYDE cropland
    float cropland_rfd;   // value to harmonize HYDE cropland

	float *crop_grid;  // 1d array to store current crop data; start up left corner, row by row; lon varies faster
    float *pasture_grid;  // 1d array to store current pasture data; start up left corner, row by row; lon varies faster
    float *urban_grid;  // 1d array to store current urban data; start up left corner, row by row; lon varies faster
	float **lu_detail_grid;		// for the rest of the hyde types; dim1=hyde types, dim2=cells
	
    // output tables as 3-d arrays; ctry, glu, crop; crop varies fastest
    float ****irr_out;		// the irrigated crop area in ha
    float ****rfd_out;		// the rainfed crop area in ha
    
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
    char tmp_unit[MAXCHAR];		// stores a temporary string
    
    FILE *fpout;                // out file pointer for irrigation
    FILE *fpout2;               // out file pointer for rainfed

    // mapspam input file names
    const char mapspam_base[] = "spam2020_V2r0_global";   // mapspam file base; 5 arcmin
    const char irr_tag[] = "_I";   // mapspam irrigated file end; 5 arcmin
    const char rfd_tag[] = "_R";   // mapspam rainfed file end; 5 arcmin   
    const char envi_tag[] = ".envi"; // mapspam envi file tag
    
    // mapspam output file names
    const char ha_csv_tag[] = "_ha.csv";
    const char Mt_csv_tag[] = "_Mt.csv";
    
    // allocate arrays
    irr_grid = calloc(NUM_CELLS, sizeof(float));
    if(irr_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for irr_grid: proc_mapspam()\n");
        return ERROR_MEM;
    }
    rfd_grid = calloc(NUM_CELLS, sizeof(float));
    if(rfd_grid == NULL) {
        fprintf(fplog,"Failed to allocate memory for rfd_grid: proc_mapspam()\n");
        return ERROR_MEM;
    }
    
    irr_out = calloc(NUM_FAO_CTRY, sizeof(float***));
    if(irr_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for irr_out: proc_mapspam()\n");
        return ERROR_MEM;
    }
    rfd_out = calloc(NUM_FAO_CTRY, sizeof(float***));
    if(rfd_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for rfd_out: proc_mapspam()\n");
        return ERROR_MEM;
    }
    // allocate AEZ dimension
        for (i = 0; i < NUM_FAO_CTRY; i++) {
        irr_out[i] = calloc(ctry_aez_num[i], sizeof(float**));
        if(irr_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for irr_out[%i]: proc_mapspam()\n", i);
            return ERROR_MEM;
        }
        rfd_out[i] = calloc(ctry_aez_num[i], sizeof(float**));
        if(rfd_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for rfd_out[%i]: proc_mapspam()\n", i);
            return ERROR_MEM;
        }
    // allocate crop dimension
        for (j = 0; j < ctry_aez_num[i]; j++) {
            irr_out[i][j] = calloc(NUM_MAPSPAM_CROPS, sizeof(float*));
            if(irr_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for irr_out[%i][%i]: proc_mapspam()\n", i, j);
                return ERROR_MEM;
            }
            rfd_out[i][j] = calloc(NUM_MAPSPAM_CROPS, sizeof(float*));
            if(rfd_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for rfd_out[%i][%i]: proc_mapspam()\n", i, j);
                return ERROR_MEM;
            }
    // allocate variable dimension
    	for (k = 0; k < NUM_MAPSPAM_CROPS; k++) {
    		irr_out[i][j][k] = calloc(NUM_MAPSPAM_VARS, sizeof(float));
            if(irr_out[i][j][k] == NULL) {
                fprintf(fplog,"Failed to allocate memory for irr_out[%i][%i][%i]: proc_mapspam()\n", i, j, k);
                return ERROR_MEM;
            }
            rfd_out[i][j][k] = calloc(NUM_MAPSPAM_VARS, sizeof(float));
            if(rfd_out[i][j][k] == NULL) {
                fprintf(fplog,"Failed to allocate memory for rfd_out[%i][%i][%i]: proc_mapspam()\n", i, j, k);
                return ERROR_MEM;
    		}
    	} // end for k loop vars
        } // end for j loop over aezs
    } // end for i loop over fao country
    
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
    
    lu_detail_grid = calloc(NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN, sizeof(float*));
	if(lu_detail_grid == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_detail_grid: proc_mapspam()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		lu_detail_grid[i] = calloc(NUM_CELLS, sizeof(float));
		if(lu_detail_grid[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_detail_grid[%i]: proc_mapspam()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
    
    if((err = read_hyde32(in_args, &raster_info, MAPSPAM_YEAR, crop_grid, pasture_grid, urban_grid, lu_detail_grid)) != OK)
		{
			fprintf(fplog, "Failed to read lu hyde data for year %i: proc_mapspam()\n", MAPSPAM_YEAR);
			return err;
		}
		
    // loop over the mapspam variables and crops
    for (var_index = 0; var_index < NUM_MAPSPAM_VARS; var_index++) { // For each variable in var_names
    	add_crops_rfd = 0;
    	add_crops_irr = 0;
		for (crop_index = 0; crop_index < NUM_MAPSPAM_CROPS; crop_index++) {
			
			// read the irrigated crop files
			strcpy(fname, in_args.mapspampath);
			strcat(fname, mapspam_base);
			sprintf(tmp_str, "%s%s%s%s%s", (var_names[var_index]), "_", (crop_names[crop_index]), irr_tag, envi_tag);
			strcat(fname, tmp_str);
			
			if((err = read_mapspam(fname, irr_grid)) != OK)
			{
				fprintf(fplog, "Failed to read file %s for input: proc_mapspam()\n",fname);
				return err;
			}
			
			// read the rainfed crop file
			strcpy(fname, in_args.mapspampath);
			strcat(fname, mapspam_base);
			sprintf(tmp_str, "%s%s%s%s%s", (var_names[var_index]), "_", (crop_names[crop_index]), rfd_tag, envi_tag);
			strcat(fname, tmp_str);
			
			if((err = read_mapspam(fname, rfd_grid)) != OK)
			{
				fprintf(fplog, "Failed to read file %s for input: proc_mapspam()\n",fname);
				return err;
			}
		
			
			// loop over the valid sage land cells
			//  and skip it if no valid glu value or country value
			for (j = 0; j < num_land_cells_sage; j++) {
				aez_val = aez_bounds_new[land_cells_sage[j]]; // get the basin value for this sage pixel
				ctry_code = country_fao[land_cells_sage[j]]; // get country code for this sage pixel
				
				if (aez_val != raster_info.aez_new_nodata) { // check that this is a valid pixel
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
							fprintf(fplog, "Error finding scg ctry index: proc_mapspam()\n");
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
						fprintf(fplog, "Failed to match aez %i to country %i: proc_mapspam()\n",aez_val,ctry_code);
						return ERROR_IND;
					}
					
					// For cases where HYDE does not have cropland but mapSPAM has crop values we set the pixel to zero
					if (crop_grid[land_cells_sage[j]] < 0) {
						if (irr_grid[land_cells_sage[j]] > 0) {
							irr_grid[land_cells_sage[j]] = 0; // Harmonize cropland with HYDE
							HYDE_violation++; // count cells that had to be harmonized with the HYDE crop_grid mask
						}
						if (rfd_grid[land_cells_sage[j]] > 0) {
							rfd_grid[land_cells_sage[j]] = 0; // Harmonize cropland with HYDE
							HYDE_violation++; // count cells that had to be harmonized with the HYDE crop_grid mask
						}
					}
			
					// We want to set NaN in the irr/rfd_grid to zero to avoid -inf errors
					if (irr_grid[land_cells_sage[j]] < 0) {
						irr_grid[land_cells_sage[j]] = 0; // replace NAN values
					}
					if (rfd_grid[land_cells_sage[j]] < 0) {
						rfd_grid[land_cells_sage[j]] = 0; // replace NAN values
					}
					
					irr_out[ctry_ind][aez_ind][crop_index][var_index] = irr_out[ctry_ind][aez_ind][crop_index][var_index] + irr_grid[land_cells_sage[j]];
					rfd_out[ctry_ind][aez_ind][crop_index][var_index] = rfd_out[ctry_ind][aez_ind][crop_index][var_index] + rfd_grid[land_cells_sage[j]];
					
					// For cases where mapSPAM has no crop value but HYDE has cropland we want to add one to the 'REST' crop
					if (crop_index == 45 && crop_grid[land_cells_sage[j]] > 0) { // check for HYDE cropland and REST crop index
						for (crop_ind = 0; crop_ind < NUM_MAPSPAM_CROPS; crop_ind++) { // check if mapSPAM has no crops
							cropland_irr = cropland_irr + irr_out[ctry_ind][aez_ind][crop_ind][var_index];
							cropland_rfd = cropland_rfd + rfd_out[ctry_ind][aez_ind][crop_ind][var_index];
						}
						if (cropland_irr == 0) { // if mapSPAM had no crops add one ha to REST
							irr_out[ctry_ind][aez_ind][crop_index][var_index] = 1;
							HYDE_violation++; // count cells that had to be harmonized with the HYDE crop_grid mask
						}
						if (cropland_rfd == 0) { // if mapSPAM had no crops add one ha to REST
							rfd_out[ctry_ind][aez_ind][crop_index][var_index] = 1;
							HYDE_violation++; // count cells that had to be harmonized with the HYDE crop_grid mask
						}
					}
			
				}	// end if valid aez cell
			}	// end for j loop over valid sage land cells
		}   // end for loop over the mapspam crops
	}	// end for loop over mapspam variables
    
    fprintf(fplog, "The number of mapSPAM cells that have crops in a non-cropland HYDE cell is: %i\n", HYDE_violation);
    
    // Write values
	for (var_index = 0; var_index < NUM_MAPSPAM_VARS; var_index++) {
	
		strcpy(fname, in_args.outpath);
		strcat(fname, in_args.mapspam_irr_fname);
		sprintf(tmp_str, "%s%s", (var_names[var_index]), csv_tags[var_index]);
		strcat(fname, tmp_str);
		fpout = fopen(fname,"w"); //float
		
		if(fpout == NULL)
		{
			fprintf(fplog,"Failed to open file  %s for write:  proc_mapspam()\n", fname);
			return ERROR_FILE;
		}
		// write header lines
		fprintf(fpout,"# File: %s\n", fname);
		fprintf(fpout,"# Author: %s\n", CODENAME);
		fprintf(fpout,"# Description: mapspam irrigated %s for sage land cells in country X glu\n", var_long_names[var_index]);
		fprintf(fpout,"# Original source: mapspam2020; country raster; new glu raster\n");
		fprintf(fpout,"# ----------\n");
		fprintf(fpout,"iso,glu_code,mapspam_crop,value");
		
		// rainfed
		strcpy(fname2, in_args.outpath);
		strcat(fname2, in_args.mapspam_rfd_fname);
		sprintf(tmp_str, "%s%s", (var_names[var_index]), csv_tags[var_index]);
		strcat(fname2, tmp_str);
		fpout2 = fopen(fname2,"w"); //float
		
		if(fpout2 == NULL)
		{
			fprintf(fplog,"Failed to open file  %s for write:  proc_mapspam()\n", fname2);
			return ERROR_FILE;
		}
		// write header lines
		fprintf(fpout2,"# File: %s\n", fname2);
		fprintf(fpout2,"# Author: %s\n", CODENAME);
		fprintf(fpout2,"# Description: mapspam rainfed %s for sage land cells in country X glu\n", var_long_names[var_index]);
		fprintf(fpout2,"# Original source: mapspam2020; country raster; new glu raster\n");
		fprintf(fpout2,"# ----------\n");
		fprintf(fpout2,"iso,glu_code,mapspam_crop,value");
		
		// write the records (rounded to nearest integer)
		for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
			for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
				for (crop_index = 0; crop_index < NUM_MAPSPAM_CROPS; crop_index++) {
					// irrigated
					outval = (float) floor((double) 0.5 + irr_out[ctry_ind][aez_ind][crop_index][var_index]);
					// output only positive values
					if (outval > 0) {
						fprintf(fpout,"\n%s,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
								crop_index+1, outval);
						nrecords_irr++;
					} // end if value is positive
					// rainfed
					outval = (float) floor((double) 0.5 + rfd_out[ctry_ind][aez_ind][crop_index][var_index]);
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
	
	fprintf(fplog, "Wrote file %s: proc_mapspam(); records written=%i\n", fname, nrecords_irr);
	fprintf(fplog, "Wrote file %s: proc_mapspam(); records written=%i\n", fname2, nrecords_rfd);
    }
    
    // Generate yield grid
    var_index = 3; // Access yield var
	strcpy(fname, in_args.outpath);
	strcat(fname, in_args.mapspam_irr_fname);
	sprintf(tmp_str, "%s%s", (var_names[var_index]), csv_tags[var_index]);
	strcat(fname, tmp_str);
	fpout = fopen(fname,"w"); //float
	
	if(fpout == NULL)
	{
		fprintf(fplog,"Failed to open file  %s for write:  proc_mapspam()\n", fname);
		return ERROR_FILE;
	}
	// write header lines
	fprintf(fpout,"# File: %s\n", fname);
	fprintf(fpout,"# Author: %s\n", CODENAME);
	fprintf(fpout,"# Description: mapspam irrigated %s for sage land cells in country X glu\n", var_long_names[var_index]);
	fprintf(fpout,"# Original source: mapspam2020; country raster; new glu raster\n");
	fprintf(fpout,"# ----------\n");
	fprintf(fpout,"iso,glu_code,mapspam_crop,value");
	
	// rainfed
	strcpy(fname2, in_args.outpath);
	strcat(fname2, in_args.mapspam_rfd_fname);
	sprintf(tmp_str, "%s%s", (var_names[var_index]), csv_tags[var_index]);
	strcat(fname2, tmp_str);
	fpout2 = fopen(fname2,"w"); //float
	
	if(fpout2 == NULL)
	{
		fprintf(fplog,"Failed to open file  %s for write:  proc_mapspam()\n", fname2);
		return ERROR_FILE;
	}
	// write header lines
	fprintf(fpout2,"# File: %s\n", fname2);
	fprintf(fpout2,"# Author: %s\n", CODENAME);
	fprintf(fpout2,"# Description: mapspam rainfed %s for sage land cells in country X glu\n", var_long_names[var_index]);
	fprintf(fpout2,"# Original source: mapspam2020; country raster; new glu raster\n");
	fprintf(fpout2,"# ----------\n");
	fprintf(fpout2,"iso,glu_code,mapspam_crop,value");
	
	// write the records (rounded to nearest integer)
	for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
		for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
			for (crop_index = 0; crop_index < NUM_MAPSPAM_CROPS; crop_index++) {
				// irrigated
				outval = (float) floor((double) 0.5 + (irr_out[ctry_ind][aez_ind][crop_index][2]/irr_out[ctry_ind][aez_ind][crop_index][0]));
				// output only positive values
				if (outval > 0 && !isinf(outval)) {
					fprintf(fpout,"\n%s,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
							crop_index+1, outval);
					nrecords_irr++;
				} // end if value is positive
				// rainfed
				outval = (float) floor((double) 0.5 + (rfd_out[ctry_ind][aez_ind][crop_index][2]/rfd_out[ctry_ind][aez_ind][crop_index][0]));
				// output only positive values
				if (outval > 0 && !isinf(outval)) {
					fprintf(fpout2,"\n%s,%i,%i,%.0f", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
							crop_index+1, outval);
					nrecords_rfd++;
				} // end if value is positive
			} // end for crop loop
		} // end for aez loop
	} // end for country loop
		
	fclose(fpout);
	fclose(fpout2);
	
	fprintf(fplog, "Wrote file %s: proc_mapspam(); records written=%i\n", fname, nrecords_irr);
	fprintf(fplog, "Wrote file %s: proc_mapspam(); records written=%i\n", fname2, nrecords_rfd);
    
    free(irr_grid);
    free(rfd_grid);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
        	for (k = 0; k < NUM_MAPSPAM_CROPS; k++) {
        		free(irr_out[i][j][k]);
        		free(rfd_out[i][j][k]);
        	}
            free(irr_out[i][j]);
            free(rfd_out[i][j]);
        }
        free(irr_out[i]);
        free(rfd_out[i]);
    }
    free(irr_out);
    free(rfd_out);
    free(crop_grid);
    free(pasture_grid);
    free(urban_grid);
    for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		free(lu_detail_grid[i]);
	}
	free(lu_detail_grid);
    
    return OK;}

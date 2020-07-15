/**********
 read_soil_carbon.c
 
 read the soil carbon (kg/m^2) associated with sage pot veg cats
    soil carbon is soil only; it does not include root carbon
    data based on the previous level0 file
    sage pot veg cats are 1-15
    
 NOTE: use the pot veg soil carbon table from literature
 the values are averages across different climate zones
 
 NOTE: do not use the gridded data because it is for mixed land types
 2014 revision of IGBP-DIS soil carbon characteristecs database
    the text files have been converted to a binary image, with the two layers added together
    so this is soil carbon for 0-100cm
 
 arguments:
 char* fname:          file name to open, with path
 float* soil_carbon_sage:    the array to load the data into
 args_struct in_args: the input file arguments
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 14 Jan 2016
 
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
 
 **********/

#include "moirai.h"

int read_soil_carbon(args_struct in_args, rinfo_struct *raster_info) {
    
    // use this function to input data to the working grid
    
    // soil c data
    // image file with one band (starts at upper left corner)
    // 4 byte float
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // values are soil carbon (kg/m^2)
    
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize = 4;					// 4 byte floats
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary
    int rv_ind;
    int i,j, k, l, m,n=0;
    int grid_ind;
    char fname[MAXCHAR];			// file name to open
    //char rec_str[MAXRECSIZE];		// string to hold one record
    //const char *delim = ",";		// delimiter string for space separated file
    FILE *fpin;
    int num_read;					// how many values read in
    float *wavg_array;
    float *median_array;
    float *min_array;
    float *max_array;
    float *q1_array;
    float *q3_array;
	float ****soil_carbon_array_cells;
    int err = OK;								// store error code from the dignostic write file
    char out_name1[] = "soil_carbon_wavg.bil";		// file name for output diagnostics raster file
    char out_name2[] = "soil_carbon_median.bil";
    char out_name3[] = "soil_carbon_min.bil";
    char out_name4[] = "soil_carbon_max.bil";
    char out_name5[] = "soil_carbon_q1.bil";
    char out_name6[] = "soil_carbon_q3.bil";
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    int rv_value;           // current ref veg value
    int aez_val;            // current glu value
    int ctry_code;          // current fao country code
    int aez_ind;            // current glu index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    int cur_lt_cat;             // current land type category
    int cur_lt_cat_ind;             // current land type category index
    int num_carbon_states = 5;
    float memory_median=0;
    float memory_min=0;
    float memory_max=0;
    float memory_q1=0;
    float memory_q3=0;

    raster_info->protected_nrows = nrows;
    raster_info->protected_ncols = ncols;
    raster_info->protected_ncells = ncells;
    raster_info->protected_insize = insize;
    raster_info->protected_res = res;
    raster_info->protected_xmin = xmin;
    raster_info->protected_xmax = xmax;
    raster_info->protected_ymin = ymin;
    raster_info->protected_ymax = ymax;
    
   
    
    

soil_carbon_array_cells = calloc(NUM_FAO_CTRY, sizeof(float***));
    if(soil_carbon_array_cells == NULL) {
        fprintf(fplog,"Failed to allocate memory for refveg_carbon_out: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        soil_carbon_array_cells[i] = calloc(ctry_aez_num[i], sizeof(float**));
        if(soil_carbon_array_cells[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            soil_carbon_array_cells[i][j] = calloc(num_lt_cats, sizeof(float*));
            if(soil_carbon_array_cells[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }
                for (k=0 ; k <= num_lt_cats; k++){
                soil_carbon_array_cells[i][j][k] = calloc(NUM_CARBON, sizeof(float));
                if(soil_carbon_array_cells[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k,l);
                    return ERROR_MEM;
                } 
                }
             // end for k loop over output values
            } // end for j loop over aezs
        } // end for j loop over glus
    
   

 
    
    wavg_array = calloc(ncells, sizeof(float));
    if(wavg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }
    
    
    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_wavg_fname);
    printf(in_args.soil_carbon_wavg_fname);
    printf("printing");
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    
    
    
    
    // read the data and check for same size as the working grid
    num_read = fread(wavg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    median_array = calloc(ncells, sizeof(float));
    if(median_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = fread(median_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //min
    min_array = calloc(ncells, sizeof(float));
    if(min_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = fread(min_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //max
    max_array = calloc(ncells, sizeof(float));
    if(max_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = fread(max_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //q1
    q1_array = calloc(ncells, sizeof(float));
    if(q1_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = fread(q1_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //q3
    q3_array = calloc(ncells, sizeof(float));
    if(q3_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = fread(q3_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    } 

                
    

    //First loop to get the number of cells per array
     for (j = 0; j < ncells ; j++){//for each cell
               //assign a grid index
               grid_ind = land_cells_hyde[j];                              
               //assign aez and country code
               aez_val = aez_bounds_new[grid_ind];
               ctry_code = country_fao[grid_ind];

               if (aez_val != -9999) {
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

            aez_ind = NOMATCH;
            for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                if (ctry_aez_list[ctry_ind][i] == aez_val) {
                    aez_ind = i;
                    break;
                }
            }// end for i loop to get aez index

            // get index of sage pot veg; set value to 0 if unknown
            rv_ind = NOMATCH;
            for (i = 0; i < NUM_SAGE_PVLT; i++) {
                if (refveg_thematic[grid_ind] == landtypecodes_sage[i]) {
                    rv_ind = i;
                    break;
                }
            }
			// set the c values for this cell; use existing variables
            if (rv_ind == NOMATCH) {
                rv_value = 0;
				}else {
                rv_value = refveg_thematic[grid_ind];
				
            }

            for (k=0; k< NUM_EPA_PROTECTED; k++){
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

                //calculate the cells to hold within each array
                if(median_array[grid_ind]>0){
                    soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][2]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][2] + 1;} 
                if(min_array[grid_ind]>0){ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][3]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][3] + 1;} 
                if(max_array[grid_ind]>0){ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][4]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][4] + 1;}
                if(q1_array[grid_ind]>0){ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][5]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][5] + 1;}
                if(q3_array[grid_ind]>0){ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][6]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][6] + 1;}
                
                

            }//finish loop for protected areas
        }//finish loop for aez
    }//finish loop for cells


if (in_args.diagnostics) {
        if ((err = write_raster_float(wavg_array, ncells, out_name2, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }

//Second loop for memory allocation
for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++){//for each country
                for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++){//for each aez within each country
                     for (cur_lt_cat_ind = 0; cur_lt_cat_ind < num_lt_cats; cur_lt_cat_ind++){//for each land type
                          n=0;//Setting n to 1 before we start looping over the cells so that we get a value from 0 to whatever for each array         
                               
                
             //create memory for array  
             memory_median = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][2]);
             memory_min = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][3]);
             memory_max = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][4]);
             memory_q1 = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][5]);
             memory_q3 = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind][6]);
            
            //assign memory

            if(memory_median>0) {soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][2]=calloc(memory_median,sizeof(float));}
            if(memory_min>0) {soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][3]=calloc(memory_min,sizeof(float));}
            if(memory_max>0) {soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][4]=calloc(memory_max,sizeof(float));}
            if(memory_q1>0) {soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][5]=calloc(memory_q1,sizeof(float));}
            if(memory_q3>0) {soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][6]=calloc(memory_q3,sizeof(float));}

            }//finish loop for land types
        }//finish loop for aez
    }//finish loop for countries

if (in_args.diagnostics) {
        if ((err = write_raster_float(wavg_array, ncells, out_name3, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }

//final loop to create soil_carbon_array
for (j = 0; j < ncells ; j++){//for each cell
               //assign a grid index
               grid_ind = land_cells_hyde[j];
               
               //assign aez and country code
               aez_val = aez_bounds_new[grid_ind];
               ctry_code = country_fao[grid_ind];

               if (aez_val != -9999) {
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

            aez_ind = NOMATCH;
            for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                if (ctry_aez_list[ctry_ind][i] == aez_val) {
                    aez_ind = i;
                    break;
                }
            }

            // get index of sage pot veg; set value to 0 if unknown
            rv_ind = NOMATCH;
            for (i = 0; i < NUM_SAGE_PVLT; i++) {
                if (refveg_thematic[grid_ind] == landtypecodes_sage[i]) {
                    rv_ind = i;
                    break;
                }
            }
			// set the c values for this cell; use existing variables
            if (rv_ind == NOMATCH) {
                rv_value = 0;
				}else {
                rv_value = refveg_thematic[grid_ind];
				
            }

            for (k=0; k< NUM_EPA_PROTECTED; k++){
				//temporary fractions for protected areas
				//temp_frac = protected_EPA[k][grid_ind];
				
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
                //Allocate the cells to hold within each array
                soil_carbon_sage[1][grid_ind] = wavg_array[grid_ind];
                soil_carbon_sage[2][grid_ind] = wavg_array[grid_ind];
                soil_carbon_sage[3][grid_ind] = wavg_array[grid_ind];
                soil_carbon_sage[4][grid_ind] = wavg_array[grid_ind];
                soil_carbon_sage[5][grid_ind] = wavg_array[grid_ind];
                soil_carbon_sage[6][grid_ind] = wavg_array[grid_ind];
                
                if(median_array[grid_ind]>0){
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][2][grid_ind] = median_array[grid_ind];}
              
                if(min_array[grid_ind]>0){
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][3][grid_ind] = min_array[grid_ind];}

                if(max_array[grid_ind]>0){
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][4][grid_ind] = max_array[grid_ind];}

                if(q1_array[grid_ind]>0){
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][5][grid_ind] = q1_array[grid_ind];}

                if(q3_array[grid_ind]>0){
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][6][grid_ind] = q3_array[grid_ind];}

                 
        


            }//finish loop for protected areas
        }//finish loop for aez
    }//finish loop for cells


    if (in_args.diagnostics) {
        if ((err = write_raster_float(wavg_array, ncells, out_name4, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }    

    free(wavg_array);
    free(median_array);
    free(min_array);
    free(max_array);
    free(q1_array);
    free(q3_array);

    //exit(0);
    return OK;
}
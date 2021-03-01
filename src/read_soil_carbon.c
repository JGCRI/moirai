/**********
 read_soil_carbon.c
 
 read the soil carbon (MgC/ha) associated with sage pot veg cats
    soil carbon is soil only (for a depth of 0-30 cms); it does not include biomass carbon
    data based on the gridded data for each state
    sage pot veg cats are 1-15
    
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
    
    
    //Dimensions of the grid
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
    int i,j, k=0;
    int grid_ind;
    char fname[MAXCHAR];			// file name to open
    FILE *fpin;
    int num_read;					// how many values read in
    float *wavg_array;              //Arrays for each state of carbon
    float *median_array;            //Arrays for each state of carbon
    float *min_array;               //Arrays for each state of carbon
    float *max_array;               //Arrays for each state of carbon
    float *q1_array;                //Arrays for each state of carbon  
    float *q3_array;                //Arrays for each state of carbon
    int err = OK;								// store error code from the dignostic write file
    char out_name1[] = "soil_carbon_wavg.bil";		// file name for output diagnostics raster file
    char out_name2[] = "soil_carbon_median.bil";    // file name for output diagnostics raster file
    char out_name3[] = "soil_carbon_min.bil";       // file name for output diagnostics raster file
    char out_name4[] = "soil_carbon_max.bil";       // file name for output diagnostics raster file
    char out_name5[] = "soil_carbon_q1.bil";        // file name for output diagnostics raster file
    char out_name6[] = "soil_carbon_q3.bil";        // file name for output diagnostics raster file
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
    int memory_median=0;        // This is used to calculate memory to be allocated
    int memory_min=0;           // This is used to calculate memory to be allocated 
    int memory_max=0;           // This is used to calculate memory to be allocated
    int memory_q1=0;            // This is used to calculate memory to be allocated 
    int memory_q3=0;            // This is used to calculate memory to be allocated
    
    
    //This is just overwriting protected areas raster info. But does not matter as both arrays have similar domensions. 
    raster_info->protected_nrows = nrows;
    raster_info->protected_ncols = ncols;
    raster_info->protected_ncells = ncells;
    raster_info->protected_insize = insize;
    raster_info->protected_res = res;
    raster_info->protected_xmin = xmin;
    raster_info->protected_xmax = xmax;
    raster_info->protected_ymin = ymin;
    raster_info->protected_ymax = ymax;
    
   
    
    
    //1. Start with weighted average
    wavg_array = calloc(ncells, sizeof(float));
    if(wavg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }
    
    
    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.soil_carbon_wavg_fname);
    
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    

    //2. Median soil carbon
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
    num_read = (int) fread(median_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //3. Minimum soil carbon
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
    num_read = (int) fread(min_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //4. Maximum soil carbon
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
    num_read = (int) fread(max_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //5. Q1 soil carbon
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
    num_read = (int) fread(q1_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //6. Q3 soil carbon
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
    num_read = (int) fread(q3_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    } 

                
    //fprintf(stdout, "\nSuccessfully starting first for loop in read_soil_c  at %s\n", grid_ind,ctry_ind,aez_ind,cur_lt_cat_ind, get_systime());

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
            if (aez_ind == NOMATCH) {
                fprintf(fplog, "Failed to match aez %i to country %i: proc_refveg_carbon()\n",aez_val,ctry_code);
                return ERROR_IND;
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
                //assign the actual soil carbon numbers
                soil_carbon_sage[0][grid_ind]=wavg_array[grid_ind];
                soil_carbon_sage[1][grid_ind]=median_array[grid_ind];
                soil_carbon_sage[2][grid_ind]=min_array[grid_ind];
                soil_carbon_sage[3][grid_ind]=max_array[grid_ind];
                soil_carbon_sage[4][grid_ind]=q1_array[grid_ind];
                soil_carbon_sage[5][grid_ind]=q3_array[grid_ind];

                //calculate the number of cells to hold within each array
                
                soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]+ 1; 
                
             
             //Convert this to an integer
              memory_median = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]);
              memory_min = (float) floor((double) 0.5+  soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]);
              memory_max = (float) floor((double) 0.5+  soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]);
              memory_q1 = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]);
              memory_q3 = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind]);   
              
            
                //use the calculated number of cells to allocate memory for the soil_carbon_array. The number of cells won't change for veg_carbon so allocate the size of that array here as well. 
                //Don't allocate 0 memory. If size is 0, then keep size at 1. This reduces problems during the free() calls later
                if(memory_median>0){
                
                free(soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][0]);  
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][0]=calloc(memory_median,sizeof(float));
                
                
                free(soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][1]);  
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][1]=calloc(memory_median,sizeof(float));
            
                free(soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][2]);
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][2]=calloc(memory_min,sizeof(float));
            
                free(soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][3]);
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][3]=calloc(memory_max,sizeof(float));
            
                free(soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][4]);
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][4]=calloc(memory_q1,sizeof(float));
            
                free(soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][5]);
                soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][5]=calloc(memory_q3,sizeof(float));

                free(veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][0]);  
                veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][0]=calloc(memory_median,sizeof(float));

                free(veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][1]);  
                veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][1]=calloc(memory_median,sizeof(float));
            
                free(veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][2]);
                veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][2]=calloc(memory_min,sizeof(float));
            
                free(veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][3]);
                veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][3]=calloc(memory_max,sizeof(float));
            
                free(veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][4]);
                veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][4]=calloc(memory_q1,sizeof(float));
            
                free(veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][5]);
                veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind][5]=calloc(memory_q3,sizeof(float));
                }

            }//finish loop for protected areas
        }//finish loop for aez
    }//finish loop for cells


//Print all diagnostics
if (in_args.diagnostics) {
        if ((err = write_raster_float(wavg_array, ncells, out_name1, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }
if (in_args.diagnostics) {
        if ((err = write_raster_float(median_array, ncells, out_name2, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }


    if (in_args.diagnostics) {
        if ((err = write_raster_float(min_array, ncells, out_name3, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }

if (in_args.diagnostics) {
        if ((err = write_raster_float(max_array, ncells, out_name4, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }

if (in_args.diagnostics) {
        if ((err = write_raster_float(q1_array, ncells, out_name5, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }

if (in_args.diagnostics) {
        if ((err = write_raster_float(q3_array, ncells, out_name6, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        }

//Free arrays
    free(wavg_array);
    free(median_array);
    free(min_array);
    free(max_array);
    free(q1_array);
    free(q3_array);
    
    return OK;}

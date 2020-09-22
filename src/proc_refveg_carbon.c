/***********
 proc_refveg_carbon.c
 
 generate csv table of total reference vegetation carbon density (MgC/ha), based on REF_YEAR pot veg area
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
#include <stdlib.h>

 //create a function for comparisons. This function will be used later with qsort    
   int cmpfunc (const void * a, const void * b) {
   return ( *(float*)a - *(float*)b );
}

int proc_refveg_carbon(args_struct in_args, rinfo_struct raster_info) {
    
    // valid values in the hyde land area data set determine the land cells to process
    //kbn 2020-01-06 Adding one more layer for carbon states 
    int i, j, k, l = 0;
    int grid_ind;               // the index for looping over the raster grid
    int rv_ind;                 // the index of the current sage reference veg land type
    int err = OK;				// store error code from the read/write functions
    
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    
    float global_soilc = 0;         // total pot veg soil carbon
    float global_soilc_median = 0;
    float global_soilc_max = 0;
    float global_soilc_min = 0;
    float global_soilc_q1 = 0;
    float global_soilc_q3 = 0;
    float global_vegc = 0;          // total pot veg veg carbon
    float global_vegc_median = 0;
    float global_vegc_max = 0;
    float global_vegc_min = 0;
    float global_vegc_q1 = 0;
    float global_vegc_q3 = 0;
    float outval_soilc;                 // for the output values
    float outval_soilc_median;
    float outval_soilc_min;
    float outval_soilc_max;
    float outval_soilc_q1;
    float outval_soilc_q3;
    float outval_vegc;                 // for the output values
    float outval_vegc_median;
    float outval_vegc_min;
    float outval_vegc_max;
    float outval_vegc_q1;
    float outval_vegc_q3;
    float temp_float;           // temporary float
    int ***soil_carbon_array_size; //temporary size of array, used to get the grid index
    int memory_median=0;           //These are the cell indices to be used for soil_carbon_array and veg_carbon_array
    int memory_min=0;              //These are the cell indices to be used for soil_carbon_array and veg_carbon_array
    int memory_max=0;              //These are the cell indices to be used for soil_carbon_array and veg_carbon_array      
    int memory_q1=0;               //These are the cell indices to be used for soil_carbon_array and veg_carbon_array
    int memory_q3=0;               //These are the cell indices to be used for soil_carbon_array and veg_carbon_array
    int size_max=0;                //These are the cell indices to be used for soil_carbon_array and veg_carbon_array
    float global_soil_temp=0;
    // output table as 4-d array
    float ***refveg_carbon_area;        // the reference area for carbon calculation  
    int soilc_ind = 0;                  // index in output array
    int vegc_ind = 1;                   // index in output array
    int rv_value;           // current ref veg value
    int aez_val;            // current glu value
    int ctry_code;          // current fao country code
    int aez_ind;            // current glu index in ctry_aez_list[ctry_ind]
    int ctry_ind;           // current country index in ctry_aez_list
    int cur_lt_cat;             // current land type category
    int cur_lt_cat_ind;             // current land type category index
    int cur_lt_cat_ind_temp;
    int cur_lt_cat_temp;
    int num_out_vals = 2;   // the number of values to output (soil c den, veg c den, area for averaging)
    int nrecords = 0;       // count # of records written
    
    char fname[MAXCHAR];        // current file name to write
    FILE *fpout;                // out file pointer
    float temp_frac;           //Create temporary fraction for protected areas
    int size=0;                //Integer representing size of array
    int size_temp=0;           //
    // allocate arrays
    

   

    refveg_carbon_out = calloc(NUM_FAO_CTRY, sizeof(float****));
    if(refveg_carbon_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for refveg_carbon_out: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        refveg_carbon_out[i] = calloc(ctry_aez_num[i], sizeof(float***));
        if(refveg_carbon_out[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            refveg_carbon_out[i][j] = calloc(num_lt_cats, sizeof(float**));
            if(refveg_carbon_out[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }
            for (k = 0; k < num_lt_cats; k++) {
                refveg_carbon_out[i][j][k] = calloc(num_out_vals, sizeof(float*));
                if(refveg_carbon_out[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k);
                    return ERROR_MEM;
                }
                for (l=0 ; l < num_out_vals; l++){
                refveg_carbon_out[i][j][k][l] = calloc(NUM_CARBON, sizeof(float));
                if(refveg_carbon_out[i][j][k][l] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i][%i][%i]: proc_refveg_carbon()\n", i, j, k,l);
                    return ERROR_MEM;
                } 
                }// end l loop for carbon states
            } // end for k num_lt_cats
        } // end for j loop over aez
    } // end for i loop over fao country
    


  //Allocate carbon area here
   refveg_carbon_area = calloc(NUM_FAO_CTRY, sizeof(float**));
    if(refveg_carbon_out == NULL) {
        fprintf(fplog,"Failed to allocate memory for refveg_carbon_out: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        refveg_carbon_area[i] = calloc(ctry_aez_num[i], sizeof(float*));
        if(refveg_carbon_area[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            refveg_carbon_area[i][j] = calloc(num_lt_cats, sizeof(float));
            if(refveg_carbon_area[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }    
            } // end for num_lt_cats
        } // end for j loop over aez
    
    //Use this variable to calculate size of arrays since we cannot use Sizeof in a for loop
    soil_carbon_array_size = calloc(NUM_FAO_CTRY, sizeof(int**));
    if(soil_carbon_array_size == NULL) {
        fprintf(fplog,"Failed to allocate memory for refveg_carbon_out: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        soil_carbon_array_size[i] = calloc(ctry_aez_num[i], sizeof(int*));
        if(soil_carbon_array_size[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            soil_carbon_array_size[i][j] = calloc(num_lt_cats, sizeof(int));
            if(soil_carbon_array_size[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }   
             // end for k loop over output values
            } // end for j loop over aezs
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
                if (refveg_thematic[grid_ind] == landtypecodes_sage[i]) {
                    rv_ind = i;
                    break;
                }
            }
			// set the c values for this cell; use existing variables
            if (rv_ind == NOMATCH) {
                rv_value = 0;
				outval_soilc = 0;
				outval_vegc = 0;
				//fprintf(fplog, "Unknown ref veg, cell %i\n", grid_ind);
            } else {
                rv_value = refveg_thematic[grid_ind];
				outval_soilc = 0;
				outval_vegc = 0;
            }
			
            //Calculate temporary land type category. The carbon per fraction of protected area is the same. This will get split out later when we multiply each fraction's total land.
            //To save on time, we are calculating a temporary land type category.
            cur_lt_cat_temp = rv_value * SCALE_POTVEG + 0;
            cur_lt_cat_ind_temp = NOMATCH;
            
            for (i = 0; i < num_lt_cats; i++) {
					if (lt_cats[i] == cur_lt_cat_temp) {
						cur_lt_cat_ind_temp = i;
						break;
					}
				}
				if (cur_lt_cat_ind_temp == NOMATCH) {
					fprintf(fplog, "Failed to match lt_cat %i: proc_refveg_carbon()\n", cur_lt_cat);
					return ERROR_IND;
				}
             //End calculation of temporary land type category



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
                
                
                //fprintf(stdout, "\nStarting cell %i country %i aez %i lt_cat %i started at %s\n", grid_ind,ctry_ind,aez_ind,cur_lt_cat_ind, get_systime());
              if (cur_lt_cat_ind==cur_lt_cat_ind_temp){  
              //Calculate the size here
              soil_carbon_array_size[ctry_ind][aez_ind][cur_lt_cat_ind_temp]=soil_carbon_array_size[ctry_ind][aez_ind][cur_lt_cat_ind_temp]+1;

              
             
             //Now reduce the cells by 1
             //This ensures that we are always allocating in accordance with the size of the arrays and avoiding segmentation faults.
             soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]= soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]-1;
             //Make sure the number of cells is not 0              
              //Convert the curent number of cells to an integer
              //If number of cells is 10 we want to allocate from grid index 0-9. That's why we deduct 1 from above
              memory_median = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]);
              memory_min = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]);
              memory_max = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]);
              memory_q1 = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]);
              memory_q3 = (float) floor((double) 0.5+ soil_carbon_array_cells[ctry_ind][aez_ind][cur_lt_cat_ind_temp]);
              
             //Now use that as the grid index both for soil dnd vegetation carbon
             soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][0][memory_median]=soil_carbon_sage[0][grid_ind];
             soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][1][memory_median]=soil_carbon_sage[1][grid_ind];
             soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][2][memory_min]=soil_carbon_sage[2][grid_ind];
             soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][3][memory_max]=soil_carbon_sage[3][grid_ind];
             soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][4][memory_q1]=soil_carbon_sage[4][grid_ind];
             soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][5][memory_q3]=soil_carbon_sage[5][grid_ind];


             veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][0][memory_median]=veg_carbon_sage[0][grid_ind];
             veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][1][memory_median]=veg_carbon_sage[1][grid_ind];
             veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][2][memory_min]=veg_carbon_sage[2][grid_ind];
             veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][3][memory_max]=veg_carbon_sage[3][grid_ind];
             veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][4][memory_q1]=veg_carbon_sage[4][grid_ind];
             veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][5][memory_q3]=veg_carbon_sage[5][grid_ind];
				
               
				// calculate an area weighted average based on ref veg area for REF_YEAR
				// the unit conversion cancels out when the average is calculated, so don't do it here
				//kbn 2020 Updating below for protected area fractions
                //kbn 2020-06-02 Updating below with revised calculation for carbon states
				// soil c
                //sort arrays 
                
                //size is the current size of the array
                size=soil_carbon_array_size[ctry_ind][aez_ind][cur_lt_cat_ind_temp];
                

                //sort the arrays based on size
                qsort( soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][1],size,sizeof(float),cmpfunc);
                qsort( soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][2],size,sizeof(float),cmpfunc);
                qsort( soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][3],size,sizeof(float),cmpfunc);
                qsort( soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][4],size,sizeof(float),cmpfunc);
                qsort( soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][5],size,sizeof(float),cmpfunc);
              

                qsort( veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][1],size,sizeof(float),cmpfunc);
                qsort( veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][2],size,sizeof(float),cmpfunc);
                qsort( veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][3],size,sizeof(float),cmpfunc);
                qsort( veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][4],size,sizeof(float),cmpfunc);
                qsort( veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][5],size,sizeof(float),cmpfunc);
                }

                //Recalculate size since we need the 'index' now not the size. This is an issue for the max array, q3 array. 
                size_max=size-1;
                //1. weighted average
				refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][0] =
				refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][0] +
				soil_carbon_sage[0][grid_ind] * refveg_area[grid_ind]*temp_frac;

                //2. Median
				size= soil_carbon_array_size[ctry_ind][aez_ind][cur_lt_cat_ind_temp];
                size_temp=size/2;
                temp_float= soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][1][size_temp];
                
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][1] =
				temp_float;

                //3. Min
                size_temp=size/2;
                temp_float= soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][2][0];
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][2] =
				temp_float;
                
                //4. Max
                size_temp=size/2;
                
                temp_float= soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][3][size_max];
                               
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][3] =
				temp_float ;

                //5. Q1
                size_temp=size*0.25;
                temp_float= soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][4][size_temp];


                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][4] =
				temp_float;

                //6. Q3
                size_temp=size*0.75;
                temp_float= soil_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][5][size_temp];

                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][5] =
				temp_float;

				// veg c
                //1. weighted average
				refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][0] =
				refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][0] +
				veg_carbon_sage[0][grid_ind] * refveg_area[grid_ind] * temp_frac;
				
                //2. Median
                size_temp=size/2;
                temp_float= veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][1][size_temp];
                
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][1] =
				temp_float;

                //3. Min
                size_temp=size/2;
                temp_float= veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][2][0];
                
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][2] =
				temp_float;
                
                //4. Max
                size_temp=size/2;
                temp_float= veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][3][size_max];
                               
                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][3] =
				temp_float ;

                //5. Q1
                size_temp=size*0.25;
                temp_float= veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][4][size_temp];


                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][4] =
				temp_float;

                //6. Q3
                size_temp=size*0.75;
                temp_float= veg_carbon_array[ctry_ind][aez_ind][cur_lt_cat_ind_temp][5][size_temp];

                refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][5] =
				temp_float;

				// area
				refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind] =
				refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind] +
				refveg_area[grid_ind]*temp_frac;
				
				
			//fprintf(stdout, "\nSuccessfully processed cell %i country %i aez %i lt_cat %i size %d size_temp %d started at %s\n", grid_ind,ctry_ind,aez_ind,cur_lt_cat_ind,size,size_temp, get_systime());	
			}	// end if valid aez cell
		}   //end k loop for protected areas
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
    fprintf(fpout,"iso,glu_code,land_type,c_type,value,median_value,min_value,max_value,q1_value,q3_value");
    
    // write the records (rounded to integer)
    //Trying to free memory here since the free command seems to crash below
    for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY ; ctry_ind++) {
        for (aez_ind = 0; aez_ind < ctry_aez_num[ctry_ind]; aez_ind++) {
            for (cur_lt_cat_ind = 0; cur_lt_cat_ind < num_lt_cats; cur_lt_cat_ind++) {
				// round the area first to match the proc_land_type area output categories
				temp_float = (float) floor((double) 0.5 + refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind] * KMSQ2HA);
				if (temp_float > 0) {
					
					// output only the carbon values - soil is the first index
					// dived by area to get average, convert to Mg/ha, and round at the end
					
					// soil carbon for each state
					temp_float =  refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][0] /
					refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind];
					outval_soilc = (float) floor((double) 0.5 + temp_float);

                    temp_float =  refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][1];
					
					outval_soilc_median = (float) floor((double) 0.5 + temp_float);
                    
                    temp_float =  refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][2];
					outval_soilc_min = (float) floor((double) 0.5 + temp_float);
                    
                    temp_float =  refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][3];
					outval_soilc_max = (float) floor((double) 0.5 + temp_float);
                    
                    temp_float =  refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][4];
					outval_soilc_q1 = (float) floor((double) 0.5 + temp_float);

                    temp_float =  refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][5];
					outval_soilc_q3 = (float) floor((double) 0.5 + temp_float);

					// sum the total. Need to multiply by 100 to convert km2 to ha
					global_soilc = global_soilc + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][0]*KMSQ2HA);
                    global_soil_temp= refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][1]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA;
                    global_soilc_median = global_soilc_median + global_soil_temp;
                    
                    global_soil_temp= refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][2]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA;
                    global_soilc_min = global_soilc_min + global_soil_temp;

                    
                    global_soil_temp=refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][3]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA;
                    global_soilc_max = global_soilc_max + global_soil_temp;
                    
                    global_soil_temp=refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][4]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA;
                    global_soilc_q1 = global_soilc_q1 + global_soil_temp;
                    
                    global_soil_temp= refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][soilc_ind][5]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA;
                    global_soilc_q3 = global_soilc_q3 + global_soil_temp;
                    
					// write the value only if weighted average is over 0 and all other values are 0 or above.
					if (outval_soilc > 0 &&  outval_soilc_median >=0 && outval_soilc_min >=0 && outval_soilc_max >=0 &&  outval_soilc_q1 >=0  && outval_soilc_q3 >= 0 ) {
						fprintf(fpout,"\n%s,%i,%i,%s", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
								lt_cats[cur_lt_cat_ind], "soil_c (0-30 cms)");
						fprintf(fpout,",%.0f", outval_soilc);
                        fprintf(fpout,",%.0f", outval_soilc_median);
                        fprintf(fpout,",%.0f", outval_soilc_min);
                        fprintf(fpout,",%.0f", outval_soilc_max);
                        fprintf(fpout,",%.0f", outval_soilc_q1);
                        fprintf(fpout,",%.0f", outval_soilc_q3);
						nrecords++;
					}
					
					// veg carbon for each state
					temp_float = refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][0] /
					refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind];
					outval_vegc = (float) floor((double) 0.5 + temp_float);
					
                    temp_float = refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][1];
					outval_vegc_median = (float) floor((double) 0.5 + temp_float);
                    
                    temp_float = refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][2];
					outval_vegc_min = (float) floor((double) 0.5 + temp_float);
                    
                    temp_float = refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][3];
					outval_vegc_max = (float) floor((double) 0.5 + temp_float);

                    temp_float = refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][4];
					outval_vegc_q1 = (float) floor((double) 0.5 + temp_float);

                    temp_float = refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][5];
					outval_vegc_q3 = (float) floor((double) 0.5 + temp_float);
                    // sum the total. Need to multiply by 100 for converting land from km2 to ha
					global_vegc = global_vegc + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][0]*KMSQ2HA);
                    global_vegc_median = global_vegc_median + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][1]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA);
                    global_vegc_min = global_vegc_min + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][2]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA);
                    global_vegc_max = global_vegc_max + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][3]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA);
                    global_vegc_q1 = global_vegc_q1 + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][4]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA);
                    global_vegc_q3 = global_vegc_q3 + (refveg_carbon_out[ctry_ind][aez_ind][cur_lt_cat_ind][vegc_ind][5]*refveg_carbon_area[ctry_ind][aez_ind][cur_lt_cat_ind]*KMSQ2HA);
					
                    // write the value only if weighted average is over 0 and all other values are 0 or above.
					if (outval_vegc > 0 &&  outval_vegc_median >=0 && outval_vegc_min >=0 && outval_vegc_max >=0 &&  outval_vegc_q1 >=0  && outval_vegc_q3 >= 0 ) {
						fprintf(fpout,"\n%s,%i,%i,%s", countryabbrs_iso[ctry_ind], ctry_aez_list[ctry_ind][aez_ind],
								lt_cats[cur_lt_cat_ind], "veg_c (above and below ground biomass)");
						fprintf(fpout,",%.0f", outval_vegc);
                        fprintf(fpout,",%.0f", outval_vegc_median);
                        fprintf(fpout,",%.0f", outval_vegc_min);
                        fprintf(fpout,",%.0f", outval_vegc_max);
                        fprintf(fpout,",%.0f", outval_vegc_q1);
                        fprintf(fpout,",%.0f", outval_vegc_q3);
						nrecords++;
					                    }

                                        
					
				}// end if positive area value 
			} // end for land type loop
        } // end for glu loop
    } // end for country loop
	
    fclose(fpout);	

    // also write the total global carbon values to the log file
    // in Mg 
    fprintf(fplog, "\nGlobal reference carbon values, in Mg: proc_refveg_carbon()\n");
    fprintf(fplog, "Soil C = %f\n", global_soilc);
    fprintf(fplog, "Soil C Median = %f\n", global_soilc_median);
    fprintf(fplog, "Soil C Min = %f\n", global_soilc_min);
    fprintf(fplog, "Soil C Max = %f\n", global_soilc_max);
    fprintf(fplog, "Soil C Q1 = %f\n", global_soilc_q1);
    fprintf(fplog, "Soil C Q3 = %f\n", global_soilc_q3);
    fprintf(fplog, "Veg C = %f\n\n", global_vegc);
    fprintf(fplog, "Veg C Median = %f\n", global_vegc_median);
    fprintf(fplog, "Veg C Min = %f\n", global_vegc_min);
    fprintf(fplog, "Veg C Max = %f\n", global_vegc_max);
    fprintf(fplog, "Veg C Q1 = %f\n", global_vegc_q1);
    fprintf(fplog, "Veg C Q3 = %f\n", global_vegc_q3);
    
  //Free all arrays
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            free(soil_carbon_array_size[i][j]);
        }free(soil_carbon_array_size[i]);
   }free(soil_carbon_array_size);
    
    
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            free(refveg_carbon_area[i][j]);
        }free(refveg_carbon_area[i]);
    }free(refveg_carbon_area);
    
    
  
  for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            for (k = 0; k < num_lt_cats; k++) {
                for(l = 0; l < num_out_vals; l++){
                free(refveg_carbon_out[i][j][k][l]);    
                }
                free(refveg_carbon_out[i][j][k]);
            }
            free(refveg_carbon_out[i][j]);
        }
        free(refveg_carbon_out[i]);
    }
    free(refveg_carbon_out);
  
    
    return err;
    
}
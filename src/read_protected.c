/**********
 read_protected.c
 
 read the protected/suitable data into protected_EPA[ncells][0-7]
 there are six files, and the 0 index is for land area with unkown suitability/protection, which does not appear to occur
 
 The six input layers are:
 L1: all suitable land (including existing cropland and protected area, excluding water and urban)
 L2: L1 minus IUCN protected areas 1a, 1b, and 2
 L3: L2 plus (deforested land in protected areas 1a, 1b, and 2); this is the epa default availability
 L4: L2 minus protected areas 3-6
 All_IUCN: all IUCN protected area (this may include water andor urban and all land cover types)
 IUCN_1a_1b_2: IUCN protected areas 1a, 1b, 2 (this may include water andor urban and all land cover types)
 
 The input values are fractions of grid cell for the input data
 L1-L4 include land only
 Data are converted to mutually exclusive fractions of land area for 8 categories
 Suitable categories 2-5 contain only land
 Unsuitable categories can contain water
 High protection means IUCN categories 1a, 1b, and 2
 Low protection means IUCN categories 3-6
 
0 = Unknown; not currently present with current data
1 = UnsuitableUnprotected
2 = SuitableUnprotected
3 = SuitableHighProtectionIntact
4 = SuitbaleHighProtectionDeforested
5 = SuitableLowProtection
6 = UnsuitableHighProtection
7 = UnsuitableLowProtection
 
 The epa data have been preprocessed to fractions of grid cell area using gdal
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 12 Jan 2016
 
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

int read_protected(args_struct in_args, rinfo_struct *raster_info) {
    
    // use this function to input data to the working grid
    
    // there are seven files for suitable/protected area
    // image file with one band (starts at upper left corner)
    // unsigned char
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // values are integers
    
    int i,j,k;
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize_IUCN = 4;			// 1 byte unsigned char for input
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary
    
    char fname[MAXCHAR];			// file name to open
    FILE *fpin;						// file pointer
    int num_read;					// how many values read in
    float tmp_check = 0.0;          // temporary value to check sum of values
	float land_check = 0.0;          // temporary value to check sum of protected land cat values
	float tmp_sum = 0.0;          	// temporary value to sum fractions
	float fact = 0.0;          		// used for scaling fractions
	float tmp_float;				// for checking
    //kbn 2020-02-29 introduce temporary input arrays for all 6 suitability,protected area raster files
    float *L1_array;
    float *L2_array;
    float *L3_array;
    float *L4_array;
    float *ALL_IUCN_array;
    float *IUCN_1a_1b_2_array;
	
    int err = OK;								// store error code from the write file
    //kbn 2020-02-29 introduce output arrays for all 7 categories
    char out_name_Cat0[]= "Unknown.bil";
    char out_name_Cat1[]= "UnsuitableUnprotected.bil";
    char out_name_Cat2[]= "SuitableUnprotected.bil";
    char out_name_Cat3[]= "SuitableHighProtectionIntact.bil";
    char out_name_Cat4[]= "SuitbaleHighProtectionDeforested.bil";
    char out_name_Cat5[]= "SuitableLowProtection.bil";
    char out_name_Cat6[]= "UnsuitableHighProtection.bil";
    char out_name_Cat7[]= "UnsuitableLowProtection.bil";

    // store file specific info
    raster_info->protected_nrows = nrows;
    raster_info->protected_ncols = ncols;
    raster_info->protected_ncells = ncells;
    raster_info->protected_insize = insize_IUCN;
    raster_info->protected_res = res;
    raster_info->protected_xmin = xmin;
    raster_info->protected_xmax = xmax;
    raster_info->protected_ymin = ymin;
    raster_info->protected_ymax = ymax;
	
	
    //kbn 2020-02-29 Start code to read in suitability and protected area raster files
    
    //1. Start with processing for Category 2 
    
    // allocate the temp array
    //1. L1_array
    L1_array = calloc(ncells, sizeof(float));
    if(L1_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }
    

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.L1_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

     	
    // read the data and check for same size as the working grid
    num_read = fread(L1_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //2. L2_array
    L2_array = calloc(ncells, sizeof(float));
    if(L2_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L2_array: read_protected()\n");
        return ERROR_MEM;
    }
    

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.L2_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

     	
    // read the data and check for same size as the working grid
    num_read = fread(L2_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //L3_Array
    L3_array = calloc(ncells, sizeof(float));
    if(L3_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L3_array: read_protected()\n");
        return ERROR_MEM;
    }
    

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.L3_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

     	
    // read the data and check for same size as the working grid
    num_read = fread(L3_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

    //4. L4_array
    L4_array = calloc(ncells, sizeof(float));
    if(L4_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L4_array: read_protected()\n");
        return ERROR_MEM;
    }
    

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.L4_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

     	
    // read the data and check for same size as the working grid
    num_read = fread(L4_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //5.ALL_IUCN
    ALL_IUCN_array = calloc(ncells, sizeof(float));
    if(ALL_IUCN_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for ALL_IUCN_array: read_protected()\n");
        return ERROR_MEM;
    }
    

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.ALL_IUCN_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

     	
    // read the data and check for same size as the working grid
    num_read = fread(ALL_IUCN_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
   //6. IUCN_1a_1b_2
    IUCN_1a_1b_2_array = calloc(ncells, sizeof(float));
    if(IUCN_1a_1b_2_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for IUCN_1a_1b_2_array: read_protected()\n");
        return ERROR_MEM;
    }
    

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.IUCN_1a_1b_2_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

     	
    // read the data and check for same size as the working grid
    num_read = fread(IUCN_1a_1b_2_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


   //kbn calc category data from input arrays
    for (i = 0; i < ncells; i++) {
		
        //Category 1
        protected_EPA[1][i] = 1 - ALL_IUCN_array[i] - L4_array[i];
        //protected_EPA[1][i] =floor()
        //Category 2
        protected_EPA[2][i] = L4_array[i];
        //Category 3
        protected_EPA[3][i] = L1_array[i] - L3_array[i];
        //Category 4
        protected_EPA[4][i] = L3_array[i] - L2_array[i];
        //Category 5
        protected_EPA[5][i] = L2_array[i] - L4_array[i];
        //Category 6
        protected_EPA[6][i] = IUCN_1a_1b_2_array[i] - L1_array[i] + L2_array[i];
        //Category 7
        protected_EPA[7][i] = ALL_IUCN_array[i] - L2_array[i] + L4_array[i] - IUCN_1a_1b_2_array[i];
		
		// check for negative category values
		// only cat 6 or 7 may be negative, and can be adjusted
		// also sum the categories
		land_check = 0.0;
		for (j = 1; j < NUM_EPA_PROTECTED; j++) {
			if (protected_EPA[j][i] < 0) {
				if (j==6 || j==7) {	// this adjustment is sometimes necessary
					if (j==6) { k = 7;
					} else { k = 6; }
					tmp_check = protected_EPA[k][i];
					protected_EPA[k][i] = protected_EPA[k][i] + protected_EPA[j][i];
					// check for adjustment going negative, which happens due to previous adjustments
					if (protected_EPA[k][i] < 0) {
						if (protected_EPA[k][i] < -ROUND_TOLERANCE) {
							//fprintf(fplog, "Warning: prior fraction %f, corrected fraction %f cat %i, cell %i set to zero: read_protected()\n", tmp_check, protected_EPA[k][i], j, i);
							// correct this by adjusting cat 1 - unsuitable unprotected
							if (protected_EPA[1][i] >= -protected_EPA[k][i]) {
								protected_EPA[1][i] = protected_EPA[1][i] + protected_EPA[k][i];
							} else {
								protected_EPA[1][i] = 0;
							}
						} // end if correction is more negative than tolerance
						protected_EPA[k][i] = 0;
					} // end if correction is negative
					protected_EPA[j][i] = 0;
				} else {
					// this shouldn't happen because of preprocessing, but preprocessing missed a couple of cases
					// but sometimes it happens due to rounding and other times due to small erroneous values
					if (protected_EPA[j][i] > -ROUND_TOLERANCE) {
						// just rounding error
						protected_EPA[j][i] = 0;
					} else {
						if (protected_EPA[5][i] < 0) {
							// this happens when L4 > L2
							// reduce L4 and adjust cats 1, 2, and 7 accordingly
							protected_EPA[1][i] = protected_EPA[1][i] - protected_EPA[5][i];
							protected_EPA[2][i] = protected_EPA[2][i] + protected_EPA[5][i];
							protected_EPA[7][i] = protected_EPA[7][i] + protected_EPA[5][i];
							protected_EPA[5][i] = 0;
						} else if (protected_EPA[3][i] < 0) {
							// this happens only once: when L3 > L1 in cell 2700721
							// reduce L3 and adjust cat 4
							protected_EPA[4][i] = protected_EPA[4][i] + protected_EPA[3][i];
							protected_EPA[3][i] = 0;
						} else {
							fprintf(fplog, "Error in protected fraction cat %i, cell %i: read_protected(); %f is negative\n",
									j, i, protected_EPA[j][i]);
							
							return ERROR_CALC;
						}
					} // end if rounding error else try to fix negative
					
					// need to recheck for negatives again, but 6 and 7 are checked after this separtely
					for (j = 1; j <= 4; j++) {
						if (protected_EPA[j][i] < 0) {
							fprintf(fplog, "Error after correction in protected fraction cat %i, cell %i: read_protected(); %f is negative\n",
									j, i, protected_EPA[j][i]);
							return ERROR_CALC;
						}
					}
					
				} // end if 6,7, else error
			} // end if negative
		} // end for j loop over protected category negative check
		
		// Check for negative or zero grid cells
		land_check = protected_EPA[2][i] + protected_EPA[3][i] + protected_EPA[4][i] + protected_EPA[5][i];
		tmp_check = land_check + protected_EPA[1][i] + protected_EPA[6][i] + protected_EPA[7][i];
		
		// Check if there is hyde area where there is no protected area.
		// so far this does not exist
		if(tmp_check == 0 ){
			if (land_area_hyde[i] > 0){
				protected_EPA[0][i] = 1;
			}
		}
		
		//Check if total value is negative in any grid cell. This should never happen as negatives are captured above.
		if(tmp_check < 0)
		{
			fprintf(fplog, "Error before land normalizattion: cell %i has negative sum %f; read_protected()\n", i, tmp_check);
			return ERROR_CALC;
		}
		
		// Check to ensure grid cells add up to 1
		// The fractions get rescaled below to land area, so this doesn't nee to be fixed here
		// And this condition is currently always false
		tmp_sum = 1 + ROUND_TOLERANCE;
		tmp_float = 1 - ROUND_TOLERANCE;
		if((tmp_check + protected_EPA[0][i]) > (1 + ROUND_TOLERANCE) || (tmp_check + protected_EPA[0][i]) < (1 - ROUND_TOLERANCE))
		{
			fprintf(fplog, "Error before land normalization: cell sum %f != 1+-tolerance in cell=%i; read_protected()\n",tmp_check + protected_EPA[0][i],i);
			return ERROR_CALC;
		}
		
		// fill non-land cells with nodata value, and normalize the rest to fraction of land area
		if (land_area_hyde[i] == raster_info->land_area_hyde_nodata) {
			for (j = 0; j < NUM_EPA_PROTECTED; j++) {
				protected_EPA[j][i] = NODATA;
			}
		} else {
			// don't need to do this if protected area is unknown
			if (protected_EPA[0][i] != 1) {
			
				// scale the values if there isn't enough land for cats 2-5
				tmp_check = land_check * cell_area_hyde[i];
				if (tmp_check > land_area_hyde[i] && tmp_check > 0) {
					//fprintf(fplog, "Warning: protected land cat area %f > land area %f in cell %i; read_protected()\n",
							//tmp_check,land_area_hyde[i], i);
					fact = land_area_hyde[i] / tmp_check;
					tmp_sum = 0.0;
					for (j = 2; j < 6; j++) {
						protected_EPA[j][i] = fact * protected_EPA[j][i];
						tmp_sum += protected_EPA[j][i];
					}
					// don't need to worry about unkown cat0 cuz it is only non-zero (1) if all others are zero
					tmp_check = 1 - tmp_sum;
					tmp_sum = protected_EPA[1][i] + protected_EPA[6][i] + protected_EPA[7][i];
					if (tmp_sum == 0) {
						// put the remainder in unsuitable unprotected as it likely is water
						protected_EPA[1][i] = tmp_check;
						protected_EPA[6][i] = 0;
						protected_EPA[7][i] = 0;
					} else{
						// distribute the remainder proportionally
						fact = tmp_check / tmp_sum;
						protected_EPA[1][i] = fact * protected_EPA[1][i];
						protected_EPA[6][i] = fact * protected_EPA[6][i];
						protected_EPA[7][i] = fact * protected_EPA[7][i];
					}
				} // end if scale to land area
				
				// normalize the total cell fractions to fractions of land area
				// cats 2-5 are all land
				// cats 1, 6, and 7 may include water
				// so loop over 2-5 first
				tmp_sum = 0.0;
				for (j = 2; j < 6; j++) {
					tmp_check = protected_EPA[j][i] * cell_area_hyde[i];
					if (land_area_hyde[i] > 0) {
						protected_EPA[j][i] = tmp_check / land_area_hyde[i];
					} else {
						protected_EPA[j][i] = 0.0;
					}
					tmp_sum += protected_EPA[j][i];
				} // end for loop over protected land categories
				
				// need to assign rest of cats to land as necessary, proportionally
				land_check = land_area_hyde[i] - tmp_sum * land_area_hyde[i];
				if (land_check > 0 && land_area_hyde[i] > 0) {   // this shouldn't be negative as it is scaled above
					tmp_sum = protected_EPA[1][i] + protected_EPA[6][i] + protected_EPA[7][i];
					if (tmp_sum == 0) {
						// this shouldn't happen cuz cat 1 is filled above if this sum is zero, but do it again in case
						// due to rounding error land_check can be ~3x10^-6 while tmp_sum==0
						// since land_check is just above the current round tolerance, just give cat 1 a tiny value
						protected_EPA[1][i] = land_check / land_area_hyde[i];
						protected_EPA[6][i] = 0;
						protected_EPA[7][i] = 0;
					} else {
						// distribute the remaining land proportionally
						fact = land_check / tmp_sum / land_area_hyde[i];
						protected_EPA[1][i] = fact * protected_EPA[1][i];
						protected_EPA[6][i] = fact * protected_EPA[6][i];
						protected_EPA[7][i] = fact * protected_EPA[7][i];
					}
				} else if (land_area_hyde[i] > 0) {
					// reset these only if there is land and land_check is zero (other cats cover all land)
					protected_EPA[1][i] = 0.0;
					protected_EPA[6][i] = 0.0;
					protected_EPA[7][i] = 0.0;
				}
				
			} // end if protected area status is known
		} // end else normalize land cells to land area
		
		// final check on valid cells
		if (land_area_hyde[i] != raster_info->land_area_hyde_nodata) {
		
			tmp_check = 0.0;
			for (j = 0; j < NUM_EPA_PROTECTED; j++) {
				tmp_check += protected_EPA[j][i];
			}
			
			// Check again if total value is negative in any grid cell. This should never happen as negatives are captured above.
			if(tmp_check < 0)
			{
				fprintf(fplog, "Error after land normalization: cell %i has negative sum %f; read_protected()\n", i, tmp_check);
				return ERROR_CALC;
			}
			
			// Check again to ensure grid cells add up to 1
			// currently it is always within rounding tolerance
			tmp_sum = 1 + ROUND_TOLERANCE;
			tmp_float = 1 - ROUND_TOLERANCE;
			if((tmp_check + protected_EPA[0][i]) > (1 + ROUND_TOLERANCE) || (tmp_check + protected_EPA[0][i]) < (1 - ROUND_TOLERANCE))
			{
				fprintf(fplog, "Warning after land normalization: cell sum %f != 1+-tolerance in cell=%i; read_protected()\n",tmp_check + protected_EPA[0][i],i);
				return ERROR_CALC;
			}
			
		} // end if valid cell check protected fractions
		
    } // end for loop over cells
	
   //Write Category data out for diagnostics
    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[1], ncells, out_name_Cat1, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat1);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[2], ncells, out_name_Cat2, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat2);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[3], ncells, out_name_Cat3, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat3);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[4], ncells, out_name_Cat4, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat4);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[5], ncells, out_name_Cat5, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat5);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[6], ncells, out_name_Cat6, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat6);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[7], ncells, out_name_Cat7, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat7);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(protected_EPA[0], ncells, out_name_Cat0, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name_Cat0);
            return ERROR_FILE;
        }
	} // end if diagnostics

    free(L1_array);
    free(L2_array);
    free(L3_array);
    free(L4_array);
    free(IUCN_1a_1b_2_array);
    free(ALL_IUCN_array);
	

    return OK;
}

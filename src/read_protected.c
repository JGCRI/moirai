/**********
 read_protected.c
 
 read the protected designamtion into protected[NUM_CELLS]
 1=protected; 255=unprotected
 no other values
 
 This function changes the input values to:
 1=protected; 2=unprotected
 
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
    
    // the protected image file
    // image file with one band (starts at upper left corner)
    // unsigned char
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // values are integers
    
    int i;
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize = 1;
    int insize_IUCN = 4;					// 1 byte unsigned char for input
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary
    
    char fname[MAXCHAR];			// file name to open
    FILE *fpin;						// file pointer
    int num_read;					// how many values read in
    float tmp_check;                 // temporary array to check values
    unsigned char *in_array;       // temporary array for input
    //kbn 2020-02-29 introduce temporary input arrays for all 6 suitability,protected area raster files
    float *L1_array;
    float *L2_array;
    float *L3_array;
    float *L4_array;
    float *ALL_IUCN_array;
    float *IUCN_1a_1b_2_array;
    float *hyde_temp_array;
    
    int err = OK;								// store error code from the write file
    char out_name[] = "protected.bil";		// diagnositic output raster file name
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
    
    // allocate the temp array
    in_array = calloc(ncells, sizeof(unsigned char));
    if(in_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for in_array: read_protected()\n");
        return ERROR_MEM;
    }
    
    
    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.protected_fname);
	
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }

	
    
    // read the data and check for same size as the working grid
    num_read = fread(in_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    




    // put the data in a short array for storage and further processing
    // also change the values to match the output land categories generation scheme
    for (i = 0; i < ncells; i++) {
        if (in_array[i] == 1) {
            protected_thematic[i] = 1;
        } else {
            protected_thematic[i] = 2;
        }
    }
    
      
    
    // reads and stores input
    //scanf("%d", ncells);
    // displays output
    
    
    if (in_args.diagnostics) {
        if ((err = write_raster_short(protected_thematic, ncells, out_name, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }
    
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
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
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
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
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
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
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
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
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
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
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
   
   //7. Temporary hyde array
   hyde_temp_array = calloc(ncells, sizeof(float));
    if(hyde_temp_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }
    
   strcpy(fname, in_args.inpath);
   strcat(fname, in_args.land_area_hyde_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_protected()\n", fname);
        return ERROR_FILE;
    }
  	
    // read the data and check for same size as the working grid
    num_read = fread(hyde_temp_array, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_protected(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


   //kbn read category  data from arrays as a test
    for (i = 0; i < ncells; i++) {
        //Category 1
        protected_EPA[1][i] = 1 - ALL_IUCN_array[i]- L4_array[i];  
        //protected_EPA[1][i] =floor()
        //Category 2
        protected_EPA[2][i] = L4_array[i];
        //Category 3
        protected_EPA[3][i] = L1_array[i]- L3_array[i];
        //Category 4
        protected_EPA[4][i] = L3_array[i]- L2_array[i];
        //Category 5
        protected_EPA[5][i] = L2_array[i]- L4_array[i];
        //Category 6
        protected_EPA[6][i] = IUCN_1a_1b_2_array[i]- L1_array[i]+L2_array[i];
        //Category 7
        protected_EPA[7][i] = ALL_IUCN_array[i]- L2_array[i] + L4_array[i] - IUCN_1a_1b_2_array[i];
            
    //Check for negative grid cells
    tmp_check = protected_EPA[1][i]+protected_EPA[2][i]+protected_EPA[3][i]+protected_EPA[4][i]+protected_EPA[5][i]+protected_EPA[6][i]+protected_EPA[7][i];
    
    //Check if there is hyde area where there is no protected area. 
    if(tmp_check = 0 ){
        if (hyde_temp_array[i] > 0){
        protected_EPA[0][i] = 1;
    }}
    
    //Check if total value is negative in any grid cell.
    if(tmp_check < 0)
    {
        fprintf(fplog, "Grid cells from EPA are below 0. Please check inputs"
                );
        return ERROR_FILE;
    }
    
    //Check to ensure grid cells add up to 1
    if((tmp_check+protected_EPA[0][i]) > 1)
    {
        fprintf(fplog, "Grid cells from EPA do not add up to 1. Please check inputs sum_CELLS=%i in cell=%i\n",tmp_check,i);
        return ERROR_FILE;
    }
    }
   //Write Category data out for diagnostics
    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[1], ncells, out_name_Cat1, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }

    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[2], ncells, out_name_Cat2, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }
    
    
    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[3], ncells, out_name_Cat3, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }

    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[4], ncells, out_name_Cat4, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }
    
    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[5], ncells, out_name_Cat5, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }

    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[6], ncells, out_name_Cat6, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }

    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[7], ncells, out_name_Cat7, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }
    
    if (in_args.diagnostics) {
        if ((err = write_raster_float(protected_EPA[0], ncells, out_name_Cat0, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }
    
    
    

    
       //Comment in this exit function as a break point for testing.
    //exit(0);

    free(in_array);
    free(L1_array);
    free(L2_array);
    free(L3_array);
    free(L4_array);
    free(IUCN_1a_1b_2_array);
    free(ALL_IUCN_array);
    free(hyde_temp_array);
    

    return OK;
}

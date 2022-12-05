/**********
 read_veg_carbon.c
 
 read the veg carbon input file (MgC/ha) associated with the sage potential vegetation categories
 data based on the gridded inputs
 sage pot veg cats are 1-15
 
 veg carbon includes above ground and below ground biomass
 
 arguments:
 char* fname:          file name to open, with path
 float* veg_carbon_sage:    the array to load the table data into
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

int read_veg_carbon(args_struct in_args, rinfo_struct *raster_info) {
    
    //Grid dimensions
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize = 4;					// 4 byte floats
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary
    
    int i;
    char fname[MAXCHAR];			// file name to open
    
    FILE *fpin;
    int num_read;					// how many values read in
    float *wavg_array;  //Temporary arrays for above ground biomass
    float *wavg_crop_array;  //Temporary arrays for above ground biomass
    float *wavg_pasture_array;  //Temporary arrays for above ground biomass
    float *wavg_urban_array;  //Temporary arrays for above ground biomass
    float *median_crop_array; //Temporary arrays for above ground biomass
    float *median_urban_array; //Temporary arrays for above ground biomass
    float *median_pasture_array; //Temporary arrays for above ground biomass
    float *median_array; //Temporary arrays for above ground biomass
    float *min_array; //Temporary arrays for above ground biomass
    float *min_crop_array; //Temporary arrays for above ground biomass
    float *min_urban_array; //Temporary arrays for above ground biomass
    float *min_pasture_array; //Temporary arrays for above ground biomass
    float *max_array; //Temporary arrays for above ground biomass
    float *max_crop_array; //Temporary arrays for above ground biomass
    float *max_urban_array; //Temporary arrays for above ground biomass
    float *max_pasture_array; //Temporary arrays for above ground biomass
    float *q1_array;  //Temporary arrays for above ground biomass
    float *q1_crop_array;  //Temporary arrays for above ground biomass
    float *q1_urban_array;  //Temporary arrays for above ground biomass
    float *q1_pasture_array;  //Temporary arrays for above ground biomass
    float *q3_crop_array;  //Temporary arrays for above ground biomass
    float *q3_urban_array;  //Temporary arrays for above ground biomass
    float *q3_pasture_array;  //Temporary arrays for above ground biomass
    float *q3_array;  //Temporary arrays for above ground biomass
    float *wavg_bg_array;  //Temporary arrays for above ground biomass
    float *wavg_bg_crop_array;  //Temporary arrays for above ground biomass
    float *wavg_bg_pasture_array;  //Temporary arrays for above ground biomass
    float *wavg_bg_urban_array;  //Temporary arrays for above ground biomass
    float *median_bg_crop_array; //Temporary arrays for above ground biomass
    float *median_bg_urban_array; //Temporary arrays for above ground biomass
    float *median_bg_pasture_array; //Temporary arrays for above ground biomass
    float *median_bg_array; //Temporary arrays for above ground biomass
    float *min_bg_array; //Temporary arrays for above ground biomass
    float *min_bg_crop_array; //Temporary arrays for above ground biomass
    float *min_bg_urban_array; //Temporary arrays for above ground biomass
    float *min_bg_pasture_array; //Temporary arrays for above ground biomass
    float *max_bg_array; //Temporary arrays for above ground biomass
    float *max_bg_crop_array; //Temporary arrays for above ground biomass
    float *max_bg_urban_array; //Temporary arrays for above ground biomass
    float *max_bg_pasture_array; //Temporary arrays for above ground biomass
    float *q1_bg_array;  //Temporary arrays for above ground biomass
    float *q1_bg_crop_array;  //Temporary arrays for above ground biomass
    float *q1_bg_urban_array;  //Temporary arrays for above ground biomass
    float *q1_bg_pasture_array;  //Temporary arrays for above ground biomass
    float *q3_bg_crop_array;  //Temporary arrays for above ground biomass
    float *q3_bg_urban_array;  //Temporary arrays for above ground biomass
    float *q3_bg_pasture_array;  //Temporary arrays for above ground biomass
    float *q3_bg_array;  //Temporary arrays for above ground biomass
	
    int err = OK;								// store error code from the dignostic write file
    char out_name1[] = "veg_carbon_wavg.bil";		// file name for output diagnostics raster file
    char out_name2[] = "veg_carbon_median.bil";     // file name for output diagnostics raster file
    char out_name3[] = "veg_carbon_min.bil";        // file name for output diagnostics raster file
    char out_name4[] = "veg_carbon_max.bil";        // file name for output diagnostics raster file
    char out_name5[] = "veg_carbon_q1.bil";         // file name for output diagnostics raster file
    char out_name6[] = "veg_carbon_q3.bil";         // file name for output diagnostics raster file
    char out_name7[] = "veg_carbon_q3_crop.bil";         // file name for output diagnostics raster file
    char out_name8[] = "veg_carbon_q3_urban.bil";         // file name for output diagnostics raster file
    char out_name9[] = "veg_carbon_q3_pasture.bil";         // file name for output diagnostics raster file
    


    //Similar to read_soil_carbon.c, this is just overwriting protected area raster info. But does not matter since both grids have same dimensions
    raster_info->protected_nrows = nrows;
    raster_info->protected_ncols = ncols;
    raster_info->protected_ncells = ncells;
    raster_info->protected_insize = insize;
    raster_info->protected_res = res;
    raster_info->protected_xmin = xmin;
    raster_info->protected_xmax = xmax;
    raster_info->protected_ymin = ymin;
    raster_info->protected_ymax = ymax;
   

   //1. Allocate weighted average array
    wavg_array = calloc(ncells, sizeof(float));
    if(wavg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_array: read_protected()\n");
        return ERROR_MEM;
    }

    wavg_bg_array =calloc(ncells, sizeof(float));
    if(wavg_bg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_bg_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    //1a. Above ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_wavg_fname);
    
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
   
   //1b. Below ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_bg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


   //2a. Median array
    median_array = calloc(ncells, sizeof(float));
    if(median_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_median_fname);
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
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //2b. Median array (below ground biomass)
    median_bg_array = calloc(ncells, sizeof(float));
    if(median_bg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_bg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

   

    //3a min array (above ground biomass)
    min_array = calloc(ncells, sizeof(float));
    if(min_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //3b min array (below ground biomass)
    min_bg_array = calloc(ncells, sizeof(float));
    if(min_bg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_bg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
     

    //4a. max array
    max_array = calloc(ncells, sizeof(float));
    if(max_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //4b. max array (below ground biomass)
    max_bg_array = calloc(ncells, sizeof(float));
    if(max_bg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_bg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    


    //5a q1 array
    q1_array = calloc(ncells, sizeof(float));
    if(q1_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //5b q1 array (below ground biomass)
    q1_bg_array = calloc(ncells, sizeof(float));
    if(q1_bg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_bg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }



    //6a. q3 array
    q3_array = calloc(ncells, sizeof(float));
    if(q3_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //6b. q3 array (below ground biomass)
    q3_bg_array = calloc(ncells, sizeof(float));
    if(q3_bg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_bg_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

	//Crop
	// create file name and open it
	//1. Allocate weighted average array
    wavg_crop_array = calloc(ncells, sizeof(float));
    if(wavg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    wavg_bg_crop_array =calloc(ncells, sizeof(float));
    if(wavg_bg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_bg_crop_array: read_protected()\n");
        return ERROR_MEM;
    }
    //1a. Above ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_crop_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
   
   //1b. Below ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_crop_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_bg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


   //2a. Median array
    median_crop_array = calloc(ncells, sizeof(float));
    if(median_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_crop_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //2b. Median array (below ground biomass)
    median_bg_crop_array = calloc(ncells, sizeof(float));
    if(median_bg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_crop_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_bg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

   

    //3a min array (above ground biomass)
    min_crop_array = calloc(ncells, sizeof(float));
    if(min_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_crop_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //3b min array (below ground biomass)
    min_bg_crop_array = calloc(ncells, sizeof(float));
    if(min_bg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_crop_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_bg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
     

    //4a. max array
    max_crop_array = calloc(ncells, sizeof(float));
    if(max_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_crop_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //4b. max array (below ground biomass)
    max_bg_crop_array = calloc(ncells, sizeof(float));
    if(max_bg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_crop_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_bg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    


    //5a q1 array
    q1_crop_array = calloc(ncells, sizeof(float));
    if(q1_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_crop_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //5b q1 array (below ground biomass)
    q1_bg_crop_array = calloc(ncells, sizeof(float));
    if(q1_bg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_crop_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_bg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }



    //6a. q3 array
    q3_crop_array = calloc(ncells, sizeof(float));
    if(q3_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_crop_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //6b. q3 array (below ground biomass)
    q3_bg_crop_array = calloc(ncells, sizeof(float));
    if(q3_bg_crop_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_crop_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_crop_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_bg_crop_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


	//Pasture
	// create file name and open it
	//1. Allocate weighted average array
    wavg_pasture_array = calloc(ncells, sizeof(float));
    if(wavg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    wavg_bg_pasture_array =calloc(ncells, sizeof(float));
    if(wavg_bg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_bg_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }
    //1a. Above ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_pasture_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
   
   //1b. Below ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_pasture_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_bg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


   //2a. Median array
    median_pasture_array = calloc(ncells, sizeof(float));
    if(median_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_pasture_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //2b. Median array (below ground biomass)
    median_bg_pasture_array = calloc(ncells, sizeof(float));
    if(median_bg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_pasture_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_bg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

   

    //3a min array (above ground biomass)
    min_pasture_array = calloc(ncells, sizeof(float));
    if(min_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_pasture_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //3b min array (below ground biomass)
    min_bg_pasture_array = calloc(ncells, sizeof(float));
    if(min_bg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_pasture_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_bg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
     

    //4a. max array
    max_pasture_array = calloc(ncells, sizeof(float));
    if(max_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_pasture_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //4b. max array (below ground biomass)
    max_bg_pasture_array = calloc(ncells, sizeof(float));
    if(max_bg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_pasture_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_bg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    


    //5a q1 array
    q1_pasture_array = calloc(ncells, sizeof(float));
    if(q1_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_pasture_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //5b q1 array (below ground biomass)
    q1_bg_pasture_array = calloc(ncells, sizeof(float));
    if(q1_bg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_pasture_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_bg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }



    //6a. q3 array
    q3_pasture_array = calloc(ncells, sizeof(float));
    if(q3_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_pasture_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //6b. q3 array (below ground biomass)
    q3_bg_pasture_array = calloc(ncells, sizeof(float));
    if(q3_bg_pasture_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_pasture_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_pasture_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_bg_pasture_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }



	//Urban
	// create file name and open it
	//1. Allocate weighted average array
    wavg_urban_array = calloc(ncells, sizeof(float));
    if(wavg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    wavg_bg_urban_array =calloc(ncells, sizeof(float));
    if(wavg_bg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for wavg_bg_urban_array: read_protected()\n");
        return ERROR_MEM;
    }
    //1a. Above ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_urban_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
   
   //1b. Below ground biomass (weighted average) 
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_urban_wavg_fname);
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(wavg_bg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }


   //2a. Median array
    median_urban_array = calloc(ncells, sizeof(float));
    if(median_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_urban_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //2b. Median array (below ground biomass)
    median_bg_urban_array = calloc(ncells, sizeof(float));
    if(median_bg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_urban_median_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(median_bg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }

   

    //3a min array (above ground biomass)
    min_urban_array = calloc(ncells, sizeof(float));
    if(min_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_urban_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //3b min array (below ground biomass)
    min_bg_urban_array = calloc(ncells, sizeof(float));
    if(min_bg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_urban_min_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(min_bg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
     

    //4a. max array
    max_urban_array = calloc(ncells, sizeof(float));
    if(max_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_urban_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //4b. max array (below ground biomass)
    max_bg_urban_array = calloc(ncells, sizeof(float));
    if(max_bg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_urban_max_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(max_bg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    


    //5a q1 array
    q1_urban_array = calloc(ncells, sizeof(float));
    if(q1_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_urban_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //5b q1 array (below ground biomass)
    q1_bg_urban_array = calloc(ncells, sizeof(float));
    if(q1_bg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_urban_q1_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q1_bg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }



    //6a. q3 array
    q3_urban_array = calloc(ncells, sizeof(float));
    if(q3_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_urban_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    //6b. q3 array (below ground biomass)
    q3_bg_urban_array = calloc(ncells, sizeof(float));
    if(q3_bg_urban_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_urban_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_BG_urban_q3_fname);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_c()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(q3_bg_urban_array, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_veg_c(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }




      //kbn calc category data from input arrays
    for (i = 0; i < ncells; i++) {
        //above ground +below ground * scaling factor (0.1)
        //TODO: based on feedback, we may want to write out above and below ground biomass separately. Currently we aggegate the two for speed. 
        // First, check if we have only below ground data
        if(wavg_array[i] == NODATA && wavg_bg_array[i] != NODATA ){
        veg_carbon_sage[0][i] = (wavg_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[1][i] = (median_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[2][i] = (min_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[3][i] = (max_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[4][i] = (q1_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[5][i] = (q3_bg_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio[0][i] = 0;
        above_ground_ratio[1][i] = 0;
        above_ground_ratio[2][i] = 0;
        above_ground_ratio[3][i] = 0;
        above_ground_ratio[4][i] = 0;
        above_ground_ratio[5][i] = 0;
       

        // Now, check if we have only above ground data
        }else if(wavg_bg_array[i] == NODATA && wavg_array[i] != NODATA){
        veg_carbon_sage[0][i] = (wavg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[1][i] = (median_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[2][i] = (min_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[3][i] = (max_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[4][i] = (q1_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[5][i] = (q3_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio[0][i] = 1;
        above_ground_ratio[1][i] = 1;
        above_ground_ratio[2][i] = 1;
        above_ground_ratio[3][i] = 1;
        above_ground_ratio[4][i] = 1;
        above_ground_ratio[5][i] = 1;
        
        //Now, check if we don't have both. Assume that the ratio is 0.5. It won't be used in the actual processing.
        }else if(wavg_bg_array[i] == NODATA && wavg_array[i] == NODATA){
        veg_carbon_sage[0][i] = -9999;
        veg_carbon_sage[1][i] = -9999;
        veg_carbon_sage[2][i] = -9999;
        veg_carbon_sage[3][i] = -9999;
        veg_carbon_sage[4][i] = -9999;
        veg_carbon_sage[5][i] = -9999;

        above_ground_ratio[0][i] = 0.5;
        above_ground_ratio[1][i] = 0.5;
        above_ground_ratio[2][i] = 0.5;
        above_ground_ratio[3][i] = 0.5;
        above_ground_ratio[4][i] = 0.5;
        above_ground_ratio[5][i] = 0.5;
        

        //Now, if we have both data,
        }else{
        veg_carbon_sage[0][i] = (wavg_array[i]+wavg_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[1][i] = (median_array[i]+median_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[2][i] = (min_array[i]+min_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[3][i] = (max_array[i]+max_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[4][i] = (q1_array[i]+q1_bg_array[i])*VEG_CARBON_SCALER;
        veg_carbon_sage[5][i] = (q3_array[i]+q3_bg_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio[0][i] = wavg_array[i]/((wavg_array[i]+wavg_bg_array[i]));
        above_ground_ratio[1][i] = median_array[i]/((median_array[i]+median_bg_array[i]));
        above_ground_ratio[2][i] = min_array[i]/((min_array[i]+min_bg_array[i]));
        above_ground_ratio[3][i] = max_array[i]/((max_array[i]+max_bg_array[i]));
        above_ground_ratio[4][i] = q1_array[i]/((q1_array[i]+q1_bg_array[i]));
        above_ground_ratio[5][i] = q3_array[i]/((q3_array[i]+q3_bg_array[i]));
        }



        
        
        //Below ground should be 1 - above ground.
        below_ground_ratio[0][i] = 1 - above_ground_ratio[0][i];  
        below_ground_ratio[1][i] = 1 - above_ground_ratio[1][i];
        below_ground_ratio[2][i] = 1 - above_ground_ratio[2][i];
        below_ground_ratio[3][i] = 1 - above_ground_ratio[3][i];
        below_ground_ratio[4][i] = 1 - above_ground_ratio[4][i];
        below_ground_ratio[5][i] = 1 - above_ground_ratio[5][i];

        /// Above was for unmanaged. Repeat calculations for Crop
        if(wavg_crop_array[i] == NODATA && wavg_bg_crop_array[i] != NODATA ){
        veg_carbon_crop_sage[0][i] = (wavg_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[1][i] = (median_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[2][i] = (min_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[3][i] = (max_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[4][i] = (q1_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[5][i] = (q3_bg_crop_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_crop[0][i] = 0;
        above_ground_ratio_crop[1][i] = 0;
        above_ground_ratio_crop[2][i] = 0;
        above_ground_ratio_crop[3][i] = 0;
        above_ground_ratio_crop[4][i] = 0;
        above_ground_ratio_crop[5][i] = 0;
       

        // Now, check if we have only above ground data
        }else if(wavg_bg_crop_array[i] == NODATA && wavg_crop_array[i] != NODATA){
        veg_carbon_crop_sage[0][i] = (wavg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[1][i] = (median_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[2][i] = (min_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[3][i] = (max_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[4][i] = (q1_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[5][i] = (q3_crop_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_crop[0][i] = 1;
        above_ground_ratio_crop[1][i] = 1;
        above_ground_ratio_crop[2][i] = 1;
        above_ground_ratio_crop[3][i] = 1;
        above_ground_ratio_crop[4][i] = 1;
        above_ground_ratio_crop[5][i] = 1;
        
        //Now, check if we don't have both. Assume that the ratio is 0.5. It won't be used in the actual processing.
        }else if(wavg_crop_array[i] == NODATA && wavg_bg_crop_array[i] == NODATA){
        veg_carbon_crop_sage[0][i] = -9999;
        veg_carbon_crop_sage[1][i] = -9999;
        veg_carbon_crop_sage[2][i] = -9999;
        veg_carbon_crop_sage[3][i] = -9999;
        veg_carbon_crop_sage[4][i] = -9999;
        veg_carbon_crop_sage[5][i] = -9999;

        above_ground_ratio_crop[0][i] = 0.5;
        above_ground_ratio_crop[1][i] = 0.5;
        above_ground_ratio_crop[2][i] = 0.5;
        above_ground_ratio_crop[3][i] = 0.5;
        above_ground_ratio_crop[4][i] = 0.5;
        above_ground_ratio_crop[5][i] = 0.5;
        

        //Now, if we have both data,
        }else{
        veg_carbon_crop_sage[0][i] = (wavg_crop_array[i]+wavg_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[1][i] = (median_crop_array[i]+median_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[2][i] = (min_crop_array[i]+min_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[3][i] = (max_crop_array[i]+max_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[4][i] = (q1_crop_array[i]+q1_bg_crop_array[i])*VEG_CARBON_SCALER;
        veg_carbon_crop_sage[5][i] = (q3_crop_array[i]+q3_bg_crop_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_crop[0][i] = wavg_crop_array[i]/((wavg_crop_array[i]+wavg_bg_crop_array[i]));
        above_ground_ratio_crop[1][i] = median_crop_array[i]/((median_crop_array[i]+median_bg_crop_array[i]));
        above_ground_ratio_crop[2][i] = min_crop_array[i]/((min_crop_array[i]+min_bg_crop_array[i]));
        above_ground_ratio_crop[3][i] = max_crop_array[i]/((max_crop_array[i]+max_bg_crop_array[i]));
        above_ground_ratio_crop[4][i] = q1_crop_array[i]/((q1_crop_array[i]+q1_bg_crop_array[i]));
        above_ground_ratio_crop[5][i] = q3_crop_array[i]/((q3_crop_array[i]+q3_bg_crop_array[i]));
        }
   
        
        //Below ground should be 1 - above ground.
        below_ground_ratio_crop[0][i] = 1 - above_ground_ratio_crop[0][i];  
        below_ground_ratio_crop[1][i] = 1 - above_ground_ratio_crop[1][i];
        below_ground_ratio_crop[2][i] = 1 - above_ground_ratio_crop[2][i];
        below_ground_ratio_crop[3][i] = 1 - above_ground_ratio_crop[3][i];
        below_ground_ratio_crop[4][i] = 1 - above_ground_ratio_crop[4][i];
        below_ground_ratio_crop[5][i] = 1 - above_ground_ratio_crop[5][i];


        ///Repeat calculations for Pasture
        if(wavg_pasture_array[i] == NODATA && wavg_bg_pasture_array[i] != NODATA ){
        veg_carbon_pasture_sage[0][i] = (wavg_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[1][i] = (median_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[2][i] = (min_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[3][i] = (max_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[4][i] = (q1_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[5][i] = (q3_bg_pasture_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_pasture[0][i] = 0;
        above_ground_ratio_pasture[1][i] = 0;
        above_ground_ratio_pasture[2][i] = 0;
        above_ground_ratio_pasture[3][i] = 0;
        above_ground_ratio_pasture[4][i] = 0;
        above_ground_ratio_pasture[5][i] = 0;
       

        // Now, check if we have only above ground data
        }else if(wavg_bg_pasture_array[i] == NODATA && wavg_pasture_array[i] != NODATA){
        veg_carbon_pasture_sage[0][i] = (wavg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[1][i] = (median_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[2][i] = (min_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[3][i] = (max_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[4][i] = (q1_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[5][i] = (q3_pasture_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_pasture[0][i] = 1;
        above_ground_ratio_pasture[1][i] = 1;
        above_ground_ratio_pasture[2][i] = 1;
        above_ground_ratio_pasture[3][i] = 1;
        above_ground_ratio_pasture[4][i] = 1;
        above_ground_ratio_pasture[5][i] = 1;
        
        //Now, check if we don't have both. Assume that the ratio is 0.5. It won't be used in the actual processing.
        }else if(wavg_pasture_array[i] == NODATA && wavg_bg_pasture_array[i] == NODATA){
        veg_carbon_pasture_sage[0][i] = -9999;
        veg_carbon_pasture_sage[1][i] = -9999;
        veg_carbon_pasture_sage[2][i] = -9999;
        veg_carbon_pasture_sage[3][i] = -9999;
        veg_carbon_pasture_sage[4][i] = -9999;
        veg_carbon_pasture_sage[5][i] = -9999;

        above_ground_ratio_pasture[0][i] = 0.5;
        above_ground_ratio_pasture[1][i] = 0.5;
        above_ground_ratio_pasture[2][i] = 0.5;
        above_ground_ratio_pasture[3][i] = 0.5;
        above_ground_ratio_pasture[4][i] = 0.5;
        above_ground_ratio_pasture[5][i] = 0.5;
        

        //Now, if we have both data,
        }else{
        veg_carbon_pasture_sage[0][i] = (wavg_pasture_array[i]+wavg_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[1][i] = (median_pasture_array[i]+median_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[2][i] = (min_pasture_array[i]+min_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[3][i] = (max_pasture_array[i]+max_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[4][i] = (q1_pasture_array[i]+q1_bg_pasture_array[i])*VEG_CARBON_SCALER;
        veg_carbon_pasture_sage[5][i] = (q3_pasture_array[i]+q3_bg_pasture_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_pasture[0][i] = wavg_pasture_array[i]/((wavg_pasture_array[i]+wavg_bg_pasture_array[i]));
        above_ground_ratio_pasture[1][i] = median_pasture_array[i]/((median_pasture_array[i]+median_bg_pasture_array[i]));
        above_ground_ratio_pasture[2][i] = min_pasture_array[i]/((min_pasture_array[i]+min_bg_pasture_array[i]));
        above_ground_ratio_pasture[3][i] = max_pasture_array[i]/((max_pasture_array[i]+max_bg_pasture_array[i]));
        above_ground_ratio_pasture[4][i] = q1_pasture_array[i]/((q1_pasture_array[i]+q1_bg_pasture_array[i]));
        above_ground_ratio_pasture[5][i] = q3_pasture_array[i]/((q3_pasture_array[i]+q3_bg_pasture_array[i]));
        }



        
        
        //Below ground should be 1 - above ground.
        below_ground_ratio_pasture[0][i] = 1 - above_ground_ratio_pasture[0][i];  
        below_ground_ratio_pasture[1][i] = 1 - above_ground_ratio_pasture[1][i];
        below_ground_ratio_pasture[2][i] = 1 - above_ground_ratio_pasture[2][i];
        below_ground_ratio_pasture[3][i] = 1 - above_ground_ratio_pasture[3][i];
        below_ground_ratio_pasture[4][i] = 1 - above_ground_ratio_pasture[4][i];
        below_ground_ratio_pasture[5][i] = 1 - above_ground_ratio_pasture[5][i];

        ///Repeat calculations for Urban
        if(wavg_urban_array[i] == NODATA && wavg_bg_urban_array[i] != NODATA ){
        veg_carbon_urban_sage[0][i] = (wavg_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[1][i] = (median_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[2][i] = (min_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[3][i] = (max_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[4][i] = (q1_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[5][i] = (q3_bg_urban_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_urban[0][i] = 0;
        above_ground_ratio_urban[1][i] = 0;
        above_ground_ratio_urban[2][i] = 0;
        above_ground_ratio_urban[3][i] = 0;
        above_ground_ratio_urban[4][i] = 0;
        above_ground_ratio_urban[5][i] = 0;
       

        // Now, check if we have only above ground data
        }else if(wavg_bg_urban_array[i] == NODATA && wavg_urban_array[i] != NODATA){
        veg_carbon_urban_sage[0][i] = (wavg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[1][i] = (median_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[2][i] = (min_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[3][i] = (max_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[4][i] = (q1_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[5][i] = (q3_urban_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_urban[0][i] = 1;
        above_ground_ratio_urban[1][i] = 1;
        above_ground_ratio_urban[2][i] = 1;
        above_ground_ratio_urban[3][i] = 1;
        above_ground_ratio_urban[4][i] = 1;
        above_ground_ratio_urban[5][i] = 1;
        
        //Now, check if we don't have both. Assume that the ratio is 0.5. It won't be used in the actual processing.
        }else if(wavg_urban_array[i] == NODATA && wavg_bg_urban_array[i] == NODATA){
        veg_carbon_urban_sage[0][i] = -9999;
        veg_carbon_urban_sage[1][i] = -9999;
        veg_carbon_urban_sage[2][i] = -9999;
        veg_carbon_urban_sage[3][i] = -9999;
        veg_carbon_urban_sage[4][i] = -9999;
        veg_carbon_urban_sage[5][i] = -9999;

        above_ground_ratio_urban[0][i] = 0.5;
        above_ground_ratio_urban[1][i] = 0.5;
        above_ground_ratio_urban[2][i] = 0.5;
        above_ground_ratio_urban[3][i] = 0.5;
        above_ground_ratio_urban[4][i] = 0.5;
        above_ground_ratio_urban[5][i] = 0.5;
        

        //Now, if we have both data,
        }else{
        veg_carbon_urban_sage[0][i] = (wavg_urban_array[i]+wavg_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[1][i] = (median_urban_array[i]+median_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[2][i] = (min_urban_array[i]+min_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[3][i] = (max_urban_array[i]+max_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[4][i] = (q1_urban_array[i]+q1_bg_urban_array[i])*VEG_CARBON_SCALER;
        veg_carbon_urban_sage[5][i] = (q3_urban_array[i]+q3_bg_urban_array[i])*VEG_CARBON_SCALER;

        above_ground_ratio_urban[0][i] = wavg_urban_array[i]/((wavg_urban_array[i]+wavg_bg_urban_array[i]));
        above_ground_ratio_urban[1][i] = median_urban_array[i]/((median_urban_array[i]+median_bg_urban_array[i]));
        above_ground_ratio_urban[2][i] = min_urban_array[i]/((min_urban_array[i]+min_bg_urban_array[i]));
        above_ground_ratio_urban[3][i] = max_urban_array[i]/((max_urban_array[i]+max_bg_urban_array[i]));
        above_ground_ratio_urban[4][i] = q1_urban_array[i]/((q1_urban_array[i]+q1_bg_urban_array[i]));
        above_ground_ratio_urban[5][i] = q3_urban_array[i]/((q3_urban_array[i]+q3_bg_urban_array[i]));
        }



        
        
        //Below ground should be 1 - above ground.
        below_ground_ratio_urban[0][i] = 1 - above_ground_ratio_urban[0][i];  
        below_ground_ratio_urban[1][i] = 1 - above_ground_ratio_urban[1][i];
        below_ground_ratio_urban[2][i] = 1 - above_ground_ratio_urban[2][i];
        below_ground_ratio_urban[3][i] = 1 - above_ground_ratio_urban[3][i];
        below_ground_ratio_urban[4][i] = 1 - above_ground_ratio_urban[4][i];
        below_ground_ratio_urban[5][i] = 1 - above_ground_ratio_urban[5][i]; 
         
    }



	
   //Write diagnostics
    if (in_args.diagnostics) {
        if ((err = write_raster_float(veg_carbon_sage[0], ncells, out_name1, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        
        if ((err = write_raster_float(veg_carbon_sage[1], ncells, out_name2, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name2);
            return ERROR_FILE;
        }
        
        if ((err = write_raster_float(veg_carbon_sage[2], ncells, out_name3, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name3);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_sage[3], ncells, out_name4, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name4);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_sage[4], ncells, out_name5, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name5);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_sage[5], ncells, out_name6, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name6);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_crop_sage[5], ncells, out_name7, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name6);
            return ERROR_FILE;
        }
        if ((err = write_raster_float(veg_carbon_urban_sage[5], ncells, out_name8, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name6);
            return ERROR_FILE;
        }
        if ((err = write_raster_float(veg_carbon_pasture_sage[5], ncells, out_name9, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name6);
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
    free(wavg_bg_array);
    free(median_bg_array);
    free(min_bg_array);
    free(max_bg_array);
    free(q1_bg_array);
    free(q3_bg_array);
    free(wavg_crop_array);
    free(median_crop_array);
    free(min_crop_array);
    free(max_crop_array);
    free(q1_crop_array);
    free(q3_crop_array);
    free(wavg_bg_crop_array);
    free(median_bg_crop_array);
    free(min_bg_crop_array);
    free(max_bg_crop_array);
    free(q1_bg_crop_array);
    free(q3_bg_crop_array);
    free(wavg_urban_array);
    free(median_urban_array);
    free(min_urban_array);
    free(max_urban_array);
    free(q1_urban_array);
    free(q3_urban_array);
    free(wavg_bg_urban_array);
    free(median_bg_urban_array);
    free(min_bg_urban_array);
    free(max_bg_urban_array);
    free(q1_bg_urban_array);
    free(q3_bg_urban_array);
    free(wavg_pasture_array);
    free(median_pasture_array);
    free(min_pasture_array);
    free(max_pasture_array);
    free(q1_pasture_array);
    free(q3_pasture_array);
    free(wavg_bg_pasture_array);
    free(median_bg_pasture_array);
    free(min_bg_pasture_array);
    free(max_bg_pasture_array);
    free(q1_bg_pasture_array);
    free(q3_bg_pasture_array);
    
    return OK;}

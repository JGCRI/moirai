/**********
 read_veg_carbon.c
 
 read the veg carbon text file (kg/m^2) associated with the sage potential vegetation categories
 data based on the previous level0 file
 sage pot veg cats are 1-15
 
 veg carbon includes root carbon, in addition to above ground carbon
 
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
    
    int i;
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
	
    int err = OK;								// store error code from the dignostic write file
    char out_name1[] = "veg_carbon_wavg.bil";		// file name for output diagnostics raster file
    char out_name2[] = "veg_carbon_median.bil";
    char out_name3[] = "veg_carbon_min.bil";
    char out_name4[] = "veg_carbon_max.bil";
    char out_name5[] = "veg_carbon_q1.bil";
    char out_name6[] = "veg_carbon_q3.bil";
    
    raster_info->protected_nrows = nrows;
    raster_info->protected_ncols = ncols;
    raster_info->protected_ncells = ncells;
    raster_info->protected_insize = insize;
    raster_info->protected_res = res;
    raster_info->protected_xmin = xmin;
    raster_info->protected_xmax = xmax;
    raster_info->protected_ymin = ymin;
    raster_info->protected_ymax = ymax;

    wavg_array = calloc(ncells, sizeof(float));
    if(wavg_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for L1_array: read_protected()\n");
        return ERROR_MEM;
    }

    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.veg_carbon_wavg_fname);
    printf(in_args.veg_carbon_wavg_fname);
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
    strcat(fname, in_args.veg_carbon_median_fname);
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
    strcat(fname, in_args.veg_carbon_min_fname);
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
    strcat(fname, in_args.veg_carbon_max_fname);
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
    strcat(fname, in_args.veg_carbon_q1_fname);
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
    strcat(fname, in_args.veg_carbon_q3_fname);
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
      //kbn calc category data from input arrays
    for (i = 0; i < ncells; i++) {

        veg_carbon_sage[1][i] = wavg_array[i];
        veg_carbon_sage[2][i] = median_array[i];
        veg_carbon_sage[3][i] = min_array[i];
        veg_carbon_sage[4][i] = max_array[i];
        veg_carbon_sage[5][i] = q1_array[i];
        veg_carbon_sage[6][i] = q3_array[i];
    }


    if (in_args.diagnostics) {
        if ((err = write_raster_float(veg_carbon_sage[1], ncells, out_name1, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name1);
            return ERROR_FILE;
        }
        
        if ((err = write_raster_float(veg_carbon_sage[2], ncells, out_name2, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name2);
            return ERROR_FILE;
        }
        
        if ((err = write_raster_float(veg_carbon_sage[3], ncells, out_name3, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name3);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_sage[4], ncells, out_name4, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name4);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_sage[5], ncells, out_name5, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name5);
            return ERROR_FILE;
        }

        if ((err = write_raster_float(veg_carbon_sage[6], ncells, out_name6, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name6);
            return ERROR_FILE;
        }
        
        }
    /*
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_carbon()\n", fname);
        return ERROR_FILE;
    }
    
    // read the data and check for consistency
    num_read = fread(soil_carbon_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_carbon(); num_read=%i != NUMCELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
     
    if (in_args.diagnostics) {
        if ((err = write_raster_float(soil_carbon_grid, ncells, out_name, in_args))) {
            fprintf(fplog, "Error writing file %s: read_soil_carbon()\n", out_name);
            return err;
        }
    }
     
     */
    free(wavg_array);
    free(median_array);
    free(min_array);
    free(max_array);
    free(q1_array);
    free(q3_array);
    //exit(0);
    
    return OK;
}
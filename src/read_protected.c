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
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
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
    int insize = 1;					// 1 byte unsigned char for input
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary
    
    char fname[MAXCHAR];			// file name to open
    FILE *fpin;						// file pointer
    int num_read;					// how many values read in
    
    unsigned char *in_array;       // temporary array for input
    
    int err = OK;								// store error code from the write file
    char out_name[] = "protected.bil";		// diagnositic output raster file name
    
    // store file specific info
    raster_info->protected_nrows = nrows;
    raster_info->protected_ncols = ncols;
    raster_info->protected_ncells = ncells;
    raster_info->protected_insize = insize;
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
    
    if (in_args.diagnostics) {
        if ((err = write_raster_short(protected_thematic, ncells, out_name, in_args))) {
            fprintf(fplog, "Error writing file %s: read_protected()\n", out_name);
            return err;
        }
    }
    
    free(in_array);
    
    return OK;
}

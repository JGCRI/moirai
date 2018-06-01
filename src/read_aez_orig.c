/**********
 read_aez_orig.c
 
 read the original aez boundaries into aez_bounds_orig[NUM_CELLS]
 
 read the orig aez boundaries with the GCAM GIS countries numbers attached
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 9 May 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

int read_aez_orig(args_struct in_args, rinfo_struct *raster_info) {
	
	// use this function to input data to the working grid
	
	// the original aez image file, corrected for the piece of australia in western africa
	// BIL file with one band (starts at upper left corner)
	// 4 byte signed integers
	// 5 arcmin resolution, extent = (-180,180, -90, 90), WGS84
	// values are 1 - 18 global climate aezs
	
	int nrows = 2160;				// num input lats
	int ncols = 4320;				// num input lons
	int ncells = nrows * ncols;		// number of input grid cells
	int nodata = -9999;			// nodata value
	int insize = 4;					// 4 byte integers for input
	double res = 5.0 / 60.0;		// resolution
	double xmin = -180.0;			// longitude min grid boundary
	double xmax = 180.0;			// longitude max grid boundary
	double ymin = -90.0;			// latitude min grid boundary
	double ymax = 90.0;				// latitude max grid boundary
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	int num_read;					// how many values read in
	
	int err = OK;								// store error code from the write file
	char out_name[] = "aez_bounds_orig.bil";	// diagnositic output raster file name
	
	// store file specific info
	raster_info->aez_orig_nrows = nrows;
	raster_info->aez_orig_ncols = ncols;
	raster_info->aez_orig_ncells = ncells;
	raster_info->aez_orig_nodata = nodata;
	raster_info->aez_orig_insize = insize;
	raster_info->aez_orig_res = res;
	raster_info->aez_orig_xmin = xmin;
	raster_info->aez_orig_xmax = xmax;
	raster_info->aez_orig_ymin = ymin;
	raster_info->aez_orig_ymax = ymax;
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.aez_orig_fname);
	
	if((fpin = fopen(fname, "rb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_aez_orig()\n", fname);
		return ERROR_FILE;
	}
	
	// read the data
	num_read = fread(aez_bounds_orig, insize, ncells, fpin);
	fclose(fpin);
	if(num_read != ncells)
	{
		fprintf(fplog, "Error reading file %s: read_aez_orig(); num_read=%i != overlap_cols=%i\n",
				fname, num_read, ncells);
		return ERROR_FILE;
	}
		
	if (in_args.diagnostics) {
		if ((err = write_raster_int(aez_bounds_orig, ncells, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: read_aez_orig()\n", out_name);
			return err;
		}
	}
	
	return OK;
}
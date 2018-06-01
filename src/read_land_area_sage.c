/**********
 read_land_area_sage.c
 
 read the sage land fraction
 use spherical earth cell area to convert to area (km^2)
	(need to calculate this because the hyde data are only for the hyde land cells)
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 10 May 2013
 Updated to read actual SAGE land fraction data: April 2014
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

int read_land_area_sage(args_struct in_args, rinfo_struct *raster_info) {
	
	// use this function to input data to the working grid
	
	// from sage land fraction data
	// BIL file with one band (starts at upper left corner)
	// 4 byte float
	// 5 arcmin resolution, extent = (-180,180, -90, 90), WGS84
	// values are unitless fraction of grid cell (0 to 1)
	
	int i;
	int nrows = 2160;				// num input lats
	int ncols = 4320;				// num input lons
	int ncells = nrows * ncols;		// number of input grid cells
	float nodata = NODATA;			// nodata value
	int insize = 4;					// 4 byte floats
	double res = 5.0 / 60.0;		// resolution
	double xmin = -180.0;			// longitude min grid boundary
	double xmax = 180.0;			// longitude max grid boundary
	double ymin = -90.0;			// latitude min grid boundary
	double ymax = 90.0;				// latitude max grid boundary
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	int num_read;					// how many values read in
	
	int err = OK;								// store error code from the write file
	char out_name[] = "land_area_sage.bil";		// file name for output diagnostics raster file
	
	// store file specific info
	raster_info->land_area_sage_nrows = nrows;
	raster_info->land_area_sage_ncols = ncols;
	raster_info->land_area_sage_ncells = ncells;
	raster_info->land_area_sage_nodata = nodata;
	raster_info->land_area_sage_insize = insize;
	raster_info->land_area_sage_res = res;
	raster_info->land_area_sage_xmin = xmin;
	raster_info->land_area_sage_xmax = xmax;
	raster_info->land_area_sage_ymin = ymin;
	raster_info->land_area_sage_ymax = ymax;
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.land_area_sage_fname);
	
	if((fpin = fopen(fname, "rb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_land_area_sage()\n", fname);
		return ERROR_FILE;
	}
	
	// read the data
	num_read = fread(land_area_sage, insize, ncells, fpin);
	fclose(fpin);
	if(num_read != ncells)
	{
		fprintf(fplog, "Error reading file %s: read_land_area_sage(); num_read=%i != ncells=%i\n",
				fname, num_read, ncells);
		return ERROR_FILE;
	}
	
	// use spherical earth grid cell area to convert land fraction to land area
	for (i = 0; i < ncells; i++) {
		if (land_area_sage[i] != nodata) {
			land_area_sage[i] = cell_area[i] * land_area_sage[i];
		}
	}
	
	if (in_args.diagnostics) {
		if ((err = write_raster_float(land_area_sage, ncells, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: read_land_area_sage()\n", out_name);
			return err;
		}
	}
	
	return OK;
}
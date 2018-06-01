/**********
 read_cropland_sage.c
 
 read the sage cropland data into cropland_area_sage[NUM_CELLS]
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 14 May 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

int read_cropland_sage(args_struct in_args, rinfo_struct *raster_info) {
	
	// use this function to input data to the working grid
	
	// SAGE 2000 cropland data
	// netcdf file, all variables are float values
	// variable farea has 4 dimensions, in order: time (1), level (1), latitude (2160), longitude (4320)
	// latitude ranges from 90 to -90; longitude from -180 to 180
	// so the data will be read row by row from the upper left corner
	// 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
	// input units are fraction of land area
	// convert to working units of_sage km^2, based on sage land area data
	
	int i;							// loop variable
	int nrows = 2160;				// num input lats
	int ncols = 4320;				// num input lons
	int ncells = nrows * ncols;		// number of input grid cells
	float nodata = 9E20;			// nodata value
	double res = 5.0 / 60.0;		// resolution
	double xmin = -180.0;			// longitude min grid boundary
	double xmax = 180.0;			// longitude max grid boundary
	double ymin = -90.0;			// latitude min grid boundary
	double ymax = 90.0;				// latitude max grid boundary
	
	char fname[MAXCHAR];			// file name to open
	int ncid;						// netcdf file id
	int ncvarid;					// variable id returned by nc_inq_varid()
	int ncerr;						// error return value; 0 = ok
	char *varname = "farea";		// name of the variable to read
	
	double sage_cropland_lost = 0;		// number of sage cropland cells lost due to no sage land area
	
	int err = OK;									// store error code from the write file
	char out_name[] = "cropland_area_sage.bil";		// diagnositic output raster file name
	
	// store file specific info
	raster_info->cropland_sage_nrows = nrows;
	raster_info->cropland_sage_ncols = ncols;
	raster_info->cropland_sage_ncells = ncells;
	raster_info->cropland_sage_nodata = nodata;
	raster_info->cropland_sage_res = res;
	raster_info->cropland_sage_xmin = xmin;
	raster_info->cropland_sage_xmax = xmax;
	raster_info->cropland_sage_ymin = ymin;
	raster_info->cropland_sage_ymax = ymax;
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.cropland_sage_fname);
	
	if ((ncerr = nc_open(fname, NC_NOWRITE, &ncid))) {
		fprintf(fplog,"Failed to open %s for reading: read_cropland_sage(); ncerr = %i\n", fname, ncerr);
		return ERROR_FILE;
	}
	
	if ((ncerr = nc_inq_varid(ncid, varname, &ncvarid))) {
		fprintf(fplog,"Error %i when getting netcdf var id for %s: read_cropland_sage()\n", ncerr, varname);
		return ERROR_FILE;
	}
	
	if ((ncerr = nc_get_var_float(ncid, ncvarid, cropland_area_sage))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_cropland_sage()\n", ncerr, varname);
		return ERROR_FILE;
	}
	
	nc_close(ncid);
	
	// convert fraction to area
	// also count any cells that are sage cropland but not sage land area
		
	// loop over all the cells
	for (i = 0; i < ncells; i++) {
		
		if (land_area_sage[i] == 0 || land_area_sage[i] == raster_info->land_area_sage_nodata) {
			// zero land value so set cropland to 0 area
			cropland_area_sage[i] = 0;
			//  count whether there is a cropland value here
			if (cropland_area_sage[i] > 0 && cropland_area_sage[i] != raster_info->cropland_sage_nodata) {
				sage_cropland_lost = sage_cropland_lost + cropland_area_sage[i];
			}
		} else {
			if (cropland_area_sage[i] != raster_info->cropland_sage_nodata) {
				cropland_area_sage[i] = cropland_area_sage[i] * land_area_sage[i];
			}
		}
	} // end i loop over all cells

	// write the global area tracking value to the log file
	fprintf(fplog, "\nGlobal sage cropland fraction tracking (summed fraction): read_cropland_sage():\n");
	fprintf(fplog, "sage_cropland_lost = %f\n\n", sage_cropland_lost);
	
	if (in_args.diagnostics) {
		if ((err = write_raster_float(cropland_area_sage, ncells, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: read_cropland_sage()\n", out_name);
			return err;
		}
	}
	
	return OK;
}
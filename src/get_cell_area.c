/**********
 get_cell_area.c

 calculate the spherical earth grid cell area for all cells and store it in cell_area[NUM_CELLS] (km^2)
 
 This is the area of the surface of a sphere delineated by the lat/lon values given
 from integral r^2.cos(lat).dlat.dlon; lat from -pi/2 to pi/2, lon from 0 to 2pi
 Uses the AVE_ER earth radius defined in moirai.h
 assumes that lat and long values are valid: within +-90 and +-180
	longitude values can also be between 0 and 360
	it is really the difference that matters, so be careful at the longitude number boundary
	if area of a cell straddling +-180 is needed, shift lon inputs to maintain same dlon
 the cells should never straddle the pole (there is a check for this)
 if area of pole cap is needed, make lat1=pole, lat2=cap limit, lon1=180, lon2=-180
 the lat/lon limits can be in any order: also taken care of by the absoulte values
 
 I estimated the error with respect to spheroid earth to range from .22% (equator) to .45% (pole) of spherical value,
 with an unlikely maximum of .67% in the case where r1 and r2 are the axes of the spheroid
 This is based on spherical comparisons of area at the different radii, assuming similar curvature to the spheroid
 
 also read the cell area for the hyde data into cell_area_hyde[NUM_CELLS]
 
 arguments:
 args_struct in_args: the input file arguments
 rinfo_struct *raster_info: information about input raster data
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 14 May 2013
 
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

int get_cell_area(args_struct in_args, rinfo_struct *raster_info) {
	
	// use this function to input data to the working grid
	// the cell area is calculated for all cells of the working grid
	
	// the HYDE data grid is the same as the working grid
	// HYDE data is originally an ascii file, but using convertd binary file
	// BIL file with one band (starts at upper left corner)
    // 4 byte float values
	// 5 arcmin resolution, extent = (-180,180, -90, 90), spherical earth, ?WGS84?
	// read in float values
	// input units are km^2
	// working units are km^2
	
	int i;
	int nrows = 2160;				// num input lats
	int ncols = 4320;				// num input lons
	int ncells = nrows * ncols;		// number of input grid cells
	float nodata = -9999;			// nodata value
    int insize = 4;                 // 4 byte float
	double res = 5.0 / 60.0;		// resolution
	double xmin = -180.0;			// longitude min grid boundary
	double xmax = 180.0;			// longitude max grid boundary
	double ymin = -90.0;			// latitude min grid boundary
	double ymax = 90.0;				// latitude max grid boundary
	
	int rowind, colind;				// keep track of which cell
	double dlon, conv, lat1, lat2;	// temporary values for calculating cell area
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
    int num_read;					// how many values read in
	
	int err = OK;							// store error code from the write file
	char out_name[] = "cell_area.bil";		// diagnostic output raster file name
	char out_name_hyde[] = "cell_area_hyde.bil";		// diagnostic output raster file name
	
	// store file/data specific info
	// the hyde grid matches the sage grid
	raster_info->cell_area_nrows = nrows;
	raster_info->cell_area_ncols = ncols;
	raster_info->cell_area_ncells = ncells;
	raster_info->cell_area_nodata = NODATA;
	raster_info->cell_area_res = res;
	raster_info->cell_area_xmin = xmin;
	raster_info->cell_area_xmax = xmax;
	raster_info->cell_area_ymin = ymin;
	raster_info->cell_area_ymax = ymax;
	
	raster_info->cell_area_hyde_nrows = nrows;
	raster_info->cell_area_hyde_ncols = ncols;
	raster_info->cell_area_hyde_ncells = ncells;
	raster_info->cell_area_hyde_nodata = nodata;
	raster_info->cell_area_hyde_res = res;
	raster_info->cell_area_hyde_xmin = xmin;
	raster_info->cell_area_hyde_xmax = xmax;
	raster_info->cell_area_hyde_ymin = ymin;
	raster_info->cell_area_hyde_ymax = ymax;
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.cell_area_fname);
	
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  get_cell_area()\n", fname);
        return ERROR_FILE;
    }
    
    // read the data
    num_read = fread(cell_area_hyde, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != ncells)
    {
        fprintf(fplog, "Error reading file %s: get_cell_area(); num_read=%i != ncells=%i\n",
                fname, num_read, ncells);
        return ERROR_FILE;
    }
	fclose(fpin);
    
	// calculate the grid cell area
	for (i = 0; i < ncells; i++) {
         
		// first get the lat boundaries and lon diff of the cell, in arc-seconds
		rowind = (int) (i / ncols);
		colind = i - rowind * ncols;
		lat1 = 90 * DEG2SEC - rowind * GRID_RES_SEC;
		lat2 = lat1 - GRID_RES_SEC;
		dlon = GRID_RES_SEC;
		conv = DEG2RAD * SEC2DEG;
		
		// check for pole straddle
		if((lat1 > 90 * DEG2SEC && lat2 < 90 * DEG2SEC) || (lat2 > 90 * DEG2SEC && lat1 < 90 * DEG2SEC) ||
		   (lat1 > -90 * DEG2SEC && lat2 < -90 * DEG2SEC) || (lat2 > -90 * DEG2SEC && lat1 < -90 * DEG2SEC))
		{
			fprintf(stderr, "Error calculating cell area: lat1=%lf and lat2=%lf straddle a pole; get_cell_area()\n", lat1, lat2);
			return ERROR_CALC;
		}

		cell_area[i] = MSQ2KMSQ * AVE_ER * AVE_ER * dlon * conv * fabs(sin(lat2 * conv) - sin(lat1 * conv));
		
	}	// end for i loop to read and calculate the data
	
	if (in_args.diagnostics) {
		if ((err = write_raster_float(cell_area, ncells, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: get_cell_area()\n", out_name);
			return err;
		}
		if ((err = write_raster_float(cell_area_hyde, ncells, out_name_hyde, in_args))) {
			fprintf(fplog, "Error writing hyde file %s: get_cell_area()\n", out_name_hyde);
			return err;
		}
	}
	
	return OK;
}
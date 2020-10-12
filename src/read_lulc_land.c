/**********
 read_lulc_land.c
 
 read one ISAM LULC netcdf file to get the land mask
	the files are gzipped orignially
	this function will unzip them if necessary and then leave them unzipped
 these are half-degree files (for now)
 origin is: lower left corner at -90 lat and 0 lon
 
 store the mask in the working grid
 so need to disaggregate and match the cells
 
 the file has 3 attribute variables and one additional dimension index:
	longitude (360), latitude (720), lc_type (34, enumerated, no actual attribute variable), time (1)
 the 34 lc_types are merely an index in the LC_fraction variable
 the data variables are:
 byte Dominant_type(latitude, longitude); only 17 land types, but different index than LC_fraction
 float Grid_area(latitude, longitude); sq meters of entire grid cell, but masked by a land mask (see note below)
 float LC_fraction(lc_type, latitude, longitude); percent of grid cell * 100
 byte Mask(latitude, longitude); land=1, ocean = 0; !!! check a couple of spots in africa, india, and australia, and kalimantan and papua new guinea that have been masked out!!!
 byte Regionmask(latitude, longitude); ocean plus 10 regions
 float latitude; center of pixel
 float longitude; center of pixel
 short time; year
 
 arguments:
 args_struct in_args:   the input file arguments
 int year
 int* land_mask_lulc:      the 1-d array to read the land mask into
  
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on Dec 7 Sep 2017
 
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

int read_lulc_land(args_struct in_args, int year, rinfo_struct *raster_info, int *land_mask_lulc) {
	
	int i, m, n;
	int nrows = 360;				// num input lats
	int ncols = 720;				// num input lons
	int ncells = nrows * ncols;		// number of input grid cells
	float nodata = -99.0;           // nodata value - appears to be only in Dominant_type and Grid_area
	double res = 30.0 / 60.0;		// input resolution
	double xmin = 0.0;			// input longitude min grid boundary
	double xmax = 360.0;			// input longitude max grid boundary
	double ymin = -90.0;			// input latitude min grid boundary
	double ymax = 90.0;				// input latitude max grid boundary
	
	char lname[MAXCHAR];			// file name to open
	char tmp_str[MAXCHAR];			// temporary string
	FILE *fpin;						// file pointer
	int sysrv;						// system return value
	int ncid;						// netcdf file id
	int ncvarid;					// variable id returned by nc_inq_varid()
	int ncerr;						// error return value; 0 = ok
	const char varname[] = "Mask";         // the land mask variable to read
	static size_t start_grid[] = {0, 0};            // start indices for other data variables
	static size_t count_grid[] = {360, 720};        // lengths for reading other data variables
	
	int num_split;					// the number of cells to disaggregate to in 1 dimension
	int grid_y_ul;				// row for ul corner working grid cell in input cell
	int grid_x_ul;				// col for ul corner working grid cell in input cell
	int ind_1d;					// index of working grid cell to set
	double rem_dbl;				// used to get the remainder of a decimal number
	double int_dbl;				// used to get the integer of a decimal number
	int *lulc_input_mask;			// read into here
	
	// some input data file name prefixes and suffixes
	const char basename[] = "ISAM_HYDE32_LANDCOVER_";		// base file name
	const char nctag[] = ".nc";					// suffix for file names, netcdf, unzipped
	const char ncgztag[] = ".nc.gz";			// suffix for file names, netcdf, gzipped
	
	int err = OK;								// store error code from the write file
	char out_name[] = "land_mask_lulc.bil";		// diagnositic output raster file name
	
	// store file specific info
	raster_info->lulc_input_nrows = nrows;
	raster_info->lulc_input_ncols = ncols;
	raster_info->lulc_input_ncells = ncells;
	raster_info->lulc_input_nodata = nodata;
	raster_info->lulc_input_res = res;
	raster_info->lulc_input_xmin = xmin;
	raster_info->lulc_input_xmax = xmax;
	raster_info->lulc_input_ymin = ymin;
	raster_info->lulc_input_ymax = ymax;
	
	// allcate array for the grid cell area
	lulc_input_mask = calloc(ncells, sizeof(int));
	if(lulc_input_mask == NULL) {
		fprintf(fplog,"Failed to allocate memory for lulc_input_mask: read_lulc_land()\n");
		return ERROR_MEM;
	}
	
	// finish file name and try to open it; if it fails, then it has not been unzipped
	strcpy(lname, in_args.lulcpath);
	strcat(lname, basename);
	sprintf(tmp_str, "%i%s", year, nctag);
	strcat(lname, tmp_str);
	if((fpin = fopen(lname, "rb")) == NULL)
	{
		// gunzip this file
		strcpy(lname, "gunzip -k ");
		strcat(lname, in_args.lulcpath);
		strcat(lname, basename);
		sprintf(tmp_str, "%i%s", year, ncgztag);
		strcat(lname, tmp_str);
		sysrv = system(lname);
	} else {
		fclose(fpin);
	}
	
	// now make the unzipped file name again
	strcpy(lname, in_args.lulcpath);
	strcat(lname, basename);
	sprintf(tmp_str, "%i%s", year, nctag);
	strcat(lname, tmp_str);
	
	if ((ncerr = nc_open(lname, NC_NOWRITE, &ncid))) {
		fprintf(fplog,"Failed to open %s for reading: read_lulc_land(); ncerr = %i\n", lname, ncerr);
		return ERROR_FILE;
	}
	
	// get the land mask
	if ((ncerr = nc_inq_varid(ncid, varname, &ncvarid))) {
		fprintf(fplog,"Error %i when getting netcdf var id for %s: read_lulc_land()\n", ncerr, varname);
		return ERROR_FILE;
	}
	if ((ncerr = nc_get_vara_int(ncid, ncvarid, start_grid, count_grid, lulc_input_mask))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_lulc_land()\n", ncerr, varname);
		return ERROR_FILE;
	}
	nc_close(ncid);
	
	// loop over all the data to convert the values to working grid
	num_split = NUM_LON / ncols;
	for (i = 0; i < ncells; i++) {
		rem_dbl = fmod((double) i, (double) ncols);
		modf((double) (i / ncols), &int_dbl);
		grid_y_ul = NUM_LAT - (int) int_dbl * (num_split) - num_split;
		if (rem_dbl < ncols / 2) {
			grid_x_ul = (int) rem_dbl * (num_split) + NUM_LON / 2;
		} else {
			grid_x_ul = ((int) rem_dbl - ncols / 2) * (num_split);
		}
		
		// now loop over the working grid cells to set the land mask
		for (m = grid_y_ul; m < grid_y_ul + num_split; m++) {
			// first calc the 1-d index of the first pixel in this row
			ind_1d = m * NUM_LON + grid_x_ul;
			for (n = ind_1d; n < ind_1d + num_split; n++) {
				land_mask_lulc[n] = lulc_input_mask[i];
			} // end for n loop over the cells to set
		} // end for m loop over the rows to set
		
	}	// end for i loop over all input grid cells
	
	free(lulc_input_mask);
	
	if (in_args.diagnostics) {
		if ((err = write_raster_int(land_mask_lulc, NUM_CELLS, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: read_lulc_land()\n", out_name);
			return err;
		}
	}
	
	return OK;}
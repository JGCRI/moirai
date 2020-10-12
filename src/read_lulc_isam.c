/**********
 read_lulc_isam.c
 
 read one ISAM LULC netcdf file
	the files are gzipped orignially
	this function will unzip them if necessary and then leave them unzipped (and keep the zipped file)
    these are half-degree files (for now)
    origin is: lower left corner at -90 lat and 0 lon

 store the area in km^2 of each land type (1-34)
    store in a 2-d array [NUM_LULC_TYPES][NUM_LAT_LULC * NUM_LON_LULC]
 	starting with upper left corner, lon varying fastest (like other rasters)
 	so the input data has to be shifted
 
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
 float** lulc_input_grid:     the array to read the data into; first dim is the land type, second is the data grid in 1-d format
 
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

int read_lulc_isam(args_struct in_args, int year, float **lulc_input_grid) {
    
    int i, j;
    int nrows = NUM_LAT_LULC;				// num input lats
    int ncols = NUM_LON_LULC;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    //float nodata = -99.0;             // nodata value - appears to be only in Dominant_type and Grid_area
    //double res = 30.0 / 60.0;		// resolution
    //double xmin = 0.0;			// longitude min grid boundary
    //double xmax = 360.0;			// longitude max grid boundary
    //double ymin = -90.0;			// latitude min grid boundary
    //double ymax = 90.0;			// latitude max grid boundary
	float frac_scalar = 0.0001;		// scalar to convert the input value to an actual fraction
    
    char lname[MAXCHAR];			// file name to open
	char tmp_str[MAXCHAR];			// temporary string
    FILE *fpin;						// file pointer
    int sysrv;						// system return value
    int ncid;						// netcdf file id
    int ncvarid;					// variable id returned by nc_inq_varid()
    int ncerr;						// error return value; 0 = ok
    const char lcfrac_name[] = "LC_fraction";         // the lc frac variable to read
    const char cell_area_name[] = "Grid_area";        // the grid cell area variable to read
    size_t start_lcfrac[] = {0, 0, 0};              // start indices for lc fraction - the first value will change for each lc type
    static size_t start_grid[] = {0, 0};            // start indices for other data variables
    static size_t count_lcfrac[] = {1, 360, 720};   // lengths for reading lc fraction
    static size_t count_grid[] = {360, 720};        // lengths for reading other data variables
	
	int grid_y;				// row for ul corner working grid cell in input cell
	int grid_x;				// col for ul corner working grid cell in input cell
	int grid_index;					// index of working grid cell to set
	double rem_dbl;				// used to get the remainder of a decimal number
	double int_dbl;				// used to get the integer of a decimal number
	
	float *lulc_cell_area;			// needed to get the area
	float **temp_grid;				// needed for reading in so can shift the data
	
    // some input data file name prefixes and suffixes
    const char basename[] = "ISAM_HYDE32_LANDCOVER_";		// base file name
    const char nctag[] = ".nc";					// suffix for file names, netcdf, unzipped
    const char ncgztag[] = ".nc.gz";			// suffix for file names, netcdf, gzipped
	
	// allcate array for the grid cell area
	lulc_cell_area = calloc(ncells, sizeof(float));
	if(lulc_cell_area == NULL) {
		fprintf(fplog,"Failed to allocate memory for lulc_cell_area: read_lulc_isam()\n");
		return ERROR_MEM;
	}
	
	// allocate temp array for the data
	temp_grid = calloc(NUM_LULC_TYPES, sizeof(float*));
	if(temp_grid == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for temp_grid: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		temp_grid[i] = calloc(NUM_CELLS_LULC, sizeof(float));
		if(temp_grid[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for temp_grid[%i]: main()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
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
        fprintf(fplog,"Failed to open %s for reading: read_lulc_isam(); ncerr = %i\n", lname, ncerr);
        return ERROR_FILE;
    }
    
    // get the grid cell area
	if ((ncerr = nc_inq_varid(ncid, cell_area_name, &ncvarid))) {
		fprintf(fplog,"Error %i when getting netcdf var id for %s: read_lulc_isam()\n", ncerr, cell_area_name);
		return ERROR_FILE;
	}
	if ((ncerr = nc_get_vara_float(ncid, ncvarid, start_grid, count_grid, lulc_cell_area))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_lulc_isam()\n", ncerr, cell_area_name);
		return ERROR_FILE;
	}
	
    // loop over the land cover types to read them in
	if ((ncerr = nc_inq_varid(ncid, lcfrac_name, &ncvarid))) {
		fprintf(fplog,"Error %i when getting netcdf var id for %s: read_lulc_isam()\n", ncerr, lcfrac_name);
		return ERROR_FILE;
	}
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		start_lcfrac[0] = i;
		if ((ncerr = nc_get_vara_float(ncid, ncvarid, start_lcfrac, count_lcfrac, &temp_grid[i][0]))) {
			fprintf(fplog,"Error %i when reading netcdf var %s: read_lulc_isam()\n", ncerr, lcfrac_name);
			return ERROR_FILE;
		}
	} // end for i loop over the lulc types
	
    // loop over all the data to convert the values to working units and shift the data to start at upper left
    // do the land type aggregation and the grid disaggregation in a different function
	//	because eventually they may not be necessary
    for (i = 0; i < ncells; i++) {
		// loop over the land types
		for (j = 0; j < NUM_LULC_TYPES; j++) {
			
			rem_dbl = fmod((double) i, (double) ncols);
			modf((double) (i / ncols), &int_dbl);
			grid_y = NUM_LAT_LULC - (int) int_dbl - 1;
			if (rem_dbl < ncols / 2) {
				grid_x = (int) rem_dbl + NUM_LON_LULC / 2;
			} else {
				grid_x = ((int) rem_dbl - ncols / 2);
			}
			grid_index = grid_y * NUM_LON_LULC + grid_x;
			if (lulc_cell_area[i] > 0) {
				lulc_input_grid[j][grid_index] = temp_grid[j][i] * frac_scalar * MSQ2KMSQ * lulc_cell_area[i];
			} else {
				lulc_input_grid[j][grid_index] = 0;
			}
		} // end j loop over the land types
    }	// end for i loop over all grid cells
    
    nc_close(ncid);
	
	free(lulc_cell_area);
	
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		free(temp_grid[i]);
	}
	free(temp_grid);
	
    return OK;}

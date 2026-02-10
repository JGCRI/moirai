/**********
 read_mapspam.c
 
 read one file of the mapSPAM 2020 irragated/rainfed area into irr_grid[NUM_CELLS] and rfd_grid[NUM_CELLS]
    the stored data are in the working grid, but without unit conversion
 
 there are separate files for irrigated and rainfed data
 
 there is a file for each of 46 crops, with the files labelled with crop names
 
 the 5 arcmin mapSPAM files
  starts at upper left corner (-180,90)
  units are hectares
  No header
  the crop # has 1 digit for #<10, and 2 digits for #>=10
 
 also store hectares - no unit conversion
 
 arguments:
  char* fname:          file name to open, with path
  float* mapspam_grid:    the array to load the data into
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Roan Chadsey on 10 Feb 2026
 
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
 
 int read_mapspam(char *fname, float *mapspam_grid) { // Inputs are the same as the original for now. A pointer to the file we want to read and a pointer to the array we want to write to
 
 	// use this function to input data to the working grid
    
    // mapSPAM 2020 irrigated/raindfed data
    // envi ascii grid file
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // read in double values
    
    // define variables
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize_IUCN = 4;			// 1 byte unsigned char for input // check that this is the correct format still
    double res = 5.0 / 60.0;		// resolution
    double xmin = -180.0;			// longitude min grid boundary
    double xmax = 180.0;			// longitude max grid boundary
    double ymin = -90.0;			// latitude min grid boundary
    double ymax = 90.0;				// latitude max grid boundary

    FILE *fpin;						// file pointer
    int num_read;					// check number of values we read in to confirm dimensions are correct

    // open fname to fpin pointer
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_mapspam()\n", fname);
        return ERROR_FILE;
    }

    // read the data and check for same size as the working grid
    num_read = (int) fread(mapspam_grid, insize_IUCN, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_mapspam(); num_read=%i != NUM_CELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
 }
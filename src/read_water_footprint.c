/**********************
  read_water_footprint.c

  read a single water footprint file
  these are single band esri grid files

 water footprint files (converted to simple binary files from arc grid files by an r script):
 4320 columns, 2160 rows, 5 arcmin res
 geographic projection of wgs84
 the original data have been resampled to the full grid with upper left corner at -180, +90
 float data type
 the nodata value is -9999
 units are average annual mm for entire grid cell area (1996-2005)
 the .gri files are the binary raster data
 the .grd files are the text header info

  units are average annual mm for entire grid cell area (1996-2005)

  see Mekonnen and Hoekstra 2011:
    Mekonnen, M. M., Hoekstra, A. Y., 2011. The green, blue and grey water footprint of crops and derived crop products, Hydrol. Earth Syst. Sci., 15(5): 1577-1600.

  ARGUMENTS
      char* fname:       file name to open, with path
      float* wf_grid:    the array to load the data into; this is the full globe, and has been initialized to the file nodata value

  so read the data into the appropriate location in the grid array
  row index: (90-83)*60/5 - 1
  num columns = 4320
  1-d grid index to start at = 4320 * ((90-83)*60/5 - 1)
 
  return value:
  integer error code: OK = 0, otherwise a non-zero error code

  Created by Alan Di Vittorio on 2/25/16.
 
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

***********************/
 
#include "moirai.h"

int read_water_footprint(char *fname, float *wf_grid) {
    
    int ncols = 4320;
    int nrows = 2160;
    int ncells = nrows * ncols;		// number of input grid cells
    int insize = 4;					// 4 byte floats
    
    FILE *fpin;						// file pointer
    int num_read;					// how many values read in
    
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_water_footprint()\n", fname);
        return ERROR_FILE;
    }
    
    // read the data
    num_read = fread(wf_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != ncells)
    {
        fprintf(fplog, "Error reading file %s: read_water_footprint(); num_read=%i != ncells=%i\n",
                fname, num_read, ncells);
        return ERROR_FILE;
    }
    
    return OK;
}

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
 Copyright 2018 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 This file is part of Moirai.
 
 Moirai is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Moirai is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Moirai (/moirai/docs/COPYING.txt). If not, see <https://www.gnu.org/licenses/>.

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

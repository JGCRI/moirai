/**********
 read_mirca.c
 
 read one file of the mirca 2000 irragated/rainfed area into irr_grid[NUM_CELLS] and rfd_grid[NUM_CELLS]
    the stored data are in the working grid, but without unit conversion
 
 there are separate files for irrigated and rainfed data
 
 there is a file for each of 26 crops, with the files labelled with crop numbers
 
 the 5 arcmin MIRCA files
  starts at upper left corner (-180,90)
  units are hectares
  6 header lines: ncols, nrows, xllcorner (pixel corner), yllcorner (pixel corner), cellsize, NODATA_value
  file names are built from the input_MIRCA_area.txt lines
  the crop # has 1 digit for #<10, and 2 digits for #>=10
 
 also store hectares - no unit conversion
 
 arguments:
  char* fname:          file name to open, with path
  float* mirca_grid:    the array to load the data into
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 11 Jan 2016
 
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

int read_mirca(char *fname, float *mirca_grid) {
    
    // use this function to input data to the working grid
    
    // MIRCA 2000 irrigated/raindfed data
    // envi ascii grid file
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // read in double values
    
    int i;
    int nrows = 0;			// num input lats = 2160
    int ncols = 0;			// num input lons = 4320
    int ncells = 0;         // number of input grid cells = nrows*ncols
    int nodata = 0;			// nodata value = -9
    double res = 0;         // resolution = 5.0 / 60.0 = 0.083333333333333
    double xmin = 0;		// longitude min grid boundary = xllcorner = -180
    //double xmax = 180.0;	// longitude max grid boundary
    double ymin = 0;		// latitude min grid boundary = yllcorner = -90
    //double ymax = 90.0;		// latitude max grid boundary
    
    FILE *fpin;						// file pointer
    float value;						// each value read in
    
    if((fpin = fopen(fname, "r")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_mirca()\r\n", fname);
        return ERROR_FILE;
    }
    
    // read the header lines
    if(fscanf(fpin,"%*s%i%*s%i%*s%lf%*s%lf%*s%lf%*s%i%*[^\r\n]\r\n", &ncols, &nrows, &xmin, &ymin, &res, &nodata) == EOF)
    {
        fprintf(fplog, "Failed to read file %s header:  read_mirca()\n", fname);
        return ERROR_FILE;
    }
    
    // check the res
    if (ncols != NUM_LON || nrows != NUM_LAT) {
        printf("File %s dims do not match expected values:  read_mirca()\n", fname);
        return ERROR_FILE;
    }
    
    //fprintf(fplog,"Start reading mirca at %s :  read_mirca()\n", get_systime());
    
    // read the data
    ncells = nrows * ncols;
    for (i = 0; i < ncells; i++) {
        if (fscanf(fpin, "%f", &value) != EOF) {
            // no need to convert units
            mirca_grid[i] = value;
        } else {
            if (i == ncells) {
                //fprintf(fplog,"Finished reading mirca at %s:  read_mirca()\n", get_systime());
            } else {
                fprintf(fplog, "Failed to read mirca at %s:  read_mirca()\n", get_systime());
                return ERROR_FILE;
            }	// end if all data read in else error
            
        }	// end if read and set value else check for end of file or error
        
    }	// end for i loop to read the data
    
    fclose(fpin);
    
    return OK;
}
/**********
 read_soil_carbon.c
 
 read the soil carbon (kg/m^2) associated with sage pot veg cats
    soil carbon is soil only; it does not include root carbon
    data based on the previous level0 file
    sage pot veg cats are 1-15
    
 NOTE: use the pot veg soil carbon table from literature
 the values are averages across different climate zones
 
 NOTE: do not use the gridded data because it is for mixed land types
 2014 revision of IGBP-DIS soil carbon characteristecs database
    the text files have been converted to a binary image, with the two layers added together
    so this is soil carbon for 0-100cm
 
 arguments:
 char* fname:          file name to open, with path
 float* soil_carbon_sage:    the array to load the data into
 args_struct in_args: the input file arguments
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 14 Jan 2016
 
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

int read_soil_carbon(char *fname, float *soil_carbon_sage, args_struct in_args) {
    
    // use this function to input data to the working grid
    
    // soil c data
    // image file with one band (starts at upper left corner)
    // 4 byte float
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // values are soil carbon (kg/m^2)
    
    //int nrows = 2160;				// num input lats
    //int ncols = 4320;				// num input lons
    //int ncells = nrows * ncols;		// number of input grid cells
    //int insize = 4;					// 4 byte floats
    //double nodata = -9999
    //double res = 5.0 / 60.0;		// resolution
    //double xmin = -180.0;			// longitude min grid boundary
    //double xmax = 180.0;			// longitude max grid boundary
    //double ymin = -90.0;			// latitude min grid boundary
    //double ymax = 90.0;				// latitude max grid boundary
    
    int i;
    char rec_str[MAXRECSIZE];		// string to hold one record
    const char *delim = ",";		// delimiter string for space separated file
    FILE *fpin;
    //int num_read;					// how many values read in
    
    int err = OK;								// store error code from the dignostic write file
    //char out_name[] = "soil_carbon.bil";		// file name for output diagnostics raster file
    
    // use the text table for now
    if((fpin = fopen(fname, "r")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_carbon()\n", fname);
        return ERROR_FILE;
    }
    // skip the header line
    if(fscanf(fpin, "%*[^\n]\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_soil_carbon()\n", fname);
        return ERROR_FILE;
    }
    // read the records
    for (i = 0; i < NUM_SAGE_PVLT; i++) {
        if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
            // get the carbon value
            if((err = get_float_field(rec_str, delim, 2, &soil_carbon_sage[i])) != OK) {
                fprintf(fplog, "Error processing file %s: read_soil_carbon(); record=%i, column=2\n",
                        fname, i + 1);
                return err;
            }
        }else {
            fprintf(fplog, "Error reading file %s: read_soil_carbon(); record=%i\n", fname, i + 1);
            return ERROR_FILE;
        }
    } // end for loop over records
    
    fclose(fpin);
    
    /*
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_soil_carbon()\n", fname);
        return ERROR_FILE;
    }
    
    // read the data and check for consistency
    num_read = fread(soil_carbon_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_soil_carbon(); num_read=%i != NUMCELLS=%i\n",
                fname, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
     
    if (in_args.diagnostics) {
        if ((err = write_raster_float(soil_carbon_grid, ncells, out_name, in_args))) {
            fprintf(fplog, "Error writing file %s: read_soil_carbon()\n", out_name);
            return err;
        }
    }
     
     */
    
    return OK;
}
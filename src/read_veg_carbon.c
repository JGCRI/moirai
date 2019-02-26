/**********
 read_veg_carbon.c
 
 read the veg carbon text file (kg/m^2) associated with the sage potential vegetation categories
 data based on the previous level0 file
 sage pot veg cats are 1-15
 
 veg carbon includes root carbon, in addition to above ground carbon
 
 arguments:
 char* fname:          file name to open, with path
 float* veg_carbon_sage:    the array to load the table data into
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

int read_veg_carbon(char* fname, float* veg_carbon_sage) {
    
    // veg c table data
    // corresponds with the sage potential vegetation types; 1-15
    // on header line
    // two columns: sage pot veg cat, carbon value (kg/m^2)
    
    char rec_str[MAXRECSIZE];		// string to hold one record
    FILE* fpin;
    const char* delim = ",";		// delimiter string for space separated file
    
    int i;
    int err = OK;								// store error code from the dignostic write file
    
    if((fpin = fopen(fname, "r")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_veg_carbon()\n", fname);
        return ERROR_FILE;
    }
    // skip the header line
    if(fscanf(fpin, "%*[^\n]\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_veg_carbon()\n", fname);
        return ERROR_FILE;
    }
    // read the records
    for (i = 0; i < NUM_SAGE_PVLT; i++) {
        if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
            // get the carbon value
            if((err = get_float_field(rec_str, delim, 2, &veg_carbon_sage[i])) != OK) {
                fprintf(fplog, "Error processing file %s: read_veg_carbon(); record=%i, column=2\n",
                        fname, i + 1);
                return err;
            }
        }else {
            fprintf(fplog, "Error reading file %s: read_veg_carbon(); record=%i\n", fname, i + 1);
            return ERROR_FILE;
        }
    } // end for loop over records
    
    fclose(fpin);
    
    return OK;
}
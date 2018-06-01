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
 
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
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
/**********
 read_lt_info_sage.c
 
 read the SAGE potential vegetation land type categories and raster codes
 this file is in the order of the integer values in the raster data set
 the first column contains the SAGE raster codes
 the second column is SAGE category names
 
 the maximum and minimum SAGE raster forest values for use in get_land_cells() ares et in lds.h
 the assumption is that all the forest codes are consecutive with no other types mixed in the enumeration
 currently, the range is 1 to 8, so woodland is not included in forest
 
 arguments:
 args_struct in_args: the input file arguments

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 31 July 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "lds.h"

int read_lt_info_sage(args_struct in_args) {
	
	// SAGE potential vegetation categories
	// 4 header rows
	// the first column contains the SAGE raster codes
    // the second column contains the SAGE category names
	
	int i;
	int nrecords = 0;               // count number of SAGE potential vegetation land type records
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the arrays to fill
    
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.lt_sage_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_lt_info_sage()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header lines
	if(fscanf(fpin, "%*[^\n]\n%*[^\n]\n%*[^\n]\n%*[^\n]\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_lt_info_sage()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
    while (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
        nrecords++;
    }
    
    // set the number of SAGE land types
    NUM_SAGE_PVLT = nrecords;
    
    // allocate the arrays
    landtypecodes_sage = calloc(NUM_SAGE_PVLT, sizeof(int));
    if(landtypecodes_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for landtypecodes_sage: read_lt_info_sage()\n");
        return ERROR_MEM;
    }
    landtypenames_sage = calloc(NUM_SAGE_PVLT, sizeof(char*));
    if(landtypenames_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for landtypenames_sage: read_lt_info_sage()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_SAGE_PVLT; i++) {
        landtypenames_sage[i] = calloc(MAXCHAR, sizeof(char));
        if(landtypenames_sage[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for landtypenames_sage[%i]: read_lt_info_sage()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header lines
    if(fscanf(fpin, "%*[^\n]\n%*[^\n]\n%*[^\n]\n%*[^\n]\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_lt_info_sage()\n", fname);
        return ERROR_FILE;
    }
    
	// read the SAGE records
	for (i = 0; i < NUM_SAGE_PVLT; i++) {
		if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
            // get the integer code
            if((err = get_int_field(rec_str, delim, 1, &landtypecodes_sage[out_index])) != OK) {
                fprintf(fplog, "Error processing file %s: read_lt_info_sage(); record=%i, column=1\n",
                        fname, i + 1);
                return err;
            }
			// get the name
			if((err = get_text_field(rec_str, delim, 2, &landtypenames_sage[out_index++][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_lt_info_sage(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_lt_info_sage(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_SAGE_PVLT)
	{
		fprintf(fplog, "Error reading file %s: read_lt_info_sage(); records read=%i != expected=%i\n",
				fname, i, NUM_SAGE_PVLT);
		return ERROR_FILE;
	}
	
	if (in_args.diagnostics) {
		// sage land type codes
		if ((err = write_text_int(landtypecodes_sage, NUM_SAGE_PVLT, "landtypecodes_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_lt_info_sage()\n", "landtypecodes_sage.txt");
			return err;
		}
		// sage land type names
		if ((err = write_text_char(landtypenames_sage, NUM_SAGE_PVLT, "landtypenames_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_lt_info_sage()\n", "landtypenames_sage.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;
}
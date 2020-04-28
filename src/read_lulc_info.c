/**********
 read_lulc_info.c
 
 read the SAGE potential vegetation land type categories and raster codes
 read the HYDE32 land use categories and assigned integer codes
 read the ISAM lulc categories and integer codes corresponding to their order in the raster files, with mappings to sage and hyde32
 these are csv files
 
 SAGE potential vegetation
 
 this file is in the order of the integer values in the raster data set
 the first column contains the SAGE raster codes
 the second column is SAGE category names
 the maximum and minimum SAGE raster forest values for use in get_land_cells() ares et in moirai.h
 the assumption is that all the forest codes are consecutive with no other types mixed in the enumeration
 currently, the range is 1 to 8, so woodland is not included in forest
 
 HYDE 3.2 land use
 
 This file assigns integer codes to the HYDE32 land use types, with the first 3 types completely describing the land use state
 first column is integer code
 second column is the basename for HYDE32 files (uopp_ is the urban land)
 units are km^2
 
 ISAM lulc and mapping to sage and hyde32
 
 The isam integer codes are the order in which the types are stored in the files
 sage and hyde have distinct enumerations, so split this mapping to store it
 -1 and "-" are no match
 there are 23 land cover types and 11 land use types
 first column - isam integer code
 second column - isam name
 third column - corresponding sage or hyde32 integer code
 fourth column - corresponding sage or hyde32 name
 
 arguments:
 args_struct in_args: the input file arguments

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 31 July 2013
 
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
 
  Modified fall 2015 by Alan Di Vittorio
 
  Modified jan 2018 by Alan Di Vittorio to read in all the lulc info
 
 **********/

#include "moirai.h"

int read_lulc_info(args_struct in_args) {
	
	// SAGE potential vegetation categories
	// 4 header rows
	// the first column contains the SAGE raster codes
    // the second column contains the SAGE category names
	
	// HYDE32 land use categories
	// one header row
	// the first column contains the HYDE integer codes
	// the second column contains the HYDE category names
	
	// ISAM lulc categories and mapping to sage and hyde
	// one header row
	// -1 and "-" are no match
 	// first column - isam integer code
 	// second column - isam name
 	// third column - corresponding sage or hyde32 integer code
 	// fourth column - corresponding sage or hyde32 name
	
	int i;
	int nrecords = 0;               // count number of records
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the arrays to fill
	int num_lulc_lctypes = 23;		// the number of lulc land cover categories
	
	////////////////// SAGE pot veg info
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.lt_sage_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header lines
	if(fscanf(fpin, "%*[^\r\n]\r\n%*[^\r\n]\r\n%*[^\r\n]\r\n%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
	nrecords = 0;
    while (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
        nrecords++;
    }
    
    // set the number of SAGE land types
    NUM_SAGE_PVLT = nrecords;
    
    // allocate the arrays
    landtypecodes_sage = calloc(NUM_SAGE_PVLT, sizeof(int));
    if(landtypecodes_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for landtypecodes_sage: read_lulc_info()\n");
        return ERROR_MEM;
    }
    landtypenames_sage = calloc(NUM_SAGE_PVLT, sizeof(char*));
    if(landtypenames_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for landtypenames_sage: read_lulc_info()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_SAGE_PVLT; i++) {
        landtypenames_sage[i] = calloc(MAXCHAR, sizeof(char));
        if(landtypenames_sage[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for landtypenames_sage[%i]: read_lulc_info()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header lines
    if(fscanf(fpin, "%*[^\r\n]\r\n%*[^\r\n]\r\n%*[^\r\n]\r\n%*[^\r\n]\r\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_lulc_info()\n", fname);
        return ERROR_FILE;
    }
    
	// read the SAGE records
	out_index = 0;
	for (i = 0; i < NUM_SAGE_PVLT; i++) {
		if (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
            // get the integer code
            if((err = get_int_field(rec_str, delim, 1, &landtypecodes_sage[out_index])) != OK) {
                fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=1\n",
                        fname, i + 1);
                return err;
            }
			// get the name
			if((err = get_text_field(rec_str, delim, 2, &landtypenames_sage[out_index++][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_lulc_info(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_SAGE_PVLT)
	{
		fprintf(fplog, "Error reading file %s: read_lulc_info(); records read=%i != expected=%i\n",
				fname, i, NUM_SAGE_PVLT);
		return ERROR_FILE;
	}
	
	////////////////// HYDE land use info
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.lu_hyde_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header line
	if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// count the records
	nrecords = 0;
	while (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
		nrecords++;
	}
	
	// set the number of HYDE land types
	NUM_HYDE_TYPES = nrecords;
	
	// allocate the arrays
	lutypecodes_hyde = calloc(NUM_HYDE_TYPES, sizeof(int));
	if(lutypecodes_hyde == NULL) {
		fprintf(fplog,"Error: Failed to allocate memory for lutypecodes_hyde: read_lulc_info()\n");
		return ERROR_MEM;
	}
	lutypenames_hyde = calloc(NUM_HYDE_TYPES, sizeof(char*));
	if(lutypenames_hyde == NULL) {
		fprintf(fplog,"Error: Failed to allocate memory for lutypenames_hyde: read_lulc_info()\n");
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_HYDE_TYPES; i++) {
		lutypenames_hyde[i] = calloc(MAXCHAR, sizeof(char));
		if(lutypenames_hyde[i] == NULL) {
			fprintf(fplog,"Error: Failed to allocate memory for lutypenames_hyde[%i]: read_lulc_info()\n", i);
			return ERROR_MEM;
		}
	}
	
	rewind(fpin);
	
	// skip the header line
	if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// read the HYDE records
	out_index = 0;
	for (i = 0; i < NUM_HYDE_TYPES; i++) {
		if (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
			// get the integer code
			if((err = get_int_field(rec_str, delim, 1, &lutypecodes_hyde[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the name
			if((err = get_text_field(rec_str, delim, 2, &lutypenames_hyde[out_index++][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_lulc_info(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_HYDE_TYPES)
	{
		fprintf(fplog, "Error reading file %s: read_lulc_info(); records read=%i != expected=%i\n",
				fname, i, NUM_HYDE_TYPES);
		return ERROR_FILE;
	}
	
	////////////////// ISAM lulc info and mapping
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.lulc_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header line
	if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// count the records
	nrecords = 0;
	while (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
		nrecords++;
	}
	
	// set the number of HYDE land types
	NUM_LULC_TYPES = nrecords;
	
	// allocate the arrays
	lulccodes = calloc(NUM_LULC_TYPES, sizeof(int));
	if(lulccodes == NULL) {
		fprintf(fplog,"Error: Failed to allocate memory for lulccodes: read_lulc_info()\n");
		return ERROR_MEM;
	}
	lulcnames = calloc(NUM_LULC_TYPES, sizeof(char*));
	if(lulcnames == NULL) {
		fprintf(fplog,"Error: Failed to allocate memory for lulcnames: read_lulc_info()\n");
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		lulcnames[i] = calloc(MAXCHAR, sizeof(char));
		if(lulcnames[i] == NULL) {
			fprintf(fplog,"Error: Failed to allocate memory for lulcnames[%i]: read_lulc_info()\n", i);
			return ERROR_MEM;
		}
	}
	// also allocate the code mapping arrays
	lulc2sagecodes = calloc(NUM_LULC_TYPES, sizeof(int));
	if(lulc2sagecodes == NULL) {
		fprintf(fplog,"Error: Failed to allocate memory for lulc2sagecodes: read_lulc_info()\n");
		return ERROR_MEM;
	}
	// also allocate the code mapping arrays
	lulc2hydecodes = calloc(NUM_LULC_TYPES, sizeof(int));
	if(lulc2hydecodes == NULL) {
		fprintf(fplog,"Error: Failed to allocate memory for lulc2hydecodes: read_lulc_info()\n");
		return ERROR_MEM;
	}
	
	rewind(fpin);
	
	// skip the header line
	if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_lulc_info()\n", fname);
		return ERROR_FILE;
	}
	
	// read the HYDE records
	out_index = 0;
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		if (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
			// get the lulc integer code
			if((err = get_int_field(rec_str, delim, 1, &lulccodes[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the lulc name
			if((err = get_text_field(rec_str, delim, 2, &lulcnames[out_index][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
			if (i < num_lulc_lctypes) {
				// get the sage integer code for mapping
				if((err = get_int_field(rec_str, delim, 3, &lulc2sagecodes[out_index])) != OK) {
					fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=1\n",
							fname, i + 1);
					return err;
				}
				lulc2hydecodes[out_index++] = NOMATCH;
			} else {
				// get the hyde integer code for mapping
				if((err = get_int_field(rec_str, delim, 3, &lulc2hydecodes[out_index])) != OK) {
					fprintf(fplog, "Error processing file %s: read_lulc_info(); record=%i, column=1\n",
							fname, i + 1);
					return err;
				}
				lulc2sagecodes[out_index++] = NOMATCH;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_lulc_info(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_LULC_TYPES)
	{
		fprintf(fplog, "Error reading file %s: read_lulc_info(); records read=%i != expected=%i\n",
				fname, i, NUM_LULC_TYPES);
		return ERROR_FILE;
	}
	
	
	
	
	
	if (in_args.diagnostics) {
		// sage land type codes
		if ((err = write_text_int(landtypecodes_sage, NUM_SAGE_PVLT, "landtypecodes_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_lulc_info()\n", "landtypecodes_sage.txt");
			return err;
		}
		// sage land type names
		if ((err = write_text_char(landtypenames_sage, NUM_SAGE_PVLT, "landtypenames_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_lulc_info()\n", "landtypenames_sage.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;
}
/**********
 read_use_info_gtap.c
 
 read the GTAP product use categories
 this file is in the order of the land rent output file for AgLU
 
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
 
 **********/

#include "moirai.h"

int read_use_info_gtap(args_struct in_args) {
	
	// GTAP product use categories
	// one header row
	// the order of the rows is alphabetical by 3-letter abbreviation (name)
	//  first column: use code
	//  second column: 3-character abbreviation (name)
	//  third column: use description
	
	int i;
	int nrecords = 0;               // count number of records
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the gtap use arrays to fill
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.use_gtap_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_use_info_gtap()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header line
	if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_use_info_gtap()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
    while (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
        nrecords++;
    }
    
    // set the number of land rent uses
    NUM_GTAP_USE = nrecords;
    
    // allocate the arrays
    usecodes_gtap = calloc(NUM_GTAP_USE, sizeof(int));
    if(usecodes_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for usecodes_gtap: read_use_info_gtap()\n");
        return ERROR_MEM;
    }
    usenames_gtap = calloc(NUM_GTAP_USE, sizeof(char*));
    if(usenames_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for usenames_gtap: read_use_info_gtap()\n");
        return ERROR_MEM;
    }
    usedescr_gtap = calloc(NUM_GTAP_USE, sizeof(char*));
    if(usedescr_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for usedescr_gtap: read_use_info_gtap()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GTAP_USE; i++) {
        usenames_gtap[i] = calloc(MAXCHAR, sizeof(char));
        if(usenames_gtap[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for usenames_gtap[%i]: read_use_info_gtap()\n", i);
            return ERROR_MEM;
        }
        usedescr_gtap[i] = calloc(MAXCHAR, sizeof(char));
        if(usedescr_gtap[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for usedescr_gtap[%i]: read_use_info_gtap()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header line
    if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_use_info_gtap()\n", fname);
        return ERROR_FILE;
    }
    
	// read all the records
	for (i = 0; i < NUM_GTAP_USE; i++) {
		if (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
			
			// get the integer code
			if((err = get_int_field(rec_str, delim, 1, &usecodes_gtap[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_use_info_gtap(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the abbreviation (name)
			if((err = get_text_field(rec_str, delim, 2, &usenames_gtap[out_index][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_use_info_gtap(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
			// get the description
			if((err = get_text_field(rec_str, delim, 3, &usedescr_gtap[out_index++][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_use_info_gtap(); record=%i, column=3\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_use_info_gtap(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_GTAP_USE)
	{
		fprintf(fplog, "Error reading file %s: read_use_info_gtap(); records read=%i != expected=%i\n",
				fname, i, NUM_GTAP_USE);
		return ERROR_FILE;
	}
	
	if (in_args.diagnostics) {
		// use codes
		if ((err = write_text_int(usecodes_gtap, NUM_GTAP_USE, "usecodes_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_use_info_gtap()\n", "usecodes_gtap.txt");
			return err;
		}
		// use descriptions
		if ((err = write_text_char(usedescr_gtap, NUM_GTAP_USE, "usedescr_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_use_info_gtap()\n", "usedescr_gtap.txt");
			return err;
		}
		// use names
		if ((err = write_text_char(usenames_gtap, NUM_GTAP_USE, "usenames_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_use_info_gtap()\n", "usenames_gtap.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;}

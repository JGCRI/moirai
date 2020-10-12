/**********
 read_crop_info.c
 
 read the mapping between FAO crops, SAGE crops, and GTAP use
 the list is organized by the SAGE crops in numerical order
 value of -1 indicates no mapping to the SAGE data set
 
 the GTAP crop names are now redundant, but they are what the gcam data system expects, so use them for now
 
 don't need to read the gtap use name from this file
 
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

int read_crop_info(args_struct in_args) {
	
	// crop codes, names across data sets and how they map to GTAP use
	// one header row
	// the order of the rows is numerical by the SAGE enumerated name list
	//  first column: SAGE crop code
	//  second column: SAGE base for the file name, which is a short name
	//  third column: SAGE crop long name / description
	//  fourth column: GTAP crop name, the one used by gcam data system
	//  fifth column: GTAP use code
	//  sixth column: GTAP use name
	//  seventh column: FAO crop code
	//	eigth column: FAO crop name
	
	int i;
	int nrecords = 0;               // count number of records in file
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the arrays to fill
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.crop_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_crop_info()\n", fname);
		return ERROR_FILE;
	}
	
	// read the data
	
	// skip the header line
	if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_crop_info()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
    while (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
        nrecords++;
    }
    
    // set the number of SAGE crops
    NUM_SAGE_CROP = nrecords;
    
    // allocate arrays
    cropcodes_sage = calloc(NUM_SAGE_CROP, sizeof(int));
    if(cropcodes_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for cropcodes_sage: read_crop_info()\n");
        return ERROR_MEM;
    }
    crop_sage2gtap_use = calloc(NUM_SAGE_CROP, sizeof(int));
    if(crop_sage2gtap_use == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for crop_sage2gtap_use: read_crop_info()\n");
        return ERROR_MEM;
    }
    cropcodes_sage2fao = calloc(NUM_SAGE_CROP, sizeof(int));
    if(cropcodes_sage2fao == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for cropcodes_sage2fao: read_crop_info()\n");
        return ERROR_MEM;
    }
    cropfilebase_sage = calloc(NUM_SAGE_CROP, sizeof(char*));
    if(cropfilebase_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for cropfilebase_sage: read_crop_info()\n");
        return ERROR_MEM;
    }
    cropdescr_sage = calloc(NUM_SAGE_CROP, sizeof(char*));
    if(cropdescr_sage == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for cropdescr_sage: read_crop_info()\n");
        return ERROR_MEM;
    }
    cropnames_gtap = calloc(NUM_SAGE_CROP, sizeof(char*));
    if(cropnames_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for cropnames_gtap: read_crop_info()\n");
        return ERROR_MEM;
    }
    cropnames_sage2fao = calloc(NUM_SAGE_CROP, sizeof(char*));
    if(cropnames_sage2fao == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for cropnames_sage2fao: read_crop_info()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_SAGE_CROP; i++) {
        cropfilebase_sage[i] = calloc(MAXCHAR, sizeof(char));
        if(cropfilebase_sage[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for cropfilebase_sage[%i]: read_crop_info()\n", i);
            return ERROR_MEM;
        }
        cropdescr_sage[i] = calloc(MAXCHAR, sizeof(char));
        if(cropdescr_sage[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for cropdescr_sage[%i]: read_crop_info()\n", i);
            return ERROR_MEM;
        }
        cropnames_gtap[i] = calloc(MAXCHAR, sizeof(char));
        if(cropnames_gtap[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for cropnames_gtap[%i]: read_crop_info()\n", i);
            return ERROR_MEM;
        }
        cropnames_sage2fao[i] = calloc(MAXCHAR, sizeof(char));
        if(cropnames_sage2fao[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for cropnames_sage2fao[%i]: read_crop_info()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header line
    if(fscanf(fpin, "%*[^\r\n]\r\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_crop_info()\n", fname);
        return ERROR_FILE;
    }
    
	// read all the records
	for (i = 0; i < NUM_SAGE_CROP; i++) {
		if (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
			// get the SAGE crop integer code
			if((err = get_int_field(rec_str, delim, 1, &cropcodes_sage[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the SAGE file name base
			if((err = get_text_field(rec_str, delim, 2, &cropfilebase_sage[out_index][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
			// get the SAGE crop description
			if((err = get_text_field(rec_str, delim, 3, &cropdescr_sage[out_index][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=3\n",
						fname, i + 1);
				return err;
			}
			// get the GTAP crop name
			if((err = get_text_field(rec_str, delim, 4, &cropnames_gtap[out_index][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=4\n",
						fname, i + 1);
				return err;
			}
			// get the GTAP use code
			if((err = get_int_field(rec_str, delim, 5, &crop_sage2gtap_use[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=5\n",
						fname, i + 1);
				return err;
			}
			// get the FAO crop codes
			if((err = get_int_field(rec_str, delim, 7, &cropcodes_sage2fao[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=7\n",
						fname, i + 1);
				return err;
			}
			// get the FAO crop names
			if((err = get_text_field(rec_str, delim, 8, &cropnames_sage2fao[out_index++][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_crop_info(); record=%i, column=8\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_crop_info(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_SAGE_CROP)
	{
		fprintf(fplog, "Error reading file %s: read_crop_info(); records read=%i != expected=%i\n",
				fname, i, NUM_SAGE_CROP);
		return ERROR_FILE;
	}
	
	if (in_args.diagnostics) {
		// sage crop codes
		if ((err = write_text_int(cropcodes_sage, NUM_SAGE_CROP, "cropcodes_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "cropcodes_sage.txt");
			return err;
		}
		// sage crop file basename
		if ((err = write_text_char(cropfilebase_sage, NUM_SAGE_CROP, "cropfilebase_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "cropfilebase_sage.txt");
			return err;
		}
		// sage crop description
		if ((err = write_text_char(cropdescr_sage, NUM_SAGE_CROP, "cropdescr_sage.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "cropdescr_sage.txt");
			return err;
		}
		// gtap crop names
		if ((err = write_text_char(cropnames_gtap, NUM_SAGE_CROP, "cropnames_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "cropnames_gtap.txt");
			return err;
		}
		// sage crop to gtap use codes
		if ((err = write_text_int(crop_sage2gtap_use, NUM_SAGE_CROP, "crop_sage2gtap_use.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "crop_sage2gtap_use.txt");
			return err;
		}
		// sage crop codes to fao crop codes
		if ((err = write_text_int(cropcodes_sage2fao, NUM_SAGE_CROP, "cropcodes_sage2fao.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "cropcodes_sage2fao.txt");
			return err;
		}
		// sage crop to fao crop name
		if ((err = write_text_char(cropnames_sage2fao, NUM_SAGE_CROP, "cropnames_sage2fao.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_crop_info()\n", "cropnames_sage2fao.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;}
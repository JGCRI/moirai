/**********
 read_production_fao.c
 
 read FAOSTAT production file for weighting the price average across years and countries
  and for calibrating yield and harvested area data to a different year
 
 the production data file must be the same formt as and contain the same years as the price, yield, and harvested area data files
 year value fields must be empty or numeric
 
 only the 175 SAGE crops are stored and the moirai countries
 
 the fao file is in ha, so convert to km^2 for storage array
 
 arguments:
 args_struct in_args: the input file arguments
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 1 Aug 2013
 
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
 
 july 2018
 Modified by A.V.D.
 Now reads a more complete, recent FAO data file, with units and flags, and newline characters
 
 **********/

#include "moirai.h"

int read_production_fao(args_struct in_args) {
	
	// all reported crops listed by country with years 1and year flags
	// one header row
	// each line now ends with a newline ('\n')
	// there can't be any blank lines for this to work properly
	//  first column: FAO country code
	//  second column: FAO country nme
	//  third column: FAO crop code
	//  fourth column: FAO crop name
	//  fifth column: FAO element code (this is the variable stored in the file, with units)
	//  sixth column: FAO element name
	//  seventh column: units = meric tonnes
	//  remaining columns: year, yearflag; repeating, with no missing years
	
	int j;
	//int nrecords = 17193;			// number of records in file
	int nhead = 1;					// number of header lines
	int yr1col = 8;					// first column of year data
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	char temp_str[MAXRECSIZE];		// string to test for blank line
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the production array to fill
	int ctry_ind = 0;				// the FAO country index with respect to countrycodes_fao[]
	int crop_ind = 0;				// the SAGE crop index with respect to int cropcodes_sage2fao[] and cropcodes_sage[]
	int temp_ctry = NODATA;			// temporary country code
	int temp_crop = NODATA;			// temporary crop code
	
	long flen = 0;					// length of file in bytes
	long count_lines = 0;			// count the number of lines to skip the header
	long count_recs = 0;			// count the number of records read
	long rec_index = 0;				// the index of the record string to fill
	int is_newrec = 0;				// flag to denote end of record
	char *sptr;						// dynamically allocated buffer for whole file as a string
	char *cptr;						// pointer to traverse the file string
	
	double tmp_dbl, dbl_int;		// for checking fao area-production consistency
	
	char out_name[] = "production_fao.csv";	// diagnositic output csv file name
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.production_fao_fname);
	
	// read the whole file at once in binary mode using fread
	if((fpin = fopen(fname, "rb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_production_fao()\n", fname);
		return ERROR_FILE;
	}
	fseek(fpin, 0L, SEEK_END);
	flen = ftell(fpin);
	rewind(fpin);
	// calloc initializes the values to zero, which is the termination character
	sptr = calloc(flen + 1, sizeof(char));
	if(sptr == NULL) {
		fprintf(fplog,"Failed to allocate memory for file %s:  read_production_fao()\n", fname);
		return ERROR_MEM;
	}
	
	fread(sptr, flen, 1, fpin);
	fclose(fpin);
	cptr = sptr;	// set the moving pointer to the beginning of the file
	
	// while loop is probably faster than for loop
	// the terminating character is zero, which will stop the while loops
	// this is the loop over the whole file
	// this reads in and skips whitespace lines, rather than throwing an error, but doesn't count them as records 
	while (*cptr) {
		rec_index = 0;
		is_newrec = 0;
		// this is the loop over the records
		while (*cptr) {
			if (!is_newrec) {
				if (*cptr == '\n') {
					is_newrec = 1;
				}
			} else {
				break;
			}
			// store the current character in the record string
			rec_str[rec_index++] = *cptr++;
		} // end while loop over record
		rec_str[rec_index] = '\0';	// terminate the record string
		
		// process this record if it is not a header and it is not a blank record
		rm_whitesp(temp_str, rec_str);
		if (!(count_lines++ < nhead) && strlen(temp_str)) {
			count_recs++;
		
			// get the country code
			if((err = get_int_field(rec_str, delim, 1, &temp_ctry)) != OK) {
				fprintf(fplog, "Error processing file %s: read_production_fao(); record=%li, country code check\n",
						fname, count_recs);
				return err;
			}
			
			// get the crop code
			if((err = get_int_field(rec_str, delim, 3, &temp_crop)) != OK) {
				fprintf(fplog, "Error processing file %s: read_production_fao(); record=%li, column=4\n",
						fname, count_recs);
				return err;
			}

			// determine the country and crop indices for this record
			// skip record if country or crop do not match fao to sage mappings
			ctry_ind = NOMATCH;
			for (j = 0; j < NUM_FAO_CTRY; j++) {
				if (countrycodes_fao[j] == temp_ctry) {
					ctry_ind = j;
					break;
				}
			}
			if(ctry_ind == NOMATCH) {
				//fprintf(fplog, "Extra FAO country code %i in %s: read_production_fao(); record=%li\n",
						//temp_ctry, fname, count_recs);
				continue;
			}
			crop_ind = NOMATCH;
			for (j = 0; j < NUM_SAGE_CROP; j++) {
				if (cropcodes_sage2fao[j] == temp_crop) {
					crop_ind = j;
					break;
				}
			}
			if(crop_ind == NOMATCH) {
				//fprintf(fplog, "Extra FAO crop code %i in %s: read_production_fao(); record=%li\n",
						//temp_crop, fname, count_recs);
				continue;
			}
			
			// get the annual data
			for (j = 0; j < NUM_FAO_YRS; j++) {
				// determine the index of the production data for this year and country and crop
				out_index = ctry_ind * NUM_SAGE_CROP * NUM_FAO_YRS + crop_ind * NUM_FAO_YRS + j;
				
				if((err = get_float_field(rec_str, delim, (j * 2) + yr1col, &production_fao[out_index])) != OK) {
					fprintf(fplog, "Error processing file %s: read_production_fao(); record=%li, year column=%i\n",
							fname, count_recs, j);
					return err;
				}
			}
		}	// end if process record
		
	} // end while loop over file
	
	free(sptr);
	
	// check for inconsistent values in the input fao data
	for (j = 0; j < NUM_SAGE_CROP * NUM_FAO_CTRY * NUM_FAO_YRS; j++) {
		tmp_dbl = modf((double) (j / (NUM_SAGE_CROP *  NUM_FAO_YRS)), &dbl_int);
		ctry_ind = (int) dbl_int;
		temp_crop = j - ctry_ind * NUM_FAO_YRS * NUM_SAGE_CROP;
		tmp_dbl = modf((double) (temp_crop / NUM_FAO_YRS), &dbl_int);
		crop_ind = (int) dbl_int;
		if (harvestarea_fao[j] != 0 && production_fao[j] == 0) {
			//fprintf(fplog, "FAO inconsistency for zero production at index %i: read_production_fao(); harvestarea_fao = %f\n\tctry_ind = %i\tcrop_ind = %i\n",
					//j, harvestarea_fao[j], ctry_ind, crop_ind);
		}
		if (harvestarea_fao[j] == 0 && production_fao[j] != 0) {
			//fprintf(fplog, "FAO inconsistency for zero area at index %i: read_production_fao(); production_fao = %f\n\tctry_ind = %i\tcrop_ind = %i\n",
					//j, production_fao[j], ctry_ind, crop_ind);
		}
	}	
	
	/* this no longer applies because the fao data includes extra records
	if(count_recs != nrecords)
	{
		fprintf(fplog, "Error reading file %s: read_production_fao(); records read=%li != nrecords=%i\n",
				fname, count_recs, nrecords);
		return ERROR_FILE;
	}
	 */
	
	if (in_args.diagnostics) {
		if ((err = write_csv_float3d(production_fao, countrycodes_fao, cropcodes_sage,
									NUM_FAO_CTRY, NUM_SAGE_CROP, NUM_FAO_YRS, out_name, in_args))) {
			fprintf(fplog, "Error writing file %s: read_production_fao()\n", out_name);
			return err;
		}
	}
	 
	return OK;}

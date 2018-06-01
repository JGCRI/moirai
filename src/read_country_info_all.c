/**********
 read_country_info_all.c
 
 read the mapping between FAO, VMAP0, and iso3 code
 the list is organized by alphabetical iso3 code - same as the mapping to ctry87
 value of -1 indicates no mapping between FAO and VMAP0
 text value of '-' indicate no mapping between FAO, VAMP0, or iso
 five VMAP0 regions have been added to the FAO list with unofficial codes
	the FAO country raster file is based on the VMAP0 country shape file
	some of the added VMAP0 regions are present in the iso list, but not the FAO list
	so the added regions make the raster file consistent with the tabular data
 
 the VMAP0 data have already been mapped to FAO codes to create the FAO country raster file,
    so the VMAP0 info does not have to be read in
    just fa_code, iso3_abbr, and fao_name
 
 there is one header line, and the number of records determines NUM_FAO_CTRY
 
 arguments:
 args_struct in_args: the input file arguments

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 31 July 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified Oct 2015 by Alan Di Vittorio
    To update geographic input and outputs to available data and iso codes
    This is in conjunction with allowing arbitrary AEZs as input
 
 **********/

#include "moirai.h"

int read_country_info_all(args_struct in_args) {
	
	// country codes and names and how they map across the data sets
	// one header row
	// the order of the rows is alphabetical by iso3 abbreviation
	//  first column: FAO country code
    //  second column: iso3 abbreviation
	//  third column: FAO country name
	//  fourth column: VMAP0 code - matches FAO code except where data not available
	//  fifth column: VMAP0 country name - includes regions that were mapped to "owner" countries
	
	int i = 0;
	int nrecords = 0;               // number of records
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = 0;				// the index of the arrays to fill
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.country_all_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_country_info_all()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header line
	if(fscanf(fpin, "%[^\n]\n", rec_str) == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_country_info_all()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
    while (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
        nrecords++;
    }
    
    // set the number of fao/vmap0 countries
    NUM_FAO_CTRY = nrecords;
    
    // now allocate the arrays
    countrycodes_fao = calloc(NUM_FAO_CTRY, sizeof(int));
    if(countrycodes_fao == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for countrycodes_fao: read_country_info_all()\n");
        return ERROR_MEM;
    }
    countryabbrs_iso = calloc(NUM_FAO_CTRY, sizeof(char*));
    if(countryabbrs_iso == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for countryabbrs_iso: read_country_info_all()\n");
        return ERROR_MEM;
    }
    countrynames_fao = calloc(NUM_FAO_CTRY, sizeof(char*));
    if(countrynames_fao == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for countrynames_fao: read_country_info_all()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        countryabbrs_iso[i] = calloc(MAXCHAR, sizeof(char));
        if(countryabbrs_iso[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for countryabbrs_iso[%i]: read_country_info_all()\n", i);
            return ERROR_MEM;
        }
        countrynames_fao[i] = calloc(MAXCHAR, sizeof(char));
        if(countrynames_fao[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for countrynames_fao[%i]: read_country_info_all()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header line
    if(fscanf(fpin, "%[^\n]\n", rec_str) == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_country_info_all()\n", fname);
        return ERROR_FILE;
    }
    
	// read all the records
	for (i = 0; i < NUM_FAO_CTRY; i++) {
		if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
			// get the FAO integer code
			if((err = get_int_field(rec_str, delim, 1, &countrycodes_fao[out_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country_info_all(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
            // get the iso3 abbreviation
            if((err = get_text_field(rec_str, delim, 2, &countryabbrs_iso[out_index][0])) != OK) {
                fprintf(fplog, "Error processing file %s: read_country_info_all(); record=%i, column=2\n",
                        fname, i + 1);
                return err;
            }
			// get the FAO name
			if((err = get_text_field(rec_str, delim, 3, &countrynames_fao[out_index++][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country_info_all(); record=%i, column=3\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_country_info_all(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records

    
	fclose(fpin);
	
    if(i != NUM_FAO_CTRY)
    {
        fprintf(fplog, "Error reading file %s: read_country_info_all(); records read=%i != expected = %i\n",
                fname, i, NUM_FAO_CTRY);
        return ERROR_FILE;
    }
    
	if (in_args.diagnostics) {
		// country codes
		if ((err = write_text_int(countrycodes_fao, NUM_FAO_CTRY, "countrycodes_fao.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country_info_all()\n", "countrycodes_fao.txt");
			return err;
		}
		// country names
		if ((err = write_text_char(countrynames_fao, NUM_FAO_CTRY, "countrynames_fao.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country_info_all()\n", "countrynames_fao.txt");
			return err;
		}
		// country fao to country iso mapping codes
		if ((err = write_text_char(countryabbrs_iso, NUM_FAO_CTRY, "countryabbrs_iso.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country_info_all()\n", "countryabbrs_iso.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;
}
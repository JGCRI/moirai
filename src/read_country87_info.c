/**********
 read_country87_info.c
 
 read the country 87 names abbreviations, and codes, and also the FAO to country 87 mapping
 the country 87 list is still the GTAP 87 list because some GTAP data is needed as input at this level
 
 the ctry87 file is in the order of the land rent output file for AgLU
  with the new enumeration based on this order for GCAM (which is alphabetical based on iso abbreviation)
 
 the country mapping file is in the order of the area and production output files for AgLU
  which is alphabetical in iso abbreviation
 the order and length matches the fao/vmap0/iso file, so only need to store in the last two columns
 
 the country mapping file determines whether a country is assigned to an economic region and output by LDS
    if a country is not assigned to a land rent region, then it is not assigned to a gcam region either 
 
 arguments:
 args_struct in_args: the input file arguments

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 30 July 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified oct 2015 to account for new country base
 
 **********/

#include "moirai.h"

int read_country87_info(args_struct in_args) {
	
	// country87 names, abbreviations, and codes, reformatted from the AgLU input files and GTAP files
	// the two reformatted csv files: ctry87 and the mapping between ctry87 and FAO ctry
	// one header row for each file
	// the order of the rows in each file is alphetical by the iso abbreviation in the leftmost column
	// ctry87: first column: code; second column: 3-letter abbreviation; third column: name
	// FAO ctry to ctry87 mapping:
	//  first column: fao code
	//  second column: iso 3-letter abbreviation
	//  third column: fao name
	//  fourth column: ctry87 code
	//  fifth column: ctry87 3-letter abbreviation
	
	int i = 0;
	int nrecords_reg_rent = 0;          // count number of records in region land rent file
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	
	// read in the region land rent info first
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.country87_gtap_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_country87_info()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header line
	if(fscanf(fpin, "%*[^\n]\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_country87_info()\n", fname);
		return ERROR_FILE;
	}
	
	// first count the records
    while (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
        nrecords_reg_rent++;
    }
    
    // set the number of gtap87 countries
    NUM_GTAP_CTRY87 = nrecords_reg_rent;
    
    // now allocate the arrays to hold the data
    country87codes_gtap = calloc(NUM_GTAP_CTRY87, sizeof(int));
    if(country87codes_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for country87codes_gtap: read_country87_info()\n");
        return ERROR_MEM;
    }
    country87names_gtap = calloc(NUM_GTAP_CTRY87, sizeof(char*));
    if(country87names_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for country87names_gtap: read_country87_info()\n");
        return ERROR_MEM;
    }
    country87abbrs_gtap = calloc(NUM_GTAP_CTRY87, sizeof(char*));
    if(country87abbrs_gtap == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for country87abbrs_gtap: read_country87_info()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        country87names_gtap[i] = calloc(MAXCHAR, sizeof(char));
        if(country87names_gtap[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for country87names_gtap[%i]: read_country87_info()\n", i);
            return ERROR_MEM;
        }
        country87abbrs_gtap[i] = calloc(MAXCHAR, sizeof(char));
        if(country87abbrs_gtap[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for country87abbrs_gtap[%i]: read_country87_info()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header line
    if(fscanf(fpin, "%*[^\n]\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_country87_info()\n", fname);
        return ERROR_FILE;
    }
    
    // now read the records
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
		if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
			
			// get the ctry87 integer code first
			if((err = get_int_field(rec_str, delim, 1, &country87codes_gtap[i])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country87_info(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the ctry87 abbreviation
			if((err = get_text_field(rec_str, delim, 2, &country87abbrs_gtap[i][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country87_info(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
			// get the ctry87 name
			if((err = get_text_field(rec_str, delim, 3, &country87names_gtap[i][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country87_info(); record=%i, column=3\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_country87_info(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over read records
    
	fclose(fpin);
	
    if(i != NUM_GTAP_CTRY87)
    {
        fprintf(fplog, "Error reading file %s: read_country87_info(); records read=%i != expected=%i\n",
                fname, i, NUM_GTAP_CTRY87);
        return ERROR_FILE;
    }
	
	//////////
	// read in the fao ctry to ctry87 mapping
	
    // allocate the arrays (already have the number of records)
    ctry2ctry87codes_gtap = calloc(NUM_FAO_CTRY, sizeof(int));
    if(ctry2ctry87codes_gtap == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for ctry2ctry87codes_gtap: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    ctry2ctry87abbrs_gtap = calloc(NUM_FAO_CTRY, sizeof(char*));
    if(ctry2ctry87abbrs_gtap == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for ctry2ctry87abbrs_gtap: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        ctry2ctry87abbrs_gtap[i] = calloc(MAXCHAR, sizeof(char));
        if(ctry2ctry87abbrs_gtap[i] == NULL) {
            fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for ctry2ctry87abbrs_gtap[%i]: main()\n", get_systime(), ERROR_MEM, i);
            return ERROR_MEM;
        }
    }
    
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.country87map_fao_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_country87_info()\n", fname);
		return ERROR_FILE;
	}

	// skip the header line
	if(fscanf(fpin, "%*[^\n]\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_country87_info()\n", fname);
		return ERROR_FILE;
	}
	
	// read all the records
	for (i = 0; i < NUM_FAO_CTRY; i++) {
		if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
			
			// do not need to retrieve the fao code, the iso abbr, or the fao name
            // these have already been stored, and the length and order of the columns match
			
			// get the matching ctry87 code
			if((err = get_int_field(rec_str, delim, 4, &ctry2ctry87codes_gtap[i])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country87_info(); record=%i, column=4\n",
						fname, i + 1);
				return err;
			}
			// get the matching ctry87 abbr
			if((err = get_text_field(rec_str, delim, 5, &ctry2ctry87abbrs_gtap[i][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country87_info(); record=%i, column=5\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_country87_info(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_FAO_CTRY)
	{
		fprintf(fplog, "Error reading file %s: read_country87_info(); records read=%i != expected=%i\n",
				fname, i, NUM_FAO_CTRY);
		return ERROR_FILE;
	}

	if (in_args.diagnostics) {
		// country to country 87 mapping codes
		if ((err = write_text_int(ctry2ctry87codes_gtap, NUM_FAO_CTRY, "ctry2ctry87codes_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country87_info()\n", "ctry2ctry87codes_gtap.txt");
			return err;
		}
		// country to country 87 mapping abbreviations
		if ((err = write_text_char(ctry2ctry87abbrs_gtap, NUM_FAO_CTRY, "ctry2ctry87abbrs_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country87_info()\n", "ctry2ctry87abbrs_gtap.txt");
			return err;
		}
		// country 87 codes
		if ((err = write_text_int(country87codes_gtap, NUM_GTAP_CTRY87, "country87codes_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country87_info()\n", "country87codes_gtap.txt");
			return err;
		}
		// country 87 abbreviations
		if ((err = write_text_char(country87abbrs_gtap, NUM_GTAP_CTRY87, "country87abbrs_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country87_info()\n", "country87abbrs_gtap.txt");
			return err;
		}
		// country 87 names
		if ((err = write_text_char(country87names_gtap, NUM_GTAP_CTRY87, "country87names_gtap.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country87_info()\n", "country87names_gtap.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;
}
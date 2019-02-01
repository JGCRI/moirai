/**********
 read_region_info_gcam.c
 
 read the GCAM region names and codes, and iso to region mapping
 reads in two files
 gcam regions are enumerated 1 to NUM_GCAM_RGN in the order in:
	GCAM_region_names_14reg.csv
	GCAM_region_names_32reg.csv
	for now just read the expected number of records
 gcam region mapping is based on iso abbreviations now:
	iso_GCAM_regID_14reg.csv
	iso_GCAM_regID_32reg.csv
 the files used by the gcam-data-system do not include the _##reg portion of the name
 
 if there is no iso in the GCAM list, or no economic region, then the record is not output
  this corresponds with FAO_ctry_GCAM_ctry87.csv
    no GTAP87 economic region also means no GCAM region
    so only the merged serbia and montenegro are output due to merged fao data
 
 arguments:
 args_struct in_args: the input file arguments
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 13 July 2013
 Copyright 2018 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 This file is part of Moirai.
 
 Moirai is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Moirai is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Moirai (/moirai/docs/COPYING.txt). If not, see <https://www.gnu.org/licenses/>.
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "moirai.h"

int read_region_info_gcam(args_struct in_args) {
	
	// the GCAM region name files have four header lines
	// the order is enumerated
	//  first column: gcam region code
	//  second column: gcam region name
	
	// the GCAM iso to region ID files have four header lines
	//  first column: iso abbreviation
	//  second column: country_name
	//  third column: gcam region name
	//  second column: gcam region code
	
	int i,j;
    int nrecords_reg = 0;           // count the records in the region list
    int nrecords_iso = 0;           // count the records in the iso mapping list
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function

	//////////
	// read in the GCAM region list
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.regionlist_gcam_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_country_info_gcam()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header lines
	if(fscanf(fpin, "%*[^\n\r]\n\r%*[^\n\r]\n%*[^\n\r]\n%*[^\n\r]\n\r") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_country_info_gcam()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
    
    while (fscanf(fpin, "%[^\n\r]\r\n", rec_str) != EOF) {
        nrecords_reg++;
    }
    
    // set the number of gcam regions
    NUM_GCAM_RGN = nrecords_reg;
    
    // allocate the arrays
    regioncodes_gcam = calloc(NUM_GTAP_CTRY87, sizeof(int));
    if(regioncodes_gcam == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for regioncodes_gcam: read_country_info_gcam()\n");
        return ERROR_MEM;
    }
    regionnames_gcam = calloc(NUM_GTAP_CTRY87, sizeof(char*));
    if(regionnames_gcam == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for regionnames_gcam: read_country_info_gcam()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        regionnames_gcam[i] = calloc(MAXCHAR, sizeof(char));
        if(regionnames_gcam[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for regionnames_gcam[%i]: read_country_info_gcam()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header lines
    if(fscanf(fpin, "%*[^\n\r]\n\r%*[^\n\r]\n%*[^\n\r]\n%*[^\n\r]\n\r") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_country_info_gcam()\n", fname);
        return ERROR_FILE;
    }
    
	// read the expected number of records
	for (i = 0; i < NUM_GCAM_RGN; i++) {
		if (fscanf(fpin, "%[^\n\r]\r\n", rec_str) != EOF) {
			
			// get the gcam region integer code
			if((err = get_int_field(rec_str, delim, 1, &regioncodes_gcam[i])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country_info_gcam(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the gcam region name
			if((err = get_text_field(rec_str, delim, 2, &regionnames_gcam[i][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country_info_gcam(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_country_info_gcam(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for loop over records
	
	fclose(fpin);
	
	if(i != NUM_GCAM_RGN)
	{
		fprintf(fplog, "Error reading file %s: read_country_info_gcam(); records read=%i != expected=%i\n",
				fname, i, NUM_GCAM_RGN);
		return ERROR_FILE;
	}
	
	//////////
	// read in the iso country to gcam region mapping - just the iso code and region code
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.countrymap_iso_gcam_region_fname);
	
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_country_info_gcam()\n", fname);
		return ERROR_FILE;
	}
	
	// skip the header lines
	if(fscanf(fpin, "%*[^\r\n]\r\n%*[^\r\n]\r\n%*[^\r\n]\n%*[^\r\n]\r\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_country_info_gcam()\n", fname);
		return ERROR_FILE;
	}
	
    // count the records
    while (fscanf(fpin, "%[^\n\r]\r\n", rec_str) != EOF) {
        nrecords_iso++;
    }
    
    // set the number of gcam iso countries
    NUM_GCAM_ISO_CTRY = nrecords_iso;
    
    // allocate the arrays
    ctry2regioncodes_gcam = calloc(NUM_FAO_CTRY, sizeof(int));
    if(ctry2regioncodes_gcam == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for ctry2regioncodes_gcam: read_country_info_gcam()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        ctry2regioncodes_gcam[i] = NOMATCH;
    }
    country_gcamiso2regioncodes_gcam = calloc(NUM_GCAM_ISO_CTRY, sizeof(int));
    if(country_gcamiso2regioncodes_gcam == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for country_gcamiso2regioncodes_gcam: read_country_info_gcam()\n");
        return ERROR_MEM;
    }
    countryabbrs_gcam_iso = calloc(NUM_GCAM_ISO_CTRY, sizeof(char*));
    if(countryabbrs_gcam_iso == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for countryabbrs_gcam_iso: read_country_info_gcam()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GCAM_ISO_CTRY; i++) {
        countryabbrs_gcam_iso[i] = calloc(MAXCHAR, sizeof(char));
        if(countryabbrs_gcam_iso[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for countryabbrs_gcam_iso[%i]: read_country_info_gcam()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header lines
    if(fscanf(fpin, "%*[^\r\n]\r\n%*[^\r\n]\r\n%*[^\r\n]\n%*[^\r\n]\r\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_country_info_gcam()\n", fname);
        return ERROR_FILE;
    }
    
	// read the expected number of records
	// should change this to read the whole file then double-check the number (or set it here)
	for (i = 0; i < NUM_GCAM_ISO_CTRY; i++) {
		if (fscanf(fpin, "%[^\n\r]\r\n", rec_str) != EOF) {
			
			// get the iso abbreviation
			if((err = get_text_field(rec_str, delim, 1, &countryabbrs_gcam_iso[i][0])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country_info_gcam(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the gcam region integer code
			if((err = get_int_field(rec_str, delim, 4, &country_gcamiso2regioncodes_gcam[i])) != OK) {
				fprintf(fplog, "Error processing file %s: read_country_info_gcam(); record=%i, column=4\n",
						fname, i + 1);
				return err;
			}
			
			// loop to the fao iso index for this gcam iso ctry and (if it exists) store the associated gcam region code
            //      this is based on the gcam iso input file
			// not all gcam iso countries are in the fao country set
            // not all fao countries are in the gcam iso set - usually spatial data with no iso3 or no GTAP87 economic region
            //       currently these are spratly islands and antarctica
            // not all fao countries are processed and output individually due to merged data
            //      serbia and montenegro are separate in the raster file, but need to be processed as merged scg
            // leave the value == NOMATCH for FAO countries with no iso3 or no GTAP87 economic region
            //      the ctry2regioncodes_gcam array has been initialized to NOMATCH above
            for (j = 0; j < NUM_FAO_CTRY; j++) {
                if (!strcmp(&countryabbrs_iso[j][0], &countryabbrs_gcam_iso[i][0]) && ctry2ctry87codes_gtap[j] != NOMATCH) {
                    ctry2regioncodes_gcam[j] = country_gcamiso2regioncodes_gcam[i];
                    break;
                }
			}	// end for j loop over fao countries to find matches with gcam iso abbrevs
		}else {
			fprintf(fplog, "Error reading file %s: read_country_info_gcam(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	} // end for i loop over records
	
	fclose(fpin);
	
	if(i != NUM_GCAM_ISO_CTRY)
	{
		fprintf(fplog, "Error reading file %s: read_country_info_gcam(); records read=%i != expected=%i\n",
				fname, i, NUM_GCAM_ISO_CTRY);
		return ERROR_FILE;
	}
	
	if (in_args.diagnostics) {
		// gcam region codes
		if ((err = write_text_int(regioncodes_gcam, NUM_GCAM_RGN, "regioncodes_gcam.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country_info_gcam()\n", "regioncodes_gcam.txt");
			return err;
		}
		// gcam region names
		if ((err = write_text_char(regionnames_gcam, NUM_GCAM_RGN, "regionnames_gcam.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country_info_gcam()\n", "regionnames_gcam.txt");
			return err;
		}
		// fao ctry to gcam region codes
		if ((err = write_text_int(ctry2regioncodes_gcam, NUM_FAO_CTRY, "ctry2regioncodes_gcam.txt", in_args))) {
			fprintf(fplog, "Error writing file %s: read_country_info_gcam()\n", "ctry2regioncodes_gcam.txt");
			return err;
		}
	}	// end if diagnostics
	
	return OK;
}
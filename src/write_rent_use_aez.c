/**********
 write_rent_use_aez.c
 
 write the land rent (million USD) for the new glus in a "melted" csv file with no zero values
  there is only only one value column; the other columns are label columns
 
 csv file
 there are 6 header lines
 there are 4 columns
 col 1: land rent region abbr
 col 2: GLU integer code
 col 3: gtap use name
 col 4: land rent in million USD
 
 there is a record for each non-zero, positive output value
 
 crop varies fastest, then aez, then land rent region
 
 arguments:
 args_struct in_args:	the input argument structure
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 21 Aug 2014
 
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

int write_rent_use_aez(args_struct in_args) {
	
	int nrecords = 0;	// count number of records in output file
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int reglr_index = 0;			// the index of the land rent region to write
    int aez_index = 0;				// the index of the aez to write
	int use_index = 0;				// the index of the use to write
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, in_args.rent_fname);
	
	if((fpout = fopen(fname, "w")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_rent_use_aez()\n", fname);
		return ERROR_FILE;
	}
	
	// write header lines
	fprintf(fpout,"# File: %s\n", fname);
	fprintf(fpout,"# Author: %s\n", CODENAME);
	fprintf(fpout,"# Description: Initialization of land value (million USD) by country87/use/GLU\n");
	fprintf(fpout,"# Original source: many, including HYDE and SAGE\n");
	fprintf(fpout,"# ----------\n");
	fprintf(fpout,"reglr_iso,glu_code,use_sector,value");
	
	// write the records (these are not rounded, but are output to 9 decimals)
	for (reglr_index = 0; reglr_index < NUM_GTAP_CTRY87 ; reglr_index++) {
        for (aez_index = 0; aez_index < reglr_aez_num[reglr_index]; aez_index++) {
            for (use_index = 0; use_index < NUM_GTAP_USE; use_index++) {
                // output only positive values
                if (rent_use_aez[reglr_index][aez_index][use_index] > 0) {
                    fprintf(fpout,"\n%s,%i,%s,%11.9f", country87abbrs_gtap[reglr_index], reglr_aez_list[reglr_index][aez_index],
                            usenames_gtap[use_index], rent_use_aez[reglr_index][aez_index][use_index]);
                    nrecords++;
                } // end if value is positive
            } // end for use loop
		} // end for aez loop
	} // end for land rent region loop
	
	fclose(fpout);
	
	fprintf(fplog, "Wrote file %s: write_rent_use_aez(); records written=%i\n", fname, nrecords);
	
	return OK;}
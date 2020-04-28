/**********
 write_harvestarea_crop_aez.c
 
 write the harvest area (ha) for the new glus in the GCAM input format
 write values only if both harvested area and production are > 0
 write values only if country is in economic region (if mapped to ctry87)
 
 csv file
 there are 6 header lines
 there are 4 columns
 col 1: country iso abbr
 col 2: GLU integer code
 col 3: gtap crop name
 col 4: harvested area in ha
 so there is a record for each non-zero output value
 
 crop varies fastest, then glu, then country
 
 all values are rounded to the nearest integer
 
 only countries with economic regions are output (see the country to land rent region mapping input file)
 
 arguments:
 args_struct in_args:	the input argument structure

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 only countries with economic regions are output
 
 Created by Alan Di Vittorio on 5 Sep 2013
 
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

int write_harvestarea_crop_aez(args_struct in_args) {
	
	int nrecords = 0;	// count number of records in output array
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int ctry_index = 0;				// the index of the country to write
    int aez_index = 0;				// the index of the aez to write
	int crop_index = 0;				// the index of the crop to write
	float outval;						// the output value to write
    int count_skip = 0;             // the number of countries skipped due to no economic region
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, in_args.harvestarea_fname);
	
	if((fpout = fopen(fname, "w")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_harvestarea_crop_aez()\n", fname);
		return ERROR_FILE;
	}
	
	// write header lines
	fprintf(fpout,"# File: %s\n", fname);
	fprintf(fpout,"# Author: %s\n", CODENAME);
	fprintf(fpout,"# Description: Initialization of harvested area (ha) by country/GLU/crop\n");
	fprintf(fpout,"# Original source: many, including HYDE and SAGE\n");
	fprintf(fpout,"# ----------\n");
	fprintf(fpout,"ctry_iso,glu_code,SAGE_crop,value");
	
	// write the records (round the values first)
    for (ctry_index = 0; ctry_index < NUM_FAO_CTRY; ctry_index++) {

        // do not write this country if not assocaited with an economic region
        if (ctry2ctry87codes_gtap[ctry_index] == NOMATCH) {
            count_skip++;
        } else {
            for (aez_index = 0; aez_index < ctry_aez_num[ctry_index]; aez_index++) {
                for (crop_index = 0; crop_index < NUM_SAGE_CROP; crop_index++) {
                    outval = (float) floor((double) 0.5 + harvestarea_crop_aez[ctry_index][aez_index][crop_index]);
                    //outval = harvestarea_crop_aez[ctry_index][aez_index][crop_index];
                    // output only positive values
                    if (outval > 0) {
                        // check the production for zero and negative values, and write only if positive
						// this doesn not occur
                        if ((float) floor((double) 0.5 + production_crop_aez[ctry_index][aez_index][crop_index]) <= 0) {
							if (in_args.diagnostics) {
                            	fprintf(fplog, "Discard harvested area due to no production: ha = %.0f and prod = 0: write_harvestarea_crop_aez(); ctrycode=%i,aezcode=%i, cropcode=%i\n", outval, countrycodes_fao[ctry_index], ctry_aez_list[ctry_index][aez_index],
                                    cropcodes_sage[crop_index]);
							}
						} else {
							fprintf(fpout,"\n%s,%i,%s,%.0f", countryabbrs_iso[ctry_index], ctry_aez_list[ctry_index][aez_index],
									cropnames_gtap[crop_index], outval);
							nrecords++;
						}
						
                        // double check this before deleting it (delete)
                        // does not happen for non-calibrated output
                        // i think it is for recalibration debugging
                        //if(outval > 30000000) { outval = NODATA; }
                    } // end if positive value
                } // end for crop_index loop
            } // end for aez_index loop
        } // end else write output
    } // end for ctry_index loop over number of records in output array
	
	fclose(fpout);
	
    fprintf(fplog, "Wrote file %s: write_harvestarea_crop_aez(); records written=%i != countries skipped=%i\n",
            fname, nrecords, count_skip);
	
	return OK;
}
/**********
 write_production_crop_aez.c
 
 write the production (metric tonnes) for the new glus in the GCAM input format
 write values only if both harvested area and production are > 0
 
 csv file
 there are 6 header lines
 there are 4 columns
    col 1: country iso abbr
    col 2: GLU integer code
    col 3: gtap crop name
    col 4: production in metric tonnes
 
 there is a record for each non-zero, positive output value
 
 crop varies fastest, then glu, then country
 
 all values are rounded to the nearest integer
 
 only countries with economic regions are output (see the country to land rent region mapping input file)
 
 arguments:
 args_struct in_args:	the input argument structure

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 5 Sep 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "lds.h"

int write_production_crop_aez(args_struct in_args) {
	
	int nrecords = 0;	// count number of records in output file
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int ctry_index = 0;				// the index of the country to write
    int aez_index = 0;				// the index of the aez to write
	int crop_index = 0;				// the index of the crop to write
	float outval;						// the output value to write
    int count_skip = 0;             // the number of countries skipped due to no economic region
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, in_args.production_fname);
	
	if((fpout = fopen(fname, "w")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_production_crop_aez()\n", fname);
		return ERROR_FILE;
	}
	
	// write header lines
	fprintf(fpout,"# File: %s\n", fname);
	fprintf(fpout,"# Author: %s\n", CODENAME);
	fprintf(fpout,"# Description: Initialization of production (t) by country/GLU/crop\n");
	fprintf(fpout,"# Original source: many, including HYDE and SAGE\n");
	fprintf(fpout,"# ----------\n");
	fprintf(fpout,"ctry_iso,glu_code,SAGE_crop,value");
	
	// write the records (round the values first)
	for (ctry_index = 0; ctry_index < NUM_FAO_CTRY; ctry_index++) {
        
        // do not write this country if not assocaited with an economic region
        if (ctry2regioncodes_gcam[ctry_index] == NOMATCH) {
            count_skip++;
        } else {
            for (aez_index = 0; aez_index < ctry_aez_num[ctry_index]; aez_index++) {
                for (crop_index = 0; crop_index < NUM_SAGE_CROP; crop_index++) {
                    outval = (float) floor((double) 0.5 + production_crop_aez[ctry_index][aez_index][crop_index]);
                    //outval = production_crop_aez[ctry_index][aez_index][crop_index];
                    // output only positive values
                    if (outval > 0) {
                        // check the harvested area for zero and negative values; write only if positive
                        if ((float) floor((double) 0.5 + harvestarea_crop_aez[ctry_index][aez_index][crop_index]) <= 0) {
                            fprintf(fplog, "Discard production due to no harvested area: prod = %.0f and ha = 0: write_production_crop_aez(); ctrycode=%i,aezcode=%i, cropcode=%i\n", outval, countrycodes_fao[ctry_index], ctry_aez_list[ctry_index][aez_index],
                                    cropcodes_sage[crop_index]);
						} else {
							fprintf(fpout,"\n%s,%i,%s,%.0f", countryabbrs_iso[ctry_index], ctry_aez_list[ctry_index][aez_index],
									cropnames_gtap[crop_index], outval);
							nrecords++;
						}
						
                        // not sure why this upper check is here, so check it before removing it (delete)
                        // this does happen for sugar cane in brazil with no calibration!
                        // i think it is for recalibration debugging
                        //if(outval > 200000000) { outval = NODATA; }
                    } // end if positive value
                } // end for crop_index loop
            } // end for aez_index loop
        } // end else write output
    } // end for ctry_index loop over number of records in output array

	fclose(fpout);
    
	fprintf(fplog, "Wrote file %s: write_production_crop_aez(); records written=%i != countries skipped=%i\n",
			fname, nrecords, count_skip);
	
	return OK;
}
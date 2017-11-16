/**********
 read_prodprice_fao.c
 
 read the FAOSTAT producer price file for use in disaggregating land rents to AEZs
 it is unclear which year of price data was used in GTAP land use 2.1V6.0
	this GTAP LU version uses 1997-2003 yield and harvested area data with values in yr2001USD
 a previous GTAP LU version used year 2001 price, yield, production, harvest data and yr2001USD
 
 the FAO PRICESTAT data is based on yrs2004-2006USD, and the source file is in USD/metric tonne
  (this has been clarified through email with faostat support)
 
 so calculate production weighted average annual price for years 1997 - 2003
	the stored average will be for the GTAP 87 countries, rather than the FAO countries
 
 the code is set up to calculate this average for years 2003-2007 if desired
  (in conjunction with yield/area recalibration, see calc_harvarea_prod_out_crop_aez())
  the recalibration year is hardcoded, and only implemented as of sep 2014
	using just the 2005 single year did not generate good recalibration (several missing values at low levels)
	the 2003-2007 average for yield/area recalibration showed that the GTAP data is based on the original data
	however, I have not tried to recalibrate to the 1997-2003 average
 
 the price data file currently contains the same years as the production, yield, and harvest area data files (1997 - 2007)
 
 only the 175 SAGE crops are stored
 
 this function also reads in a file containing factors to convert 1999-2006USD to yr2001USD
  for now, assume the input prices are based on 2004-2006 dollars, so convert from 2005USD to 2001USD
 
 it is easy to convert to other USD year values, but the GTAP LU database is based on yr2001USD
  the input usd conversion file determines which year the values are converted to
  this function is hard-coded for the particular input usd conversion file
 
 arguments:
 args_struct in_args: the input file arguments

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 1 Aug 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "lds.h"

int read_prodprice_fao(args_struct in_args) {
	
	// all reported crops listed by country with years 1997-2007 as final columns, in USD/metric tonne
	// one header row
	// each line ends with a carraige return only ('\r')
	// there can't be any blank lines for this to work properly
	// each record is a unique country-crop combination
	//  first column: FAO country name
	//  second column: FAO country code
	//  third column: FAO crop name
	//  fourth column: FAO crop code
	//  fifth column: FAO element name (this is the variable stored in the file, with units)
	//  sixth column: FAO element code
	//  columns 7-17: years 1997 - 2007
	
	int i,j;
	int nrecords = 6922;			// number of records in file
	int nhead = 1;					// number of header lines
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	char temp_str[MAXRECSIZE];		// string to test for blank line
	const char* delim = ",";		// delimiter string for csv file
	int err = OK;					// error code for the string parsing function
	int out_index = -1;				// the index of the price array to fill
	int prod_index = -1;			// the index of the production array to read
	int ctry_ind = -1;				// the FAO country index with respect to countrycodes_fao[]
	int crop_ind = -1;				// the SAGE crop index with respect to int cropcodes_sage2fao[] and cropcodes_sage[]
	int temp_ctry = NODATA;			// temporary country code
	int temp_crop = NODATA;			// temporary crop code
	float temp_flt = 0;				// float variable for read in
	double avg_sum = 0;				// the sum of (prices times production) across years
	double *prod_sum;				// the sum of production across years, per fao country, per sage crop
	double *prodprice_temp;			// the temporal average production price by fao country and sage crop
	double *prod_tot;				// the sum of production across years, per gtap 87 country, per sage crop
	float *float_out;				// diagnostic output array
	int num_avg;					// number of years to average
	int avg_cols[100];				// the columns to average
	
	long flen = 0;					// length of file in bytes
	long count_lines = 0;			// count the number of lines to skip the header
	long count_recs = 0;			// count the number of records read
	long rec_index = -1;			// the index of the record string to fill
	int is_newrec = 0;				// flag to denote end of record
	char *sptr;						// dynamically allocated buffer for whole file as a string
	char *cptr;						// pointer to traverse the file string
	
	char out_namef[] = "prodprice_fao.csv";		// diagnositic output csv file name
	char out_nameg[] = "prodprice_fao_reglr.csv";	// diagnositic output csv file name

	// these need to be initialized with zeroes, so use pointers and calloc
	prod_sum = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP, sizeof(double));
	if(prod_sum == NULL) {
		fprintf(fplog,"Failed to allocate memory for prod_sum:  read_prodprice_fao()\n");
		return ERROR_MEM;
	}
	prodprice_temp = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP, sizeof(double));
	if(prodprice_temp == NULL) {
		fprintf(fplog,"Failed to allocate memory for prodprice_temp:  read_prodprice_fao()\n");
		return ERROR_MEM;
	}
	prod_tot = calloc(NUM_GTAP_CTRY87 * NUM_SAGE_CROP, sizeof(double));
	if(prod_tot == NULL) {
		fprintf(fplog,"Failed to allocate memory for prod_tot:  read_prodprice_fao()\n");
		return ERROR_MEM;
	}
	
	float_out = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP, sizeof(float));
	if(prod_tot == NULL) {
		fprintf(fplog,"Failed to allocate memory for float_out:  read_prodprice_fao()\n");
		return ERROR_MEM;
	}
	
	if (in_args.recalibrate) {
		// columns for using 2005 only (to check the recalib routine)
		//num_avg = 1;
		//avg_cols[0] = 15;
		
		// columns for averaging 2003-2007 (if calibrating to 2005 data; must do this for the SAGE data as well)
		num_avg = 5;
		avg_cols[0] = 13;
		avg_cols[1] = 14;
		avg_cols[2] = 15;
		avg_cols[3] = 16;
		avg_cols[4] = 17;
		
		/*
		// test recalibration to original years
		num_avg = 7;
		avg_cols[0] = 7;
		avg_cols[1] = 8;
		avg_cols[2] = 9;
		avg_cols[3] = 10;
		avg_cols[4] = 11;
		avg_cols[5] = 12;
		avg_cols[6] = 13;
		 */
	} else {
		
		// columns for averaging 1997-2003 (to match input SAGE data)
		num_avg = 7;
		avg_cols[0] = 7;
		avg_cols[1] = 8;
		avg_cols[2] = 9;
		avg_cols[3] = 10;
		avg_cols[4] = 11;
		avg_cols[5] = 12;
		avg_cols[6] = 13;
	}
	
	// file info for the usd conversion factors
	// one header line
	int num_factors = 8;			// number of factors for years 1999 - 2006
	int inyear[num_factors];		// list of input usd years for conversion
	float divisor[num_factors];		// the conversion factors (yr2001USD = yr####USD / yr####USD_divisor)
	int divisor_index = 0;			// the index for storing the factors and for retrieving the needed factor
	int conv_year = 2005;			// the year to convert from
	
	// read the dollar conversion factors to convert to yr2001USD
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.convert_usd_fname);
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_prodprice_fao()\n", fname);
		return ERROR_FILE;
	}
	// skip the header line
	if(fscanf(fpin, "%*[^\n]\n") == EOF)
	{
		fprintf(fplog,"Failed to scan over file %s header:  read_prodprice_fao()\n", fname);
		return ERROR_FILE;
	}
	for (i = 0; i < num_factors; i++) {
		if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
			// get the input year
			if((err = get_int_field(rec_str, delim, 1, &inyear[divisor_index])) != OK) {
				fprintf(fplog, "Error processing file %s: read_prodprice_fao(); record=%i, column=1\n",
						fname, i + 1);
				return err;
			}
			// get the corresponding divisor
			if((err = get_float_field(rec_str, delim, 2, &divisor[divisor_index++])) != OK) {
				fprintf(fplog, "Error processing file %s: read_prodprice_fao(); record=%i, column=2\n",
						fname, i + 1);
				return err;
			}
		}else {
			fprintf(fplog, "Error reading file %s: read_prodprice_fao(); record=%i\n", fname, i + 1);
			return ERROR_FILE;
		}
	}
	fclose(fpin);
	
	// assume the input prices are based on 2004-2006 dollars, so convert from 2005USD to 2001USD
	divisor_index = 6;	// 2005 by default
	for (i = 0; i < num_factors; i++) {
		if (conv_year == inyear[i]) {
			divisor_index = i;
			break;
		}
	}
	
	// create file name and open it
	strcpy(fname, in_args.inpath);
	strcat(fname, in_args.prodprice_fao_fname);
	
	// read the whole file at once in binary mode using fread
	if((fpin = fopen(fname, "rb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s:  read_prodprice_fao()\n", fname);
		return ERROR_FILE;
	}
	fseek(fpin, 0L, SEEK_END);
	flen = ftell(fpin);
	rewind(fpin);
	// calloc initializes the values to zero, which is the termination character
	sptr = calloc(flen + 1, sizeof(char));
	if(sptr == NULL) {
		fprintf(fplog,"Failed to allocate memory for file %s:  read_prodprice_fao()\n", fname);
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
		// this is the loop to obtain each record string
		while (*cptr) {
			if (!is_newrec) {
				if (*cptr == '\r') {
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
			if((err = get_int_field(rec_str, delim, 2, &temp_ctry)) != OK) {
				fprintf(fplog, "Error processing file %s: read_prodprice_fao(); record=%li, column=2\n",
						fname, count_recs);
				return err;
			}
			
			// get the crop code
			if((err = get_int_field(rec_str, delim, 4, &temp_crop)) != OK) {
				fprintf(fplog, "Error processing file %s: read_prodprice_fao(); record=%li, column=4\n",
						fname, count_recs);
				return err;
			}
			
			// determine the output index for this record in the local temp price storage array
			ctry_ind = -1;
			for (j = 0; j < NUM_FAO_CTRY; j++) {
				if (countrycodes_fao[j] == temp_ctry) {
					ctry_ind = j;
					break;
				}
			}
			
			// process record only if there is an fao country match
			if (ctry_ind != -1) {
				crop_ind = -1;
				for (j = 0; j < NUM_SAGE_CROP; j++) {
					if (cropcodes_sage2fao[j] == temp_crop) {
						crop_ind = j;
						out_index = ctry_ind * NUM_SAGE_CROP + crop_ind;
						break;
					}
				}
				
				// process record only if there is a sage crop match
				if (crop_ind != -1) {
					// get the annual data for sevaral years and average it
					// need to weight this average by annual production
					avg_sum = 0;
					for (j = 0; j < num_avg; j++) {
						if((err = get_float_field(rec_str, delim, avg_cols[j], &temp_flt)) != OK) {
							fprintf(fplog, "Error processing file %s: read_prodprice_fao(); record=%li, column=%i\n",
									fname, count_recs, avg_cols[j]);
							return err;
						}
						
						// determine the index of the production data for this year and country and crop
						prod_index = ctry_ind * NUM_SAGE_CROP * NUM_FAO_YRS + crop_ind * NUM_FAO_YRS + j;
						if (in_args.recalibrate) {
							// adjust the index to start at 2005; no average
							prod_index = prod_index	+ 8;
							// adjust the index to start at 2003; average around 2005
							//prod_index = prod_index	+ 6;
						}
						
						// get the averaging sums
						prod_sum[out_index] = prod_sum[out_index] + production_fao[prod_index];
						avg_sum = avg_sum + production_fao[prod_index] * temp_flt;
					}
					
					// make the conversion to 2001USD after the temporal average is calculated
					// prod_sum could be zero, but divisor is non-zero by definition
					// might not need to divide by prod_sum here, because of later multiplication
					//	but keep it for now because this is the temporal average
					if (prod_sum[out_index] == 0) {
						prodprice_temp[out_index] = 0;
					}else {
						prodprice_temp[out_index] = avg_sum / prod_sum[out_index] / divisor[divisor_index];
						float_out[out_index] = (float) prodprice_temp[out_index];
						//if(float_out[out_index] != 0) {
						//	fprintf(stderr, "found non-zero float_out\n");
						//}
					}
					
				}else {
					fprintf(fplog, "Warning: processing file %s: read_prodprice_fao(); record=%li, no sage crop match\n",
							fname, count_recs);
				}	// end if sage crop match else don't process record
			}else {
				fprintf(fplog, "Warning: processing file %s: read_prodprice_fao(); record=%li, no fao country code match\n",
						fname, count_recs);
			}	// end if fao country match else don't process record
		}	// end if process record
		
	} // end while loop over file
	
	free(sptr);
	
	if(count_recs != nrecords)
	{
		fprintf(fplog, "Error reading file %s: read_prodprice_fao(); records read=%li != nrecords=%i\n",
				fname, count_recs, nrecords);
		return ERROR_FILE;
	}
	
	// now aggregate the temporal averages to the 87 GTAP countries
    // valid fao/vmap0 territories with no economic regions have value = NOMATCH for gtap87 and gcam regions
    // so just keip them in the processing
	for (crop_ind = 0; crop_ind < NUM_SAGE_CROP; crop_ind++) {
		for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY; ctry_ind++) {
			
            // aggregate if fao country has an economic region
            temp_ctry = ctry2ctry87codes_gtap[ctry_ind];
            if (temp_ctry == NOMATCH) {
                fprintf(fplog, "Warning: FAO country %i has no economic region: read_prodprice_fao()\n", countrycodes_fao[ctry_ind]);
                continue;
            }else {
                for (i = 0; i < NUM_GTAP_CTRY87; i++) {
                    if (country87codes_gtap[i] == temp_ctry) {
                        out_index = i * NUM_SAGE_CROP + crop_ind;
                        break;
                    }
                }	// end for i loop to get new out index
                
                j = ctry_ind * NUM_SAGE_CROP + crop_ind;
                prod_tot[out_index] = prod_tot[out_index] + prod_sum[j];
                prodprice_fao_reglr[out_index] = prodprice_fao_reglr[out_index] + (float) (prod_sum[j] * prodprice_temp[j]);
            } // end if region NOMATCH continue else process this fao ctry
		}	// end for loop over fao ctry_ind for spatial aggregation
		// compute the final averages for this crop across 87 countries
		for (i = crop_ind; i < NUM_SAGE_CROP * NUM_GTAP_CTRY87; i+=NUM_SAGE_CROP) {
			if (prod_tot[i] == 0) {
				prodprice_fao_reglr[i] = 0;
			}else {
				prodprice_fao_reglr[i] = prodprice_fao_reglr[i] / (float) prod_tot[i];
			}
		} // end for i loop over this crop in each ctry87 to calc final average
	}	// end for loop over sage crop_ind for spatial aggregation
	
	if (in_args.diagnostics) {
		if ((err = write_csv_float2d(float_out, countrycodes_fao,
									NUM_FAO_CTRY, NUM_SAGE_CROP, out_namef, in_args))) {
			fprintf(fplog, "Error writing file %s: read_prodprice_fao()\n", out_namef);
			return err;
		}
		if ((err = write_csv_float2d(prodprice_fao_reglr, country87codes_gtap,
									 NUM_GTAP_CTRY87, NUM_SAGE_CROP, out_nameg, in_args))) {
			fprintf(fplog, "Error writing file %s: read_prodprice_fao()\n", out_nameg);
			return err;
		}
	}
	
	free(prod_sum);
	free(prod_tot);
	free(prodprice_temp);
	free(float_out);
	
	return OK;
}
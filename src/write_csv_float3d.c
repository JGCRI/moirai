/**********
 write_csv_float3d.c
 
 write a csv text file from a flattened 3d float array
 
 arguments:
 int out_array[]:		1d array to write to file; it is a flattened 3d array
 int d1[]:				array of numeric codes to write for the first dimension
 int d2[]:				array of numeric codes to write for the second dimension
 int d1_length:			length of dimension 1
 int d2_length:			length of dimension 2
 int d3_length:			length of dimension 3
 char *out_name:		name of output file
 args_struct in_args:	the input argument structure

 the output is organized as follows:
	each row contains the 3rd dimension values for a given combination of the 1st and second dimensions
	the first column is the numeric code of the 1st dimension
	the second column is the numeric code of the 2nd dimension
	the second dimension varies faster than the first dimension
 
 only two decimal points are output
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 6 Sep 2013
 
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
 
 **********/

#include "moirai.h"

int write_csv_float3d(float out_array[], int d1[], int d2[], int d1_length, int d2_length, int d3_length, char *out_name, args_struct in_args) {
	
	int i,j;
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int nelements;					// the number of elements in the output array
	int nrecords;					// the number of records to write
	int d1_index, d2_index;			// indices of the separate arrays for the first two dimensions
	int out_index;					// index of array to write
	double remainder;				// remainder to check that there is an integral number of records
	double temp_dbl, integer_dbl;	// useful variables
	
	// make sure that the input lengths are correct
	nelements = d1_length * d2_length * d3_length;
	remainder = fmod((double) nelements, (double) d3_length);
	if (remainder != 0) {
		fprintf(fplog,"Error in array length inputs: write_csv_float3d(); nelements=%i / d3_length=%i is not an integer\n",
				nelements, d3_length);
		return ERROR_FILE;
	} else {
		nrecords = nelements / d3_length;
	}

	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, out_name);
	
	if((fpout = fopen(fname, "w")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_csv_float3d()\n", fname);
		return ERROR_FILE;
	}
	
	for (i = 0; i < nrecords; i++) {		
		temp_dbl = i / d2_length;
		modf(temp_dbl, &integer_dbl);
		d1_index = (int) integer_dbl;
		d2_index = i - d1_index * d2_length;
		fprintf(fpout,"%i,%i", d1[d1_index], d2[d2_index]);
		for (j = 0; j < d3_length; j++) {
			out_index = d2_length * d3_length * d1_index + d3_length * d2_index + j;
			
			if ( out_array[out_index] != 0) {
				;
			}
			
			fprintf(fpout,",%.2f", out_array[out_index]);
		}
		fprintf(fpout,"\n");
	}
	
	fclose(fpout);
	
	if(i != nrecords)
	{
		fprintf(fplog, "Error writing file %s: write_csv_float3d(); records written=%i != nrecords=%i\n",
				fname, i, nrecords);
		return ERROR_FILE;
	}
	
	return OK;}

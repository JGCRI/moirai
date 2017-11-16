/**********
 write_csv_float2d.c
 
 write a csv text file from a flattened 2d float array
 
 arguments:
 int out_array[]:		1d array to write to file; it is a flattened 3d array
 int d1[]:				array of numeric codes to write for the first dimension
 int d1_length:			length of dimension 1
 int d2_length:			length of dimension 2
 char *out_name:		name of output file
 args_struct in_args:	the input argument structure

 the output is organized as follows:
 each row contains the 2nd dimension values for a given 1st dimension category
 the first column is the numeric code of the 1st dimension
 
 only two decimal points are output
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 9 Sep 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "lds.h"

int write_csv_float2d(float out_array[], int d1[], int d1_length, int d2_length, char *out_name, args_struct in_args) {
	
	int i,j;
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int nelements;					// the number of elements in the output array
	int nrecords;					// the number of records to write
	int d1_index;					// index of separate array for dimension 1
	int out_index;					// index of array to write
	double remainder;				// remainder to check that there is an integral number of records
	
	// make sure that the input lengths are correct
	nelements = d1_length * d2_length;
	remainder = fmod((double) nelements, (double) d2_length);
	if (remainder != 0) {
		fprintf(fplog,"Error in array length inputs: write_csv_float2d(); nelements=%i / d2_length=%i is not an integer\n",
				nelements, d2_length);
		return ERROR_FILE;
	} else {
		nrecords = nelements / d2_length;
	}
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, out_name);
	
	if((fpout = fopen(fname, "w")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_csv_float2d()\n", fname);
		return ERROR_FILE;
	}
	
	for (i = 0; i < nrecords; i++) {
		d1_index = i;
		fprintf(fpout,"%i", d1[d1_index]);
		for (j = 0; j < d2_length; j++) {
			out_index = d2_length * d1_index + j;
			fprintf(fpout,",%.2f", out_array[out_index]);
		}
		fprintf(fpout,"\n");
	}
	
	fclose(fpout);
	
	if(i != nrecords)
	{
		fprintf(fplog, "Error writing file %s: write_csv_float2d(); records written=%i != nrecords=%i\n",
				fname, i, nrecords);
		return ERROR_FILE;
	}
	
	return OK;
}
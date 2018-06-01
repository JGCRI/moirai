/**********
 write_raster_short.c
 
 write a short raster image
 start at upper left corner and write row by row (this is how the data are stored)
 no header
 
 arguments:
 short out_array[]:		array to write to file
 int out_length:		length of array to write to file
 char *out_name:		name of output file
 args_struct in_args:	the input argument structure
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 11 Sep 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

int write_raster_short(short out_array[], int out_length, char *out_name, args_struct in_args) {
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int num_out;					// store the number of elements written
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, out_name);
	
	if((fpout = fopen(fname, "wb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_raster_short()\n", fname);
		return ERROR_FILE;
	}
	
	num_out = fwrite(out_array, sizeof(short), out_length, fpout);
	
	fclose(fpout);
	
	if(num_out != out_length)
	{
		fprintf(fplog, "Error writing file %s: write_raster_short(); records written=%i != out_length=%i\n",
				fname, num_out, out_length);
		return ERROR_FILE;
	}
	
	return OK;
}
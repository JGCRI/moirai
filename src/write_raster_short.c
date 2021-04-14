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
	
	num_out = (int) fwrite(out_array, sizeof(short), out_length, fpout);
	
	fclose(fpout);
	
	if(num_out != out_length)
	{
		fprintf(fplog, "Error writing file %s: write_raster_short(); records written=%i != out_length=%i\n",
				fname, num_out, out_length);
		return ERROR_FILE;
	}
	
	return OK;}

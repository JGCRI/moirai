/**********
 write_raster_int.c
 
 write a int raster image
 start at upper left corner and write row by row (this is how the data are stored)
 no header
 
 arguments:
 int out_array[]:		array to write to file
 int out_length:		length of array to write to file
 char *out_name:		name of output file
 args_struct in_args:	the input argument structure

 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 6 Sep 2013
 Copyright 2018 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 This file is part of Moirai.
 
 Moirai is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Moirai is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Moirai (/moirai/docs/COPYING.txt). If not, see <https://www.gnu.org/licenses/>.
 
 **********/

#include "moirai.h"

int write_raster_int(int out_array[], int out_length, char *out_name, args_struct in_args) {
	
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	int num_out;					// store the number of elements written
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, out_name);
	
	if((fpout = fopen(fname, "wb")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_raster_int()\n", fname);
		return ERROR_FILE;
	}
	
	num_out = fwrite(out_array, sizeof(int), out_length, fpout);
	
	fclose(fpout);
	
	if(num_out != out_length)
	{
		fprintf(fplog, "Error writing file %s: write_raster_int(); records written=%i != out_length=%i\n",
				fname, num_out, out_length);
		return ERROR_FILE;
	}
	
	return OK;
}
/**********
 write_text_char.c
 
 write a text file from an integer array
 
 arguments:
 char out_array[][]:	array to write to file
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

//int write_text_char(char out_array[][MAXCHAR], int out_length, char *out_name, args_struct in_args) {
int write_text_char(char **out_array, int out_length, char *out_name, args_struct in_args) {
	
	int i;
	char fname[MAXCHAR];			// file name to open
	FILE *fpout;					// file pointer
	
	// create file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, out_name);
	
	if((fpout = fopen(fname, "w")) == NULL)
	{
		fprintf(fplog,"Failed to open file %s: write_text_char()\n", fname);
		return ERROR_FILE;
	}
	
	for (i = 0; i < out_length; i++) {
		fprintf(fpout,"%s\n", &out_array[i][0]);
	}
	
	fclose(fpout);
	
	if(i != out_length)
	{
		fprintf(fplog, "Error writing file %s: write_text_char(); records written=%i != out_length=%i\n",
				fname, i, out_length);
		return ERROR_FILE;
	}
	
	return OK;
}
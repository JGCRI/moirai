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
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "lds.h"

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
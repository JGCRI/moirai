/**********
 parse_utils.c
 
 contains the following functions for parsing text records:
	get_float_field()
	get_int_field()
	get_text_field()
	get_field()
	rm_whitesp()
	is_num() - not sure i need this one
 
 Created by Alan Di Vittorio on 6 June 2013
 
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

/********
 int get_float_field(char *line, const char *delim, int findex, float *fltval)
 line:		string containing record info
 delim:		the delimiting character
 findex:	index of the desired field--this must start at one
 fltval:	the address for storing the retrieved float value
 return:	floating point field value; 0 if string is empty; ERROR_STR if field is not numeric
 ********/
int get_float_field(char *line, const char *delim, int findex, float *fltval)
{
	int error_code = OK;
	char str_field[MAXCHAR];
	char tmp[MAXCHAR];
	
	/* use get_text_field() to get the field without spaces */
	if ((error_code = get_text_field(line, delim, findex, str_field)) != OK) {
		fprintf(fplog, "Error parsing text record: get_float_field(); text field %i not retrieved\n", findex);
		return error_code;
	}
	
	// remove bracketing quotes if they exist, otherwise just copy the field as is
	rm_quotes(tmp, str_field);
	
	// atof() of a string that does not start with an inerpretable number returns 0 (including empty string)
	//  interpretable numbers include decimals and scientific notation (characters .,e,E,-,+ in the proper place)
	// so throw an error if other characters are present
	//  this restricts atof() from operating on fields with suffix character beyond the number
	// this will not catch typos because the return value will be zero or a different number
	if (is_num(tmp)) {
		*fltval = (float) atof(tmp);
	}
	else{
		fprintf(fplog, "Error parsing text record: get_float_field(); non-numeric field %i\n", findex);
		return ERROR_STR;
	}
	
	return OK;
}

/********
 int get_int_field(char *line, const char *delim, int findex, int *intval)
 line:		string containing record info
 delim:		the delimiting character
 findex:	index of the desired field--this must start at one
 fltval:	the address for storing the retrieved float value
 return:	integer field value; 0 if string is empty; ERROR_STR if field is not numeric
 ********/
int get_int_field(char *line, const char *delim, int findex, int *intval)
{
	int error_code = OK;
	char str_field[MAXCHAR];
	char tmp[MAXCHAR];
	
	/* use get_text_field() to get the field without spaces */
	if ((error_code = get_text_field(line, delim, findex, str_field)) != OK) {
		fprintf(fplog, "Error parsing text record: get_int_field(); text field %i not retrieved\n", findex);
		return error_code;
	}
	
	// remove bracketing quotes if they exist, otherwise just copy the field as is
	rm_quotes(tmp, str_field);
	
	// atoi() of a string that does not start with an inerpretable number returns 0 (including empty string)
	//  interpretable numbers include decimals and scientific notation (characters .,e,E,-,+ in the proper place)
	// so throw an error if other characters are present
	//  this restricts atoi() from operating on fields with suffix character beyond the number
	// this will not catch typos because the return value will be zero or a different number
	if (is_num(tmp)) {
		*intval = atoi(tmp);
	}
	else{
		fprintf(fplog, "Error parsing text record: get_int_field(); non-numeric field %i\n", findex);
		return ERROR_STR;
	}
	
	return OK;
}

/*******
 int get_text_field(char *line, const char *delim, int findex, char *str_field)
 line:		string containing record info
 delim:		delimiting character
 findex:	index of the desired field--this must start at one
 str_field:	address of character string for storing the contents of retrieved field:
			whitespace removed; "" if empty field or if field not found
 return:	error code
 note:		removes whitespace, but not bracketing quotes
 ******/
int get_text_field(char *line, const char *delim, int findex, char *str_field)
{
	int error_code = OK;
	char tmp[MAXCHAR];
	
	/* use get_field() to retrieve the field */
	if ((error_code = get_field(line, delim, findex, str_field)) != OK) {
		fprintf(fplog, "Error parsing text record: get_text_field(); field %i not retrieved\n", findex);
		return error_code;
	}
	
	/* remove whitespace characters if the string is not empty */
	if (strlen(str_field) > 0) {
		rm_whitesp(tmp, str_field);
		strcpy(str_field, tmp);
	}

	return OK;
}

/*******
 int get_field(char *line, const char *delim, int findex, char *str_field)
 line:		string containing record info
 delim:		the delimiting character
 findex:	index of the desired field--this must start at one
 str_field:	address of character string for storing the contents of field:
			"" if empty field or if field is not found
 return:	error code
 note:		throws error if the field is not found
 note:		does not throw whitespace error
 note:		does not remove bracketing quotes
 note:		double quote marks denote that at least one comma is embedded in the field,
			 and quoted fields are delineated by the double quote marks
 note:		if the field is the last one in the line in will contain whatever end of line characters might be present
 todo:		remove end of line characters if necessary
 ******/
int get_field(char *line, const char *delim, int findex, char *str_field)
{	
	int fld_index;			// current index in field
	int is_newfld;			// flag to denote the end of a field
	int count_flds = 0;		// count the number of fields
	char *cptr;				// pointer for looping over characters in line
	char *qchar = "\"";		// double quote for quoted fields
	char fend[MAXCHAR];		// character denoting the end of the field
	char fld_str[MAXCHAR];	// field string array

	if (findex < 1) {
		fprintf(fplog, "Error processing file record: get_field(); findex=%i < 1\n", findex);
		return ERROR_FILE;
	}

	/* loop to the desired field */
	cptr = line;
	// this is the loop over the line
	while (count_flds++ < findex) {
		fld_index = 0;
		is_newfld = 0;
		strcpy(fend, delim);
		// this is the loop over each field
		while (*cptr) {
			if (!is_newfld) {
				if (fld_index == 0 && *cptr == '\"') {	// quoted field
					strcpy(fend, qchar);
					fld_str[fld_index++] = *cptr++;
				} else {
					if (!strncmp(cptr, fend, 1)) {
						is_newfld = 1;
						if (*fend == '\"') {	// end of quoted field
							fld_str[fld_index++] = *cptr;
							cptr = cptr + 2;	// advance past the delimiter
						} else {
							cptr++;
						}
					} else {
						fld_str[fld_index++] = *cptr++;
					}
				}	// end if quoted field else not quoted field

			} else {
				break;
			}
		} // end while loop over field
		fld_str[fld_index] = '\0';	// terminate the field string
		if (!*cptr && count_flds != findex) {
			fprintf(fplog, "Error processing file record: get_field(); last field=%i reached before findex=%i\n",
					count_flds, findex);
			return ERROR_FILE;	// return if end of line is found before findex is reached
		}
	}	// end while loop over line 
	
	strcpy(str_field, fld_str);
	
	return OK;
}

/********
 int rm_whitesp(char *cln_field,char *str_field)
 cln_field:		pointer to field string with all whitespace removed
 str_field:		pointer to field string with possible whitespace
 return:		OK
 ********/
int rm_whitesp(char *cln_field,char *str_field)
{
	int i;
	int len;	/* length of string */
	int num=0;	/* number of whitespace characters */
	
	strcpy(cln_field, "\0");		//initialize the result variable
	len = (int)strlen(str_field);
	/* check each character in the string for whitespace */
	for(i = 0;i < len;i++){
		if(isspace((int)str_field[i])){
			num++;
		}
		else
			strncat(cln_field, &str_field[i], 1);
	}
	
	return OK;
}

/********
 int rm_quotes(char *cln_field,char *str_field)
 cln_field:		pointer to field string with the outer quotes removed
 str_field:		pointer to field string with outer quotes, and whitespace removed
 return:		OK
 ********/
int rm_quotes(char *cln_field,char *str_field)
{
	int len;	/* length of string */
	char *qchar = "\"";		/* double quote for quoted fields */
	
	// remove quotes if necessary
	if (!strncmp(str_field, qchar, 1)) {
		len = strlen(str_field);
		strncpy(cln_field, &str_field[1], len - 2);
	} else {
		strcpy(cln_field, str_field);
	}

	return OK;
}

/*********
 int is_num(char *str_field)
 str_field:	character string representing a field
 return:		1 if the field is numeric; 0 otherwise
 note: this function allows the characters .,e,E,-,+ because these can be interpreted by atoi(), atof(), etc
 note: this function does not check whether the format is correct, just whether these are the only characters
 **********/
int is_num(char *str_field)
{
	int i;
	int len;	/* length of string */
	char *onechar;
	
	len = (int)strlen(str_field);
	onechar = str_field;
	/* check each character in the string for digit or "." */
	for(i = 0;i < len;i++){
		if (!(isdigit((int) *onechar) || *onechar == '.' || *onechar == 'e' ||
			  *onechar == 'E' || *onechar == '-' || *onechar == '+')) {
			return 0;
		} else {
			onechar++;
		}
	}
	return 1;}


//
//  read_aez_new_info.c
//
//  ids and names corresponding to aez_bounds_new
//  the number ids must range from 1 to NUM_NEW_AEZ
//
//  arguments:
//  args_struct in_args: the input file arguments
//
//  return value:
//  integer error code: OK = 0, otherwise a non-zero error code
//
//  Created by Alan Di Vittorio on 10/13/15.
//
//

#include "lds.h"

int read_aez_new_info(args_struct in_args) {
    
    // names of the new AEZs
    // 1 header row
    // the first column is the integer id corresponding with the input raster in aez_bounds_new
    // the second column is the name of the aez
    
    int i;
    int nrecords = 0;               // count number of new AEZs
    
    char fname[MAXCHAR];			// file name to open
    FILE *fpin;						// file pointer
    char rec_str[MAXRECSIZE];		// string to hold one record
    const char* delim = ",";		// delimiter string for csv file
    int err = OK;					// error code for the string parsing function
    int out_index = 0;				// the index of the arrays to fill
    
    // create file name and open it
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.aez_new_info_fname);
    
    if((fpin = fopen(fname, "r")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_aez_new_info()\n", fname);
        return ERROR_FILE;
    }
    
    // skip the header line
    if(fscanf(fpin, "%*[^\n]\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_aez_new_info()\n", fname);
        return ERROR_FILE;
    }
    
    // count the records
    while (fscanf(fpin, "%*[^\n]\n") != EOF) {
        nrecords++;
    }
    
    // set the number of new aezs
    NUM_NEW_AEZ = nrecords;
    
    // allocate the arrays
    aez_codes_new = calloc(NUM_NEW_AEZ, sizeof(int));
    if(aez_codes_new == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for aez_codes_new: read_aez_new_info()\n");
        return ERROR_MEM;
    }
    aez_names_new = calloc(NUM_NEW_AEZ, sizeof(char*));
    if(aez_names_new == NULL) {
        fprintf(fplog,"Error: Failed to allocate memory for aez_names_new: read_aez_new_info()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_NEW_AEZ; i++) {
        aez_names_new[i] = calloc(MAXCHAR, sizeof(char));
        if(aez_names_new[i] == NULL) {
            fprintf(fplog,"Error: Failed to allocate memory for aez_names_new[%i]: read_aez_new_info()\n", i);
            return ERROR_MEM;
        }
    }
    
    rewind(fpin);
    
    // skip the header line
    if(fscanf(fpin, "%*[^\n]\n") == EOF)
    {
        fprintf(fplog,"Failed to scan over file %s header:  read_aez_new_info()\n", fname);
        return ERROR_FILE;
    }
    
    // read the aez new info records
    for (i = 0; i < NUM_NEW_AEZ; i++) {
        if (fscanf(fpin, "%[^\n]\n", rec_str) != EOF) {
            // get the intger code
            if((err = get_int_field(rec_str, delim, 1, &aez_codes_new[out_index])) != OK) {
                fprintf(fplog, "Error processing file %s: read_aez_new_info(); record=%i, column=1\n",
                        fname, i + 1);
                return err;
            }
            // get the name
            if((err = get_text_field(rec_str, delim, 2, &aez_names_new[out_index++][0])) != OK) {
                fprintf(fplog, "Error processing file %s: read_aez_new_info(); record=%i, column=2\n",
                        fname, i + 1);
                return err;
            }
        }else {
            fprintf(fplog, "Error reading file %s: read_aez_new_info(); record=%i\n", fname, i + 1);
            return ERROR_FILE;
        }
    } // end for loop over records
    
    fclose(fpin);
    
    if(i != NUM_NEW_AEZ)
    {
        fprintf(fplog, "Error reading file %s: read_aez_new_info(); records read=%i != expected=%i\n",
                fname, i, nrecords);
        return ERROR_FILE;
    }
    
    if (in_args.diagnostics) {
        // aez new info codes
        if ((err = write_text_int(aez_codes_new, NUM_NEW_AEZ, "aez_codes_new.txt", in_args))) {
            fprintf(fplog, "Error writing file %s: read_aez_new_info()\n", "aez_codes_new.txt");
            return err;
        }
        // aez new info names
        if ((err = write_text_char(aez_names_new, NUM_NEW_AEZ, "aez_names_new.txt", in_args))) {
            fprintf(fplog, "Error writing file %s: read_aez_new_info()\n", "aez_names_new.txt");
            return err;
        }
    }	// end if diagnostics
    
    return OK;
}

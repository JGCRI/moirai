/**********
 copy_to_destpath.c
 
 copy the gcam data system input files to the lds destination directory
    the lds destination directory is set in the lds_input.txt file as the 'ldsdestpath'
 
 copy the two mapping files o the mappings destination directory
    the mapping destination directory is set in the lds_input.txt file as the 'mapdestpath'
 
 the names of these files are set in the lds_input.txt file
 
 NOTE: this call automatically overwrites any file of the same name
 
 there are currenlty 9, and there will be one more
 
 to do: add the water footprint file
 
 arguments:
 args_struct in_args: the input file arguments
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 29 jan 2016
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "moirai.h"

int copy_to_destpath(args_struct in_args) {
    
    int err = OK;
    char fname[MAXCHAR];            // full path to filename
    char sys_string[MAXCHAR];       // string to pass to system()
    
    char cp_str[] = "cp -f ";
    char space_str[] = " ";
    
    // harvested area
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.harvestarea_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }

    // production
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.production_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // land rent
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.rent_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // irrigated harvested area
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.mirca_irr_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // rainfed harvested area
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.mirca_rfd_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // land type area
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.land_type_area_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // reference vegetation carbon
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.refveg_carbon_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // water footprint file
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.wf_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.ldsdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.ldsdestpath);
        return ERROR_COPY;
    }
    
    // iso mapping file
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.iso_map_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.mapdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.mapdestpath);
        return ERROR_COPY;
    }
    
    // land type mapping file
    strcpy(fname, in_args.outpath);
    strcat(fname, in_args.lt_map_fname);
    strcpy(sys_string, cp_str);
    strcat(sys_string, fname);
    strcat(sys_string, space_str);
    strcat(sys_string, in_args.mapdestpath);
    if((err = system(sys_string)) == -1) {
        fprintf(fplog, "\nError copying file %s to %s\n", fname, in_args.mapdestpath);
        return ERROR_COPY;
    }
    
    return OK;
}

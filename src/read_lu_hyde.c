/**********
 read_lu_hyde.c
 
 read one year/band of crop, pasture, and urban data into crop_grid[NUM_CELLS], pasture_grid[NUM_CELLS], and urban_grid[NUM_CELLS]
 area is in km^2
 
 the input files are bsq files that have been made by concatenating the individual year files
 33 years: 1700 - 2000 every 10 years, 2005, 2010
 the three file names are in the input file
 
 arguments:
 args_struct in_args: the input file arguments
 int year
 float* crop_grid:      the array to load the crop data into
 float* pasture_grid:   the array to load the pasture data into
 float* urban_grid:     the array to load the urban data into
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 12 Jan 2016
 
 Copyright 2016 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 **********/

#include "lds.h"

int read_lu_hyde(args_struct in_args, int year, float* crop_grid, float* pasture_grid, float* urban_grid) {
    
    // use this function to input data to the working grid
    
    // historical hyde land use data
    // bsq file with 33 bands (starts at upper left corner)
    // 4 byte float
    // 5 arcmin resolution, extent = (-180,180, -90, 90), ?WGS84?
    // values are area in km^2
    
    int nrows = 2160;				// num input lats
    int ncols = 4320;				// num input lons
    int ncells = nrows * ncols;		// number of input grid cells
    int insize = 4;					// 4 byte floats
    //int nodata = -9999;             // nodata value
    //double res = 5.0 / 60.0;		// resolution
    //double xmin = -180.0;			// longitude min grid boundary
    //double xmax = 180.0;			// longitude max grid boundary
    //double ymin = -90.0;			// latitude min grid boundary
    //double ymax = 90.0;				// latitude max grid boundary
    
    int i;
    int year_index;                 // the index of the year to read
    int hyde_years[NUM_HYDE_YEARS]; // the years in the hyde historical lu files
    
    char fname[MAXCHAR];            // file name to open
    FILE* fpin;
    long start_pos;                 // file byte index for desired year
    int num_read;					// how many values read in
    
    // create the array of available years
    hyde_years[0] = HYDE_START_YEAR;
    for (i = 1; i < (NUM_HYDE_YEARS - 2); i++) {
        hyde_years[i] = hyde_years[i-1] + 10;
    }
    hyde_years[NUM_HYDE_YEARS - 2] = 2005;
    hyde_years[NUM_HYDE_YEARS - 1] = 2010;
    
    // find the desired year index
    year_index = NOMATCH;
    for (i = 0; i < NUM_HYDE_YEARS; i++) {
        if (year == hyde_years[i]) {
            year_index = i;
            break;
        }
    }
    if (year_index == NOMATCH) {
        fprintf(fplog,"Invalid year %i for lu hyde files: read_lu_hyde()\n", year);
        return ERROR_IND;
    }
    
    // get the file position for desired year
    start_pos = ncells * insize * year_index;
    
    // crop
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.hist_crop_rast_name);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_lu_hyde()\n", in_args.hist_crop_rast_name);
        return ERROR_FILE;
    }
    
    fseek(fpin, start_pos, SEEK_SET);
    
    // read the data and check for consistency
    num_read = fread(crop_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_nfert(); num_read=%i != NUM_CELLS=%i\n",
                in_args.hist_crop_rast_name, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    fclose(fpin);
    
    // pasture
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.hist_pasture_rast_name);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_lu_hyde()\n", in_args.hist_pasture_rast_name);
        return ERROR_FILE;
    }
    
    fseek(fpin, start_pos, SEEK_SET);
    
    // read the data and check for consistency
    num_read = fread(pasture_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_nfert(); num_read=%i != NUM_CELLS=%i\n",
                in_args.hist_pasture_rast_name, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    fclose(fpin);
    
    // urban
    strcpy(fname, in_args.inpath);
    strcat(fname, in_args.hist_urban_rast_name);
    if((fpin = fopen(fname, "rb")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s:  read_lu_hyde()\n", in_args.hist_urban_rast_name);
        return ERROR_FILE;
    }
    
    fseek(fpin, start_pos, SEEK_SET);
    
    // read the data and check for consistency
    num_read = fread(urban_grid, insize, ncells, fpin);
    fclose(fpin);
    if(num_read != NUM_CELLS)
    {
        fprintf(fplog, "Error reading file %s: read_nfert(); num_read=%i != NUM_CELLS=%i\n",
                in_args.hist_urban_rast_name, num_read, NUM_CELLS);
        return ERROR_FILE;
    }
    
    fclose(fpin);
    
    return OK;
}
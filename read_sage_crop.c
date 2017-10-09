/**********
 read_sage_crop.c

 read one sage netcdf crop file
	the files are zipped orignially
	this function will unzip them if necessary and then leaves them unzipped
	this function could be modified to read the sage ascii grid files also
 get yield in metric tonnes per km^2
 get harvest area in km^2

 the file has 4 attribute variables:
	longitude (4320), latitude (2160), level (4), time (1)
 the four levels are (float type):
	harvest area (fraction of land area), yield (t per ha), quality-area, quality-yield
 the data variable has four dimensions:
	cropdata(time, level, latitude, longitude)

 The quality fields do not appear to be of much use, although if they are zero do not read in the data
	There are no quality yield values that are 0 where sage land and yield values are valid
	All quality harvest area zero values are associated with in values of zero
 There are tiny (1e-22 and smaller) area values that have quality flags of 1, but blow up the recalibration code
 The quality fields range from 0 to 1
 There are not valid input data (based on the sage land mask as checked below) where the quality fields are set to nodata

 this function ensures that valid yield and harvest area values exist for sage land cells
  sage non-land cells set yields and area to NODATA
  if yield and area values are nodata for sage land cells, these values are set to zero

 Abnormally small values  (< 1e-6) do not pose a problem for regular processing
	but they do exist in these data and pose problems for recalibration as they can produce an effectively zero
		denominator in the recalibration calculation that causes overflow and/or huge numbers
	harvest area in fraction small values are usually < ~1e-22, but there are some ~1e-7
	yield in small values are ~1e-19
	this overflow/huge# problem mainly occurs with the harvested area
	it did not happen to production values for 2003-2007 avg recalibration
		even though there are some abnormally small yield in values

 The abnormal values are filtered out in this function. This has a negligible difference on the outputs.

 arguments:
 char *fname:	path and base filename for sage crop file to read
 rinfo_struct raster_info:	raster info structure

 return value:
 integer error code: OK = 0, otherwise a non-zero error code

 Created by Alan Di Vittorio on 4 Sep 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved

 **********/

#include "lds.h"

int read_sage_crop(char *fname, char *sagepath, char *cropfilebase_sage, rinfo_struct raster_info) {

	int i;
	int nrows = 2160;				// num input lats
	int ncols = 4320;				// num input lons
	int ncells = nrows * ncols;		// number of input grid cells
	float nodata = 9E20;			// nodata value
	//double res = 5.0 / 60.0;		// resolution
	//double xmin = -180.0;			// longitude min grid boundary
	//double xmax = 180.0;			// longitude max grid boundary
	//double ymin = -90.0;			// latitude min grid boundary
	//double ymax = 90.0;				// latitude max grid boundary
	float temp_flt;
	float *qual_yield;				// quality field for yield
	float *qual_harv;				// quality field for area

	char lname[MAXCHAR];			// file name to open
	FILE *fpin;						// file pointer
	int sysrv;						// system returen value
	int ncid;						// netcdf file id
	int ncvarid;					// variable id returned by nc_inq_varid()
	int ncerr;						// error return value; 0 = ok
	// char *varname = "cropdata";		// name of the variable to read
	char varname[MAXCHAR];  // name of the variable to read
	static size_t start_yield[] = {0, 1, 0, 0};		// start indices for yield
	static size_t start_harv[] = {0, 0, 0, 0};		// start indices for harvest area
	static size_t start_qual_yield[] = {0, 3, 0, 0};		// start indices for yield
	static size_t start_qual_harv[] = {0, 2, 0, 0};		// start indices for harvest area
	static size_t count[] = {1, 1, 2160, 4320};		// lengths for reading yield

	// some input data file name suffixes
	const char sage_crop_nctag[] = "_AreaYieldProduction.nc";					// suffix for sage base file names, netcdf, unzipped
	const char sage_crop_ncztag[] = "_HarvAreaYield2000_NetCDF.zip";				// suffix for sage base file names, netcdf, zipped

	// allocate arrays for the quality fields
	qual_harv = calloc(ncells, sizeof(float));
	if(qual_harv == NULL) {
		fprintf(fplog,"Failed to allocate memory for qual_harv:  read_sage_crop()\n");
		return ERROR_MEM;
	}
	qual_yield = calloc(ncells, sizeof(float));
	if(qual_yield == NULL) {
		fprintf(fplog,"Failed to allocate memory for qual_yield:  read_sage_crop()\n");
		return ERROR_MEM;
	}

	// finish file name and try to open it; if it fails, then it has not been unzipped
	strcpy(lname, fname);
	strcat(lname, sage_crop_nctag);
	if((fpin = fopen(lname, "rb")) == NULL)
	{
			// unzip this file
			strcpy(lname, "unzip -o -j -d ");
			strcat(lname, sagepath);
      strcat(lname, " ");
      strcat(lname, fname);
			strcat(lname, sage_crop_ncztag);
		sysrv = system(lname);
	} else {
		fclose(fpin);
	}

	// now make the unzipped file name again
	strcpy(lname, fname);
	strcat(lname, sage_crop_nctag);

	if ((ncerr = nc_open(lname, NC_NOWRITE, &ncid))) {
		fprintf(fplog,"Failed to open %s for reading: read_sage_crop(); ncerr = %i\n", lname, ncerr);
		return ERROR_FILE;
	}

  strcpy(varname,cropfilebase_sage);
  strcat(varname,"Data");

	if ((ncerr = nc_inq_varid(ncid, varname, &ncvarid))) {
		fprintf(fplog,"Error %i when getting netcdf var id for %s: read_sage_crop()\n", ncerr, varname);
		return ERROR_FILE;
	}

	if ((ncerr = nc_get_vara_float(ncid, ncvarid, start_yield, count, yield_in))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_sage_crop()\n", ncerr, varname);
		return ERROR_FILE;
	}

	if ((ncerr = nc_get_vara_float(ncid, ncvarid, start_qual_yield, count, qual_yield))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_sage_crop()\n", ncerr, varname);
		return ERROR_FILE;
	}

	if ((ncerr = nc_get_vara_float(ncid, ncvarid, start_harv, count, harvestarea_in))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_sage_crop()\n", ncerr, varname);
		return ERROR_FILE;
	}

	if ((ncerr = nc_get_vara_float(ncid, ncvarid, start_qual_harv, count, qual_harv))) {
		fprintf(fplog,"Error %i when reading netcdf var %s: read_sage_crop()\n", ncerr, varname);
		return ERROR_FILE;
	}

	// loop over all the data to convert the values to working units
	//  and to make sure that valid crop values exist for sage land cells
	for (i = 0; i < ncells; i++) {
		// convert yield to tonnes per km^2
		if (yield_in[i] == nodata) {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				yield_in[i] = NODATA;
			} else {
				yield_in[i] = 0;
			}
		} else {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				yield_in[i] = NODATA;
			} else {
				temp_flt = yield_in[i];
				if (qual_yield[i] != 0) {
					yield_in[i] = yield_in[i] / HA2KMSQ;
					if (qual_yield[i] == nodata && yield_in[i] != 0) {
						// this condition does not occur
						fprintf(fplog,"Warning: qual_yield[%i] = nodata and yield_in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, yield_in[i], fname);
					}
				} else {
					if (qual_yield[i] == 0) {
						// this condition does not occur
						fprintf(fplog,"Warning: qual_yield[%i] = 0 and yield_in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, yield_in[i], fname);
					}
					temp_flt = yield_in[i] / HA2KMSQ;
					yield_in[i] = 0;
				}

				// this treshold is  0.1 t / km^2, or 0.001 t / ha, (min fao value is ~0.02 t / ha)
				// these values are usually on the order of 1e-19, which is unrealistic
				// remove these abnormal values from processing
				if (yield_in[i] < 0.1 && yield_in[i] != nodata && yield_in[i] !=0) {
					//fprintf(fplog,"Warning: yield_in[%i] = %e < 0.1 t / km^2 for crop %s: read_sage_crop()\n", i, yield_in[i], fname);
					yield_in[i] = 0;
				}	// end if bad data then remove
			}	// end if land area nodata else land area data
		}	// end if yield nodata else valid yield data
		// convert land area fraction to km^2
		if (harvestarea_in[i] == nodata) {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				harvestarea_in[i] = NODATA;
			} else {
				harvestarea_in[i] = 0;
			}
		} else {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				harvestarea_in[i] = NODATA;
			} else {
				temp_flt = harvestarea_in[i];
				if (qual_harv[i] != 0) {
					harvestarea_in[i] = harvestarea_in[i] * land_area_sage[i];
					if (qual_harv[i] == nodata && temp_flt != 0) {
						// this condition does not occur
						fprintf(fplog,"Warning: qual_harv[%i] = nodata and fraction _in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, temp_flt, fname);
					}
				} else {
					if (qual_harv[i] == 0) {
						// the in fraction is always zero where the quality flag is zero
						//fprintf(fplog,"Warning: qual_harv[%i] = 0 and fraction in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, temp_flt, fname);
					}
					temp_flt = harvestarea_in[i];
					harvestarea_in[i] = 0;
				}
				// this threshold is the fraction corresponding to 100 m^2 if a cell has 100 km^2 of land area
				//  (max sage cell land area is ~86 km^2)
				// remove these abnormal values from processing
				if (temp_flt < 1e-6 && temp_flt != nodata && temp_flt !=0) {
					//fprintf(fplog,"Warning: fraction in[%i] = %e < 1e-6 for crop %s: read_sage_crop()\n", i, temp_flt, fname);
					harvestarea_in[i] = 0;
				}	// end if bad data then remove
			}	// end if land area nodata else land area data
		}	// end if harvested area nodata else valid harevested area data
	}	// end for i loop over all grid cells

	nc_close(ncid);

	free(qual_harv);
	free(qual_yield);

	return OK;
}

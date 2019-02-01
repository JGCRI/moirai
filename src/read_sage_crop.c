/**********
 read_sage_crop.c

 read one sage netcdf crop file
	the files are zipped orignially
	this function will unzip them if necessary and then leaves them unzipped
	this function could be modified to read the sage ascii grid files also
 get yield in metric tonnes per km^2 (input is metric tonnes per ha)
 get harvest area in km^2 (first input is in fraction of land area in grid cell)

 the file has 4 attribute variables:
	longitude (4320), latitude (2160), level (6), time (1)
 the four levels are (float type):
	harvest area (fraction of land area), yield (t per ha), quality-area, quality-yield, total harvested area (ha), production (t)
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

 Abnormally small values do not pose a problem for regular processing
	but they do exist in these data and pose problems for recalibration as they can produce an effectively zero
		denominator in the recalibration calculation that causes overflow and/or huge numbers
	harvest area in fraction small values are usually < ~1e-22, but there are some ~1e-7
	yield in small values are ~1e-19
	this overflow/huge# problem mainly occurs with the harvested area
	it did not happen to production values for 2003-2007 avg recalibration
		even though there are some abnormally small yield in values

 For harvested area use 1e-8 as the threshold
 	this threshold (1e-8) is the fraction corresponding to 1 m^2 if a cell has 100 km^2 of land area
 	(max sage cell land area is ~86 km^2)
 For yield use 0.0001 t / ha as the threshold
 	this treshold is  0.01 t / km^2, or 0.0001 t / ha is 2 orders of magnitude less than the min fao value of ~0.02 t / ha
 The abnormal values less than these thresholds are filtered out in this function. This has a negligible difference on the outputs.

 arguments:
 char *fname:	path and base filename for sage crop file to read
 rinfo_struct raster_info:	raster info structure

 return value:
 integer error code: OK = 0, otherwise a non-zero error code

 Created by Alan Di Vittorio on 4 Sep 2013
 Copyright 2018 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 This file is part of Moirai.
 
 Moirai is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Moirai is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Moirai (/moirai/docs/COPYING.txt). If not, see <https://www.gnu.org/licenses/>.

 **********/

#include "moirai.h"

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

	float harvest_thresh = 1e-8;
	float yield_thresh = 0.0001;
	
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
		// do harvested area first to calibrate the sage individual crop data to the sage physical crop area
		// this applies the sage cropping fraction to the hyde physical cropland area
		// convert land area fraction to km^2
		if (harvestarea_in[i] == nodata) {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				harvestarea_in[i] = NODATA;
			} else {
				harvestarea_in[i] = 0;
			}
		} else {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				harvestarea_in[i] = 0;
			} else {
				// this threshold (1e-8) is the fraction corresponding to 1 m^2 if a cell has 100 km^2 of land area
				//  (max sage cell land area is ~86 km^2)
				// remove these very small values from processing
				if (harvestarea_in[i] < harvest_thresh && harvestarea_in[i] != nodata && harvestarea_in[i] !=0) {
					//fprintf(fplog,"Warning: fraction in[%i] = %e < %f for crop %s: read_sage_crop()\n", i, harvestarea_in[i], , harvest_thresh, fname);
					harvestarea_in[i] = 0;
					// end if bad data then remove
				} else if (qual_harv[i] != 0) {
					if (cropland_area_sage[i] == raster_info.cropland_sage_nodata || cropland_area_sage[i] == 0) {
						harvestarea_in[i] = 0;
					} else {
						// get the original harvestarea_in in km^2
						temp_flt = harvestarea_in[i]  * land_area_sage[i];
						// now store adjusted harvestarea in km^2
						// sage in harvested area fraction * sage land area / sage physical crop area * hyde physical crop area
						harvestarea_in[i] = harvestarea_in[i]  * land_area_sage[i] / cropland_area_sage[i] * cropland_area[i];
					}
					if (qual_harv[i] == nodata && harvestarea_in[i] != 0) {
						// this condition does not occur
						fprintf(fplog,"Warning: qual_harv[%i] = nodata and fraction _in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, harvestarea_in[i], fname);
					}
				} else { // no valid harvest area
					harvestarea_in[i] = 0;
					if (qual_harv[i] == 0) {
						// the in fraction is always zero where the quality flag is zero
						//fprintf(fplog,"Warning: qual_harv[%i] = 0 and fraction in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, harvestarea_in[i], fname);
					}
				} // end else no valid harvestarea_in found
			}	// end if land area sage nodata else sage land area data
		}	// end if harvested area nodata else valid harevested area data
		
		// normalize this yield to the sage input production and the normalized harvested area
		if (yield_in[i] == nodata) {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata) {
				yield_in[i] = NODATA;
			} else {
				yield_in[i] = 0;
			}
		} else {
			if (land_area_sage[i] == raster_info.land_area_sage_nodata || harvestarea_in[i] == nodata || harvestarea_in[i] == 0 || harvestarea_in[i] == NODATA) {
				yield_in[i] = 0;
			} else {
				// this treshold is  0.01 t / km^2, or 0.0001 t / ha, (min fao value is ~0.02 t / ha)
				// abnormal values are usually on the order of 1e-19, which is unrealistic
				// remove these abnormal values from processing
				if (yield_in[i] < yield_thresh && yield_in[i] != nodata && yield_in[i] !=0) {
					//fprintf(fplog,"Warning: yield_in[%i] = %e < %f t / ha for crop %s: read_sage_crop()\n", i, yield_in[i], yield_thresh, fname);
					yield_in[i] = 0;
					// end if bad data then remove
				} else if (qual_yield[i] != 0) {
					// convert yield to tonnes per km^2, calculate original production, then calculate normalized yield
					yield_in[i] = yield_in[i] / HA2KMSQ * temp_flt / harvestarea_in[i];
					if (qual_yield[i] == nodata && yield_in[i] != 0) {
						// this condition does not occur
						fprintf(fplog,"Warning: qual_yield[%i] = nodata and yield_in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, yield_in[i], fname);
					}
				} else { // no valid yield
					yield_in[i] = 0;
					if (qual_yield[i] == 0 && yield_in[i] != 0) {
						// qual == 0 and yield == 0 does occur
						// but qual ==0 and yield != 0 does not occur
						fprintf(fplog,"Warning: qual_yield[%i] = 0 and yield_in[%i] = %e for crop %s:  read_sage_crop()\n", i, i, yield_in[i], fname);
					}
				} // end else no valid yield
			}	// end if land area nodata else land area data
		}	// end if yield nodata else valid yield data
		
	}	// end for i loop over all grid cells

	nc_close(ncid);

	free(qual_harv);
	free(qual_yield);

	return OK;
}

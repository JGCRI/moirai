/**********
 moirai.h

 Created by Alan Di Vittorio on 3 May 2013
 
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
 
 September 2015 - renamed LDS (land data system) by Alan Di Vittorio
    This is to be the base code for developing a system that works with an arbitrary number of AEZs
    An AEZ (agro-ecological zone) can be defined arbitrarily
        The AEZ nomenclature is retained both for practical (legacy) reasons
        And because AEZ refers to a land unit that is to be treated as a whole with respect to:
            agriculture and vegetation and land value
    Each AEZ is now a unique area on the globe
    This will ultimately be LDS version 2.0

 December 2017 - Alan Di Vittorio
 	starting the modification to use ISAM LULC data in place of just the HYDE data
 	these ISAM data use HYDE 3.2 (from spring 2017)
 	this will become version 3
 
 NetCDF notice:
 
 Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, University Corporation for Atmospheric Research/Unidata.
 
 Portions of this software were developed by the Unidata Program at the University Corporation for Atmospheric Research.
 
 Access and use of this software shall impose the following obligations and understandings on the user. The user is granted the right, without any fee or cost, to use, copy, modify, alter, enhance and distribute this software, and any derivative works thereof, and its supporting documentation for any purpose whatsoever, provided that this entire notice appears in all copies of the software, derivative works and supporting documentation. Further, UCAR requests that the user credit UCAR/Unidata in any publications that result from the use of this software or in any product that includes this software, although this is not an obligation. The names UCAR and/or Unidata, however, may not be used in any advertising or publicity to endorse or promote any products or commercial entity unless specific written permission is obtained from UCAR/Unidata. The user also understands that UCAR/Unidata is not obligated to provide the user with any support, consulting, training or assistance of any kind with regard to the use, operation and performance of this software nor to provide the user with any updates, revisions, new versions or "bug fixes."
 
 THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
 
 
**********/

#ifndef LDSHDR
#define LDSHDR

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <netcdf.h>

#define CODENAME				"moirai"				// name of the compiled program
#define VERSION         		"3.1"           			// current version
#define MAXCHAR					1000						// maximum string length
#define MAXRECSIZE				10000						// maximum record (csv line) length in characters

// year of HYDE data to read in for calculating potential vegetation area (for carbon and forest land rent) and pasture animal land rent
#define REF_YEAR				2000
#define REF_CARBON_YEAR			2010

// the min and max forest codes for the SAGE raster data (see SAGE_PVLT.csv)
// these assume that the forest codes are all consecutive with no others mixed in the enumeration
#define MAX_SAGE_FOREST_CODE    8
#define MIN_SAGE_FOREST_CODE    1

// counts of useful variables
//kbn 2020-06-01 Updating input arguments to include 6 new carbon states for soil_carbon
#define NUM_IN_ARGS						130					// number of input variables in the input file
#define NUM_ORIG_AEZ						18							// number of original GTAP/GCAM AEZs

// necessary FAO input data info
// this applies to the yield, harvest area, production, and prod price input FAO data
// these four files all need to be in the same format with the same years, in order and contiguous
#define FAO_START_YEAR_COL					8		// the column of the start year in the fao input data
#define FAO_START_YEAR						1993	// first year in fao yield, HA, prod, and prodprice files
#define FAO_END_YEAR						2016	// last year in fao yield, HA, prod, and prodprice files
#define NUM_FAO_YRS							(FAO_END_YEAR - FAO_START_YEAR + 1)	// number of years in the files

// averaging periods and years for production, harvest area, yield, and land rent data
#define SAGE_AVG_PERIOD			7
#define SAGE_START_YEAR			1997
#define RECALIB_AVG_PERIOD		5

// useful values for processing the additional spatial data
#define NUM_MIRCA_CROPS         26              // number of crops in the mirca2000 data set
#define NUM_EPA_PROTECTED       8              // Categories of suitability and protection from the EPA
#define NUM_CARBON              6              //Categories of carbon states (0- Weighted average, 1- Median, 2- Min, 3- Max, 4- Q1 carbon, 5 -Q3 ) 
#define LULC_START_YEAR         1800            // the first lulc year
#define NUM_LULC_LC_TYPES       23            	// number of ordered lulc types that are land cover (not land use)
#define NUM_HYDE_TYPES_MAIN		3				// first 3 types that include all land use area: urban, crop, grazing
#define HYDE_START_YEAR         1700            // the first hyde year
#define NUM_HYDE_YEARS          47            	// number of available hyde years
#define NUM_HYDE_POST2000_YEARS 16            	// number of hyde years >= 2001; these are each year
#define CROP_LT_CODE            10              // used to generate land type category
#define PASTURE_LT_CODE         20              // used to generate land type category
#define URBAN_LT_CODE           30              // used to generate land type category
#define SCALE_POTVEG            100             // used to generate land type category
#define NUM_LU_CATS             4               // crop, pasture, urban, potential veg (pv code = 0)
#define NUM_WF_CROPS            18              // number of water footprint crops
#define NUM_WF_TYPES            4               // number of water footprint types (blue, green, gray, total)


// conversion factors for output
#define KMSQ2HA					100.0						// km^2 * KMSQ2HA = ha
#define MSQ2KMSQ				(1/1000000.0)					// m^2 * MSQ2KMSQ = km^2
#define KGMSQ2MGHA              10.0                        // kg/m^2 * KGMSQ2MGHA = Mg/ha
#define USD2MILUSD				(1/1000000.)0					// USD * USD2MILUSD = million USD
#define VEG_CARBON_SCALER        0.1                            //scaling factor used by Spawn et al on veg carbon rasters. We need to multiply our values by this to get actual values.

// conversion factors for inputs
#define HGHA2TKMSQ				0.01			// conversion factor from hg / ha to t / km^2; FAOSTAT yield file
#define HA2KMSQ					0.01			// conversion factor from ha to km^2; FAOSTAT harvest area file and sage crop file
#define MIL2ONE					1000000.0			// factor to convert million USD to USD; GTAP land rent file

// useful flags and values
#define NOMATCH					-1				// if there isn't a matching country across data sets
#define NA_TEXT                  "-"            // if there is no iso3 or name for a country/territory
#define FAOCTRY2GCAMCTRYAEZID   10000           // the gcam country+aez id is fao country id * 10000 + aez id; this is also used for the region-glu image
#define ZERO_THRESH				1/1000000.0		// if a landtype area value is less than this, it is zero
#define ROUND_TOLERANCE			1/1000000.0		// tolerance for checking sums and zeros in read_protected and proc_lulc_area

// working resolution is 5 arcmin (2160x4320), WGS84 spherical earth, lat-lon projection
// the origin is the upper left corner at 90 Lat and -180 Lon
// the lat/lon dimensions need to have an even number of cells
#define NUM_LAT					2160						// number of lats in working grids
#define NUM_LON					4320						// number of lons in working grids
#define NUM_CELLS				(NUM_LAT * NUM_LON)			// number of grid cells in working grids
#define GRID_RES				(5.0/60.0)					// working grid resolution; decimal degree
#define GRID_RES_SEC			300.0						// workding grid resolution; arc-seconds
#define NODATA					-9999						// nodata value

// LULC input grid; the origin corner is 0 lon and -90 lat
#define NUM_LAT_LULC			360							// number of lats in input lulc data
#define NUM_LON_LULC			720							// number of lons in input lulc
#define NUM_CELLS_LULC			(NUM_LAT_LULC * NUM_LON_LULC)			// number of grid cells in input lulc data

// some constants for calculating the area of a grid cell
#define AVE_ER					6371007.181		// average earth radius; from MODIS land products WGS84 average spherical radius;  meters
#define PI						(4.0*atan(1.0))
#define DEG2RAD					(PI/180.0)
#define SEC2DEG					(1/3600.0)
#define DEG2SEC					3600.0

// error codes
#define OK						0							// no error!
#define ERROR_USAGE				1							// incorrect command line usage
#define ERROR_FILE				2							// error associated with file i/o operation
#define ERROR_STR				3							// error associated with processing strings from csv files
#define ERROR_CALC				4							// error associated with algorithm calculations
#define ERROR_MEM				5							// error associated with failed memory allocation
#define ERROR_IND				6							// error associated with failed index finding
#define ERROR_COPY				7							// error associated with failed copy of output file


// variables for number of records based on input files
int NUM_FAO_CTRY;                       // number of FAO/VMAP0 countries, including additions (see FAO_iso_VMAP0_ctry.csv)
int NUM_GTAP_CTRY87;					// number of 87 GTAP countries (ctry87) for land rent data (see GTAP_GCAM_ctry87.csv)
int NUM_GCAM_RGN;						// number of GCAM regions (see GCAM_region_names_32reg.csv; or 14reg)
int NUM_GCAM_ISO_CTRY;                  // number of GCAM ISO countries for region mapping (see iso_GCAM_regID_32reg.csv; or 14reg)
int NUM_NEW_AEZ;						// number of unique global climate AEZs (or other delineated areas) up to 9999; must be >= NUM_ORIG_AEZ=18 for the land rent calculation to work (see Global235_CLM_0125_dissolve.csv)
int NUM_GTAP_USE;						// number of GTAP uses (GTAP_use) (see GTAP_use.csv)
int NUM_SAGE_PVLT;						// number of SAGE potential vegetation land types (see SAGE_PVLT.csv)
int NUM_SAGE_CROP;						// number of SAGE crops (SAGE_crop) (see SAGE_gtap_fao_crop2use.csv)
int NUM_HYDE_TYPES;						// number of hyde land use types/files; the first 3 describe the total land use state
int NUM_LULC_TYPES;						// number of input lulc types

// for downscaling the lulc data to the working grid
int NUM_LU_CELLS;		// the number of lu working grid cells within a coarser res lulc cell
float **rand_order;		// the array to store the within-coarse-cell-index of the lu cell, or each lulc cell
float *****refveg_carbon_out;		// the potveg carbon out table;4th dim is the state of carbon; 5th dim is the two carbon density values and the area
// useful utility variables
char systime[MAXCHAR];					// array to store current time
FILE *fplog;							// file pointer to log file for runtime output
//FILE *debug_file;
//FILE *cell_file;

// area and production arrays: now 3d arrays, dim1=country[NUM_FAO_CTRY], dim2=aez[ctry_aez_num], dim3=crop[NUM_SAGE_CROP]
//  so the second dimension is variable, and it matches the aez list arrays below
float ***harvestarea_crop_aez;            // harvested area output (ha), output to nearest integer
float ***production_crop_aez;             // production output (metric tonnes), output to nearest integer
// associated to output data array: dim1=country[NUM_FAO_CTRY], dim2=aez[ctry_aez_num]
//  so the second dimension is variable, and it matches the aez list arrays below
float **pasturearea_aez;                 // pasture area (ha)
// land rent output: dim1=land rent region[NUM_GTAP_CTRY87], dim2=aez[reglr_aez_num], dim3=use[NUM_GTAP_USE]
float ***rent_use_aez;                    // land rent output (million USD), output a total of 10 digits

// lists of AEZs within fao country and land rent region and gcam region
int **ctry_aez_list;                    // AEZ codes for each fao country - dim1=fao country, dim2=aez codes
int *ctry_aez_num;                      // number of AEZs for each fao country
int **reglr_aez_list;                   // AEZ codes for each land rent region - dim1=land rent region, dim2=aez codes
int *reglr_aez_num;                     // number of AEZs for each land rent region
int **reggcam_aez_list;                 // AEZ codes for each gcam region - dim1=gcam region, dim2=aez codes
int *reggcam_aez_num;                   // number of AEZs for each gcam region
// list of land type category mappings for the land type area and potveg carbon csv outputs
int num_lt_cats;        // the number of categories
int *lt_cats;           // the list of categories

// variables to track taiwan and hong kong GLU areas for land rent separation
// probably not more than 10 GLUs in each of these, but use NUM_ORIG_AEZ to allocate space for now
float twn_land_area;                // total valid taiwan land area
float hkg_land_area;                // total valid hong kong land area
float *twn_glu_area;                // total valid taiwan glu area
float *hkg_glu_area;                // total valid taiwan glu area

// raster data as 1-d arrays; numlat * numlon, start at upper left corner, lon varies fastest [NUM_LAT X NUM_LON]
// these are allocated and free dynamically as needed in moirai_main.c
// they are all 1d arrays of size NUM_CELLS, which is currently hardcoded for the 5 arcmin resolution
float *harvestarea_in;                  // input harvest area (km^2), reused by each individual crop
float *yield_in;                        // input yield (metric tonnes / km^2), reused by each individual crop
int *aez_bounds_new;                    // new aez boundaries (integers 1 to NUM_NEW_AEZ)
int *aez_bounds_orig;                   // original aez boundaries (integers 1 to NUM_ORIG_AEZ)
float *cropland_area_sage;              // sage cropland area for normalizing sage crop data (km^2)
float *cropland_area;                   // cropland area for ref veg area calc for forest land rent (km^2)
float *pasture_area;                    // pasture area for ref veg area calc and animal land rent calc (km^2)
float *urban_area;                      // urban area for ref veg area calc for forest land rent calc (km^2)
float **lu_detail_area;					// additional hyde data files with more detailed area (km^2); d1=hyde types
float *refveg_area;                     // reference vegetation area for forest land rent calc (km^2)
float *refcarbon_area;                     // reference vegetation area for carbon calculations
int *potveg_thematic;                   // potential vegetation thematic data (integers 1 to NUM_SAGE_PVLT)
int *refveg_thematic;                   // reference vegetation thematic data (integers 1 to NUM_SAGE_PVLT)
int *refvegcarbon_thematic;                   // reference vegetation thematic data (integers 1 to NUM_SAGE_PVLT)
short *country_fao;                     // fao country codes (integer fao code values)
float *cell_area;                       // total area of grid cell; calculated based on spherical earth (km^2)
float *cell_area_hyde;                  // total area of hyde land grid cells; from hyde data set (km^2)
float *land_area_sage;                  // max land area of sage working grid cell (km^2)
float *land_area_hyde;                  // max land area of hyde data cells (km^2)
float *sage_minus_hyde_land_area;       // difference between the sage and hyde land area (km^2)
int *country87_gtap;                    // map of gtap87 countries found
int *land_mask_ctryaez;                 // 1=used for output; 0=not used for output
int *missing_aez_mask;                  // 1=no new aez value for land sage cell haveing crop data; 0=ok
int *region_gcam;                       // gcam gis region codes, based on iso mapping and fao country raster
float *glacier_water_area_hyde;         // difference (residual) between the hyde total cell area and hyde land area for hyde land cells (km^2)
int *land_mask_aez_orig;                // 1=land; 0=no land
int *land_mask_aez_new;                 // 1=land; 0=no land
int *land_mask_sage;                    // 1=land; 0=no land
int *land_mask_hyde;                    // 1=land; 0=no land
int *land_mask_lulc;                    // 1=land; 0=no land
int *land_mask_fao;                     // 1=land; 0=no land
int *land_mask_potveg;                  // 1=land; 0=no land
int *land_mask_refveg;                  // 1=land; 0=no land
int *land_mask_forest;                  // 1=forest; 0=no forest

//kbn 2020-02-29 Introducing objects for protected area rasters from Category 1 to 7
float **protected_EPA; //dim 1 is the type of protected area, dim 2 is the grid cell
//kbn 2020-06-01 Changing soil carbon variable
//kbn 2020-06-29 Changing vegetation carbon variable
float **soil_carbon_sage; //dim 1 is the type of state, dim 2 is the grid cell
int ***soil_carbon_array_cells;//These are the total number of cells contained within each array
float *****soil_carbon_array; //soil carbon array to calculate the soil carbon values for each state
float *****soil_carbon_crop_array; //soil carbon array to calculate the soil carbon values for each state
float *****soil_carbon_pasture_array; //soil carbon array to calculate the soil carbon values for each state
float *****soil_carbon_urban_array; //soil carbon array to calculate the soil carbon values for each state
float *****veg_carbon_array; //vegetation carbon array to calculate vegetation carbon values for each state
float **veg_carbon_sage;  //dim 1 is the type of state, dim 2 is the grid cell
//Add above and below ground ratio for vegetation carbon
float **above_ground_ratio; //dim 1 is the type of state, dim 2 is the grid cell
float **below_ground_ratio; //dim 1 is the type of state, dim 2 is the grid cell
// raster arrays for inputs with different resolution
// these are also stored starting at upper left corner with lon varying fastest
float **lulc_input_grid;						// lulc input area (km^2); dim 1 = land types; dim 2 = grid cells
// indices of land cells within the raster data; these are the only cells processed
// these are allocated and free dynamically as needed in moirai_main.c
// they are all 1d arrays of size NUM_CELLS, because it would add too much time to change their size at each additional cell
int *land_cells_aez_new;                    // indices of the cells containing land in new aez file
int num_land_cells_aez_new;					// the actual number of land cell indices in land_cells_aez_new[]
int *land_cells_sage;                       // indices of the cells containing land in sage data
int num_land_cells_sage;					// the actual number of land cell indices in land_cells_sage[]
int *land_cells_hyde;                       // indices of the cells containing land in hyde data
int num_land_cells_hyde;					// the actual number of land cell indices in land_cells_hyde[]
int *forest_cells;                          // indices of the cells containing forest in sage pot veg data
int num_forest_cells;						// the actual number of land cell indices in forest_cells[]

// original GTAP data as input to GCAM; aez varies fastest, then use, then ctry87
// the order of country is the GTAP_GCAM_ctry87 order, the order of use is the GTAP use order, and the order of original AEZs is 1-NUM_ORIG_AEZ
float *rent_orig_aez;		// original land rent (million USD, the currency year of these values is an input)

// FAOSTAT data
// the four associated input files need to be in the same format with the same years, in order and contiguous
// for non-price data the year varies fastest, then crop, then fao country
// the order of country is the FAO order, the order of crop is the GTAP order, and the yeas are chronological
// these are only needed if recalibrating the SAGE data to a different year (the current input data are calibrated to 1997-2003 FAO averages)
// the yield data are actually just for diagnostics; production and harvest area are used to recalibrate yield
// the annual values are stored for weighted averaging
float *yield_fao;           // yield from the FAO file (metric tonnes / km^2 )
float *harvestarea_fao;     // harvested area from the FAO file (km^2)
// these are needed to disaggregate the land rents across AEZs; production is also needed for recalibration
// production data: year varies fastest, then crop, then fao country
// crop varies fastest for price data, then ctry87; only a single value needs to be stored rather than the annual values
// the price data are aggregated to ctry87
float *production_fao;      // production from the FAO file (metric tonnes)
float *prodprice_fao_reglr;	// producer prices from the FAO file (USD / tonne), single year or an average

// GCAM, GTAP, SAGE, LULC, and FAO variable lists and codes
char **countrynames_fao;                                    // FAO country name
int *countrycodes_fao;                                      // FAO country codes
char **countryabbrs_iso;                                    // ISO country abbreviations; "-" = no iso3 code
char **country87names_gtap;                                 // GTAP 87 country names
char **country87abbrs_gtap;                                 // GTAP 87 country iso abbreviations (3 lower case letters)
int *country87codes_gtap;                                   // GTAP 87 country codes, assigned as enumeration of alphabetical abbr
char **countryabbrs_gcam_iso;                               // GCAM iso list for mapping to GCAM regions
char **regionnames_gcam;                                    // GCAM region names
int *regioncodes_gcam;                                      // GCAM region codes
char **usenames_gtap;                                       // GTAP use names (3 characters)
char **usedescr_gtap;                                       // GTAP use descriptions
int *usecodes_gtap;                                         // GTAP use codes
char **cropnames_gtap;                                      // GTAP crop names; used by gcam data system
char **cropdescr_sage;                                      // SAGE crop long name / descriptions
int *cropcodes_sage;                                        // SAGE crop codes
char **cropfilebase_sage;                                   // SAGE crop base file names / short name
char **landtypenames_sage;                                  // SAGE land type names
int *landtypecodes_sage;                                    // SAGE land type codes
char **aez_names_new;                                       // names of the new AEZs
int *aez_codes_new;                                         // integer id codes of the new AEZs; corresponds with the input raster
char **lutypenames_hyde;									// hyde land use type names
int *lutypecodes_hyde;										// hyde land use type integer codes
char **lulcnames;											// lulc type names
int *lulccodes;												// lulc type integer codes

// mapping between some variables
int *ctry2ctry87codes_gtap;                             // GTAP 87 country codes for each FAO country; -1 = no match
char **ctry2ctry87abbrs_gtap;                           // GTAP 87 country abbrs for each FAO country; "-" = no match
int *ctry2regioncodes_gcam;                             // GCAM region codes for each FAO country; -1 = no match
int *country_gcamiso2regioncodes_gcam;                  // GCAM region codes for each iso country; this is an input
int *crop_sage2gtap_use;                                // GTAP use codes for each SAGE crop
int *cropcodes_sage2fao;                                // FAO crop codes for each SAGE crop; -1 = no match
char **cropnames_sage2fao;                              // FAO crop names for each SAGE crop
int *lulc2sagecodes;									// isam lulc types to sage pot veg
int *lulc2hydecodes;									// isam lulc types to hyde32 lu types

// data structure to store information about the input rasters
typedef struct {
	// working grid cell area; calculated
	int cell_area_nrows;		// input number of rows
	int cell_area_ncols;		// input number of columns
	int cell_area_ncells;		// input number of grid cells
	float cell_area_nodata;		// input nodata value
	double cell_area_res;		// input resolution, decimal degrees
	double cell_area_xmin;		// input longitude min grid boundary
	double cell_area_xmax;		// input longitude max grid boundary
	double cell_area_ymin;		// input latitude min grid boundary
	double cell_area_ymax;		// input latitude max grid boundary

	// sage land area file
	int land_area_sage_nrows;		// input number of rows
	int land_area_sage_ncols;		// input number of columns
	int land_area_sage_ncells;		// input number of grid cells
	float land_area_sage_nodata;	// input nodata value
	int land_area_sage_insize;		// input size of data value in bytes
	double land_area_sage_res;		// input resolution, decimal degrees
	double land_area_sage_xmin;		// input longitude min grid boundary
	double land_area_sage_xmax;		// input longitude max grid boundary
	double land_area_sage_ymin;		// input latitude min grid boundary
	double land_area_sage_ymax;		// input latitude max grid boundary

	// hyde land grid cell area file
	int cell_area_hyde_nrows;		// input number of rows
	int cell_area_hyde_ncols;		// input number of columns
	int cell_area_hyde_ncells;		// input number of grid cells
	float cell_area_hyde_nodata;	// input nodata value
	double cell_area_hyde_res;		// input resolution, decimal degrees
	double cell_area_hyde_xmin;		// input longitude min grid boundary
	double cell_area_hyde_xmax;		// input longitude max grid boundary
	double cell_area_hyde_ymin;		// input latitude min grid boundary
	double cell_area_hyde_ymax;		// input latitude max grid boundary

	// hyde land area file
	int land_area_hyde_nrows;		// input number of rows
	int land_area_hyde_ncols;		// input number of columns
	int land_area_hyde_ncells;		// input number of grid cells
	float land_area_hyde_nodata;	// input nodata value
	double land_area_hyde_res;		// input resolution, decimal degrees
	double land_area_hyde_xmin;		// input longitude min grid boundary
	double land_area_hyde_xmax;		// input longitude max grid boundary
	double land_area_hyde_ymin;		// input latitude min grid boundary
	double land_area_hyde_ymax;		// input latitude max grid boundary

	// lulc input
	int lulc_input_nrows;		// input number of rows
	int lulc_input_ncols;		// input number of columns
	int lulc_input_ncells;		// input number of grid cells
	float lulc_input_nodata;	// input nodata value
	double lulc_input_res;		// input resolution, decimal degrees
	double lulc_input_xmin;		// input longitude min grid boundary
	double lulc_input_xmax;		// input longitude max grid boundary
	double lulc_input_ymin;		// input latitude min grid boundary
	double lulc_input_ymax;		// input latitude max grid boundary
	
	// new aez boundaries file
	int aez_new_nrows;			// input number of rows
	int aez_new_ncols;			// input number of columns
	int aez_new_ncells;			// input number of grid cells
	int aez_new_nodata;			// input nodata value
	int aez_new_insize;			// input size of data value in bytes
	double aez_new_res;			// input resolution, decimal degrees
	double aez_new_xmin;		// input longitude min grid boundary
	double aez_new_xmax;		// input longitude max grid boundary
	double aez_new_ymin;		// input latitude min grid boundary
	double aez_new_ymax;		// input latitude max grid boundary

	// orig aez boundaries file
	int aez_orig_nrows;			// input number of rows
	int aez_orig_ncols;			// input number of columns
	int aez_orig_ncells;		// input number of grid cells
	int aez_orig_nodata;		// input nodata value
	int aez_orig_insize;		// input size of data value in bytes
	double aez_orig_res;		// input resolution, decimal degrees
	double aez_orig_xmin;		// input longitude min grid boundary
	double aez_orig_xmax;		// input longitude max grid boundary
	double aez_orig_ymin;		// input latitude min grid boundary
	double aez_orig_ymax;		// input latitude max grid boundary

	// sage cropland area for normalizing sgae crop data
	int cropland_sage_nrows;			// input number of rows
	int cropland_sage_ncols;			// input number of columns
	int cropland_sage_ncells;		// input number of grid cells
	float cropland_sage_nodata;		// input nodata value
	double cropland_sage_res;		// input resolution, decimal degrees
	double cropland_sage_xmin;		// input longitude min grid boundary
	double cropland_sage_xmax;		// input longitude max grid boundary
	double cropland_sage_ymin;		// input latitude min grid boundary
	double cropland_sage_ymax;		// input latitude max grid boundary
	
	// lu area for ref veg calc and animal land rent calc
	// this includes cropland, pasture, urban and any other lu data with the same input parameters
	int lu_nrows;			// input number of rows
	int lu_ncols;			// input number of columns
	int lu_ncells;		// input number of grid cells
	int lu_nodata;		// input nodata value
	double lu_res;		// input resolution, decimal degrees
	double lu_xmin;		// input longitude min grid boundary
	double lu_xmax;		// input longitude max grid boundary
	double lu_ymin;		// input latitude min grid boundary
	double lu_ymax;		// input latitude max grid boundary

	// potveg file
	int potveg_nrows;			// input number of rows
	int potveg_ncols;			// input number of columns
	int potveg_ncells;			// input number of grid cells
	int potveg_nodata;			// input nodata value
	double potveg_res;			// input resolution, decimal degrees
	double potveg_xmin;			// input longitude min grid boundary
	double potveg_xmax;			// input longitude max grid boundary
	double potveg_ymin;			// input latitude min grid boundary
	double potveg_ymax;			// input latitude max grid boundary

	// fao country codes file (this is 2-byte integer file)
	int country_fao_nrows;			// input number of rows
	int country_fao_ncols;			// input number of columns
	int country_fao_ncells;			// input number of grid cells
	short country_fao_nodata;		// input nodata value
	int country_fao_insize;			// input size of data value in bytes
	double country_fao_res;			// input resolution, decimal degrees
	double country_fao_xmin;		// input longitude min grid boundary
	double country_fao_xmax;		// input longitude max grid boundary
	double country_fao_ymin;		// input latitude min grid boundary
	double country_fao_ymax;		// input latitude max grid boundary

    // protected pixel file (this is stored as a 2-byte integer file)
    int protected_nrows;			// input number of rows
    int protected_ncols;			// input number of columns
    int protected_ncells;			// input number of grid cells
    int protected_insize;			// input size of data value in bytes
    double protected_res;			// input resolution, decimal degrees
    double protected_xmin;		// input longitude min grid boundary
    double protected_xmax;		// input longitude max grid boundary
    double protected_ymin;		// input latitude min grid boundary
    double protected_ymax;		// input latitude max grid boundary
} rinfo_struct;

// data structure to store the information from the input control file
typedef struct {
	// flags
	int diagnostics;					// 1=output diagnostics; 0=do not output diagnostics

	// data years for recalibration
	int out_year_prod_ha_lr;			// output year for crop production, harvest area, and land rent
	int in_year_sage_crops;				// input year of the 175 crop harvest area and yield data
	int out_year_usd;					// the output US dollar value year for land rent
	int in_year_lr_usd;					// the US dollar value year for the input land rent data
	
	// year to write set of output land use/cover rasters
	int lulc_out_year;					// if this year falls outside of processing years then no output
	
	// useful paths
	char inpath[MAXCHAR];				// path to the input data directory
	char outpath[MAXCHAR];				// path to the output data directory
	char sagepath[MAXCHAR];				// path to the directory containing the SAGE netcdf crop files
	char hydepath[MAXCHAR];				// path to the directory containing the HYDE files
	char lulcpath[MAXCHAR];				// path to the directory containing the LULC files
	char mircapath[MAXCHAR];			// path to the directory containing the MIRCA ascii grid files
    char wfpath[MAXCHAR];               // path to the directory containing the water footprint esri grid files
    char ldsdestpath[MAXCHAR];              // destination path for the gcam data system input files
    char mapdestpath[MAXCHAR];              // destination path for the gcam data system mapping files

	// input raster file names
	char cell_area_fname[MAXCHAR];			// file name only of the working grid cell area raster file
	char land_area_sage_fname[MAXCHAR];		// file name only of the sage working land fraction raster file
	char land_area_hyde_fname[MAXCHAR];		// file name only of the hyde land area raster file
	char aez_new_fname[MAXCHAR];			// file name only of the new aez boundaries raster file
	char aez_orig_fname[MAXCHAR];			// file name only of the original aez boundaries raster file
	char potveg_fname[MAXCHAR];				// file name only of the potential vegetation raster file
	char country_fao_fname[MAXCHAR];		// file name only of the fao country code raster file
    //kbn 2020-02-29 Introducing file names for new EPA suitability and protected area rasters
	char L1_fname[MAXCHAR];                //L1 suitability raster
	char L2_fname[MAXCHAR];                //L2 suitability raster
	char L3_fname[MAXCHAR];                //L3 suitability raster
	char L4_fname[MAXCHAR];                //L4 suitability raster 
	char ALL_IUCN_fname[MAXCHAR];          //IUCN All protected area raster
	char IUCN_1a_1b_2_fname[MAXCHAR];      //IUCN 1a_1b_2 protected area raster
	char nfert_rast_fname[MAXCHAR];         // file name only of the nfert raster file
	char cropland_sage_fname[MAXCHAR];		// file name only of the sage cropland file
	//kbn 2020-06-01 Introducing file names for soil carbon
	char soil_carbon_wavg_fname[MAXCHAR];   //Soil carbon weighted average raster
	char soil_carbon_median_fname[MAXCHAR]; //Soil carbon median raster
	char soil_carbon_min_fname[MAXCHAR];    //Soil carbon minimum raster
	char soil_carbon_max_fname[MAXCHAR];    //Soil carbon maximum raster
	char soil_carbon_q1_fname[MAXCHAR];     //Soil carbon q1 raster
	char soil_carbon_q3_fname[MAXCHAR];     //Soil carbon q3 raster
	//kbn 2020-06-30 Introducing file names for veg carbon
	char veg_carbon_wavg_fname[MAXCHAR];    //Above ground vegetation carbon weighted average raster
	char veg_carbon_median_fname[MAXCHAR];  //Above ground vegetation carbon median raster
	char veg_carbon_min_fname[MAXCHAR];     //Above ground vegetation carbon minimum raster
	char veg_carbon_max_fname[MAXCHAR];     //Above ground vegetation carbon maximum raster
	char veg_carbon_q1_fname[MAXCHAR];      //Above ground vegetation carbon q1 raster
	char veg_carbon_q3_fname[MAXCHAR];      //Above ground vegetation carbon q3 raster 
    // 2020-08-08 Introducing file names for below ground biomass
	char veg_BG_wavg_fname[MAXCHAR];        //Below ground vegetation carbon weighted averge raster
	char veg_BG_median_fname[MAXCHAR];      //Below ground vegetation carbon median raster
	char veg_BG_min_fname[MAXCHAR];         //Below ground vegetation carbon minimum raster 
	char veg_BG_max_fname[MAXCHAR];         //Below ground vegetation carbon maximum raster
	char veg_BG_q1_fname[MAXCHAR];          //Below ground vegetation carbon q1 raster
	char veg_BG_q3_fname[MAXCHAR];          //Below ground vegetation carbon q3 raster
 
 
	//2022-08-23 Introducing file names for managed carbon rasters
	//crop
	char soil_carbon_crop_wavg_fname[MAXCHAR];   //Soil carbon weighted average raster
	char soil_carbon_crop_median_fname[MAXCHAR]; //Soil carbon median raster
	char soil_carbon_crop_min_fname[MAXCHAR];    //Soil carbon minimum raster
	char soil_carbon_crop_max_fname[MAXCHAR];    //Soil carbon maximum raster
	char soil_carbon_crop_q1_fname[MAXCHAR];     //Soil carbon q1 raster
	char soil_carbon_crop_q3_fname[MAXCHAR];     //Soil carbon q3 raster
	char veg_carbon_crop_wavg_fname[MAXCHAR];    //Above ground vegetation carbon weighted average raster
	char veg_carbon_crop_median_fname[MAXCHAR];  //Above ground vegetation carbon median raster
	char veg_carbon_crop_min_fname[MAXCHAR];     //Above ground vegetation carbon minimum raster
	char veg_carbon_crop_max_fname[MAXCHAR];     //Above ground vegetation carbon maximum raster
	char veg_carbon_crop_q1_fname[MAXCHAR];      //Above ground vegetation carbon q1 raster
	char veg_carbon_crop_q3_fname[MAXCHAR];      //Above ground vegetation carbon q3 raster 
	char veg_BG_crop_wavg_fname[MAXCHAR];        //Below ground vegetation carbon weighted averge raster
	char veg_BG_crop_median_fname[MAXCHAR];      //Below ground vegetation carbon median raster
	char veg_BG_crop_min_fname[MAXCHAR];         //Below ground vegetation carbon minimum raster 
	char veg_BG_crop_max_fname[MAXCHAR];         //Below ground vegetation carbon maximum raster
	char veg_BG_crop_q1_fname[MAXCHAR];          //Below ground vegetation carbon q1 raster
	char veg_BG_crop_q3_fname[MAXCHAR];          //Below ground vegetation carbon q3 raster
	//pasture
	char soil_carbon_pasture_wavg_fname[MAXCHAR];   //Soil carbon weighted average raster
	char soil_carbon_pasture_median_fname[MAXCHAR]; //Soil carbon median raster
	char soil_carbon_pasture_min_fname[MAXCHAR];    //Soil carbon minimum raster
	char soil_carbon_pasture_max_fname[MAXCHAR];    //Soil carbon maximum raster
	char soil_carbon_pasture_q1_fname[MAXCHAR];     //Soil carbon q1 raster
	char soil_carbon_pasture_q3_fname[MAXCHAR];     //Soil carbon q3 raster
	char veg_carbon_pasture_wavg_fname[MAXCHAR];    //Above ground vegetation carbon weighted average raster
	char veg_carbon_pasture_median_fname[MAXCHAR];  //Above ground vegetation carbon median raster
	char veg_carbon_pasture_min_fname[MAXCHAR];     //Above ground vegetation carbon minimum raster
	char veg_carbon_pasture_max_fname[MAXCHAR];     //Above ground vegetation carbon maximum raster
	char veg_carbon_pasture_q1_fname[MAXCHAR];      //Above ground vegetation carbon q1 raster
	char veg_carbon_pasture_q3_fname[MAXCHAR];      //Above ground vegetation carbon q3 raster 
	char veg_BG_pasture_wavg_fname[MAXCHAR];        //Below ground vegetation carbon weighted averge raster
	char veg_BG_pasture_median_fname[MAXCHAR];      //Below ground vegetation carbon median raster
	char veg_BG_pasture_min_fname[MAXCHAR];         //Below ground vegetation carbon minimum raster 
	char veg_BG_pasture_max_fname[MAXCHAR];         //Below ground vegetation carbon maximum raster
	char veg_BG_pasture_q1_fname[MAXCHAR];          //Below ground vegetation carbon q1 raster
	char veg_BG_pasture_q3_fname[MAXCHAR];          //Below ground vegetation carbon q3 raster
	//urban
	char soil_carbon_urban_wavg_fname[MAXCHAR];   //Soil carbon weighted average raster
	char soil_carbon_urban_median_fname[MAXCHAR]; //Soil carbon median raster
	char soil_carbon_urban_min_fname[MAXCHAR];    //Soil carbon minimum raster
	char soil_carbon_urban_max_fname[MAXCHAR];    //Soil carbon maximum raster
	char soil_carbon_urban_q1_fname[MAXCHAR];     //Soil carbon q1 raster
	char soil_carbon_urban_q3_fname[MAXCHAR];     //Soil carbon q3 raster
	char veg_carbon_urban_wavg_fname[MAXCHAR];    //Above ground vegetation carbon weighted average raster
	char veg_carbon_urban_median_fname[MAXCHAR];  //Above ground vegetation carbon median raster
	char veg_carbon_urban_min_fname[MAXCHAR];     //Above ground vegetation carbon minimum raster
	char veg_carbon_urban_max_fname[MAXCHAR];     //Above ground vegetation carbon maximum raster
	char veg_carbon_urban_q1_fname[MAXCHAR];      //Above ground vegetation carbon q1 raster
	char veg_carbon_urban_q3_fname[MAXCHAR];      //Above ground vegetation carbon q3 raster 
	char veg_BG_urban_wavg_fname[MAXCHAR];        //Below ground vegetation carbon weighted averge raster
	char veg_BG_urban_median_fname[MAXCHAR];      //Below ground vegetation carbon median raster
	char veg_BG_urban_min_fname[MAXCHAR];         //Below ground vegetation carbon minimum raster 
	char veg_BG_urban_max_fname[MAXCHAR];         //Below ground vegetation carbon maximum raster
	char veg_BG_urban_q1_fname[MAXCHAR];          //Below ground vegetation carbon q1 raster
	char veg_BG_urban_q3_fname[MAXCHAR];          //Below ground vegetation carbon q3 raster
	
	// input csv file names
	char rent_orig_fname[MAXCHAR];			// file name only of the orginal GTAP land rent csv file
	char country87_gtap_fname[MAXCHAR];		// file name only of the GTAP/GCAM ctry87 list
	char country87map_fao_fname[MAXCHAR];	// file name only of the FAO ctry to ctry87 mapping
	char country_all_fname[MAXCHAR];		// file name only of the FAO/VMAP0, iso country mapping
    char aez_new_info_fname[MAXCHAR];		// file name only of the aez new code/name list
    char countrymap_iso_gcam_region_fname[MAXCHAR];	// file name only of iso country to gcam region mapping
    char regionlist_gcam_fname[MAXCHAR];	// file name only of the GCAM region list
    char use_gtap_fname[MAXCHAR];			// file name only of the GTAP product use categories
	char lt_sage_fname[MAXCHAR];			// file name only of the SAGE land types, in order
	char lu_hyde_fname[MAXCHAR];			// file name only of the HDYE land use types, in order
	char lulc_fname[MAXCHAR];				// file name only of the LULC land types, in order
	char crop_fname[MAXCHAR];				// file name only of the FAO crop, SAGE crop, GTAP use mapping
	char production_fao_fname[MAXCHAR];		// file name only of FAO production data
	char yield_fao_fname[MAXCHAR];			// file name only of FAO yield data
	char harvestarea_fao_fname[MAXCHAR];	// file name only of FAO harvested area data
	char prodprice_fao_fname[MAXCHAR];		// file name only of FAO producer price data
	char convert_usd_fname[MAXCHAR];		// file name only of the usd conversion factors

	// output file names (without path)
	char lds_logname[MAXCHAR];              // log file name for runtime output
	char harvestarea_fname[MAXCHAR];		// file name for harvested area output
	char production_fname[MAXCHAR];			// file name for produciton output
	char rent_fname[MAXCHAR];				// file name for land rent output
    char mirca_irr_fname[MAXCHAR];			// file name for mirca irrigated crop area output
    char mirca_rfd_fname[MAXCHAR];			// file name for mirca rainfed crop area output
    char land_type_area_fname[MAXCHAR];     // file name for land type area output
    char refveg_carbon_fname[MAXCHAR];      // file name for reference veg carbon output
    char wf_fname[MAXCHAR];                 // file name for water footprint output
    char iso_map_fname[MAXCHAR];            // file name for mapping the raaster fao country codes to iso
    char lt_map_fname[MAXCHAR];             // file name for mapping the land type category codes to descriptions
} args_struct;

// function declarations

// read raster file functions
int get_cell_area(args_struct in_args, rinfo_struct *raster_info);
int read_land_area_sage(args_struct in_args, rinfo_struct *raster_info);
int read_land_area_hyde(args_struct in_args, rinfo_struct *raster_info);
int read_aez_new(args_struct in_args, rinfo_struct *raster_info);
int read_aez_orig(args_struct in_args, rinfo_struct *raster_info);
int read_cropland_sage(args_struct in_args, rinfo_struct *raster_info);
int read_cropland(args_struct in_args, rinfo_struct *raster_info);
int read_pasture(args_struct in_args, rinfo_struct *raster_info);
int read_potveg(args_struct in_args, rinfo_struct *raster_info);
int read_urban(args_struct in_args, rinfo_struct *raster_info);
int read_country_fao(args_struct in_args, rinfo_struct *raster_info);
int read_country_gcam(args_struct in_args, rinfo_struct *raster_info);
int read_region_gcam(args_struct in_args, rinfo_struct *raster_info);
int read_sage_crop(char *fname, char *sagepath, char *cropfilebase_sage, rinfo_struct raster_info);
int read_mirca(char *fname, float *mirca_grid);
int read_protected(args_struct in_args, rinfo_struct *raster_info);
int read_lu_hyde(args_struct in_args, int year, float *crop_grid, float *pasture_grid, float *urban_grid);
int read_lulc_isam(args_struct in_args, int year, float **lulc_input_grid);
int read_lulc_land(args_struct in_args, int year, rinfo_struct *raster_info, int *land_mask_lulc);
int read_hyde32(args_struct in_args, rinfo_struct *raster_info, int year, float* crop_grid, float* pasture_grid, float* urban_grid, float** lu_detail);
//kbn 2020-06-01 Changing soil carbon function below
int read_soil_carbon(args_struct in_args, rinfo_struct *raster_info);
//kbn 2020-06-01 Changing veg carbon function below
int read_veg_carbon(args_struct in_args, rinfo_struct *raster_info);
// read csv file functions
int read_rent_orig(args_struct in_args);
int read_country87_info(args_struct in_args);
int read_region_info_gcam(args_struct in_args);
int read_country_info_all(args_struct in_args);
int read_aez_new_info(args_struct in_args);
int read_use_info_gtap(args_struct in_args);
int read_lulc_info(args_struct in_args);
int read_crop_info(args_struct in_args);
int read_production_fao(args_struct in_args);
int read_yield_fao(args_struct in_args);
int read_harvestarea_fao(args_struct in_args);
int read_prodprice_fao(args_struct in_args);
int read_water_footprint(char *fname, float *wf_grid);


// raster processing functions
int get_land_cells(args_struct in_args, rinfo_struct raster_info);
int calc_refveg_area(args_struct in_args, rinfo_struct *raster_info);
int calc_refcarbon_area(args_struct in_args, rinfo_struct raster_info);
int get_aez_val(int aez_array[], int index, int nrows, int ncols, int nodata_val, int *value);
int proc_water_footprint(args_struct in_args, rinfo_struct raster_info);

// additional spatial data processing functions
int proc_mirca(args_struct in_args, rinfo_struct raster_info);
int proc_lulc_area(args_struct in_args, rinfo_struct raster_info, double *lulc_area, int *lu_indices, double **lu_area, double *refveg_area_out, int *refveg_them, int num_lu_cells, int lulc_index);
int proc_land_type_area(args_struct in_args, rinfo_struct raster_info);
int proc_refveg_carbon(args_struct in_args, rinfo_struct raster_info);
int proc_refveg_crop_carbon(args_struct in_args, rinfo_struct raster_info);
int proc_refveg_pasture_carbon(args_struct in_args, rinfo_struct raster_info);
int proc_refveg_urban_carbon(args_struct in_args, rinfo_struct raster_info);



// text parsing utility functions (parse_utils.c)
int get_float_field(char *line, const char *delim, int findex, float *fltval);
int get_int_field(char *line, const char *delim, int findex, int *intval);
int get_text_field(char *line, const char *delim, int findex, char *str_field);
int get_field(char *line, const char *delim, int findex, char *str_field);
int rm_whitesp(char *cln_field,char *str_field);
int rm_quotes(char *cln_field,char *str_field);
int is_num(char *str_field);

// calculation functions
int calc_harvarea_prod_out_crop_aez(args_struct in_args, rinfo_struct raster_info);
int aggregate_crop2gcam(args_struct in_args);
int calc_rent_ag_use_aez(args_struct in_args, rinfo_struct raster_info);
int calc_rent_frs_use_aez(args_struct in_args, rinfo_struct raster_info);
int aggregate_use2gcam(args_struct in_args);

// output write functions
int write_harvestarea_crop_aez(args_struct in_args);
int write_production_crop_aez(args_struct in_args);
int write_rent_use_aez(args_struct in_args);
int write_glu_mapping(args_struct in_args, rinfo_struct raster_info);

// diagnostic write functions
int write_raster_float(float out_array[], int out_length, char *out_name, args_struct in_args);
int write_raster_int(int out_array[], int out_length, char *out_name, args_struct in_args);
int write_raster_short(short out_array[], int out_length, char *out_name, args_struct in_args);
int write_text_int(int out_array[], int out_length, char *out_name, args_struct in_args);
int write_text_char(char **out_array, int out_length, char *out_name, args_struct in_args);
int write_csv_float3d(float out_array[], int d1[], int d2[], int d1_length, int d2_length, int d3_length,
					  char *out_name, args_struct in_args);
int write_csv_float2d(float out_array[], int d1[], int d1_length, int d2_length, char *out_name, args_struct in_args);

// utility functions
char *get_systime();
int init_moirai(args_struct *in_args);
int get_in_args(const char *fname, args_struct *in_args);
int copy_to_destpath(args_struct in_args);
// sorting function  that is used with qsort in proc_refveg_carbon.c
int cmpfunc (const void * a, const void * b);

#endif

/**********
 lds.h

 Created by Alan Di Vittorio on 3 May 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved

 September 2015 - renamed LDS (land data system) by Alan Di Vittorio
    This is to be the base code for developing a system that works with an arbitrary number of AEZs
    An AEZ (agro-ecological zone) can be defined arbitrarily
        The AEZ nomenclature is retained both for practical (legacy) reasons
        And because AEZ refers to a land unit that is to be treated as a whole with respect to:
            agriculture and vegetation and land value
    Each AEZ is now a unique area on the globe
    This will ultimately be LDS version 2.0

**********/

#ifndef LDSHDR
#define LDSHDR

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "/usr/local/include/netcdf.h"

#define CODENAME				"lds"				// name of the compiled program
#define VERSION                 "2.beta4"           // current version
#define MAXCHAR					1000						// maximum string length
#define MAXRECSIZE				10000						// maximum record (csv line) length in characters

// year of HYDE data to read in for calculating potential vegetation area (for carbon and forest land rent) and pasture animal land rent
#define HYDE_YEAR               2000

// the min and max forest codes for the SAGE raster data (see SAGE_PVLT.csv)
// these assume that the forest codes are all consecutive with no others mixed in the enumeration
#define MAX_SAGE_FOREST_CODE    8
#define MIN_SAGE_FOREST_CODE    1

// counts of useful variables
#define NUM_ORIG_AEZ			18							// number of original GTAP/GCAM AEZs
#define NUM_FAO_YRS				11							// number of years in FAOSTAT production, yield, havested area, and price files
#define NUM_IN_ARGS				49							// number of input variables in the input file

// useful values for processing the additional spatial data
#define NUM_MIRCA_CROPS         26              // number of crops in the mirca2000 data set
#define PROTECTED               1               // value assigned to protected pixels for generating land category (this value - 1 is the output array index)
#define UNPROTECTED             2               // value assigned to unprotected pixels for generating land category (this value - 1 is the output array index)
#define NUM_PROTECTED           2               // number of protection categories
#define NUM_HYDE_YEARS          33              // number of years in the hyde lu bsq files
#define HYDE_START_YEAR         1700            // the first year in the hyde bsq files
#define CROP_LT_CODE            10              // used to generate land type category
#define PASTURE_LT_CODE         20              // used to generate land type category
#define URBAN_LT_CODE           30              // used to generate land type category
#define SCALE_POTVEG            100             // used to generate land type category
#define NUM_LU_CATS             4               // crop, pasture, urban, potential veg (pv code = 0)
#define NUM_WF_CROPS            18              // number of water footprint crops
#define NUM_WF_TYPES            4               // number of water footprint types

// conversion factors for output
#define KMSQ2HA					100.0						// km^2 * KMSQ2HA = ha
#define MSQ2KMSQ				1/1000000.0					// m^2 * MSQ2KMSQ = km^2
#define KGMSQ2MGHA              10.0                        // kg/m^2 * KGMSQ2MGHA = Mg/ha
#define USD2MILUSD				1/1000000.0					// USD * USD2MILUSD = million USD

// conversion factors for inputs
#define HGHA2TKMSQ				0.01			// conversion factor from hg / ha to t / km^2; FAOSTAT yield file
#define HA2KMSQ					0.01			// conversion factor from ha to km^2; FAOSTAT harvest area file and sage crop file
#define MIL2ONE					1000000.0			// factor to convert million USD to USD; GTAP land rent file

// useful flags and values
#define NOMATCH					-1				// if there isn't a matching country across data sets
#define NA_TEXT                  "-"            // if there is no iso3 or name for a country/territory
#define FAOCTRY2GCAMCTRYAEZID   10000           // the gcam country+aez id is fao country id * 10000 + aez id

// working resolution is 5 arcmin (2160x4320), WGS84 spherical earth, lat-lon projection
#define NUM_LAT					2160						// number of lats in working grids
#define NUM_LON					4320						// number of lons in working grids
#define NUM_CELLS				NUM_LAT * NUM_LON			// number of grid cells in working grids
#define GRID_RES				5.0/60.0					// working grid resolution; decimal degree
#define GRID_RES_SEC			300.0						// workding grid resolution; arc-seconds
#define NODATA					-9999						// nodata value

// some constants for calculating the area of a grid cell
#define AVE_ER					6371007.181		// average earth radius; from MODIS land products WGS84 average spherical radius;  meters
#define PI						(4.0*atan(1.0))
#define DEG2RAD					PI/180.0
#define SEC2DEG					1/3600.0
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

// useful utility variables
char systime[MAXCHAR];					// array to store current time
FILE *fplog;							// file pointer to log file for runtime output

// area and production arrays: now 3d arrays, dim1=country[NUM_FAO_CTRY], dim2=aez[ctry_aez_num], dim3=crop[NUM_SAGE_CROP]
//  so the second dimension is variable, and it matches the aez list arrays below
float ***harvestarea_crop_aez;            // harvested area output (ha), output to nearest integer
float ***production_crop_aez;             // production output (metric tonnes), output to nearest integer
// associated to output data array: dim1=country[NUM_FAO_CTRY], dim2=aez[ctry_aez_num]
//  so the second dimension is variable, and it matches the aez list arrays below
float **pasturearea_aez;                 // pasture area (ha)
// land rent output: dim1=land rent region[NUM_GTAP_CTRY87], dim2=aez[reglr_aez_num], dim3=use[NUM_GTAP_USE]
float ***rent_use_aez;                    // land rent output (million USD), output a total on 10 digits

// area and production output data arrays; aez varies fastest, then crop, then fao ctry
//float *harvestarea_crop_aez;            // harvested area output (ha), output to nearest integer
//float *production_crop_aez;             // production output (metric tonnes), output to nearest integer
// associated to output data array - aez varies fastest, then fao ctry
//float *pasturearea_aez;                 // pasture area (ha)
// land rent output array
// aez varies fastest, then use, then ctry87
//float *rent_use_aez;                    // land rent output (million USD), output a total on 10 digits

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

// aggregated output data arrays; aez varies fastest, then crop/use, then gcam region
// these two only in aggregate crop to gcam
//float harvestarea_crop_aez_gcam[NUM_GCAM_RGN * NUM_SAGE_CROP * NUM_NEW_AEZ];	// harvested area output (ha)
//float production_crop_aez_gcam[NUM_GCAM_RGN * NUM_SAGE_CROP * NUM_NEW_AEZ];	// production output (metric tonnes)
// only in aggregate use to gcam
//float rent_use_aez_gcam[NUM_GCAM_RGN * NUM_GTAP_USE * NUM_NEW_AEZ];			// land rent output (million USD)

// raster data as 1-d arrays; numlat * numlon, start at upper left corner, lon varies fastest [NUM_LAT X NUM_LON]
// these are allocated and free dynamically as needed in lds_main.c
// they are all 1d arrays of size NUM_CELLS, which is currently hardcoded for the 5 arcmin resolution
float *harvestarea_in;                  // input harvest area (km^2), reused by each individual crop
float *yield_in;                        // input yield (metric tonnes / km^2), reused by each individual crop
int *aez_bounds_new;                    // new aez boundaries (integers 1 to NUM_NEW_AEZ)
int *aez_bounds_orig;                   // original aez boundaries (integers 1 to NUM_ORIG_AEZ)
float *cropland_area;                   // cropland area for pot veg area calc for forest land rent (km^2)
float *pasture_area;                    // pasture area for pot veg area calc and animal land rent calc (km^2)
float *urban_area;                      // urban area for pot veg area calc for forest land rent calc (km^2)
float *potveg_area;                     // potential vegetation area for forest land rent calc (km^2)
int *potveg_thematic;                   // potential vegetation thematic data (integers 1 to NUM_SAGE_PVLT)
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
int *land_mask_fao;                     // 1=land; 0=no land
int *land_mask_potveg;                  // 1=land; 0=no land
int *land_mask_forest;                  // 1=forest; 0=no forest
short *protected_thematic;                // 1=protected; 2=unprotected (after conversion from file value of 255); no other values

// indices of land cells within the raster data; these are the only cells processed
// these are allocated and free dynamically as needed in lds_main.c
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
float *rent_orig_aez;		// original land rent (million USD, yr2001$)

// FAOSTAT data; year varies fastest, then crop, then fao country
// the order of country is the FAO order, the order of crop is the GTAP order, and the yeas are chronological
// these are only needed if recalibrating the SAGE data to a different year (the current input data are calibrated to 1997-2003 FAO averages)
// the annual values are stored for weighted averaging
float *yield_fao;           // yield from the FAO file (metric tonnes / km^2 )
float *harvestarea_fao;     // harvested area from the FAO file (km^2)
// these are needed to disaggregate the land rents across AEZs; production is also needed for recalibration
// production data: year varies fastest, then crop, then fao country
// crop varies fastest for price data, then ctry87; only a single value needs to be stored rather than the annual values
// the price data are aggregated to ctry87
float *production_fao;      // production from the FAO file (metric tonnes)
float *prodprice_fao_reglr;	// producer prices from the FAO file (USD / tonne), single year or an average

// GCAM, GTAP, SAGE, and FAO variable lists and codes
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

// mapping between some variables
int *ctry2ctry87codes_gtap;                             // GTAP 87 country codes for each FAO country; -1 = no match
char **ctry2ctry87abbrs_gtap;                           // GTAP 87 country abbrs for each FAO country; "-" = no match
int *ctry2regioncodes_gcam;                             // GCAM region codes for each FAO country; -1 = no match
int *country_gcamiso2regioncodes_gcam;                  // GCAM region codes for each iso country; this is an input
int *crop_sage2gtap_use;                                // GTAP use codes for each SAGE crop
int *cropcodes_sage2fao;                                // FAO crop codes for each SAGE crop; -1 = no match
char **cropnames_sage2fao;                              // FAO crop names for each SAGE crop

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

	// cropland area for pot veg calc
	int cropland_nrows;			// input number of rows
	int cropland_ncols;			// input number of columns
	int cropland_ncells;		// input number of grid cells
	float cropland_nodata;		// input nodata value
	double cropland_res;		// input resolution, decimal degrees
	double cropland_xmin;		// input longitude min grid boundary
	double cropland_xmax;		// input longitude max grid boundary
	double cropland_ymin;		// input latitude min grid boundary
	double cropland_ymax;		// input latitude max grid boundary

	// pasture area for potveg calc and animal land rent calc
	int pasture_nrows;			// input number of rows
	int pasture_ncols;			// input number of columns
	int pasture_ncells;			// input number of grid cells
	float pasture_nodata;		// input nodata value
	double pasture_res;			// input resolution, decimal degrees
	double pasture_xmin;		// input longitude min grid boundary
	double pasture_xmax;		// input longitude max grid boundary
	double pasture_ymin;		// input latitude min grid boundary
	double pasture_ymax;		// input latitude max grid boundary

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

	// urban area for pot veg calc
	int urban_nrows;		// input number of rows
	int urban_ncols;		// input number of columns
	int urban_ncells;		// input number of grid cells
	float urban_nodata;     // input nodata value
	double urban_res;		// input resolution, decimal degrees
	double urban_xmin;		// input longitude min grid boundary
	double urban_xmax;		// input longitude max grid boundary
	double urban_ymin;		// input latitude min grid boundary
	double urban_ymax;		// input latitude max grid boundary

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
	int recalibrate;					// 1=recalibrate to a different reference year; 0=no recalibration
	int diagnostics;					// 1=output diagnostics; 0=do not output diagnostics

	// useful paths
	char inpath[MAXCHAR];				// path to the input data directory
	char outpath[MAXCHAR];				// path to the output data directory
	char sagepath[MAXCHAR];				// path to the directory containing the SAGE netcdf crop files
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
    char protected_fname[MAXCHAR];          // file name only of the protected pixel raster file
    char nfert_rast_fname[MAXCHAR];         // file name only of the nfert raster file
    char hist_crop_rast_name[MAXCHAR];      // file name only of the historical crop file
    char hist_pasture_rast_name[MAXCHAR];   // file name only of the historical pasture file
    char hist_urban_rast_name[MAXCHAR];     // file name only of the historical urban file

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
	char crop_fname[MAXCHAR];				// file name only of the FAO crop, SAGE crop, GTAP use mapping
	char production_fao_fname[MAXCHAR];		// file name only of FAO production data
	char yield_fao_fname[MAXCHAR];			// file name only of FAO yield data
	char harvestarea_fao_fname[MAXCHAR];	// file name only of FAO harvested area data
	char prodprice_fao_fname[MAXCHAR];		// file name only of FAO producer price data
	char convert_usd_fname[MAXCHAR];		// file name only of the usd conversion factors
    char vegc_csv_fname[MAXCHAR];          // file name only of the veg c to pot veg file
    char soilc_csv_fname[MAXCHAR];         // file name only of the soil c file

	// output file names (without path)
	char lds_logname[MAXCHAR];              // log file name for runtime output
	char harvestarea_fname[MAXCHAR];		// file name for harvested area output
	char production_fname[MAXCHAR];			// file name for produciton output
	char rent_fname[MAXCHAR];				// file name for land rent output
    char mirca_irr_fname[MAXCHAR];			// file name for mirca irrigated crop area output
    char mirca_rfd_fname[MAXCHAR];			// file name for mirca rainfed crop area output
    char nfert_fname[MAXCHAR];              // file name for nfert output
    char land_type_area_fname[MAXCHAR];     // file name for land type area output
    char potveg_carbon_fname[MAXCHAR];      // file name for pot veg carbon output
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
int read_cropland(args_struct in_args, rinfo_struct *raster_info);
int read_pasture(args_struct in_args, rinfo_struct *raster_info);
int read_potveg(args_struct in_args, rinfo_struct *raster_info);
int read_urban(args_struct in_args, rinfo_struct *raster_info);
int read_country_fao(args_struct in_args, rinfo_struct *raster_info);
int read_country_gcam(args_struct in_args, rinfo_struct *raster_info);
int read_region_gcam(args_struct in_args, rinfo_struct *raster_info);
int read_sage_crop(char *fname, rinfo_struct raster_info);
int read_mirca(char *fname, float *mirca_grid);
int read_nfert(char *fname, float *nfert_grid, args_struct in_args);
int read_protected(args_struct in_args, rinfo_struct *raster_info);
int read_lu_hyde(args_struct in_args, int year, float *crop_grid, float *pasture_grid, float *urban_grid);

int read_soil_carbon(char *fname, float *soil_carbon_sage, args_struct in_args);

// read csv file functions
int read_rent_orig(args_struct in_args);
int read_country87_info(args_struct in_args);
int read_region_info_gcam(args_struct in_args);
int read_country_info_all(args_struct in_args);
int read_aez_new_info(args_struct in_args);
int read_use_info_gtap(args_struct in_args);
int read_lt_info_sage(args_struct in_args);
int read_crop_info(args_struct in_args);
int read_production_fao(args_struct in_args);
int read_yield_fao(args_struct in_args);
int read_harvestarea_fao(args_struct in_args);
int read_prodprice_fao(args_struct in_args);
int read_veg_carbon(char *fname, float *veg_carbon_sage);
int read_water_footprint(char *fname, float *wf_grid);

// raster processing functions
int get_land_cells(args_struct in_args, rinfo_struct raster_info);
int calc_potveg_area(args_struct in_args, rinfo_struct *raster_info);
int get_aez_val(int aez_array[], int index, int nrows, int ncols, int nodata_val, int *value);
int proc_water_footprint(args_struct in_args, rinfo_struct raster_info);

// additional spatial data processing functions
int proc_mirca(args_struct in_args, rinfo_struct raster_info);
int proc_nfert(args_struct in_args, rinfo_struct raster_info);
int proc_land_type_area(args_struct in_args, rinfo_struct raster_info);
int proc_potveg_carbon(args_struct in_args, rinfo_struct raster_info);

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
int write_gcam_lut(args_struct in_args, rinfo_struct raster_info);

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
int init_lds(args_struct *in_args);
int get_in_args(const char *fname, args_struct *in_args);
int copy_to_destpath(args_struct in_args);

#endif

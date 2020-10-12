/**********
 moirai_main.c
 
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
 
 Modified in Fall 2015 by Alan Di Vittorio to accommodate GCAM 235 water basins as GLUs
 Now an arbitrary set of GLUs can be input
 
 Modified from Fall 2017 to Summer 2018 by Alan Di Vittorio to update the GLU data and use new historical land use/cover data
 	Chris Vernon provided new 235 water basin data and set up the git repository
 	Shijie Shu and Atul Jain provided a new land use/cover historical data series based on HYDE3.2
 	A. Di Vittorio changed the name to Moirai in May/June 2018
 
 This version will interface with the refactored gcam data system
 The previous version was based on the gcam-data-system, as downloaded on 23 sep 2015
 
 The  land units for aggregation are now called Geographic Land Units (GLUs)
 Each new GLU is a unique area on the globe - this is different from the original climate AEZs
    All outputs have been updated to glu terminology, as appropriate (original aez diags still have aez)
    Some comments have been updated to glu terminology, but not all
    No code variables have been updated to glu terminology
 
 This Moirai Land Data System (Moirai LDS) is designed to produce inputs to the GCAM data system
  but the Moirai LDS outputs could also be use by other models/applications
 
 The gcam region distinction in LDS (14 vs 32) is only for diagnostic outputs
    such as raster files and aggregations to gcam regions
    so LDS does read in two GCAM data system mapping files that have region info
        and one is needed to constrain the iso-glu mapping file output to GCAM
            this may not be necessary, though
    the output that feed into the gcam data system should be identical regardless of the gcam regions used
    but some diagnostics scripts expect the correct LDS gcam region codes diagnostic file
 
 The Moirai LDS takes a given set of GLU boundaries and generates the following three files:
	gcam-data-system/aglu-data/level0/
		MOIRAI_ag_HA_ha.csv to replace GTAP_ag_HA_ha.csv (hectares)
		MOIRAI_ag_Prod_t.csv to replace GTAP_ag_Prod_t.csv (metric tonnes)
		MOIRAI_value_milUSD.csv to replace GTAP_value_milUSD.csv (million USD)
	Note that these data are based on the 1997-2003 annual average
 
 The Moirai LDS also writes these files that were previously produced from the old 'GIS' code
    gcam-data-system/aglu-data/moirai/
        MIRCA_irrHA_ha.csv
        MIRCA_rfdHA_ha.csv
        Land_type_area_ha.csv  - this replaces the Sage_Hyde15_Area.csv file
        Pot_veg_carbon_Mg_per_ha.csv - this replaces the level0 carbon files
        Water_footprint_m3.csv - this is the water footprint data
 
 It also writes these ancillary files (names can be changed in the LDS input file):
	gcam-data-system/aglu-data/mappings/
		MOIRAI_ctry_GLU.csv (depends on glu numbers in countries; need the iso mapping)
			(currently use the _common/mappings/iso_GACM_regID files for the iso)
            so this file and the LDS processing is currently contrained by this GCAM iso list
        MOIRAI_land_types.csv mapping of land type code to description for area and carbon outputs

 These ancillary files are no longer created nor used by the GCAM data system:
 Ancillary files that need to be updated manually, and which may be removed or replaced:
	gcam-data-system/aglu-data/assumptions/
		A_biocrops_R_AEZ.csv (depends on aez numbers in regions)
        A_biocrops_R_AEZ_irr.csv (depends on aez numbers in regions)
			two versions of each: A_biocrops_R_AEZ_14reg.csv, A_biocrops_R_AEZ_32reg.csv
			this is one of two files that has region dependencies
	gcam-data-system/aglu-data/mappings/
		USDA_reg_AEZ.csv (depends on aez numbers; one aez per USDA region) (not sure that this makes a difference, it is approximate) - probably will be removed

 these level0 zero are being replaced by Pot_veg_carbon_Gg_area_ha.csv:
    gcam-data-system/aglu-data/Level0/
        only mixed forest, grassland, tundra, cropland changes with aez for soil
            Various_SoilC_tCha_LTsage_AEZ.csv
        only mixed forest, grassland change with aez for veg
            Various_VegC_tCha_LTsage_AEZ.csv
 
 this level0 file is not dealt with here because there is no spatial dependence:
        but it needs generic non-aez format
            Various_MatureAge_LTsage_AEZ.csv

 
 Note: New GLU boundaries can be defined in any manner, and are input as a 4 byte signed integer raster file, 5 arcmin
    but they need to be numbered 1-numaezs, with nodata = -9999
    and each one is a unique area on the globe
 
 Note: The read functions for the data below need to be modular so they can be swapped easily
 to accommodate other data sources
 
 Note: Using Hyde 3.2.000 for land area and land use area
    2000 is the REF_YEAR data for carbon and land rent are consistent with the historical hyde area data
    sage land area is used only for sage-based processing:
        175 crop data, mirca2000 data, the deprecated N fert data
 
 Inputs (the non-SAGE175Crop files are in moirai/indata/):
	
 GLU boundaries as a raster file (with an R raster header file)
 This is currently the GCAM 235 water basin file,as derived from 1/8th degree CLM water basin data
    Chris Vernon provided a cleaned up version of this file
 The basins are spatially contiguous, unless they include islands
	GLU id number ranges from 1-235
	single-band binary file, nodata=-9999, 4-byte signed integer
	5 arcmin resolution, 2160 rows, 4320 cols, llcornboundary = -180lon, -90lat, WGS84
 Currenlty available files are:
    AEZ_orig_lds.gri: the original AEZ18 ids corrected for the patch of australia in africa
    Glboal235_CLM_5arcmin.bil: the water basin file

 A corresponding csv file that contains the new GLU integer labels and text names
    One header line
    first column: integer id
    second column: text name
 Currently available files are:
    AEZ_orig_lds.csv: the list for the original AEZs
    Global235_CLM_5arcmin.csv: the list for the water basins
 
 original GTAP AEZ boundaries as a raster file (with an R raster header file)
 This is needed to redistribute the forest land rent
	AEZ18 id number ranges from 1-18
	single-band binary file, nodata=-9999, 4-byte signed integer
	5 arcmin resolution, 2160 rows, 4320 cols, llcornboundary = -180lon, -90lat, WGS84
 AEZ_orig_lds.gri: the original AEZ18 ids corrected for the patch of australia in africa
 
 FAO country code raster file
	single-band binary file (BIL), nodata=-9999,4-byte integer
	5 arcmin resolution, 2160 rows, 4320 cols, llcornboundary = -180lon, -90lat, WGS84
	added fao country codes: antarctica (300), Hong Kong (302), Macao (301), Spratly Islands (304), Taiwan (303)
	the initial rasterization of the shapefile has been 'grown' by one cell to include coastal boundaries and all associated land area
		fao_ctry_rast.bil
 	Timor-Leste was added to the raster file
 
 NOTE: The harvested area could be non-zero where the cropland extent data is zero
	This is even the case for the sage cropland data
    So do not restrict crop processing based on cropland extent, as multiple crop periods are include in harvested area
    LDS uses the HYDE data for cropland extent and pasture and urban
		The harvested area applies to the area and production outputs
		The cropland extent applies to the land rent for forest and pasture and the carbon and land type area
        harvested area is reconciled with cropland extent in gcam data processing system
 
 SAGE crop harvested area and yield for 175 crops (ca. yr 2000)
	netcdf files at 5 arcmin resolution
	area is in fraction of land area in grid cell (p.c. Navin Ramankutty June 2013, confirmed in sep 2013, also see Monfreda et al. 2008 eq. on page 10)
 	yield is in metric tonnes per ha
 	note that the data have recently been reformatted and include harvested area in ha also
	data are based mostly on 2005 FAO database (average of years 1997 - 2003 (or 1990-1996 is a few cases))
	these data appear to match well with independent calculations of the FOA data
	the bases of the filenames are in the second column of SAGE_gtap_fao_crop2use.csv file, then append "_5min.nc"
	these files are not listed here and they have not been copied to genaez/indata/
	they are stored in: /Volumes/glabrata/geodata/sage/175crops_netcdf/
 
 SAGE potential vegetation ca. 2000
	original: converted to binary bil file from arc ascii grid file at 5 arcmin resolution, nodata val = -9999
    updated: greenland potveg added based on hyde greenland land cells
        hyde area = 0 sets potveg to ice/rock/polar desert (15)
        hyde area non-zero sets potveg to tundra (13)
        5 arcmin, nodata=-9999, binary bil file
    csv file with the raster codes and category names
	15 land cover types
			potveg_thematic.bil - original
            potveg_plus.bil - updated
            SAGE_PVLT.csv

 SAGE fraction of land area within cell
	netcdf file a 5 arcmin resolution received sep 2013 from Dany Plouffe
	based on a spherical earth, so calculate spherical grid cell area to get land area in km^2
	no land cover types excluded a priori (ice covered land is included in this fraction, except where data are missing; e.g. greenland)
	used ramankutty 2000 water mask, and greenland was also excluded
	i converted these data to a .bil raster:
		sage_land_frac.bil
 
 SAGE physical cropland area, circa 2000
 	5 arcmin netCDF file
 	fraction of land area in grid cell
 		Cropland2000_5min.nc
 
 HYDE 3.2.000 updated and original land and cell area
    area of whole grid cell (spherical earth) and area of total land (km^2) converted to binary from arc ascii grid file, and updated:
        zero land area updated with sage data (except greenland)
            because these cells are land ice
        greenland cells updated to full grid cell area to accommodate the land ice
            using all greenland cell area adds only 141828 km^2 of area over just adding grid cell area for the zero area cells
        some arctic islands have been added from the SAGE land area dat set
        the grid cell area file has been updated to match the land area cells, as they both constitute the same land mask
            updated using a spherical earth
    5 arcmin resolution, binary bil files, nodata=-9999
	the original land area is the total, excluding most glaciers,
        but supposedly including ice, rock, barren, etc. (p.c. K. Klein Goldewijk May 2013)
    the updated files are:
            hyde_land_plus.bil
            hyde_cell_plus.bil
    the original hyde files are:
			cell_area_hyde.bil
			land_area_hyde.bil
 
 HYDE 3.2.000 - historical land cover: urban, crop/rice/irr/rf, and pasture/grazing/rangeland
    some cells may have land use area here where the land area is zero, or the land use exceeds the land area
    5 arcmin binary files
    ARC ASCII grid files, zipped
    1700-2000 at 10-year intervals, 2001-2016 each year
    year 2000 data are used for the land rent forest area and the circa 2000 output data
    Earlier binary versions of crop, grassland, and urban data have been deprecated
 	These files need to be unzipped prior to running Moirai
    See the included readme_release_HYDE3.2.000.txt file for more details
 
 ISAM land cover based on HYDE 3.2.000
 	these data were generated specifically for Moirai
 	the HYDE 3.2 land use categories are preserved
 	only the years corresponding to HYDE 3.2 years are used (from 1800-2016)
 	half-degree resolution, gzipped netCDF files; these are automatically gunzipped
 	see included data document for more details
 
 
 Data for the additional GIS processing:
    suitable/protected area data from epa
 		6 files, fraction of grid cell, see read_protectd for details
        5 arcmin resolution
potential vegetation carbon (above ground and below ground biomass)
        6 different binary rasters representing 6 states of potential vegetation carbon (weighted average, median, minimum, maximum, quartile 1 and quartile 3).
        The input rasters are 5 arcmin binary files , 2160 rows, 4320 cols, llcornboundary = -180lon, -90lat, WGS84
        Data is scaled up by a factor of 0.1 in the raw data and is scaled down in our calculations
        These are calculated using a fine resolution raster dataset from Spawn et al 2020    
soil carbon (soil only for a depth of 0-30 cms)
        6 different binary rasters representing 6 states of potential vegetation carbon (weighted average, median, minimum, maximum, quartile 1 and quartile 3).
        The input rasters are 5 arcmin binary files , 2160 rows, 4320 cols, llcornboundary = -180lon, -90lat, WGS84
        The user has the option to select these rasters from 3 different sources, namely, 
        A) FAO Harmonized World Soil Database
        B) Soil grids (mean soil  carbon )
        C) Soil grids (95th percentile soil carbon)

 
 GTAP land use database R2.1V6.0:
	gcam-data-system/aglu-data/level0/
		The yield and harvested area data used in this version are the same SAGE data described above
			GTAP_value_milUSD.csv (ctry87, 13 GTAP_use, 18 AEZs; million USD (yr2001$))
    Ideally I would have used the DGTM_DATA.CSV file extracted from GTAP DGTM_DATA.HAR, but I could not get the required metadata
        specifically the sptial data and the description of the management and forest types
        I think the first two of four tables would be sufficient used:
            BSDT-timber area (units: 1000ha; con/broad/mixed, 10 ages, 18 aezs, 14 managements, countries)
            TMRN-land rent (units: usd (yr2000)/ ha / yr; 226 countries, 14 managements)
	
 Additional:
	GTAP_CAM_ctry87.csv (GTAP/GCAM 87 countries in alphabetical order of abbr (gcam code, abbr, name); the order of GTAP_value_milUSD.csv data)
	FAO_ctry_GCAM_ctry87.csv (FAO countries in alphabetical order (code, abbr, name), GCAM/GTAP 87 countries (gcam code,abbr))
        this file determines whether a country is output!
        if a country is mapped to a ctry87 region, then it is also mapped to a gcam region and output by lds
	iso_GCAM_regID_32reg.csv ; OR 14reg.csv (238 iso country abbrevs mapped to GCAM region names and region codes; in gcam_data_system/_common/mappings)
        this file must contain all of the FAO countries with valid iso3 abbrs
	GCAM_region_names_32reg.csv ; OR 14reg.csv - depends on which region set to be used (in gcam_data_system/_common/mappings)
	GTAP_use.csv (GTAP_use codes, names, and descriptions; in the order of the GTAP_value_milUSD.csv data)
	FAO_iso_VMAP0_ctry.csv (fao code, iso abbr, fao name (includes fao additions), vmap0 code, vmap0 name)
	convert_to_2001usd.csv (divisors for converting prices into year 2001 dollars)
	SAGE_gtap_fao_crop2use.csv
		(sage_crop_code, sage_file_name (just the base), sage_crop_name, gtap_crop_name, gtap_crop_use_code, gtap_crop_use_name (3 character abbreviation), fao_crop_code, fao_crop_name)
 
 FAO:
	downloaded by Alan Di Vittorio July 2018 from www.fao.org/faostat/
 	Annual 1993-2016, supposedly no empty rows
 	these file need to be in the same format with the same years (these are used as downloaded)
	harvested area (and yield) are needed only if calibrating SAGE data to a different year; SAGE is currently based on 1997-2003 FAO averages; yield is just for diagnostics; recalibration uses production and harvested area
	it appears that there are differences between these price data and those used by GTAP
	prices are in nominal USD (i.e., 2006 values are in 2006 USD); this is the basis for conversion to consisten USD year
	different files could be used here, just change the read function and the number of FAO years in the .h file
	the forage crop data is included, although it is not listed on the selection screen when downloading
		FAO_ag_HA_ha_PRODSTAT.csv
		FAO_ag_yield_hg_ha_PRODSTAT.csv
	these data are needed to disaggregrate land rents to AEZs
		FAO_ag_prod_t_PRODSTAT.csv
		FAO_ag_an_prodprice_USD_t_PRICESTAT_11yrs.csv
 
 Note: make sure that this GCAM file is consistent with the other mapping files, namely the iso files listed above:
    AGLU_ctry.csv
        does not have pci, nor yug, but the is0_GCAM_regID files do have them
        although they do not exist any more
        and are not in any data sets used here
 
 
 
 **********/

#include "moirai.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char * argv[]) {
    
    int i, j, k,l;
    int array_cells =3;
	char fname[MAXCHAR];		// used to open files
	args_struct in_args;		// data structure for holding the control input file info
	rinfo_struct raster_info;	// data structure for storing raster input file specific info
    
    char mkoutputpathcmd[MAXCHAR]; // used to create the output path
	
	// for code control
	int error_code = OK;		// 0 = ok; non-zero = error
	
	// the only argument is the name of the input control file
	if(argc != 2)
	{
		error_code = ERROR_USAGE;
		fprintf(stdout, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		fprintf(stdout, "\nProper usage:\n");
		fprintf(stdout, "%s <input file name with path>\n", CODENAME);
		return error_code;
	}
	
	fprintf(stdout, "\nProgram %s started at %s\n", CODENAME, get_systime());
	
	// set the rand() seed
	// want it the same each time the program is run
	srand(0);
	
	// initialize all of the arrays
	if((error_code = init_moirai(&in_args))) {
		fprintf(stderr, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read the input control file and fill the in_args structure
	if((error_code = get_in_args(argv[1], &in_args))) {
		fprintf(stderr, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// create log file name and open it
	strcpy(fname, in_args.outpath);
	strcat(fname, in_args.lds_logname);
    
    // create the output path
    strcpy(mkoutputpathcmd, "mkdir -p ");
    strcat(mkoutputpathcmd, in_args.outpath);
    printf("%s",mkoutputpathcmd);
    system(mkoutputpathcmd);
    
	if ((fplog = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "\nProgram terminated at %s with error_code = %i; could not open %s\n",
					get_systime(), ERROR_FILE, fname);
		return ERROR_FILE;
	}
	
	// create the paths for copying outputs to
	// data files
	strcpy(mkoutputpathcmd, "\nmkdir -p ");
	strcat(mkoutputpathcmd, in_args.ldsdestpath);
	printf("%s",mkoutputpathcmd);
	system(mkoutputpathcmd);
	// mapping files
	strcpy(mkoutputpathcmd, "\nmkdir -p ");
	strcat(mkoutputpathcmd, in_args.mapdestpath);
	printf("%s",mkoutputpathcmd);
	system(mkoutputpathcmd);
	
	fprintf(fplog, "\nProgram %s started at %s\n", CODENAME, get_systime());
	
	fprintf(fplog, "\nFor more detailed information regarding alignment of various input data set the diagnostics flag to 1 in the input file\n");

	/*
	// create a file to check each lulc cell that is easy to read into r and compare area values
	strcpy(fname, in_args.outpath);
	strcat(fname, "check_area.csv");
	if ((debug_file = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "\nProgram terminated at %s with error_code = %i; could not open %s\n",
				get_systime(), ERROR_FILE, fname);
		return ERROR_FILE;
	}
	fprintf(debug_file, "func,year,lulc_cell,rf_area,lu_area,land_area\n");
	
	// create a file for particular lu values for a given lulc cell
	strcpy(fname, in_args.outpath);
	strcat(fname, "check_lulc_cell.csv");
	if ((cell_file = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "\nProgram terminated at %s with error_code = %i; could not open %s\n",
				get_systime(), ERROR_FILE, fname);
		return ERROR_FILE;
	}
	fprintf(cell_file, "func,lu_index,wg_index,lulc_index,land_area,rf_area,lu_area,u_area,c_area,p_area\n");
	*/
	 
    //////////
    // start with the text info data
    // these are csv files that determine mappings and number of aezs, crops, counties, regions
    
    ////////// read GTAP87 and GCAM region and FAO country info text files
    
    // one file	includes the alphabetical FAO country list and the FAO, VMAP0, iso ctry mapping
    // array length and allocation done within read_country_info_all()
    if((error_code = read_country_info_all(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    // this includes both the GCAM/GTAP ctry87 list in land rent output order and the mapping between FAO ctry and GCAM/GTAP ctry87
    // array length and allocation done within read_country87_info()
    if((error_code = read_country87_info(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    // this includes GCAM region list
    // array length and allocation done within read_region_info_gcam()
    if((error_code = read_region_info_gcam(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    ////////// read the list of new aez codes and names
    
    // this is the list of new aezs
    // array length and allocation done within read_aez_new_info()
    if((error_code = read_aez_new_info(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    ////////// read crop and use and land info text files
    
    // read GTAP use info
    // this is the list in output order for land rent
    // array length and allocation done within read_use_info_gtap()
    if((error_code = read_use_info_gtap(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    // read SAGE land type info
    // array length and allocation done within read_lt_info_sage()
    if((error_code = read_lulc_info(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    // one file includes FAO to SAGE crop and to GTAP use mapping
    // array length and allocation done within read_crop_info()
    if((error_code = read_crop_info(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
	////////
	// read the raster data, except the SAGE crop data, lulc data, and hyde lu data
	
	// calculate the total area of each working grid cell (spherical earth): cell_area[NUM_CELLS]
    // and read in cell area of the hyde land cells (also spherical earth): cell_area_hyde[NUM_CELLS]
    // first allocate the arrays
    cell_area = calloc(NUM_CELLS, sizeof(float));
    if(cell_area == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for cell_area: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    cell_area_hyde = calloc(NUM_CELLS, sizeof(float));
    if(cell_area_hyde == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for cell_area_hyde: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	if((error_code = get_cell_area(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read the sage working grid land fraction and convert it to land area: land_area_sage[NUM_CELLS]
    // first allocate the arrays
    land_area_sage = calloc(NUM_CELLS, sizeof(float));
    if(land_area_sage == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_area_sage: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	if((error_code = read_land_area_sage(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read the hyde land area: land_area_hyde[NUM_CELLS]
    // first allocate the arrays
    land_area_hyde = calloc(NUM_CELLS, sizeof(float));
    if(land_area_hyde == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_area_hyde: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	if((error_code = read_land_area_hyde(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read new AEZ boundaries: aez_bounds_new[NUM_CELLS]
    // first allocate the array
    aez_bounds_new = calloc(NUM_CELLS, sizeof(int));
    if(aez_bounds_new == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for aez_bounds_new: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    if((error_code = read_aez_new(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read original AEZ boundaries:aez_bounds_orig[NUM_CELLS]
    // first allocate the array
    aez_bounds_orig = calloc(NUM_CELLS, sizeof(int));
    if(aez_bounds_orig == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for aez_bounds_orig: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	if((error_code = read_aez_orig(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read potential vegetation data: potveg_thematic[NUM_CELLS]
    // first allocate the array
    potveg_thematic = calloc(NUM_CELLS, sizeof(int));
    if(potveg_thematic == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for potveg_thematic: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	if((error_code = read_potveg(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read FAO country code data: country_fao[NUM_CELLS]
    // first allocate array
    country_fao = calloc(NUM_CELLS, sizeof(short));
    if(country_fao == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for country_fao: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	if((error_code = read_country_fao(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read lulc land mask: land_mask_lulc[NUM_CELLS]
	// first allocate array
	land_mask_lulc = calloc(NUM_CELLS, sizeof(int));
	if(land_mask_lulc == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_lulc: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	if((error_code = read_lulc_land(in_args, REF_YEAR, &raster_info, land_mask_lulc))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
    /////////
    // reconcile the raster data
	
    // allocate some raster arrays
    cropland_area = calloc(NUM_CELLS, sizeof(float));
    if(cropland_area == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for cropland_area: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    pasture_area = calloc(NUM_CELLS, sizeof(float));
    if(pasture_area == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for pasture_area: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    urban_area = calloc(NUM_CELLS, sizeof(float));
    if(urban_area == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for urban_area: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	lu_detail_area = calloc(NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN, sizeof(float*));
	if(lu_detail_area == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_detail_area: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		lu_detail_area[i] = calloc(NUM_CELLS, sizeof(float));
		if(lu_detail_area[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lu_detail_area[%i]: main()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
    refveg_area = calloc(NUM_CELLS, sizeof(float));
    if(refveg_area == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_area: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    region_gcam = calloc(NUM_CELLS, sizeof(int));
    if(region_gcam == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for region_gcam: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    sage_minus_hyde_land_area = calloc(NUM_CELLS, sizeof(float));
    if(sage_minus_hyde_land_area == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for sage_minus_hyde_land_area: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    glacier_water_area_hyde = calloc(NUM_CELLS, sizeof(float));
    if(glacier_water_area_hyde == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for glacier_water_area_hyde: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    country87_gtap = calloc(NUM_CELLS, sizeof(int));
    if(country87_gtap == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for country87_gtap: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    missing_aez_mask = calloc(NUM_CELLS, sizeof(int));
    if(missing_aez_mask == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for missing_aez_mask: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_ctryaez = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_ctryaez == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_ctryaez: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_aez_orig = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_aez_orig == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_aez_orig: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_aez_new = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_aez_new == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_aez_new: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_sage = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_sage == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_sage: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_hyde = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_hyde == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_hyde: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_fao = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_fao == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_fao: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_mask_potveg = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_potveg == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_potveg: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	land_mask_refveg = calloc(NUM_CELLS, sizeof(int));
	if(land_mask_refveg == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_refveg: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
    land_mask_forest = calloc(NUM_CELLS, sizeof(int));
    if(land_mask_forest == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_mask_forest: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
	lulc_input_grid = calloc(NUM_LULC_TYPES, sizeof(float*));
	if(lulc_input_grid == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_input_grid: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		lulc_input_grid[i] = calloc(NUM_CELLS_LULC, sizeof(float));
		if(lulc_input_grid[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for lulc_input_grid[%i]: main()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
	refveg_thematic = calloc(NUM_CELLS, sizeof(int));
	if(refveg_thematic == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for refveg_thematic: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	
    // allocate some arrays to keep track of valid raster cells
    land_cells_aez_new = calloc(NUM_CELLS, sizeof(int));
    if(land_cells_aez_new == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_cells_aez_new: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_cells_sage = calloc(NUM_CELLS, sizeof(int));
    if(land_cells_sage == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_cells_sage: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    land_cells_hyde = calloc(NUM_CELLS, sizeof(int));
    if(land_cells_hyde == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for land_cells_hyde: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    forest_cells = calloc(NUM_CELLS, sizeof(int));
    if(forest_cells == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for forest_cells: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    
    // it would be more efficient to write a loop over all cells here,
    //  and write the following two functions to operate on a single cell
    // the second function would be called only if the first one finds a land cell
    
	////
	// determine the indices of the relevant land and forest cells in aez, sage, hyde, and fao data: land_cells_####[NUM_CELLS]
	if((error_code = get_land_cells(in_args, raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	////
	// convert the hyde land use, lulc, and sage potential veg input data to working grid area
	// the rand_order array is allocated here in calc_refvef_area and is deallocated in proc_refveg_carbon
	if((error_code = calc_refveg_area(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}	

    // free some raster arrays
    free(region_gcam);
    free(sage_minus_hyde_land_area);
    free(glacier_water_area_hyde);
    free(land_mask_aez_orig);
    free(land_mask_aez_new);
    free(land_mask_sage);
    free(land_mask_hyde);
    free(land_mask_fao);
    free(land_mask_potveg);
	free(land_mask_lulc);
	
	    free(land_mask_forest);
		free(land_mask_refveg);
    
	// store the country/land rent region + aez lists
    // the arrays are allocated within write_glu_mapping()
	if((error_code = write_glu_mapping(in_args, raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
    
    // process the mirca data
    //  mirca grid is allocated/freed within proc_mirca()
    if((error_code = proc_mirca(in_args, raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
	
    //kbn 2020
    protected_EPA = calloc(NUM_EPA_PROTECTED, sizeof(float*));
    if(protected_EPA == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for protected_EPA: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_EPA_PROTECTED; i++) {
		protected_EPA[i] = calloc(NUM_CELLS, sizeof(float));
		if(protected_EPA[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for protected_EPA[%i]: main()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
    
    
    if((error_code = read_protected(in_args, &raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    //kbn 2020/06/01 Add code for read_soil_c here
    soil_carbon_sage = calloc(NUM_CARBON, sizeof(float*));
    if(soil_carbon_sage == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for soil_carbon_sage: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_CARBON; i++) {
		soil_carbon_sage[i] = calloc(NUM_CELLS, sizeof(float));
		if(soil_carbon_sage[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for soil_carbon_sage[%i]: main()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
    
    //Allocate the arrays to hold the number of cells
    soil_carbon_array_cells = calloc(NUM_FAO_CTRY, sizeof(int**));
    if(soil_carbon_array_cells == NULL) {
        fprintf(fplog,"Failed to allocate memory for refveg_carbon_out: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        soil_carbon_array_cells[i] = calloc(ctry_aez_num[i], sizeof(int*));
        if(soil_carbon_array_cells[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            soil_carbon_array_cells[i][j] = calloc(num_lt_cats, sizeof(int));
            if(soil_carbon_array_cells[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for refveg_carbon_out[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }
             // end for k loop over output values
            } // end for j loop over aezs
        }
    
  //Allocate the main soil_carbon_array
    soil_carbon_array = calloc(NUM_FAO_CTRY, sizeof(float****));
    if(soil_carbon_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for soil_carbon_array: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        soil_carbon_array[i] = calloc(ctry_aez_num[i], sizeof(float***));
        if(soil_carbon_array[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            soil_carbon_array[i][j] = calloc(num_lt_cats, sizeof(float**));
            if(soil_carbon_array[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }
            for (k = 0; k < num_lt_cats; k++) {
                soil_carbon_array[i][j][k] = calloc(NUM_CARBON, sizeof(float*));
                if(soil_carbon_array[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k);
                    return ERROR_MEM;
                }//The last dimension will be assigned in read_soil_carbon.c
                for (l = 0; l < NUM_CARBON; l++) {
                soil_carbon_array[i][j][k][l] = calloc(array_cells, sizeof(float));
                if(soil_carbon_array[i][j][k][l] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k);
                    return ERROR_MEM;
                }
                }// end for k loop over output values
            }// end for j loop over aezs
        }// end for j loop over glus
     }// end for i loop over fao country 
    
    //Allocate the vegetation carbon arrays
    veg_carbon_array = calloc(NUM_FAO_CTRY, sizeof(float****));
    if(veg_carbon_array == NULL) {
        fprintf(fplog,"Failed to allocate memory for soil_carbon_array: proc_refveg_carbon()\n");
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        veg_carbon_array[i] = calloc(ctry_aez_num[i], sizeof(float***));
        if(veg_carbon_array[i] == NULL) {
            fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i]: proc_refveg_carbon()\n", i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            veg_carbon_array[i][j] = calloc(num_lt_cats, sizeof(float**));
            if(veg_carbon_array[i][j] == NULL) {
                fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i][%i]: proc_refveg_carbon()\n", i, j);
                return ERROR_MEM;
            }
            for (k = 0; k < num_lt_cats; k++) {
                veg_carbon_array[i][j][k] = calloc(NUM_CARBON, sizeof(float*));
                if(veg_carbon_array[i][j][k] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k);
                    return ERROR_MEM;
                }//The last dimension will be assigned in read_soil_carbon.c
                for (l = 0; l < NUM_CARBON; l++) {
                veg_carbon_array[i][j][k][l] = calloc(array_cells, sizeof(float));
                if(veg_carbon_array[i][j][k][l] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for soil_carbon_array[%i][%i][%i]: proc_refveg_carbon()\n", i, j, k);
                    return ERROR_MEM;
                }
                }// end for k loop over output values
            }// end for j loop over aezs
        }// end for j loop over glus
     }// end for i loop over fao country





    //Call the read soil carbon function
    if((error_code = read_soil_carbon(in_args, &raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    } 
    
    //kbn 2020/06/30 Add code for read_veg_c here
    veg_carbon_sage = calloc(NUM_CARBON, sizeof(float*));
    if(veg_carbon_sage == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for soil_carbon_sage: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_CARBON; i++) {
		veg_carbon_sage[i] = calloc(NUM_CELLS, sizeof(float));
		if(veg_carbon_sage[i] == NULL) {
			fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for soil_carbon_sage[%i]: main()\n", get_systime(), ERROR_MEM, i);
			return ERROR_MEM;
		}
	}
        
    if((error_code = read_veg_carbon(in_args, &raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }


    // process the land type area data
    //  lu grids are allocated/freed within proc_land_type_area()
    if((error_code = proc_land_type_area(in_args, raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    // process the reference vegetation carbon data
    //  needed arrays are allocated/freed within proc_refveg_carbon()
	// the rand_order array that is allocated in calc_refvef_area is deallocated in proc_refveg_carbon
    if((error_code = proc_refveg_carbon(in_args, raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
  //  fprintf(stdout, "\nStart freeing other carbon arrays %s\n", get_systime());
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            free(soil_carbon_array_cells[i][j]);
        }free(soil_carbon_array_cells[i]);
   }free(soil_carbon_array_cells);

//fprintf(stdout, "\n Freed carbon array cells  %s\n", get_systime());
       
     
  

    // process the water footprint data
    //  needed arrays are allocated/freed within proc_water_footprint()
 
 //fprintf(stdout, "\n Start water footprint %s\n", get_systime());
 
    if((error_code = proc_water_footprint(in_args, raster_info))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
	
    // free the land type category array
    free(lt_cats);
    
    // free some rasters
	free(urban_area);
	free(cell_area);
	free(land_area_hyde);
	free(cell_area_hyde);
    free(land_cells_aez_new);
    //kbn 2020
    for (i = 0; i < NUM_EPA_PROTECTED; i++) {
		free(protected_EPA[i]);
	}
	free(protected_EPA);
    //kbn 2020/06/01 Add code for soil carbon here
    for (i = 0; i < NUM_CARBON; i++) {
		free(soil_carbon_sage[i]);
	}
	free(soil_carbon_sage);
    //kbn 2020/06/30 Add code for veg carbon here
    for (i = 0; i < NUM_CARBON; i++) {
		free(veg_carbon_sage[i]);
	}
	free(veg_carbon_sage);
    free(potveg_thematic);
	free(refveg_thematic);
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		free(lulc_input_grid[i]);
	}
	free(lulc_input_grid);
    //free soil and veg carbon arrays
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            for (k = 0; k < num_lt_cats; k++) {
	           for(l=0; l< NUM_CARBON;l++){
                   free(soil_carbon_array[i][j][k][l]);
               }free(soil_carbon_array[i][j][k]);
            }free(soil_carbon_array[i][j]);
        }free(soil_carbon_array[i]);
   }free(soil_carbon_array);
   

    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            for (k = 0; k < num_lt_cats; k++) {
	           for(l=0; l<NUM_CARBON;l++){
                   free(veg_carbon_array[i][j][k][l]);
               }free(veg_carbon_array[i][j][k]);
            }free(veg_carbon_array[i][j]);
        }free(veg_carbon_array[i]);
   }free(veg_carbon_array);
  
 

    // allocate the arrays for all the fao input data (initialized to zero)
    yield_fao = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_FAO_YRS, sizeof(float));
    if(yield_fao == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for yield_fao: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    harvestarea_fao = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_FAO_YRS, sizeof(float));
    if(harvestarea_fao == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for harvestarea_fao: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    production_fao = calloc(NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_FAO_YRS, sizeof(float));
    if(production_fao == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for production_fao: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    prodprice_fao_reglr = calloc(NUM_GTAP_CTRY87 * NUM_SAGE_CROP, sizeof(float));
    if(prodprice_fao_reglr == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for prodprice_fao_reglr: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    
    
	// read in the FAO yield and harvest area data for optional harvested area and yield calibration
	
	// read FAO yield: yield_fao[NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_FAO_YRS]
	if((error_code = read_yield_fao(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read FAO harvested area: harvestarea_fao[NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_FAO_YRS]
	if((error_code = read_harvestarea_fao(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read in the FAO production data for disaggregating the land rents and re-calibrating yield and harvest inputs
	// read FAO production: production_fao[NUM_FAO_CTRY * NUM_SAGE_CROP * NUM_FAO_YRS]
	if((error_code = read_production_fao(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
    // allocate the arrays for reading in the sage crops (initialized to zero)
    harvestarea_in = calloc(NUM_CELLS, sizeof(float));
    if(harvestarea_in == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for harvestarea_in: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    yield_in = calloc(NUM_CELLS, sizeof(float));
    if(yield_in == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for yield_in: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    
    // allocate the output harvested area and production arrays, and the pasture area array (initialized to zero)
    harvestarea_crop_aez = calloc(NUM_FAO_CTRY, sizeof(float**));
    if(harvestarea_crop_aez == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for harvestarea_crop_aez: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        harvestarea_crop_aez[i] = calloc(ctry_aez_num[i], sizeof(float*));
        if(harvestarea_crop_aez[i] == NULL) {
            fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for harvestarea_crop_aez[%i]: main()\n", get_systime(), ERROR_MEM, i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            harvestarea_crop_aez[i][j] = calloc(NUM_SAGE_CROP, sizeof(float));
            if(harvestarea_crop_aez[i][j] == NULL) {
                fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for harvestarea_crop_aez[%i][%i]: main()\n", get_systime(), ERROR_MEM, i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
    
    production_crop_aez = calloc(NUM_FAO_CTRY, sizeof(float**));
    if(production_crop_aez == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for production_crop_aez: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        production_crop_aez[i] = calloc(ctry_aez_num[i], sizeof(float*));
        if(production_crop_aez[i] == NULL) {
            fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for production_crop_aez[%i]: main()\n", get_systime(), ERROR_MEM, i);
            return ERROR_MEM;
        }
        for (j = 0; j < ctry_aez_num[i]; j++) {
            production_crop_aez[i][j] = calloc(NUM_SAGE_CROP, sizeof(float));
            if(production_crop_aez[i][j] == NULL) {
                fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for production_crop_aez[%i][%i]: main()\n", get_systime(), ERROR_MEM, i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
    
    pasturearea_aez = calloc(NUM_FAO_CTRY, sizeof(float*));
    if(pasturearea_aez == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for pasturearea_aez: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        pasturearea_aez[i] = calloc(ctry_aez_num[i], sizeof(float));
        if(pasturearea_aez[i] == NULL) {
            fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for pasturearea_aez[%i]: main()\n", get_systime(), ERROR_MEM, i);
            return ERROR_MEM;
        }
    } // end for i loop over fao country
	
	////
	// get the sage physical cropland area for normalizing the crop inputs
	// allocate the raster array
	cropland_area_sage = calloc(NUM_CELLS, sizeof(float));
	if(cropland_area_sage == NULL) {
		fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for cropland_area_sage: main()\n", get_systime(), ERROR_MEM);
		return ERROR_MEM;
	}
	if((error_code = read_cropland_sage(in_args, &raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// calculate harvested area and production for SAGE_crop from FAO-calibrated SAGE crop data
	//		read in data and perform calcs one crop at a time
	//			these crop data are normalized to sage physical crop area then applied to hyde physical crop area
	//				so that the input production is represented on a potentially different land base
	//		if desired, calibrate SAGE crop harvested area to FAO PRODSTAT crop harvested area for a different reference year
	//		if desired, calibrate SAGE crop yield to FAO PRODSTAT national production for a different reference year
	//			original GTAP reference year is the same as the SAGE data (ca. 2000 as average of 1997-2003)
	//			pixel-by-pixel calibration to country level data
	//		calculate output values: country by aez by SAGE_crop
	//			harvestarea_crop_aez[NUM_FAO_CTRY][ctry_aez_num][NUM_SAGE_CROP]
	//			production_crop_aez[NUM_FAO_CTRY][ctry_aez_num][NUM_SAGE_CROP]
	if((error_code = calc_harvarea_prod_out_crop_aez(in_args, raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
    // free some raster arrays
    free(harvestarea_in);
    free(yield_in);
    free(pasture_area);
    free(country_fao);
    free(land_area_sage);
    free(land_mask_ctryaez);
    free(land_cells_sage);
	free(cropland_area);
	free(cropland_area_sage);
	for (i = 0; i < NUM_HYDE_TYPES - NUM_HYDE_TYPES_MAIN; i++) {
		free(lu_detail_area[i]);
	}
	free(lu_detail_area);
	
	// aggregate harvest area and production to gcam land units
	if((error_code = aggregate_crop2gcam(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// write the output harvested area and production values
	if((error_code = write_harvestarea_crop_aez(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	if((error_code = write_production_crop_aez(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	//////////////////
	// land rents
	
    // allocate the original input and new output land rent arrays (initialized to zero)
    rent_orig_aez = calloc(NUM_GTAP_CTRY87 * NUM_GTAP_USE * NUM_ORIG_AEZ, sizeof(float));
    if(rent_orig_aez == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for rent_orig_aez: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    rent_use_aez = calloc(NUM_GTAP_CTRY87, sizeof(float**));
    if(rent_use_aez == NULL) {
        fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for rent_use_aez: main()\n", get_systime(), ERROR_MEM);
        return ERROR_MEM;
    }
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        rent_use_aez[i] = calloc(reglr_aez_num[i], sizeof(float*));
        if(rent_use_aez[i] == NULL) {
            fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for rent_use_aez[%i]: main()\n", get_systime(), ERROR_MEM, i);
            return ERROR_MEM;
        }
        for (j = 0; j < reglr_aez_num[i]; j++) {
            rent_use_aez[i][j] = calloc(NUM_GTAP_USE, sizeof(float));
            if(rent_use_aez[i][j] == NULL) {
                fprintf(fplog,"\nProgram terminated at %s with error_code = %i\nFailed to allocate memory for rent_use_aez[%i][%i]: main()\n", get_systime(), ERROR_MEM, i, j);
                return ERROR_MEM;
            }
        } // end for j loop over aezs
    } // end for i loop over fao country
    
	// read in original AgLU GTAP land rent data and fao price data needed for calculating new land rents
	
	// read original AgLU GTAP land rent: rent_orig_aez[NUM_GTAP_CTRY87 * NUM_GTAP_USE * NUM_ORIG_AEZ]
	if((error_code = read_rent_orig(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// read FAO producer prices: prodprice_fao[NUM_FAO_CTRY * NUM_FAO_CROP]
	if((error_code = read_prodprice_fao(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// calculate agricultural (including livestock) land rent values for new AEZs by GTAP_use
	//		current GTAP reference year is ca. 2000
	if((error_code = calc_rent_ag_use_aez(in_args, raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	 
	// calculate forest land rent values for new AEZs by GTAP_use
	//		current GTAP reference year is ca. 2000
	if((error_code = calc_rent_frs_use_aez(in_args, raster_info))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	 
    // free some raster arrays
    free(aez_bounds_new);
    free(aez_bounds_orig);
    free(refveg_area);
    free(country87_gtap);
    free(forest_cells);
    free(land_cells_hyde);
    free(missing_aez_mask);
    
	// write the land rent values
	if((error_code = write_rent_use_aez(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
	// aggregate land rent to gcam land units
	if((error_code = aggregate_use2gcam(in_args))) {
		fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
		return error_code;
	}
	
    // copy the gcam data system input files to the LDS destination directory
    if((error_code = copy_to_destpath(in_args))) {
        fprintf(fplog, "\nProgram terminated at %s with error_code = %i\n", get_systime(), error_code);
        return error_code;
    }
    
    // free the reglr+aez arrays
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        free(reglr_aez_list[i]);
    }
    free(reglr_aez_list);
    
    // free the country+aez arrays
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        free(ctry_aez_list[i]);
    }
    free(ctry_aez_list);
    
    // free the gcam+aez arrays
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        free(reggcam_aez_list[i]);
    }
    free(reggcam_aez_list);

    // free the info arrays
    free(countrycodes_fao);
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        free(countryabbrs_iso[i]);
        free(countrynames_fao[i]);
        free(ctry2ctry87abbrs_gtap[i]);
    }
    free(countryabbrs_iso);
    free(countrynames_fao);
    free(ctry2ctry87codes_gtap);
    free(ctry2ctry87abbrs_gtap);
    free(country87codes_gtap);
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        free(country87names_gtap[i]);
        free(country87abbrs_gtap[i]);
    }
    free(country87names_gtap);
    free(country87abbrs_gtap);
    free(regioncodes_gcam);
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        free(regionnames_gcam[i]);
    }
    free(regionnames_gcam);
    free(ctry2regioncodes_gcam);
    free(country_gcamiso2regioncodes_gcam);
    for (i = 0; i < NUM_GCAM_ISO_CTRY; i++) {
        free(countryabbrs_gcam_iso[i]);
    }
    free(countryabbrs_gcam_iso);
    free(aez_codes_new);
    for (i = 0; i < NUM_NEW_AEZ; i++) {
        free(aez_names_new[i]);
    }
    free(aez_names_new);
    free(usecodes_gtap);
    for (i = 0; i < NUM_GTAP_USE; i++) {
        free(usenames_gtap[i]);
        free(usedescr_gtap[i]);
    }
    free(usenames_gtap);
    free(usedescr_gtap);
    free(landtypecodes_sage);
    for (i = 0; i < NUM_SAGE_PVLT; i++) {
        free(landtypenames_sage[i]);
    }
    free(landtypenames_sage);
    free(cropcodes_sage);
    free(crop_sage2gtap_use);
    free(cropcodes_sage2fao);
    for (i = 0; i < NUM_SAGE_CROP; i++) {
        free(cropnames_gtap[i]);
        free(cropdescr_sage[i]);
        free(cropfilebase_sage[i]);
        free(cropnames_sage2fao[i]);
    }
    free(cropnames_gtap);
    free(cropdescr_sage);
    free(cropfilebase_sage);
    free(cropnames_sage2fao);
	
	free(lutypecodes_hyde);
	free(lulccodes);
	free(lulc2sagecodes);
	free(lulc2hydecodes);
	for (i = 0; i < NUM_HYDE_TYPES; i++) {
		free(lutypenames_hyde[i]);
	}
	for (i = 0; i < NUM_LULC_TYPES; i++) {
		free(lulcnames[i]);
	}
	free(lulcnames);
	
    // free the fao input data arrays
    free(yield_fao);
    free(harvestarea_fao);
    free(production_fao);
    free(prodprice_fao_reglr);
    
    // free the original land rent array
    free(rent_orig_aez);
    
    // free the output and associated arrays
    for (i = 0; i < NUM_FAO_CTRY; i++) {
        for (j = 0; j < ctry_aez_num[i]; j++) {
            free(harvestarea_crop_aez[i][j]);
            free(production_crop_aez[i][j]);
        }
        free(harvestarea_crop_aez[i]);
        free(production_crop_aez[i]);
        free(pasturearea_aez[i]);
    }
    free(harvestarea_crop_aez);
    free(production_crop_aez);
    free(pasturearea_aez);
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        for (j = 0; j < reglr_aez_num[i]; j++) {
            free(rent_use_aez[i][j]);
        }
        free(rent_use_aez[i]);
    }
    free(rent_use_aez);
    
    free(reglr_aez_num);
    free(ctry_aez_num);
    free(reggcam_aez_num);
    
    fprintf(stdout, "\nSuccessful completion of program %s at %s\n", CODENAME, get_systime());
    
	fprintf(fplog, "\nSuccessful completion of program %s at %s\n", CODENAME, get_systime());
	fclose(fplog);
	
	//fclose(debug_file);
	//fclose(cell_file);
	
    return OK;}	// end moirai_main()

/**********
 init_moirai.c
 
 initialize the output variable arrays
 initialize the input argument structure

 not all of the variables need to be initialized here:
  variables filled by file reads are not initialized because they are checked for full read-in
  varibles set directly by read functions are not initialized (e.g. rinfo_struct raster_info)
  the calculated area arrays are initialized by get_land_cells() so an extra loop is not needed here
  the recalibration arrays are also initialized by get_land_cells()
  the land cells arrays are constrained by number of valid values stored, and also do not need to be initialized
  everything else is initialized upon memory allocation
 
 arguements:
 args_struct *in_args:	pointer to input arguemtnstructure
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 5 Sep 2013
 
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
 
 Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "moirai.h"

int init_moirai(args_struct *in_args) {
    
	// input argument structure
	in_args->diagnostics = 0;
	// data years for calibration
	in_args->out_year_prod_ha_lr = 0;
	in_args->in_year_sage_crops = 0;
	in_args->out_year_usd = 0;
	in_args->in_year_lr_usd = 0;
	in_args->lulc_out_year = 0;
	// file paths; must include final "/"
	memset(in_args->inpath, '\0', MAXCHAR);
	memset(in_args->outpath, '\0', MAXCHAR);
	memset(in_args->sagepath, '\0', MAXCHAR);
	memset(in_args->hydepath, '\0', MAXCHAR);
	memset(in_args->lulcpath, '\0', MAXCHAR);
    memset(in_args->mircapath, '\0', MAXCHAR);
    memset(in_args->wfpath, '\0', MAXCHAR);
    memset(in_args->ldsdestpath, '\0', MAXCHAR);
    memset(in_args->mapdestpath, '\0', MAXCHAR);
	// input raster file names (without path)
	memset(in_args->cell_area_fname, '\0', MAXCHAR);
	memset(in_args->land_area_sage_fname, '\0', MAXCHAR);
	memset(in_args->land_area_hyde_fname, '\0', MAXCHAR);
	memset(in_args->aez_new_fname, '\0', MAXCHAR);
	memset(in_args->aez_orig_fname, '\0', MAXCHAR);
	memset(in_args->potveg_fname, '\0', MAXCHAR);
	memset(in_args->country_fao_fname, '\0', MAXCHAR);
    //memset(in_args->protected_fname, '\0', MAXCHAR);
    //kbn 2020-02-29 Introduce memory set for each of the suitability, protected area rasters
	memset(in_args->L1_fname, '\0', MAXCHAR);
	memset(in_args->L2_fname, '\0', MAXCHAR);
	memset(in_args->L3_fname, '\0', MAXCHAR);
	memset(in_args->L4_fname, '\0', MAXCHAR);
	memset(in_args->ALL_IUCN_fname, '\0', MAXCHAR);
	memset(in_args->IUCN_1a_1b_2_fname, '\0', MAXCHAR);
	memset(in_args->nfert_rast_fname, '\0', MAXCHAR); // still in input file but not used
	memset(in_args->cropland_sage_fname, '\0', MAXCHAR);
	//kbn 2020-06-01 Add new soil carbon states inputs here
	memset(in_args->soil_carbon_wavg_fname, '\0', MAXCHAR);
	memset(in_args->soil_carbon_min_fname, '\0', MAXCHAR);
	memset(in_args->soil_carbon_median_fname, '\0', MAXCHAR);
	memset(in_args->soil_carbon_max_fname, '\0', MAXCHAR);
	memset(in_args->soil_carbon_q1_fname, '\0', MAXCHAR);
	memset(in_args->soil_carbon_q3_fname, '\0', MAXCHAR);
    //kbn 2020-06-30 Add new veg carbon states inputs here
    memset(in_args->veg_carbon_wavg_fname, '\0', MAXCHAR);
	memset(in_args->veg_carbon_min_fname, '\0', MAXCHAR);
	memset(in_args->veg_carbon_median_fname, '\0', MAXCHAR);
	memset(in_args->veg_carbon_max_fname, '\0', MAXCHAR);
	memset(in_args->veg_carbon_q1_fname, '\0', MAXCHAR);
	memset(in_args->veg_carbon_q3_fname, '\0', MAXCHAR);
	//kbn 2020-08-08 Add below ground biomass
	memset(in_args->veg_BG_wavg_fname, '\0', MAXCHAR);
	memset(in_args->veg_BG_median_fname, '\0', MAXCHAR);
	memset(in_args->veg_BG_min_fname, '\0', MAXCHAR);
	memset(in_args->veg_BG_max_fname, '\0', MAXCHAR);
	memset(in_args->veg_BG_q1_fname, '\0', MAXCHAR);
	memset(in_args->veg_BG_q3_fname, '\0', MAXCHAR);
	// input csv file names (without path)
	memset(in_args->rent_orig_fname, '\0', MAXCHAR);
	memset(in_args->country87_gtap_fname, '\0', MAXCHAR);
	memset(in_args->country87map_fao_fname, '\0', MAXCHAR);
	memset(in_args->country_all_fname, '\0', MAXCHAR);
    memset(in_args->aez_new_info_fname, '\0', MAXCHAR);
	memset(in_args->countrymap_iso_gcam_region_fname, '\0', MAXCHAR);
    memset(in_args->regionlist_gcam_fname, '\0', MAXCHAR);
	memset(in_args->use_gtap_fname, '\0', MAXCHAR);
	memset(in_args->lt_sage_fname, '\0', MAXCHAR);
	memset(in_args->lu_hyde_fname, '\0', MAXCHAR);
	memset(in_args->lulc_fname, '\0', MAXCHAR);
	memset(in_args->crop_fname, '\0', MAXCHAR);
	memset(in_args->production_fao_fname, '\0', MAXCHAR);
	memset(in_args->yield_fao_fname, '\0', MAXCHAR);
	memset(in_args->harvestarea_fao_fname, '\0', MAXCHAR);
	memset(in_args->prodprice_fao_fname, '\0', MAXCHAR);
	memset(in_args->convert_usd_fname, '\0', MAXCHAR);
    memset(in_args->vegc_csv_fname, '\0', MAXCHAR);
    memset(in_args->soilc_csv_fname, '\0', MAXCHAR);
	// output file names (without path)
    memset(in_args->lds_logname, '\0', MAXCHAR);
    memset(in_args->harvestarea_fname, '\0', MAXCHAR);
    memset(in_args->production_fname, '\0', MAXCHAR);
    memset(in_args->rent_fname, '\0', MAXCHAR);
    memset(in_args->mirca_irr_fname, '\0', MAXCHAR);
    memset(in_args->mirca_rfd_fname, '\0', MAXCHAR);
    memset(in_args->land_type_area_fname, '\0', MAXCHAR);
    memset(in_args->refveg_carbon_fname, '\0', MAXCHAR);
    memset(in_args->wf_fname, '\0', MAXCHAR);
    memset(in_args->iso_map_fname, '\0', MAXCHAR);
    memset(in_args->lt_map_fname, '\0', MAXCHAR);
	
	// number of land cells
	num_land_cells_aez_new = 0;			// the actual number of land cell indices in land_cells_aez_new[]
	num_land_cells_sage = 0;			// the actual number of land cell indices in land_cells_sage[]
	num_land_cells_hyde = 0;			// the actual number of land cell indices in land_cells_hyde[]
    num_forest_cells = 0;				// the actual number of land cell indices in forest_cells[]
	
	return OK;
}
/**********
 init_lds.c
 
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
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "lds.h"

int init_lds(args_struct *in_args) {
    
	// input argument structure
	in_args->recalibrate = 0;
	in_args->diagnostics = 0;
	memset(in_args->inpath, '\0', MAXCHAR);
	memset(in_args->outpath, '\0', MAXCHAR);
	memset(in_args->sagepath, '\0', MAXCHAR);
	memset(in_args->hydepath, '\0', MAXCHAR);
	memset(in_args->lulcpath, '\0', MAXCHAR);
    memset(in_args->mircapath, '\0', MAXCHAR);
    memset(in_args->wfpath, '\0', MAXCHAR);
    memset(in_args->ldsdestpath, '\0', MAXCHAR);
    memset(in_args->mapdestpath, '\0', MAXCHAR);
	memset(in_args->cell_area_fname, '\0', MAXCHAR);
	memset(in_args->land_area_sage_fname, '\0', MAXCHAR);
	memset(in_args->land_area_hyde_fname, '\0', MAXCHAR);
	memset(in_args->aez_new_fname, '\0', MAXCHAR);
	memset(in_args->aez_orig_fname, '\0', MAXCHAR);
	memset(in_args->potveg_fname, '\0', MAXCHAR);
	memset(in_args->country_fao_fname, '\0', MAXCHAR);
    memset(in_args->protected_fname, '\0', MAXCHAR);
    memset(in_args->nfert_rast_fname, '\0', MAXCHAR);
	memset(in_args->cropland_sage_fname, '\0', MAXCHAR);
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
    memset(in_args->lds_logname, '\0', MAXCHAR);
    memset(in_args->harvestarea_fname, '\0', MAXCHAR);
    memset(in_args->production_fname, '\0', MAXCHAR);
    memset(in_args->rent_fname, '\0', MAXCHAR);
    memset(in_args->mirca_irr_fname, '\0', MAXCHAR);
    memset(in_args->mirca_rfd_fname, '\0', MAXCHAR);
    memset(in_args->nfert_fname, '\0', MAXCHAR);
    memset(in_args->land_type_area_fname, '\0', MAXCHAR);
    memset(in_args->potveg_carbon_fname, '\0', MAXCHAR);
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
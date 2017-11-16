/**********
 get_land_cells.c
 
 find and put the indices of land cells to process for harvested area and irrigated harvested area and land rent
    into land_cells_sage[num_land_cells_sage] (based on sage land fraction data)

 find and put the indices of land cells to process for land type area and forest area
    into land_cells_hyde[num_land_cells_hyde] (based on hyde land area, which is an effecive hyde land mask)
    the hyde grid cell area matches the land area as a land mask
 
 also store the indies of SAGE forest potential vegetation, for the land rent processing forest_cells[num_forest_cells]
 
 also store the land cells of the new aez data in land_cells_aez_new[num_land_cells_aez_new]
 
 land cells are those that do not contain a nodata value
 
 NOTE: since harvested area can exceed physical area due to multiple cropping,
    it can be dealt with separately from the land type areas based on hyde
    resolution of the harvested area with the physical cropland area happens in the gcam data processing system
    so the main land area base is from HYDE
    but calculations using the SAGE crop data will be based on the sage land cells
 
 generate gcam region raster image based on iso mapping and fao raster file and hyde land - for visualization only
 
 generate land masks for diagnostics

 spatial grid initializations happen here because it is the first time that there is a loop over the working grid
 
 also initialize the area and calibration arrays to NODATA
 
 also initialize the land mask arrays to 0
 
 the num_land_cells_#### variables are initialized in init_lds.c
 
 recall that serbia and montenegro have separate raster fao code values but are processed merged
    so need to assign the proper gcam region based on the merged fao code
 
 add some diagnostics regarding the number of mismatched country and land cells
 
 area units are km^2
 
 arguments:
 args_struct in_args:	input argument structure
 rinfo_struct raster_info: information about input raster data

 return value:
	integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 16 May 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
 Modified oct 2015 by Alan Di Vittorio
    Aligning spatial and FAO data for processing and output, using iso3 codes
    In conjunction with allowing arbitrary AEZs as input
 
 Updated jan 2016 to keep track of hyde land cells for the working grid
 
 **********/

#include "lds.h"

int get_land_cells(args_struct in_args, rinfo_struct raster_info) {
	
	// valid values in the sage land area data set determine the land cells to process for harvested area
    // correspondence with gcam regions will be determined by iso to gcam region mapping
	
	int i, j, k = 0;
	int err = OK;				// store error code from the write functions
	int fao_index;				// store the fao country index
	
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    int scg_index;              // the index in the fao country info arrays of merged serbia and montenegro
    
    float temp_float;
    
    // for tracking land area
    double total_sage_land_area = 0;        // global sage land area
    double extra_sage_area = 0;             // sage land cells not covered by hyde land cells
    double new_aez_sage_area_lost = 0;      // sage cells not covered by new aez data
    double orig_aez_sage_area_lost = 0;     // sage cells not covered by orig aez data
    double potveg_sage_area_lost = 0;       // sage cells not covered by pot veg data
    double fao_sage_area_lost = 0;          // sage cells not covered by fao country data
    double fao_new_aez_sage_area_lost = 0;  // sage cells not covered by fao country data or new aez data
    double total_hyde_land_area = 0;        // global hyde land area
    double extra_hyde_area = 0;             // hyde land cells not covered by sage land cells
    double new_aez_hyde_area_lost = 0;      // hyde cells not covered by new aez data
    double orig_aez_hyde_area_lost = 0;     // hyde cells not covered by orig aez data
    double potveg_hyde_area_lost = 0;       // hyde cells not covered by pot veg data
    double fao_hyde_area_lost = 0;          // hyde cells not covered by fao country data
    double fao_new_aez_hyde_area_lost = 0;  // hyde cells not covered by fao country data or new aez data
    
	char out_name_ctry87[] = "country87_gtap.bil";	// diagnostic output name for found gtap country87 map
	char out_name_region[] = "region_gcam.bil";	// output name for new gcam region raster map
        
	// loop over the all grid cells
	for (i = 0; i < NUM_CELLS; i++) {
		// initialize the land masks and country maps
		land_mask_aez_orig[i] = 0;
		land_mask_aez_new[i] = 0;
		land_mask_sage[i] = 0;
		land_mask_hyde[i] = 0;
		land_mask_fao[i] = 0;
		land_mask_potveg[i] = 0;
		land_mask_forest[i] = 0;
		land_mask_forest[i] = 0;
        land_mask_ctryaez[i] = 0;
		country87_gtap[i] = NODATA;
        glacier_water_area_hyde[i] = NODATA;
        region_gcam[i] = NODATA;
		
		// if valid original aez id value, then add cell index to land_mask_aez_orig
		if (aez_bounds_orig[i] != raster_info.aez_orig_nodata) {
			land_mask_aez_orig[i] = 1;
		}
		// if valid new aez id value, then add cell index to land_cells_aez_new array
		if (aez_bounds_new[i] != raster_info.aez_new_nodata) {
			land_cells_aez_new[num_land_cells_aez_new++] = i;
			land_mask_aez_new[i] = 1;
		}
		// if sage land area, then add cell index to land_cells_sage array and land_mask_sage
		if (land_area_sage[i] != raster_info.land_area_sage_nodata) {
			land_cells_sage[num_land_cells_sage++] = i;
			land_mask_sage[i] = 1;
		}
		// if hyde land area, then add cell index to land_cells_hyde array and land_mask_hyde
        // also keep track of residual water/ice area
		if (land_area_hyde[i] != raster_info.land_area_hyde_nodata) {
            temp_float = land_area_hyde[i];
            land_cells_hyde[num_land_cells_hyde++] = i;
			land_mask_hyde[i] = 1;
            if (cell_area_hyde[i] != raster_info.cell_area_hyde_nodata) {
                temp_float = cell_area_hyde[i];
                glacier_water_area_hyde[i] = cell_area_hyde[i] - land_area_hyde[i];
            }
		}
		// if fao country, then add cell index to land_mask_fao
        // and update gcam region image, only if a hyde land cell
        // valid fao/vmap0 territories with no iso3 or gcam region or gtap ctry87 will have values == NOMATCH for ctry87 and gcam regions
        // serbia and montenegro are also not assigned to a gcam region by the ctry87 file, but they need to be counted here
        // so leave the NOMATCH regions as the NODATA value in the gcam region image
		if ((int) country_fao[i] != raster_info.country_fao_nodata) {
			land_mask_fao[i] = 1;
            if (land_mask_hyde[i] == 1) {
                // need to find the fao index
                for (j = 0; j < NUM_FAO_CTRY; j++) {
                    if (countrycodes_fao[j] == country_fao[i]) {
                        if (ctry2regioncodes_gcam[j] != NOMATCH) {
                            region_gcam[i] = ctry2regioncodes_gcam[j];
                        } else {
                            // check for serbia and montenegro
                            if (countrycodes_fao[j] == srb_code || countrycodes_fao[j] == mne_code) {
                                scg_index = NOMATCH;
                                for (k = 0; k < NUM_FAO_CTRY; k++) {
                                    if (countrycodes_fao[k] == scg_code) {
                                        scg_index = k;
                                        break;
                                    }
                                }
                                region_gcam[i] = ctry2regioncodes_gcam[scg_index];
                            } // end if serbia or montenegro
                        } // end else check for srb and mne
                        break;
                    } // end if this raster country is found in the fao/iso list
                } // end for j loop to find the gcam region for this cell
                if (j == NUM_FAO_CTRY) {
                    // this should not happen
                    fprintf(fplog, "Error determining fao country index for region_gcam: get_land_cells(); cellind = %i\n", i);
                    return ERROR_IND;
                }
            } // end if hyde land cell
		} // end if valid country fao
		// if sage pot veg, then add cell index to land_mask_potveg
		if (potveg_thematic[i] != raster_info.potveg_nodata) {
			land_mask_potveg[i] = 1;
			// store the indices of the forest cells
			if (potveg_thematic[i] <= MAX_SAGE_FOREST_CODE && potveg_thematic[i] >= MIN_SAGE_FOREST_CODE) {
				forest_cells[num_forest_cells++] = i;
				land_mask_forest[i] = 1;
			}
		}
		
        // track some area differences
        if (land_mask_sage[i] == 1) {
            total_sage_land_area = total_sage_land_area + land_area_sage[i];
            if (land_mask_hyde[i] == 0) {
                extra_sage_area = extra_sage_area + land_area_sage[i];
            }
            if (land_mask_aez_new[i] == 0) {
                new_aez_sage_area_lost = new_aez_sage_area_lost + land_area_sage[i];
            }
            if (land_mask_aez_orig[i] == 0) {
                orig_aez_sage_area_lost = orig_aez_sage_area_lost + land_area_sage[i];
            }
            if (land_mask_potveg[i] == 0) {
                potveg_sage_area_lost = potveg_sage_area_lost + land_area_sage[i];
            }
            if (land_mask_fao[i] == 0) {
                fao_sage_area_lost = fao_sage_area_lost + land_area_sage[i];
            }
            // this is the actual area not used because either there is no country or no aez
            if (land_mask_fao[i] == 0 || land_mask_aez_new[i] == 0) {
                fao_new_aez_sage_area_lost = fao_new_aez_sage_area_lost + land_area_sage[i];
            }
        }
        if (land_mask_hyde[i] == 1) {
            total_hyde_land_area = total_hyde_land_area + land_area_hyde[i];
            if (land_mask_sage[i] == 0) {
                extra_hyde_area = extra_hyde_area + land_area_hyde[i];
            }
            if (land_mask_aez_new[i] == 0) {
                new_aez_hyde_area_lost = new_aez_hyde_area_lost + land_area_hyde[i];
            }
            if (land_mask_aez_orig[i] == 0) {
                orig_aez_hyde_area_lost = orig_aez_hyde_area_lost + land_area_hyde[i];
            }
            if (land_mask_potveg[i] == 0) {
                potveg_hyde_area_lost = potveg_hyde_area_lost + land_area_hyde[i];
            }
            if (land_mask_fao[i] == 0) {
                fao_hyde_area_lost = fao_hyde_area_lost + land_area_hyde[i];
            }
            // this is the actual area not used because either there is no country or no aez
            if (land_mask_fao[i] == 0 || land_mask_aez_new[i] == 0) {
                fao_new_aez_hyde_area_lost = fao_new_aez_hyde_area_lost + land_area_hyde[i];
            }
        }
        
        // the sage cell area is within 0.000229 km^2 against the available hyde cell area
        // the land areas are not directly comprable because original hyde does not include all glacier area
        //  the updated hyde land area does include much of the glacial area, but it is not perfect
        // this raster shows only where they overlap
        if (land_area_hyde[i] != raster_info.land_area_hyde_nodata && land_area_sage[i] != raster_info.land_area_sage_nodata) {
            sage_minus_hyde_land_area[i] = land_area_sage[i] - land_area_hyde[i];
        }
        
		// initialize the working area arrays
		cropland_area[i] = NODATA;
		pasture_area[i] = NODATA;
		urban_area[i] = NODATA;
		potveg_area[i] = NODATA;
		sage_minus_hyde_land_area[i] = NODATA;
		
		// initialize the aez value diagnostic array
		missing_aez_mask[i] = 0;

		// get the ctry87 codes to store raster maps
		// only if this is a hyde land cell
        // valid fao/vmap0 territories with no iso3 or gcam region or gtap ctry87 will have values == NOMATCH for ctry87 and gcam region
		if (land_mask_hyde[i] == 1) {
			// fao country index
			if ((int) country_fao[i] != raster_info.country_fao_nodata) {
				fao_index = NOMATCH;
				for (j = 0; j < NUM_FAO_CTRY; j++) {
                    if (countrycodes_fao[j] == (int) country_fao[i]) {
						fao_index = j;
						break;
					}
				}	// end for j loop over fao ctry to find fao index for gtap 87 code
                if (fao_index == NOMATCH) {
                    fprintf(fplog, "Error determining fao country index for land cell: get_land_cells(); cellind = %i\n", i);
                    return ERROR_IND;
                }
			} else {
				fprintf(fplog, "Warning: No FAO country exists for this hyde land cell: get_land_cells(); cellind = %i\n", i);
				continue;	// no country associated with these data so don't use this cell and go to the next one
			}	// end if fao country else if gcam gis country else no country
			
            // store the ctry87 code in a raster
            // leave the NOMATCH regions as the NODATA value
            if (ctry2ctry87codes_gtap[fao_index] != NOMATCH) {
                country87_gtap[i] = ctry2ctry87codes_gtap[fao_index];
            }
		}	// end if sage land cell (if working land cell)
	}	// end for i loop over all cells
	
    // write the new gcam region raster map
    if ((err = write_raster_int(region_gcam, NUM_CELLS, out_name_region, in_args))) {
        fprintf(fplog, "Error writing file %s: get_land_cells()\n", out_name_region);
        return err;
    }
    
    // write the global area tracking values to the log file
    fprintf(fplog, "\nGlobal land area tracking (km^2): get_land_cells():\n");
    fprintf(fplog, "total_sage_land_area = %f\n", total_sage_land_area);
    fprintf(fplog, "extra_sage_area = %f\n", extra_sage_area);
    fprintf(fplog, "new_aez_sage_area_lost = %f\n", new_aez_sage_area_lost);
    fprintf(fplog, "orig_aez_sage_area_lost = %f\n", orig_aez_sage_area_lost);
    fprintf(fplog, "potveg_sage_area_lost = %f\n", potveg_sage_area_lost);
    fprintf(fplog, "fao_sage_area_lost = %f\n", fao_sage_area_lost);
    fprintf(fplog, "sage area not used due to no fao country or no new aez = %f\n\n", fao_new_aez_sage_area_lost);
    fprintf(fplog, "total_hyde_land_area = %f\n", total_hyde_land_area);
    fprintf(fplog, "extra_hyde_area = %f\n", extra_hyde_area);
    fprintf(fplog, "new_aez_hyde_area_lost = %f\n", new_aez_hyde_area_lost);
    fprintf(fplog, "orig_aez_hyde_area_lost = %f\n", orig_aez_hyde_area_lost);
    fprintf(fplog, "potveg_hyde_area_lost = %f\n", potveg_hyde_area_lost);
    fprintf(fplog, "fao_hyde_area_lost = %f\n", fao_hyde_area_lost);
    fprintf(fplog, "hyde area not used due to no fao country or no new aez = %f\n\n", fao_new_aez_hyde_area_lost);
    
	if (in_args.diagnostics) {
		// aez orig land mask
		if ((err = write_raster_int(land_mask_aez_orig, NUM_CELLS, "land_mask_aez_orig.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_aez_orig.bil");
			return err;
		}
		// aez new land mask
		if ((err = write_raster_int(land_mask_aez_new, NUM_CELLS, "land_mask_aez_new.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_aez_new.bil");
			return err;
		}
		// sage land mask
		if ((err = write_raster_int(land_mask_sage, NUM_CELLS, "land_mask_sage.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_sage.bil");
			return err;
		}
		// hyde land mask
		if ((err = write_raster_int(land_mask_hyde, NUM_CELLS, "land_mask_hyde.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_hyde.bil");
			return err;
		}
		// fao land mask
		if ((err = write_raster_int(land_mask_fao, NUM_CELLS, "land_mask_fao.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_fao.bil");
			return err;
		}
		// pot veg land mask
		if ((err = write_raster_int(land_mask_potveg, NUM_CELLS, "land_mask_potveg.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_potveg.bil");
			return err;
		}
		// forest land mask
		if ((err = write_raster_int(land_mask_forest, NUM_CELLS, "land_mask_forest.bil", in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", "land_mask_forest.bil");
			return err;
		}
        // hyde glacier-water area for land
        if ((err = write_raster_float(glacier_water_area_hyde, NUM_CELLS, "residual_ice_wat_area_hyde.bil", in_args))) {
            fprintf(fplog, "Error writing file %s: get_land_cells()\n", "residual_ice_wat_area_hyde.bil");
            return err;
        }
		// this is the map of found gtap 87 countries
		if ((err = write_raster_int(country87_gtap, NUM_CELLS, out_name_ctry87, in_args))) {
			fprintf(fplog, "Error writing file %s: get_land_cells()\n", out_name_ctry87);
			return err;
		}
        // sage minus hyde cell area
        if ((err = write_raster_float(sage_minus_hyde_land_area, NUM_CELLS, "sage_minus_hyde_land_area.bil", in_args))) {
            fprintf(fplog, "Error writing file %s: get_land_cells()\n", "sage_minus_hyde_land_area.bil");
            return err;
        }
	}	// end if diagnostics
	
	return OK;
}
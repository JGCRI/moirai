/**********
 write_glu_mapping.c
 
 glu = geographic land unit (or come up with a better name)
    need to distinguish the input glu (no longer aez) from the gcam land use region (geographic land unit X geopolitical region)
 
 Store the GLU codes for each fao country in int **ctry_aez_list
 Store the number of GLUs for each fao country in int *ctry_aez_num
 
 Store the GLU codes for each land rent region in int **reglr_aez_list
 Store the number of GLUs for each land rent region in int *reglr_aez_num
 
 write only countries that are assigned to an economic regions (i.e., if mapped to ctry87)
 
 there can be zero GLUs in an fao country or land rent region
 
 write the GCAM country+glu to iso lookup table for the new aezs (this name can be changed in the LDS input file):
		LDS_ctry_GLU.csv (depends on aez numbers in countries; this is iso abbr mapping)
        records are written only if there is at least one glu in a country/region
 
 write the GIS land type file and store it in an array for use by later functions (this name can be changed in the LDS input file):
    LDS_land_types.csv
    SAGE potveg cat * 100 + land use code + protected code
        potveg cat: 0 = unknown, 1-15 are sage pot veg cats
        land use code: 0=unmanaged, 10=cropland, 20=pasture, 30=urbanland (crop, pasture, and urban are set in lds.h)
        protected code: 1 = protected, 2= non-protected
    corresponds with the land type area and potveg carbon output csv files

 write only as a diagnostic:
    LDS_reglr_GLU.csv    // file name for diagnostic gcam reglr+gluid to lr region abbr mapping
    LDS_reggcam_GLU.csv  // file name for diagnostic gcam reggcam+gluid to gcam reg name mapping
    and some other useful files, including raster country+glu and region+glu files
 
 Serbia and Montenegro are merged for processing and output, but they are also included separately here
    serbia (272, srb) and montenegro (273, mne) are merged into (186, scg)
 
 Do not write the GCAM biocrop aez name definition per region file (needs to be done manually):
	AgLU_Data_System/aglu-data/Assumptions/
		A_biocrops_R_AEZ.csv (depends on aez numbers in regions)
	only seven regions are in this bioenergy crop mapping file
 
 This currently assumes that new AEZs are regions rather than individual grid cells:
 If AEZs are not from a grid, also write a country+aez and a gcamregion+aez image file for diagnostics:
 Then the GCAM AEZ_ID is:
	country*10000 + AEZ value (with no data value = -9999) (country is replaced with gcam region id for the other image)
 there can be up to 9999 input AEZs
 
 arguments:
 args_struct in_args:	input argument structure
 rinfo_struct raster_info: information about input raster data
 
 return value:
 integer error code: OK = 0, otherwise a non-zero error code
 
 Created by Alan Di Vittorio on 4 Oct 2013
 Copyright 2013 Alan Di Vittorio, Lawrence Berkeley National Laboratory, All rights reserved
 
  Modified fall 2015 by Alan Di Vittorio
 
 **********/

#include "lds.h"

int write_glu_mapping(args_struct in_args, rinfo_struct raster_info) {
	
	int i,j,k;
	int land_cell_ind;	// the index in the new aez land cell array of the current land cell
	int gcam_id;		// gcam country+aez id (country*10000 + AEZ value)
	int ctry_code;		// fao country code
	int ctry_ind;		// fao country index
    int reglr_ind;		// land rent region index
    int reggcam_ind;    // gcam region index
	int aez_val;		// new aez value
    int cur_lt_cat_ind; // for creating the land type category array
    
	int temp_vals[NUM_NEW_AEZ];	// temp storage for the aezs and region indices
	
    int scg_code = 186;         // fao code for serbia and montenegro
    int srb_code = 272;         // fao code for serbia
    int mne_code = 273;         // fao code for montenegro
    
    char oname1[MAXCHAR];     // file name for gcam ctry+aezid to iso mapping
    char oname2[] = "LDS_reglr_GLU.csv";    // file name for diagnostic gcam reglr+gluid to lr region abbr mapping
    char oname3[] = "LDS_reggcam_GLU.csv";  // file name for diagnostic gcam reggcam+gluid to gcam reg name mapping
    char oname4[MAXCHAR];   // file name for land type category mapping
    char fname1[MAXCHAR];                   // outpath plus out name 1
    char fname2[MAXCHAR];                   // outpath plus out name 2
    char fname3[MAXCHAR];                   // outpath plus out name 3
    FILE *fpout1;					// file 1 pointer
    FILE *fpout2;					// file 2 pointer
    FILE *fpout3;					// file 2 pointer
	
    char *lult_names[NUM_LU_CATS] = {"Unmanaged","Cropland","Pasture","UrbanLand"};
    char *protected_names[NUM_PROTECTED] = {"Protected","Non-protected"};
    
    // get the output mapping file names
    strcpy(oname1, in_args.iso_map_fname);
    strcpy(oname4, in_args.lt_map_fname);
    
	// allocate memory for the country output list
	ctry_aez_num = calloc(NUM_FAO_CTRY, sizeof(int));
	if(ctry_aez_num == NULL) {
		fprintf(fplog,"Failed to allocate memory for ctry_aez_num:  write_glu_mapping()\n");
		return ERROR_MEM;
	}
	// store the new aezs associated with the countries
	ctry_aez_list = calloc(NUM_FAO_CTRY, sizeof(int *));
	if(ctry_aez_list == NULL) {
		fprintf(fplog,"Failed to allocate memory for dim1 of ctry_aez_list:  write_glu_mapping()\n");
		return ERROR_MEM;
	}
	// the second dimension will be dynamically reallocated for each dim1 index as needed
	// but put initial allocation here
	for (i = 0; i < NUM_FAO_CTRY; i++) {
		ctry_aez_list[i] = calloc(1, sizeof(int));
		if(ctry_aez_list[i] == NULL) {
			fprintf(fplog,"Failed to allocate initial dummy memory for ctry_aez_list[i]; i=%i:  write_glu_mapping()\n", i);
			return ERROR_MEM;
		}
	}
    
    // allocate memory for the land rent region output list
    reglr_aez_num = calloc(NUM_GTAP_CTRY87, sizeof(int));
    if(reglr_aez_num == NULL) {
        fprintf(fplog,"Failed to allocate memory for reglr_aez_num:  write_glu_mapping()\n");
        return ERROR_MEM;
    }
    // store the new aezs associated with the countries
    reglr_aez_list = calloc(NUM_GTAP_CTRY87, sizeof(int *));
    if(reglr_aez_list == NULL) {
        fprintf(fplog,"Failed to allocate memory for dim1 of reglr_aez_list:  write_glu_mapping()\n");
        return ERROR_MEM;
    }
    // the second dimension will be dynamically reallocated for each dim1 index as needed
    // but put initial allocation here
    for (i = 0; i < NUM_GTAP_CTRY87; i++) {
        reglr_aez_list[i] = calloc(1, sizeof(int));
        if(reglr_aez_list[i] == NULL) {
            fprintf(fplog,"Failed to allocate initial dummy memory for reglr_aez_list[i]; i=%i:  write_glu_mapping()\n", i);
            return ERROR_MEM;
        }
    }
    
    // allocate memory for the gcam region output list
    reggcam_aez_num = calloc(NUM_GCAM_RGN, sizeof(int));
    if(reggcam_aez_num == NULL) {
        fprintf(fplog,"Failed to allocate memory for reggcam_aez_num:  write_glu_mapping()\n");
        return ERROR_MEM;
    }
    // store the new aezs associated with the countries
    reggcam_aez_list = calloc(NUM_GCAM_RGN, sizeof(int *));
    if(reggcam_aez_list == NULL) {
        fprintf(fplog,"Failed to allocate memory for dim1 of reggcam_aez_list:  write_glu_mapping()\n");
        return ERROR_MEM;
    }
    // the second dimension will be dynamically reallocated for each dim1 index as needed
    // but put initial allocation here
    for (i = 0; i < NUM_GCAM_RGN; i++) {
        reggcam_aez_list[i] = calloc(1, sizeof(int));
        if(reggcam_aez_list[i] == NULL) {
            fprintf(fplog,"Failed to allocate initial dummy memory for reggcam_aez_list[i]; i=%i:  write_glu_mapping()\n", i);
            return ERROR_MEM;
        }
    }
    
    // generate the LDS_land_types.csv array, and write it on the fly
    strcpy(fname1, in_args.outpath);
    strcat(fname1, oname4);
    if((fpout1 = fopen(fname1, "w")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s: write_glu_mapping()\n", fname1);
        return ERROR_FILE;
    }
    
    // write header lines for the land type file
    fprintf(fpout1,"# File: %s\n", fname1);
    fprintf(fpout1,"# Description: Mapping from LDS land categories to specific categories in databases\n");
    fprintf(fpout1,"# Original source: land type area and pot veg carbon calc in: %s\n", CODENAME);
    fprintf(fpout1,"# ----------\n");
    fprintf(fpout1,"Category,LT_SAGE,LT_HYDE,LT_WDPA");
    
    // add one pv cat for unknown
    num_lt_cats = (NUM_SAGE_PVLT + 1) * NUM_LU_CATS * NUM_PROTECTED;
    lt_cats = calloc(num_lt_cats, sizeof(int));
    if(lt_cats == NULL) {
        fprintf(fplog,"Failed to allocate memory for lt_cats: write_glu_mapping()\n");
        return ERROR_MEM;
    }
    cur_lt_cat_ind = 0;
    for (k = 0; k <= NUM_SAGE_PVLT; k++) {
        for (j = 0; j < NUM_LU_CATS; j++) {
            for (i = 1; i <= NUM_PROTECTED; i++) {
                lt_cats[cur_lt_cat_ind++] = (k * SCALE_POTVEG) + (j * 10) + i;
                if (k == 0) {
                    fprintf(fpout1,"\n%i,%s,%s,%s", lt_cats[cur_lt_cat_ind-1], "Unknown",
                            lult_names[j], protected_names[i-1]);
                } else {
                fprintf(fpout1,"\n%i,%s,%s,%s", lt_cats[cur_lt_cat_ind-1], landtypenames_sage[k-1],
                        lult_names[j], protected_names[i-1]);
                }
            }
        }
    }
    
    fclose(fpout1);
    
    // get the aezs associated with the countries
    // include all fao countries here
	for (land_cell_ind = 0; land_cell_ind < num_land_cells_aez_new; land_cell_ind++) {
		aez_val = aez_bounds_new[land_cells_aez_new[land_cell_ind]];
        ctry_code = country_fao[land_cells_aez_new[land_cell_ind]];
        ctry_ind = NOMATCH;
        for (i = 0; i < NUM_FAO_CTRY; i++) {
            if (countrycodes_fao[i] == ctry_code) {
                ctry_ind = i;
                break;
            }
        } // end for i loop to get ctry index
        
        if (ctry_ind == NOMATCH) {
            continue;
        }
		
		// skip this cell if this new aez has already been stored for this country
        for (j = 0; j < ctry_aez_num[ctry_ind]; j++) {
			if (aez_val == ctry_aez_list[ctry_ind][j]) {
				break;
			}
		}
		if (j == ctry_aez_num[ctry_ind]) {
			// store this aez
			// grow the dim2 as needed, per country
			// aez values
			for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
				temp_vals[i] = ctry_aez_list[ctry_ind][i];
			}
			free(ctry_aez_list[ctry_ind]);
			ctry_aez_list[ctry_ind] = calloc(ctry_aez_num[ctry_ind] + 1, sizeof(int));
			if(ctry_aez_list[ctry_ind] == NULL) {
				fprintf(fplog,"Failed to allocate memory for ctry_aez_list[ctry_ind]; ctry_ind=%i:  write_glu_mapping()\n", ctry_ind);
				return ERROR_MEM;
			}
			for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
				ctry_aez_list[ctry_ind][i] = temp_vals[i];
			}
            ctry_aez_list[ctry_ind][ctry_aez_num[ctry_ind]++] = aez_val;
            
            // merge serbia and montenegro for scg record
            if (ctry_code == mne_code || ctry_code == srb_code) {
                ctry_code = scg_code;
                ctry_ind = NOMATCH;
                for (j = 0; j < NUM_FAO_CTRY; j++) {
                    if (countrycodes_fao[j] == ctry_code) {
                        ctry_ind = j;
                        break;
                    }
                }
                if (ctry_ind == NOMATCH) {
                    // this should never happen
                    fprintf(fplog, "Error finding scg ctry index: write_glu_mapping()\n");
                    return ERROR_IND;
                }
                
                // check to see if it is already stored
                for (j = 0; j < ctry_aez_num[ctry_ind]; j++) {
                    if (aez_val == ctry_aez_list[ctry_ind][j]) {
                        break;
                    }
                }
                if (j == ctry_aez_num[ctry_ind]) {
                    // now store this aez for scg
                    for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                        temp_vals[i] = ctry_aez_list[ctry_ind][i];
                    }
                    free(ctry_aez_list[ctry_ind]);
                    ctry_aez_list[ctry_ind] = calloc(ctry_aez_num[ctry_ind] + 1, sizeof(int));
                    if(ctry_aez_list[ctry_ind] == NULL) {
                        fprintf(fplog,"Failed to allocate memory for ctry_aez_list[ctry_ind]; ctry_ind=%i:  write_glu_mapping()\n", ctry_ind);
                        return ERROR_MEM;
                    }
                    for (i = 0; i < ctry_aez_num[ctry_ind]; i++) {
                        ctry_aez_list[ctry_ind][i] = temp_vals[i];
                    }
                    ctry_aez_list[ctry_ind][ctry_aez_num[ctry_ind]++] = aez_val;
                } // end if new aez val for scg
            } // end if serbia or montenegro
            
            // now store this aez for the land rent region
            // use scg index as set above because serbia and montenegro are not separately mapped to a region
            reglr_ind = NOMATCH;
            for (i = 0; i < NUM_GTAP_CTRY87; i++) {
                if (country87codes_gtap[i] == ctry2ctry87codes_gtap[ctry_ind]) {
                    reglr_ind = i;
                    break;
                }
            }
            if (reglr_ind == NOMATCH) {
                // this happens when a country is not assigned to a land rent region
                // which means that it is not output
                // so skip this cell
                continue;
            }
            // now store this aez for the land rent region, if it isn't there already
            for (j = 0; j < reglr_aez_num[reglr_ind]; j++) {
                if (aez_val == reglr_aez_list[reglr_ind][j]) {
                    break;
                }
            }
            if (j == reglr_aez_num[reglr_ind]) {
                for (i = 0; i < reglr_aez_num[reglr_ind]; i++) {
                    temp_vals[i] = reglr_aez_list[reglr_ind][i];
                }
                free(reglr_aez_list[reglr_ind]);
                reglr_aez_list[reglr_ind] = calloc(reglr_aez_num[reglr_ind] + 1, sizeof(int));
                if(reglr_aez_list[reglr_ind] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for reglr_aez_list[reglr_ind]; reglr_ind=%i:  write_glu_mapping()\n", reglr_ind);
                    return ERROR_MEM;
                }
                for (i = 0; i < reglr_aez_num[reglr_ind]; i++) {
                    reglr_aez_list[reglr_ind][i] = temp_vals[i];
                }
                reglr_aez_list[reglr_ind][reglr_aez_num[reglr_ind]++] = aez_val;
            } // end if new aez for a land rent region
            
            // now store this aez for the gcam region
            // use scg index as set above because serbia and montenegro are not separately mapped to a region
			// countries are only assigned to gcam regions if they are also assigned to ctry87, so no need to check here
            reggcam_ind = NOMATCH;
            for (i = 0; i < NUM_GCAM_RGN; i++) {
                if (regioncodes_gcam[i] == ctry2regioncodes_gcam[ctry_ind]) {
                    reggcam_ind = i;
                    break;
                }
            }
            if (reggcam_ind == NOMATCH) {
                // this happens when a country is not assigned to a gcam region or ctry87
                // which means that it is not output
                // so skip this cell
                continue;
            }
            // now store this aez for the gcam region, if it isn't there already
            for (j = 0; j < reggcam_aez_num[reggcam_ind]; j++) {
                if (aez_val == reggcam_aez_list[reggcam_ind][j]) {
                    break;
                }
            }
            if (j == reggcam_aez_num[reggcam_ind]) {
                for (i = 0; i < reggcam_aez_num[reggcam_ind]; i++) {
                    temp_vals[i] = reggcam_aez_list[reggcam_ind][i];
                }
                free(reggcam_aez_list[reggcam_ind]);
                reggcam_aez_list[reggcam_ind] = calloc(reggcam_aez_num[reggcam_ind] + 1, sizeof(int));
                if(reggcam_aez_list[reggcam_ind] == NULL) {
                    fprintf(fplog,"Failed to allocate memory for reggcam_aez_list[reggcam_ind]; reggcam_ind=%i:  write_glu_mapping()\n", reggcam_ind);
                    return ERROR_MEM;
                }
                for (i = 0; i < reggcam_aez_num[reggcam_ind]; i++) {
                    reggcam_aez_list[reggcam_ind][i] = temp_vals[i];
                }
                reggcam_aez_list[reggcam_ind][reggcam_aez_num[reggcam_ind]++] = aez_val;
            } // end if new aez for a gcam region
            
		}	// end if this cell is a new aez to be stored
	}	// end for land_cell_ind loop over land_cells_aez_new
	
    // write the country and aez mapping to iso gcam file
    strcpy(fname1, in_args.outpath);
    strcat(fname1, oname1);
    if((fpout1 = fopen(fname1, "w")) == NULL)
    {
        fprintf(fplog,"Failed to open file %s: write_glu_mapping()\n", fname1);
        return ERROR_FILE;
    }
    
    // write header lines for fname1
    fprintf(fpout1,"# File: %s\n", fname1);
    fprintf(fpout1,"# Description: Mapping from LDS-based country+GLU dataset to iso\n");
    fprintf(fpout1,"# Original source: GCAM iso to region mapping and the new country+GLUs from %s\n", CODENAME);
    fprintf(fpout1,"# ----------\n");
    fprintf(fpout1,"country_code,glu,iso,fao_country_name");
    
	// write diagnostic files
    if (in_args.diagnostics) {
        // land rent region file
        // create file name and open it
        strcpy(fname2, in_args.outpath);
        strcat(fname2, oname2);
        if((fpout2 = fopen(fname2, "w")) == NULL)
        {
            fprintf(fplog,"Failed to open file %s: write_glu_mapping()\n", fname2);
            return ERROR_FILE;
        }
        
        // write header lines for fname2
        fprintf(fpout2,"# File: %s\n", fname2);
        fprintf(fpout2,"# Description: Mapping from LDS-based land rent region+GLU dataset to region abbr\n");
        fprintf(fpout2,"# Original source: country to land rent mapping and new GLUs from %s\n", CODENAME);
        fprintf(fpout2,"# ----------\n");
        fprintf(fpout2,"lr_reg_code,glu,lr_reg_iso_abbr,lr_reg_name");
        
        // gcam region file
        // create file name and open it
        strcpy(fname3, in_args.outpath);
        strcat(fname3, oname3);
        if((fpout3 = fopen(fname3, "w")) == NULL)
        {
            fprintf(fplog,"Failed to open file %s: write_glu_mapping()\n", fname3);
            return ERROR_FILE;
        }
        
        // write header lines for fname3
        fprintf(fpout3,"# File: %s\n", fname3);
        fprintf(fpout3,"# Description: Mapping from LDS-based gcam region+GLU dataset to region name\n");
        fprintf(fpout3,"# Original source: GCAM iso to region mapping and new GLUs from %s\n", CODENAME);
        fprintf(fpout3,"# ----------\n");
        fprintf(fpout3,"gcam_reg_code,glu,gcam_reg_name");
        
    } // end if diagnostic output
    
    // country file
	// sort the aezs in each country by integer code
	for (ctry_ind = 0; ctry_ind < NUM_FAO_CTRY; ctry_ind++) {
		// sort the aez values first; not pretty, but it works
		for (j = 0; j < ctry_aez_num[ctry_ind]; j++) {
			temp_vals[j] = ctry_aez_list[ctry_ind][j];
		}
		for (j = 0; j < ctry_aez_num[ctry_ind]; j++) {
			ctry_aez_list[ctry_ind][j] = temp_vals[j];
			for (i = j + 1; i < ctry_aez_num[ctry_ind]; i++) {
				if (temp_vals[i] < ctry_aez_list[ctry_ind][j]) {
					ctry_aez_list[ctry_ind][j] = temp_vals[i];
					aez_val = temp_vals[j];
					temp_vals[j] = temp_vals[i];
					temp_vals[i] = aez_val;
				}
			}	// end for i loop over remaining elements
        }	// end for j loop to sort
        
        // now write the sorted values, but only if country mapped to ctry87
        for (j = 0; j < ctry_aez_num[ctry_ind]; j++) {
			if (ctry2ctry87codes_gtap[ctry_ind] != NOMATCH) {
            	// make the country+aez id
            	gcam_id = countrycodes_fao[ctry_ind] * FAOCTRY2GCAMCTRYAEZID + ctry_aez_list[ctry_ind][j];
            	fprintf(fpout1,"\n%i,%i,%s,%s", countrycodes_fao[ctry_ind], ctry_aez_list[ctry_ind][j],
            	        countryabbrs_iso[ctry_ind], countrynames_fao[ctry_ind]);
			} // end if country is assigned to ctry87
        }	// end for j loop over the aezs within countries
    }	// end for ctry_ind loop for sorting the country aez lists
	fclose(fpout1);
    
    // land rent region file
    // sort the aezs in each land rent region by integer code
    for (reglr_ind = 0; reglr_ind < NUM_GTAP_CTRY87; reglr_ind++) {
        // sort the aez values first; not pretty, but it works
        for (j = 0; j < reglr_aez_num[reglr_ind]; j++) {
            temp_vals[j] = reglr_aez_list[reglr_ind][j];
        }
        for (j = 0; j < reglr_aez_num[reglr_ind]; j++) {
            reglr_aez_list[reglr_ind][j] = temp_vals[j];
            for (i = j + 1; i < reglr_aez_num[reglr_ind]; i++) {
                if (temp_vals[i] < reglr_aez_list[reglr_ind][j]) {
                    reglr_aez_list[reglr_ind][j] = temp_vals[i];
                    aez_val = temp_vals[j];
                    temp_vals[j] = temp_vals[i];
                    temp_vals[i] = aez_val;
                }
            }	// end for i loop over remaining elements
        }	// end for j loop to sort
        
        if (in_args.diagnostics) {
            // now write the sorted values
            for (j = 0; j < reglr_aez_num[reglr_ind]; j++) {
                // make the reglr+aez id
                gcam_id = country87codes_gtap[reglr_ind] * FAOCTRY2GCAMCTRYAEZID + reglr_aez_list[reglr_ind][j];
                fprintf(fpout2,"\n%i,%i,%s,%s", country87codes_gtap[reglr_ind], reglr_aez_list[reglr_ind][j],
                        country87abbrs_gtap[reglr_ind], country87names_gtap[reglr_ind]);
            }	// end for j loop over the aezs within regions
        } // end if diagnostic output
    }	// end for reglr_ind loop for sorting the land rent region lists
    
    // gcam region file
    // sort the aezs in each gcam region by integer code
    for (reggcam_ind = 0; reggcam_ind < NUM_GCAM_RGN; reggcam_ind++) {
        // sort the aez values first; not pretty, but it works
        for (j = 0; j < reggcam_aez_num[reggcam_ind]; j++) {
            temp_vals[j] = reggcam_aez_list[reggcam_ind][j];
        }
        for (j = 0; j < reggcam_aez_num[reggcam_ind]; j++) {
            reggcam_aez_list[reggcam_ind][j] = temp_vals[j];
            for (i = j + 1; i < reggcam_aez_num[reggcam_ind]; i++) {
                if (temp_vals[i] < reggcam_aez_list[reggcam_ind][j]) {
                    reggcam_aez_list[reggcam_ind][j] = temp_vals[i];
                    aez_val = temp_vals[j];
                    temp_vals[j] = temp_vals[i];
                    temp_vals[i] = aez_val;
                }
            }	// end for i loop over remaining elements
        }	// end for j loop to sort
        
        if (in_args.diagnostics) {
            // now write the sorted values
            for (j = 0; j < reggcam_aez_num[reggcam_ind]; j++) {
                // make the reggcam+aez id
                gcam_id = regioncodes_gcam[reggcam_ind] * FAOCTRY2GCAMCTRYAEZID + reggcam_aez_list[reggcam_ind][j];
                fprintf(fpout3,"\n%i,%i,%s", regioncodes_gcam[reggcam_ind], reggcam_aez_list[reggcam_ind][j],
                        regionnames_gcam[reggcam_ind]);
            }	// end for j loop over the aezs within regions
        } // end if diagnostic output
    }	// end for reggcam_ind loop for sorting the gcam region lists
    
    if (in_args.diagnostics) {
        fclose(fpout2);
        fclose(fpout3);
        
        if(ctry_ind != NUM_FAO_CTRY)
        {
            fprintf(fplog, "Error writing file %s: write_glu_mapping(); countries looped over=%i != n countries=%i\n",
                    fname1, ctry_ind, NUM_FAO_CTRY);
            return ERROR_FILE;
        }
        if(reglr_ind != NUM_GTAP_CTRY87)
        {
            fprintf(fplog, "Error writing file %s: write_glu_mapping(); lr regions looped over=%i != n lr regions=%i\n",
                    fname2, reglr_ind, NUM_GTAP_CTRY87);
            return ERROR_FILE;
        }
        if(reggcam_ind != NUM_GCAM_RGN)
        {
            fprintf(fplog, "Error writing file %s: write_glu_mapping(); gcam regions looped over=%i != n regions=%i\n",
                    fname3, reggcam_ind, NUM_GCAM_RGN);
            return ERROR_FILE;
        }
	
	}	// end if diagnostics
	
	return OK;
}
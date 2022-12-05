/**********
 get_in_args.c
 
 read the input file and fill the args_struct
 
 each line is a record with one input argument
 comments are denoted by the pound character: #
 this function assumes a specific order for the input values
 all blank and commented lines are ignored
 
 arguments:
 char *fname:			the name of the input file, with path
 args_struct *in_args:	pointer to the in_args structure

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

int get_in_args(const char *fname, args_struct *in_args) {
	
	int length;						// length of input value string
	int nrecords = NUM_IN_ARGS;		// number of input variables in file
	int count;						// record counter
	FILE *fpin;						// file pointer
	char rec_str[MAXRECSIZE];		// string to hold one record
	char cln_str[MAXRECSIZE];		// whitespace removed string
	char fld_str[MAXRECSIZE];		// whitespace removed input value
	const char comment[] = "#";		// character that denotes a comment to disregard rest of line
	
	// open input file
	if((fpin = fopen(fname, "r")) == NULL)
	{
		fprintf(stderr,"Failed to open file %s:  get_in_args()\n", fname);
		return ERROR_FILE;
	}
	
	// read the input file
	count = 0;
	while (fscanf(fpin, "%[^\r\n]\r\n", rec_str) != EOF) {
		rm_whitesp(cln_str, rec_str);
		memset(fld_str, '\0', MAXRECSIZE);
		// skip blank and comment lines, and count the records
		if (cln_str[0] != '\0' && strncmp(comment, cln_str, 1) != 0) {
			count++;
			// strip off the comment, if it exists
			length = (int) strcspn(cln_str, comment);
			strncpy(fld_str, cln_str, length);
			// set the input variable
			switch (count) {
				case 1:
               in_args->diagnostics = atoi(fld_str);
					break;
				case 2:
					in_args->out_year_prod_ha_lr = atoi(fld_str);
					break;
				case 3:
					in_args->in_year_sage_crops = atoi(fld_str);
					break;
				case 4:
					in_args->out_year_usd = atoi(fld_str);
					break;
				case 5:
					in_args->in_year_lr_usd = atoi(fld_str);
					break;
				case 6:
					in_args->lulc_out_year = atoi(fld_str);
					break;
				case 7:
					strcpy(in_args->inpath, fld_str);
					break;
				case 8:
					strcpy(in_args->outpath, fld_str);
					break;
            case 9:
					strcpy(in_args->sagepath, fld_str);
					break;
				case 10:
					strcpy(in_args->hydepath, fld_str);
					break;
				case 11:
					strcpy(in_args->lulcpath, fld_str);
					break;
            case 12:
               strcpy(in_args->mircapath, fld_str);
               break;
            case 13:
               strcpy(in_args->wfpath, fld_str);
               break;
            case 14:
               strcpy(in_args->ldsdestpath, fld_str);
               break;
            case 15:
               strcpy(in_args->mapdestpath, fld_str);
               break;
            case 16:
					strcpy(in_args->cell_area_fname, fld_str);
					break;
				case 17:
					strcpy(in_args->land_area_sage_fname, fld_str);
					break;
				case 18:
					strcpy(in_args->land_area_hyde_fname, fld_str);
					break;
				case 19:
					strcpy(in_args->aez_new_fname, fld_str);
					break;
				case 20:
					strcpy(in_args->aez_orig_fname, fld_str);
					break;
				case 21:
					strcpy(in_args->potveg_fname, fld_str);
					break;
				case 22:
					strcpy(in_args->country_fao_fname, fld_str);
					break;
				case 23:
               strcpy(in_args->L1_fname, fld_str);
               break;
				case 24:
               strcpy(in_args->L2_fname, fld_str);
               break;
				case 25:
               strcpy(in_args->L3_fname, fld_str);
               break;
				case 26:
               strcpy(in_args->L4_fname, fld_str);
               break;
				case 27:
               strcpy(in_args->ALL_IUCN_fname, fld_str);
               break;
				case 28:
               strcpy(in_args->IUCN_1a_1b_2_fname, fld_str);
               break;
				case 29:
               strcpy(in_args->nfert_rast_fname, fld_str);
               break;
				case 30:
					strcpy(in_args->cropland_sage_fname, fld_str);
					break;
				case 31:
					strcpy(in_args->soil_carbon_wavg_fname, fld_str);
					break;
				case 32:
					strcpy(in_args->soil_carbon_min_fname, fld_str);
					break;
				case 33:
					strcpy(in_args->soil_carbon_median_fname, fld_str);
					break;
				case 34:
					strcpy(in_args->soil_carbon_max_fname, fld_str);
					break;
				case 35:
					strcpy(in_args->soil_carbon_q1_fname, fld_str);
					break;
				case 36:
					strcpy(in_args->soil_carbon_q3_fname, fld_str);
					break;				
				case 37:
					strcpy(in_args->veg_carbon_wavg_fname, fld_str);
					break;
				case 38:
					strcpy(in_args->veg_carbon_min_fname, fld_str);
					break;
				case 39:
					strcpy(in_args->veg_carbon_median_fname, fld_str);
					break;
				case 40:
					strcpy(in_args->veg_carbon_max_fname, fld_str);
					break;
				case 41:
					strcpy(in_args->veg_carbon_q1_fname, fld_str);
					break;
				case 42:
					strcpy(in_args->veg_carbon_q3_fname, fld_str);
					break;
				case 43:
					strcpy(in_args->veg_BG_wavg_fname, fld_str);
					break;
				case 44:
					strcpy(in_args->veg_BG_median_fname, fld_str);
					break;
				case 45:
					strcpy(in_args->veg_BG_min_fname, fld_str);
					break;				
				case 46:
					strcpy(in_args->veg_BG_max_fname, fld_str);
					break;
				case 47:
					strcpy(in_args->veg_BG_q1_fname, fld_str);
					break;
				case 48:
					strcpy(in_args->veg_BG_q3_fname, fld_str);
					break;
				case 49:
					strcpy(in_args->soil_carbon_pasture_wavg_fname, fld_str);
					break;
				case 50:
					strcpy(in_args->soil_carbon_pasture_min_fname, fld_str);
					break;
				case 51:
					strcpy(in_args->soil_carbon_pasture_median_fname, fld_str);
					break;
				case 52:
					strcpy(in_args->soil_carbon_pasture_max_fname, fld_str);
					break;
				case 53:
					strcpy(in_args->soil_carbon_pasture_q1_fname, fld_str);
					break;
				case 54:
					strcpy(in_args->soil_carbon_pasture_q3_fname, fld_str);
					break;				
				case 55:
					strcpy(in_args->veg_carbon_pasture_wavg_fname, fld_str);
					break;
				case 56:
					strcpy(in_args->veg_carbon_pasture_min_fname, fld_str);
					break;
				case 57:
					strcpy(in_args->veg_carbon_pasture_median_fname, fld_str);
					break;
				case 58:
					strcpy(in_args->veg_carbon_pasture_max_fname, fld_str);
					break;
				case 59:
					strcpy(in_args->veg_carbon_pasture_q1_fname, fld_str);
					break;
				case 60:
					strcpy(in_args->veg_carbon_pasture_q3_fname, fld_str);
					break;
				case 61:
					strcpy(in_args->veg_BG_pasture_wavg_fname, fld_str);
					break;
				case 62:
					strcpy(in_args->veg_BG_pasture_median_fname, fld_str);
					break;
				case 63:
					strcpy(in_args->veg_BG_pasture_min_fname, fld_str);
					break;				
				case 64:
					strcpy(in_args->veg_BG_pasture_max_fname, fld_str);
					break;
				case 65:
					strcpy(in_args->veg_BG_pasture_q1_fname, fld_str);
					break;
				case 66:
					strcpy(in_args->veg_BG_pasture_q3_fname, fld_str);
					break;
				case 67:
					strcpy(in_args->soil_carbon_crop_wavg_fname, fld_str);
					break;
				case 68:
					strcpy(in_args->soil_carbon_crop_min_fname, fld_str);
					break;
				case 69:
					strcpy(in_args->soil_carbon_crop_median_fname, fld_str);
					break;
				case 70:
					strcpy(in_args->soil_carbon_crop_max_fname, fld_str);
					break;
				case 71:
					strcpy(in_args->soil_carbon_crop_q1_fname, fld_str);
					break;
				case 72:
					strcpy(in_args->soil_carbon_crop_q3_fname, fld_str);
					break;				
				case 73:
					strcpy(in_args->veg_carbon_crop_wavg_fname, fld_str);
					break;
				case 74:
					strcpy(in_args->veg_carbon_crop_min_fname, fld_str);
					break;
				case 75:
					strcpy(in_args->veg_carbon_crop_median_fname, fld_str);
					break;
				case 76:
					strcpy(in_args->veg_carbon_crop_max_fname, fld_str);
					break;
				case 77:
					strcpy(in_args->veg_carbon_crop_q1_fname, fld_str);
					break;
				case 78:
					strcpy(in_args->veg_carbon_crop_q3_fname, fld_str);
					break;
				case 79:
					strcpy(in_args->veg_BG_crop_wavg_fname, fld_str);
					break;
				case 80:
					strcpy(in_args->veg_BG_crop_median_fname, fld_str);
					break;
				case 81:
					strcpy(in_args->veg_BG_crop_min_fname, fld_str);
					break;				
				case 82:
					strcpy(in_args->veg_BG_crop_max_fname, fld_str);
					break;
				case 83:
					strcpy(in_args->veg_BG_crop_q1_fname, fld_str);
					break;
				case 84:
					strcpy(in_args->veg_BG_crop_q3_fname, fld_str);
					break;
				case 85:
					strcpy(in_args->soil_carbon_urban_wavg_fname, fld_str);
					break;
				case 86:
					strcpy(in_args->soil_carbon_urban_min_fname, fld_str);
					break;
				case 87:
					strcpy(in_args->soil_carbon_urban_median_fname, fld_str);
					break;
				case 88:
					strcpy(in_args->soil_carbon_urban_max_fname, fld_str);
					break;
				case 89:
					strcpy(in_args->soil_carbon_urban_q1_fname, fld_str);
					break;
				case 90:
					strcpy(in_args->soil_carbon_urban_q3_fname, fld_str);
					break;				
				case 91:
					strcpy(in_args->veg_carbon_urban_wavg_fname, fld_str);
					break;
				case 92:
					strcpy(in_args->veg_carbon_urban_min_fname, fld_str);
					break;
				case 93:
					strcpy(in_args->veg_carbon_urban_median_fname, fld_str);
					break;
				case 94:
					strcpy(in_args->veg_carbon_urban_max_fname, fld_str);
					break;
				case 95:
					strcpy(in_args->veg_carbon_urban_q1_fname, fld_str);
					break;
				case 96:
					strcpy(in_args->veg_carbon_urban_q3_fname, fld_str);
					break;
				case 97:
					strcpy(in_args->veg_BG_urban_wavg_fname, fld_str);
					break;
				case 98:
					strcpy(in_args->veg_BG_urban_median_fname, fld_str);
					break;
				case 99:
					strcpy(in_args->veg_BG_urban_min_fname, fld_str);
					break;				
				case 100:
					strcpy(in_args->veg_BG_urban_max_fname, fld_str);
					break;
				case 101:
					strcpy(in_args->veg_BG_urban_q1_fname, fld_str);
					break;
				case 102:
					strcpy(in_args->veg_BG_urban_q3_fname, fld_str);
					break;						
				case 103:
					strcpy(in_args->rent_orig_fname, fld_str);
					break;
				case 104:
					strcpy(in_args->country87_gtap_fname, fld_str);
					break;
				case 105:
					strcpy(in_args->country87map_fao_fname, fld_str);
					break;
				case 106:
					strcpy(in_args->country_all_fname, fld_str);
					break;
            case 107:
               strcpy(in_args->aez_new_info_fname, fld_str);
               break;
            case 108:
					strcpy(in_args->countrymap_iso_gcam_region_fname, fld_str);
					break;
				case 109:
					strcpy(in_args->regionlist_gcam_fname, fld_str);
					break;
				case 110:
					strcpy(in_args->use_gtap_fname, fld_str);
					break;
				case 111:
					strcpy(in_args->lt_sage_fname, fld_str);
					break;
				case 112:
					strcpy(in_args->lu_hyde_fname, fld_str);
					break;
				case 113:
					strcpy(in_args->lulc_fname, fld_str);
					break;
				case 114:
					strcpy(in_args->crop_fname, fld_str);
					break;
				case 115:
					strcpy(in_args->production_fao_fname, fld_str);
					break;
				case 116:
					strcpy(in_args->yield_fao_fname, fld_str);
					break;
				case 117:
					strcpy(in_args->harvestarea_fao_fname, fld_str);
					break;
				case 118:
					strcpy(in_args->prodprice_fao_fname, fld_str);
					break;
				case 119:
					strcpy(in_args->convert_usd_fname, fld_str);
					break;
            case 120:
               strcpy(in_args->lds_logname, fld_str);
               break;
				case 121:
					strcpy(in_args->harvestarea_fname, fld_str);
					break;
				case 122:
					strcpy(in_args->production_fname, fld_str);
					break;
				case 123:
					strcpy(in_args->rent_fname, fld_str);
					break;
            case 124:
               strcpy(in_args->mirca_irr_fname, fld_str);
               break;
            case 125:
               strcpy(in_args->mirca_rfd_fname, fld_str);
               break;
            case 126:
               strcpy(in_args->land_type_area_fname, fld_str);
               break;
            case 127:
               strcpy(in_args->refveg_carbon_fname, fld_str);
               break;
            case 128:
               strcpy(in_args->wf_fname, fld_str);
               break;
            case 129:
               strcpy(in_args->iso_map_fname, fld_str);
               break;
            case 130:
               strcpy(in_args->lt_map_fname, fld_str);
               break;
					
                    
				default:
					break;
			}	// end switch
		}	// end if record
	}	// end while loop over input file lines
	
	fclose(fpin);
	
	if(count != nrecords)
	{
		fprintf(stderr, "Error reading file %s: get_in_args(); records read=%i != nrecords=%i\n",
				fname, count, nrecords);
		return ERROR_FILE;
	}
	
	return OK;}

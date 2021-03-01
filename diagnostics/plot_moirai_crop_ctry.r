######
# plot_moirai_crop_ctry.r
#
# There are only four variables that the user should set (lines 87-100)
#   gtap:		TRUE=compare against GTAP input data; FALSE=compare against original version of the land data system
#   papergray:	TRUE=plot grayscale figures; FALSE=plot color figures
#   newdir:		The directory containing the moirai outputs to diagnose (can be either 235 basins or 18 aezs)
#				If you have a custom glu file with > 235 units then increase num_aez in line 118 accordingly
#   outdir:		The directory to write diagnostic figures to
#
# note that the Moirai land data system is sometimes labelled as "lds"
#	and "Original" refers to GENAEZECON, the first version of the land data system, or GTAP,
#		both of which use only 18 AEZs (the included data are for the original GTAP 18 AEZs)
#
# modified 19 may 2015 to add a couple of grayscale plots for the paper
# modified from plot_crop_ctry.r oct 2015 to accommodate Moirai LDS outputs
# addtional modification through jan 2016
#
# calc stats and make plots of the country level production and harvested area outputs from moirai lds
#	harvested area is in ha (MOIRAI_ag_HA_ha.csv)
#	production is in metric tonnes (MOIRAI_ag_prod_t.csv)
# 		
#	these inputs are rounded to the integer
#	the working directory is expected to be:
#	 .../moirai/diagnostics/
#
# compare the Moirai LDS country data with Original (either GTAP or GENAEZECON data) and FAO data
# FAO data need to be averaged over the years 1997 - 2003
#  these fao data are the moirai lds diagnostic outputs, so the area is in km^2, and are rounded to 2 digits, so need to be rounded here
# all files should not have any missing or NA values
# zeros represent no data
#
# also calculate regressions across the dataset pairs for prodcution and area for each crop
#  average regression outputs across crops for a summary plot (set a minimum crop regression sample size to use here)
#
# Moirai LDS data format
# six header lines (the sixth contains the column labels)
# no zero values
# four colums: reglr_iso, glu_code, use_sector, value
#
# GENAEZECON and GTAP data format
# there are six header lines
# there are 226 countries and 175 crops, with crop varying faster
# first column is country abbreviation
# second column is crop name
# next 18 columns are the climate aezs in order 1-18
#
# FAO data format
# no header line
# 235 FAO countries, 175 GTAP crops, 24 years (1993-2016)
# column 1: country code
# column 2: crop code
# columns 3 - 26: values for years 1993 - 2016 in order

# this takes about 10 minutes to run with 235 moirai glus and about 10 minutes for 18 aezs

# Created by Alan Di Vittorio
 
# Moirai Land Data System (Moirai) Copyright (c) 2019, The
# Regents of the University of California, through Lawrence Berkeley National
# Laboratory (subject to receipt of any required approvals from the U.S.
# Dept. of Energy).  All rights reserved.
 
# If you have questions about your rights to use or distribute this software,
# please contact Berkeley Lab's Intellectual Property Office at
# IPO@lbl.gov.
 
# NOTICE.  This Software was developed under funding from the U.S. Department
# of Energy and the U.S. Government consequently retains certain rights.  As
# such, the U.S. Government has been granted for itself and others acting on
# its behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit other to do
# so.
 
# This file is part of Moirai.
 
# Moirai is free software: you can use it under the terms of the modified BSD-3 license (see …/moirai/license.txt)

library(stringr)

cat("started plot_moirai_crop_ctry.r at ",date(), "\n")

# make sure that the working directory is .../moirai/diagnostics
#setwd("./")

# compare against gtap or older genaezecon output
# comparing to another moirai lds output should be identical because the outputs are aggregated to country (unless a new source data set is used)
gtap = TRUE

# plot the paper figures in grayscale
papergray = FALSE

# this is the new data directory (include final "/")
newdir = "../example_outputs/basins235/"
#newdir = "../example_outputs/aeZ_orig/"

# recommended outdir is in diagnostics because these are comparisons between cases  (include final "/")
outdir = paste("./basins235_example_outputs_stats_ctry_gtap/", sep="")
#outdir = paste("./aez_orig_example_outputs_stats_ctry_gtap/", sep="")

dir.create(outdir, recursive = TRUE)

# fao info
fao_avg_year = 2000
num_faoavg = 7
fao_start_year = 1993
fao_end_year= 2016
num_faoyears = fao_end_year - fao_start_year + 1
fao_meta_cols = 2
fao_startavg = (fao_avg_year - num_faoavg %/% 2) - fao_start_year + 1

# output names
ptag_ctry = "_ctry.pdf"
ctag_ctry = "_ctry.csv"

# number of GLUs in current moirai lds output
num_aez = 235
# number of gtap AEZs
num_aez_gtap = 18
# country counts
num_ctry = 231			# number of countries output by moirai lds
num_ctry_fao = 235		# number of countries in fao diagnostic output from moirai lds
num_ctry_gtap = 226		# number of gtap/genaezecon countries

# input data files
# the fao files are diagnostic moirai lds outputs

if(gtap) {
	# use moirai lds "new" data vs gtap and fao
	prodname = paste(newdir,"MOIRAI_ag_prod_t.csv", sep="")
	areaname = paste(newdir,"MOIRAI_ag_HA_ha.csv", sep="")
	prodname_fao = paste(newdir,"production_fao.csv", sep="")
	areaname_fao = paste(newdir,"harvestarea_fao.csv", sep="")
	# and compare with the gtap data
	prodname_orig = "./GTAP_ag_prod_t.csv"
	areaname_orig = "./GTAP_ag_HA_ha.csv"
	num_aez_ref = num_aez_gtap
	num_ctry_ref = num_ctry_gtap
} else {
	# use "new" moirai lds output for new glus and genaecon outputs for original 18 aezs, and also fao
	prodname = paste(newdir,"MOIRAI_ag_prod_t.csv", sep="")
	areaname = paste(newdir,"MOIRAI_ag_HA_ha.csv", sep="")
	prodname_fao = paste(newdir,"production_fao.csv", sep="")
	areaname_fao = paste(newdir,"harvestarea_fao.csv", sep="")
	# and compare with the reference lds output
	prodname_orig = paste("./GENAEZECON_ag_prod_t.csv", sep="")
	areaname_orig = paste("./GENAEZECON_ag_HA_ha.csv", sep="")
	num_aez_ref = num_aez_gtap
	num_ctry_ref = num_aez_gtap
}

# more counts
num_crop = 175
num_recs = num_ctry * num_crop
num_recs_gtap = num_ctry_ref * num_crop
num_colskip = 2
lds_colskip = 3

# input mapping files
# five columns, one header line
countryfname_gtap = "./GTAP_ctry_GCAM_ctry87.csv"
# five columns, one header line
# now use the 235 country fao/vmap0 file used by the lds because this determines the fao diagnostic output
countrymapfname = "../indata/FAO_iso_VMAP0_ctry.csv"

# five columns, one header line
# this file determines which countries are in the Moirai LDS output file
countryfname_lds = "../indata/FAO_ctry_GCAM_ctry87.csv"
# the crop names are here, fourth column, one header line
cropfname = "../indata/SAGE_gtap_fao_crop2use.csv"

#intype = as.list(character(1 + lds_colskip))
nhead_lds = 5
intype_gtap = as.list(character(num_aez_ref + num_colskip))
nhead = 6
intype_fao = as.list(character(fao_meta_cols + num_faoyears))
nhead_fao = 0
intype_crop = as.list(character(8))
nhead_crop = 1

# minimum number of crop values for regression
min_crop_n = 20

# 3d array for in values
prod = array(dim=c(num_ctry, num_crop, num_aez))
area = array(dim=c(num_ctry, num_crop, num_aez))
prod_orig = array(dim=c(num_ctry, num_crop, num_aez_ref))
area_orig = array(dim=c(num_ctry, num_crop, num_aez_ref))
prod_fao = array(dim=c(num_ctry, num_crop, num_faoyears))
area_fao = array(dim=c(num_ctry, num_crop, num_faoyears))
prev_flag_area = array(dim=c(num_faoyears, num_ctry, num_crop))
prev_flag_prod = array(dim=c(num_faoyears, num_ctry, num_crop))
prod[,,] = 0.0
area[,,] = 0.0
prod_orig[,,] = 0.0
area_orig[,,] = 0.0
prod_fao[,,] = 0.0
area_fao[,,] = 0.0
prev_flag_area[,,] = 0.0
prev_flag_prod[,,] = 0.0

# 2d array of crops by country
prod_ctry = array(dim=c(num_ctry, num_crop))
area_ctry = array(dim=c(num_ctry, num_crop))
prod_ctry_orig = array(dim=c(num_ctry, num_crop))
area_ctry_orig = array(dim=c(num_ctry, num_crop))
prod_ctry_fao = array(dim=c(num_ctry, num_crop))
area_ctry_fao = array(dim=c(num_ctry, num_crop))
year_prod_ctry_fao = array(dim=c(num_faoyears, num_ctry, num_crop))
year_area_ctry_fao = array(dim=c(num_faoyears, num_ctry, num_crop))
divisor_faoavg_area = array(dim=c(num_ctry, num_crop))
divisor_faoavg_prod = array(dim=c(num_ctry, num_crop))
prod_ctry[,] = 0.0
area_ctry[,] = 0.0
prod_ctry_orig[,] = 0.0
area_ctry_orig[,] = 0.0
prod_ctry_fao[,] = 0.0
area_ctry_fao[,] = 0.0
year_prod_ctry_fao[,,] = 0.0
year_area_ctry_fao[,,] = 0.0
divisor_faoavg_area[,] = 0.0
divisor_faoavg_prod[,] = 0.0

# 1d arrays of differences (genaezecon - other)
prod_diff_ctry_orig = array(dim = num_ctry)
area_diff_ctry_orig = array(dim = num_ctry)
prod_diff_ctry_fao = array(dim = num_ctry)
area_diff_ctry_fao = array(dim = num_ctry)
prod_diff_ctry_orig_fao = array(dim = num_ctry)
area_diff_ctry_orig_fao = array(dim = num_ctry)

# 1d arrays of percent differences (genaezecon - other)
prod_pctdiff_ctry_orig = array(dim = num_ctry)
area_pctdiff_ctry_orig = array(dim = num_ctry)
prod_pctdiff_ctry_fao = array(dim = num_ctry)
area_pctdiff_ctry_fao = array(dim = num_ctry)
prod_pctdiff_ctry_orig_fao = array(dim = num_ctry)
area_pctdiff_ctry_orig_fao = array(dim = num_ctry)

# arrays for regression values
neworigind = 1
newfaoind = 2
faoorigind = 3
prod_intercept = array(dim = c(num_crop, 3))
prod_intercepterr = array(dim = c(num_crop, 3))
prod_slope = array(dim = c(num_crop, 3))
prod_slopeerr = array(dim = c(num_crop, 3))
prod_r2 = array(dim = c(num_crop, 3))
prod_pval_slope1 = array(dim = c(num_crop, 3))
prod_n = array(dim = c(num_crop, 3))
area_intercept = array(dim = c(num_crop, 3))
area_intercepterr = array(dim = c(num_crop, 3))
area_slope = array(dim = c(num_crop, 3))
area_slopeerr = array(dim = c(num_crop, 3))
area_r2 = array(dim = c(num_crop, 3))
area_pval_slope1 = array(dim = c(num_crop, 3))
area_n = array(dim = c(num_crop, 3))
min_neworig_prod = 9000000000
max_neworig_prod = -9999
min_neworig_area = 9000000000
max_neworig_area = -9999
min_faoorig_prod = 9000000000
max_faoorig_prod = -9999
min_faoorig_area = 9000000000
max_faoorig_area = -9999

# production output
# the moirai lds diagnostic outputs have no header lines, but are otherwise like the original format,
#  but with integer country and crop codes, and without being rounded to integers

# now read in the new format output files as data frames
prod_in <- read.csv(prodname, skip = nhead_lds)
#prod_in<-scan(prodname, what=intype, skip=nhead_diag, sep = ",", quote = "\"")
prod_in_orig<-scan(prodname_orig, what=intype_gtap, skip=nhead, sep = ",", quote = "\"")
prod_in_fao<-scan(prodname_fao, what=intype_fao, skip=nhead_fao, sep = ",", quote = "\"")
# harvested area output
area_in <- read.csv(areaname, skip = nhead_lds)
#area_in<-scan(areaname, what=intype, skip=nhead_diag, sep = ",", quote = "\"")
area_in_orig<-scan(areaname_orig, what=intype_gtap, skip=nhead, sep = ",", quote = "\"")
area_in_fao<-scan(areaname_fao, what=intype_fao, skip=nhead_fao, sep = ",", quote = "\"")

# crop names
crop_in = scan(cropfname, what=intype_crop, skip=nhead_crop, sep = ",", quote = "\"")
crop_names = str_trim(unlist(crop_in[4])[1:num_crop])

# gtap country mapping info
temp<-scan(countryfname_gtap, what=as.list(character(5)), skip=1, sep = ",", quote = "\"")
countrycode_gtap = as.integer(unlist(temp[1]))
countryabbr_gtap = unlist(temp[2])
countryname_gtap = unlist(temp[3])
country87code_gtap = as.integer(unlist(temp[4]))
country87abbr_gtap = unlist(temp[5])

# fao gtap gcam country mapping info
temp<-scan(countrymapfname, what=as.list(character(5)), skip=1, sep = ",", quote = "\"")
countrymapcode_fao = as.integer(unlist(temp[1]))
countrymapname_fao = unlist(temp[2])
countrymapcode_gtap = as.integer(unlist(temp[3]))
countrymapabbr_gtap = unlist(temp[4])
countrymapname_gtap = unlist(temp[5])
countrymapcode_gcam = as.integer(unlist(temp[6]))
countrymapname_gcam = unlist(temp[7])

# moirai lds iso country info
# not all of the countries in this mapping file are in the moirai lds output
temp<-scan(countryfname_lds, what=as.list(character(5)), skip=1, sep = ",", quote = "\"")
countrycode_lds_all = as.integer(unlist(temp[1]))
countryabbr_lds_all = unlist(temp[2])
countryname_lds_all = unlist(temp[3])
ctry87code_lds_all = as.integer(unlist(temp[4]))
ctry87abbr_lds_all = unlist(temp[5])
ctry_codes = array(dim = num_ctry)
ctry_abbrs = array(dim = num_ctry)
ctry_names = array(dim = num_ctry)
ctry_codes[] = NA
ctry_abbrs[] = NA
ctry_names[] = NA
ctry_index = 0
for(i in 1:length(countrycode_lds_all)) {
	if(ctry87code_lds_all[i] != -1) {
		ctry_index = ctry_index + 1
		ctry_codes[ctry_index] = countrycode_lds_all[i]
		ctry_abbrs[ctry_index] = countryabbr_lds_all[i]
		ctry_names[ctry_index] = countryname_lds_all[i]
	}
}

if(ctry_index != num_ctry) {
	cat("Error: did not fill moirai lds ctry info correctly\n")
}

# put the data in 3d arrays

cat("filling moirai lds 3d arrays\n")

for(j in 1:num_ctry) {
	for(k in 1:num_crop) {
		s = subset(prod_in, ctry_iso == ctry_abbrs[j] & SAGE_crop == crop_names[k])
		v = s$value
		a = s$glu_code
		if(length(v) > 0) {prod[j, k, a] = v}
		s = subset(area_in, ctry_iso == ctry_abbrs[j] & SAGE_crop == crop_names[k])
		v = s$value
		a = s$glu_code
		if(length(v) > 0) {area[j, k, a] = v}
	}	# end for k loop over number of crops
}	# end for j loop over number of countries

cat("filling genaezecon/orig 3d arrays\n")
for(i in 1:num_aez_ref) {
	prod_col_orig = as.double(unlist(prod_in_orig[i+num_colskip]))
	area_col_orig = as.double(unlist(area_in_orig[i+num_colskip]))
	for(j in 1:num_ctry_ref) {
		# get the moirai lds index and process only if it is in the moirai lds list (which it should be)
		lds_index = 0
		for(k in 1:num_ctry) {
			if(countryabbr_gtap[j] == ctry_abbrs[k]) {
				lds_index = k
				break
			}
		}
		if(lds_index != 0) {
			for(k in 1:num_crop) {
				in_index = (j - 1) * num_crop + k
				prod_orig[lds_index, k, i] = prod_col_orig[in_index]
				area_orig[lds_index, k, i] = area_col_orig[in_index]
				if(prod_col_orig[in_index] != 0) {
				}		
			}	# end for k loop over number of crops
		}
	}	# end for j loop over number of countries
}	# end for i loop over aez columns

# get sums to average the 7 years surrounding year 2000 for FAO,
#  and aggregate to GTAP country here because need to keep track if multiple FAO countries go to one GTAP country
# production average is not area weighted; only do this for yield data
# this is using the diagnostic output of genaezecon, so there should not be any NA values
# store the annual values for diagnostics
cat("aggregating fao arrays\n")
for(i in 1:num_faoavg) {
	prod_col_fao = as.double(unlist(prod_in_fao[(i-1) + fao_startavg+num_colskip]))
	area_col_fao = as.double(unlist(area_in_fao[(i-1) + fao_startavg+num_colskip])) * 100	# convert from km^2 to ha
	for(j in 1:num_ctry_fao) {
		# determine the moirai lds country index, and process fao data if it exists
		lds_index = 0
		for(l in 1:num_ctry) {
			if(countrymapcode_fao[j] == ctry_codes[l]) {
				lds_index = l
				break
			}
		}	# end for l loop to get moirai lds index
		if(lds_index != 0) {
			for(k in 1:num_crop) {
				in_index = (j - 1) * num_crop + k
				# do area first
				if(area_col_fao[in_index] != 0) {
					year_area_ctry_fao[i, lds_index, k] = year_area_ctry_fao[i, lds_index, k] + area_col_fao[in_index]
					# determine the number of years to average over
					# need to keep track of multiple fao countries mapping to one gtap country with values in the same year
					if(prev_flag_area[i,lds_index,k] != 1) {
						divisor_faoavg_area[lds_index, k] = divisor_faoavg_area[lds_index, k] + 1
						prev_flag_area[i,lds_index,k] = 1
					}
				}	# end if harvested area for this fao country
				# now do production
				if(prod_col_fao[in_index] != 0) {
					year_prod_ctry_fao[i, lds_index, k] = year_prod_ctry_fao[i, lds_index, k] + prod_col_fao[in_index]
					# determine the number of years to average over
					# need to keep track of multiple fao countries mapping to one gtap country with values in the same year
					if(prev_flag_prod[i,lds_index,k] != 1) {
						divisor_faoavg_prod[lds_index, k] = divisor_faoavg_prod[lds_index, k] + 1
						prev_flag_prod[i,lds_index,k] = 1
					}
				}	# end if production for this fao country
			}	# end for k loop over number of crops
		}	# end if corresponding moirai lds country exists
	}	# end for j loop over number of fao countries
	# sum values across years
	for(j in 1:num_ctry) {
		for(k in 1:num_crop) {
			prod_ctry_fao[j, k] = prod_ctry_fao[j, k] + year_prod_ctry_fao[i, j, k]
			area_ctry_fao[j, k] = area_ctry_fao[j, k] + year_area_ctry_fao[i, j, k]
		}
	}
}	# end for i loop over number of fao years to average

# start the ks test result files here
if(gtap) {
	ksname_prod = paste(outdir, "ks_prod_t_gtap", ctag_ctry, sep="")
	ksname_area = paste(outdir, "ks_harvarea_ha_gtap", ctag_ctry, sep="")
	cat("crop_prod,ds_new_gtap,pv_new_gtap,ds_new_fao,pv_new_fao,ds_gtap_fao,pv_gtap_fao", sep = "", file = ksname_prod)
	cat("crop_harvarea,ds_new_gtap,pv_new_gtap,ds_new_fao,pv_new_fao,ds_gtap_fao,pv_gtap_fao", sep = "", file = ksname_area)
}else {
	ksname_prod = paste(outdir, "ks_prod_t_orig", ctag_ctry, sep="")
	ksname_area = paste(outdir, "ks_harvarea_ha_orig", ctag_ctry, sep="")
	cat("crop_prod,ds_new_orig,pv_new_orig,ds_new_fao,pv_new_fao,ds_orig_fao,pv_orig_fao", sep = "", file = ksname_prod)
	cat("crop_harvarea,ds_new_orig,pv_new_orig,ds_new_fao,pv_new_fao,ds_orig_fao,pv_orig_fao", sep = "", file = ksname_area)
}

# aggregate Moirai LDS and GENAEZECON/GTAP to country and calc stats by crop
# also finalize fao averages
for(j in 1:num_crop) {
	cat("processing crop", j, crop_names[j], "\n")
	if(gtap) {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_gtap", ctag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_gtap", ctag_ctry, sep="")
		cat("fao_code,iso,country,lds,gtap,fao,lds-gtap,lds-fao,gtap-fao,(lds-gtap)/gtap,(lds-fao)/fao,(gtap-fao)/fao", sep = "", file = oname_prod)
		cat("fao_code,iso,country,lds,gtap,fao,lds-gtap,lds-fao,gtap-fao,(lds-gtap)/gtap,(lds-fao)/fao,(gtap-fao)/fao", sep = "", file = oname_area)
	} else {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_orig", ctag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_orig", ctag_ctry, sep="")
		cat("fao_code,iso,country,lds,orig,fao,lds-orig,lds-fao,orig-fao,(lds-orig)/orig,(lds-fao)/fao,(orig-fao)/fao", sep = "", file = oname_prod)
		cat("fao_code,iso,country,lds,orig,fao,lds-orig,lds-fao,orig-fao,(lds-orig)/orig,(lds-fao)/fao,(orig-fao)/fao", sep = "", file = oname_area)
	}
	
	# initialize arrays to store differences for this crop
	prod_diff_ctry_orig[] = 0.0
	area_diff_ctry_orig[] = 0.0
	prod_diff_ctry_fao[] = 0.0
	area_diff_ctry_fao[] = 0.0
	prod_diff_ctry_orig_fao[] = 0.0
	area_diff_ctry_orig_fao[] = 0.0
	prod_pctdiff_ctry_orig[] = 0.0
	area_pctdiff_ctry_orig[] = 0.0
	prod_pctdiff_ctry_fao[] = 0.0
	area_pctdiff_ctry_fao[] = 0.0
	prod_pctdiff_ctry_orig_fao[] = 0.0
	area_pctdiff_ctry_orig_fao[] = 0.0
	
	# arrays to store valid indices for this crop
	prod_inds = NULL
	prod_orig_inds = NULL
	prod_fao_inds = NULL
	prod_diff_orig_inds = NULL
	prod_diff_fao_inds = NULL
	prod_diff_orig_fao_inds = NULL
	area_inds = NULL
	area_orig_inds = NULL
	area_fao_inds = NULL
	area_diff_orig_inds = NULL
	area_diff_fao_inds = NULL
	area_diff_orig_fao_inds = NULL
	
	# count variables for this crop
	prod_num_new_no_orig = 0
	prod_num_fao_no_orig = 0
	prod_num_new_no_fao = 0
	prod_num_orig_no_fao = 0
	prod_num_orig_no_new = 0
	prod_num_fao_no_new = 0
	prod_num_no_new_no_orig = 0
	prod_num_no_new_no_fao = 0
	prod_num_no_orig_no_fao = 0
	area_num_new_no_orig = 0
	area_num_fao_no_orig = 0
	area_num_new_no_fao = 0
	area_num_orig_no_fao = 0
	area_num_orig_no_new = 0
	area_num_fao_no_new = 0
	area_num_no_new_no_orig = 0
	area_num_no_new_no_fao = 0
	area_num_no_orig_no_fao = 0
	
	for(i in 1:num_ctry) {
		# sum over the aezs
		prod_ctry[i, j] = sum(prod[i, j,])
		area_ctry[i, j] = sum(area[i, j,])
		prod_ctry_orig[i, j] = sum(prod_orig[i, j,])
		area_ctry_orig[i, j] = sum(area_orig[i, j,])
		# calc the fao averages
		if(divisor_faoavg_area[i, j] != 0) {
			area_ctry_fao[i, j] = round(area_ctry_fao[i, j] / divisor_faoavg_area[i, j])
		} else {
			area_ctry_fao[i, j] = 0
		}
		if(divisor_faoavg_prod[i, j] != 0) {
			prod_ctry_fao[i, j] = round(prod_ctry_fao[i, j] / divisor_faoavg_prod[i, j])
		} else {
			prod_ctry_fao[i, j] = 0
		}
		
		# find valid indices of absolute values
		# find valid difference indices (both data sets are valid)
		# count the number of mismatched data entries (one data set is zero and the other is not)
		# count the number of both zero data entries
		
		# production
		if(prod_ctry[i, j] != 0) { # new
			prod_inds = c(prod_inds, i)
			if(prod_ctry_orig[i, j] != 0) { # new and orig
				prod_orig_inds = c(prod_orig_inds, i)
				prod_diff_ctry_orig[i] = prod_ctry[i, j] - prod_ctry_orig[i, j]
				prod_pctdiff_ctry_orig[i] = prod_diff_ctry_orig[i] / prod_ctry_orig[i, j] * 100
				prod_diff_orig_inds = c(prod_diff_orig_inds, i)
				if(prod_ctry_fao[i, j] != 0) { # orig and fao (when new)
					prod_diff_ctry_orig_fao[i] = prod_ctry_orig[i, j] - prod_ctry_fao[i, j]
					prod_pctdiff_ctry_orig_fao[i] = prod_diff_ctry_orig_fao[i] / prod_ctry_fao[i, j] * 100
					prod_diff_orig_fao_inds = c(prod_diff_orig_fao_inds, i)
				} else { # orig and no fao (when new)
					prod_num_orig_no_fao = prod_num_orig_no_fao + 1
					prod_diff_ctry_orig_fao[i] = 0
					prod_pctdiff_ctry_orig_fao[i] = 0
				}
			} else { # new and no orig
				prod_num_new_no_orig = prod_num_new_no_orig + 1
				prod_diff_ctry_orig[i] = 0
				prod_pctdiff_ctry_orig[i] = 0
				prod_diff_ctry_orig_fao[i] = 0
				prod_pctdiff_ctry_orig_fao[i] = 0
				if(prod_ctry_fao[i, j] != 0) { # no orig and fao (when new)
					prod_num_fao_no_orig = prod_num_fao_no_orig + 1
				} else { # no orig and no fao (when new)
					prod_num_no_orig_no_fao = prod_num_no_orig_no_fao + 1
				}	
			}
			if(prod_ctry_fao[i, j] != 0) { # new and fao
				prod_fao_inds = c(prod_fao_inds, i)
				prod_diff_ctry_fao[i] = prod_ctry[i, j] - prod_ctry_fao[i, j]
				prod_pctdiff_ctry_fao[i] = prod_diff_ctry_fao[i] / prod_ctry_fao[i, j] * 100
				prod_diff_fao_inds = c(prod_diff_fao_inds, i)
			} else { # new and no fao
				prod_num_new_no_fao = prod_num_new_no_fao + 1
				prod_diff_ctry_fao[i] = 0
				prod_pctdiff_ctry_fao[i] = 0
			}
		} else { # no new
			prod_diff_ctry_orig[i] = 0
			prod_pctdiff_ctry_orig[i] = 0
			prod_diff_ctry_fao[i] = 0
			prod_pctdiff_ctry_fao[i] = 0
			if(prod_ctry_orig[i, j] != 0) { # no new and orig
				prod_orig_inds = c(prod_orig_inds, i)
				prod_num_orig_no_new = prod_num_orig_no_new + 1
				if(prod_ctry_fao[i, j] != 0) { # orig and fao (when no new)
					prod_diff_ctry_orig_fao[i] = prod_ctry_orig[i, j] - prod_ctry_fao[i, j]
					prod_pctdiff_ctry_orig_fao[i] = prod_diff_ctry_orig_fao[i] / prod_ctry_fao[i, j] * 100
					prod_diff_orig_fao_inds = c(prod_diff_orig_fao_inds, i)
				} else { # orig an no fao (when no new)
					prod_num_orig_no_fao = prod_num_orig_no_fao + 1
					prod_diff_ctry_orig_fao[i] = 0
					prod_pctdiff_ctry_orig_fao[i] = 0
				}
			} else { # no new and no orig
				prod_num_no_new_no_orig = prod_num_no_new_no_orig + 1
				prod_diff_ctry_orig_fao[i] = 0
				prod_pctdiff_ctry_orig_fao[i] = 0
				if(prod_ctry_fao[i, j] != 0) { # no orig and fao (when no new)
					prod_num_fao_no_orig = prod_num_fao_no_orig + 1
				} else { # no orig and no fao (when no new)
					prod_num_no_orig_no_fao = prod_num_no_orig_no_fao + 1
				}
			}
			if(prod_ctry_fao[i, j] != 0) { # no new and fao
				prod_fao_inds = c(prod_fao_inds, i)
				prod_num_fao_no_new = prod_num_fao_no_new + 1
			} else { # no new and no fao
				prod_num_no_new_no_fao = prod_num_no_new_no_fao + 1
			}
		}
		
		# area
		if(area_ctry[i, j] != 0) { # new
			area_inds = c(area_inds, i)
			if(area_ctry_orig[i, j] != 0) { # new and orig
				area_orig_inds = c(area_orig_inds, i)
				area_diff_ctry_orig[i] = area_ctry[i, j] - area_ctry_orig[i, j]
				area_pctdiff_ctry_orig[i] = area_diff_ctry_orig[i] / area_ctry_orig[i, j] * 100
				area_diff_orig_inds = c(area_diff_orig_inds, i)
				if(area_ctry_fao[i, j] != 0) { # orig and fao (when new)
					area_diff_ctry_orig_fao[i] = area_ctry_orig[i, j] - area_ctry_fao[i, j]
					area_pctdiff_ctry_orig_fao[i] = area_diff_ctry_orig_fao[i] / area_ctry_fao[i, j] * 100
					area_diff_orig_fao_inds = c(area_diff_orig_fao_inds, i)
				} else { # orig and no fao (when new)
					area_num_orig_no_fao = area_num_orig_no_fao + 1
					area_diff_ctry_orig_fao[i] = 0
					area_pctdiff_ctry_orig_fao[i] = 0
				}
			} else { # new and no orig
				area_num_new_no_orig = area_num_new_no_orig + 1
				area_diff_ctry_orig[i] = 0
				area_pctdiff_ctry_orig[i] = 0
				area_diff_ctry_orig_fao[i] = 0
				area_pctdiff_ctry_orig_fao[i] = 0
				if(area_ctry_fao[i, j] != 0) { # no orig and fao (when new)
					area_num_fao_no_orig = area_num_fao_no_orig + 1
				} else { # no orig and no fao (when new)
					area_num_no_orig_no_fao = area_num_no_orig_no_fao + 1
				}	
			}
			if(area_ctry_fao[i, j] != 0) { # new and fao
				area_fao_inds = c(area_fao_inds, i)
				area_diff_ctry_fao[i] = area_ctry[i, j] - area_ctry_fao[i, j]
				area_pctdiff_ctry_fao[i] = area_diff_ctry_fao[i] / area_ctry_fao[i, j] * 100
				area_diff_fao_inds = c(area_diff_fao_inds, i)
			} else { # new and no fao
				area_num_new_no_fao = area_num_new_no_fao + 1
				area_diff_ctry_fao[i] = 0
				area_pctdiff_ctry_fao[i] = 0
			}
		} else { # no new
			area_diff_ctry_orig[i] = 0
			area_pctdiff_ctry_orig[i] = 0
			area_diff_ctry_fao[i] = 0
			area_pctdiff_ctry_fao[i] = 0
			if(area_ctry_orig[i, j] != 0) { # no new and orig
				area_orig_inds = c(area_orig_inds, i)
				area_num_orig_no_new = area_num_orig_no_new + 1
				if(area_ctry_fao[i, j] != 0) { # orig and fao (when no new)
					area_diff_ctry_orig_fao[i] = area_ctry_orig[i, j] - area_ctry_fao[i, j]
					area_pctdiff_ctry_orig_fao[i] = area_diff_ctry_orig_fao[i] / area_ctry_fao[i, j] * 100
					area_diff_orig_fao_inds = c(area_diff_orig_fao_inds, i)
				} else { # orig an no fao (when no new)
					area_num_orig_no_fao = area_num_orig_no_fao + 1
					area_diff_ctry_orig_fao[i] = 0
					area_pctdiff_ctry_orig_fao[i] = 0
				}
			} else { # no new and no orig
				area_num_no_new_no_orig = area_num_no_new_no_orig + 1
				area_diff_ctry_orig_fao[i] = 0
				area_pctdiff_ctry_orig_fao[i] = 0
				if(area_ctry_fao[i, j] != 0) { # no orig and fao (when no new)
					area_num_fao_no_orig = area_num_fao_no_orig + 1
				} else { # no orig and no fao (when no new)
					area_num_no_orig_no_fao = area_num_no_orig_no_fao + 1
				}
			}
			if(area_ctry_fao[i, j] != 0) { # no new and fao
				area_fao_inds = c(area_fao_inds, i)
				area_num_fao_no_new = area_num_fao_no_new + 1
			} else { # no new and no fao
				area_num_no_new_no_fao = area_num_no_new_no_fao + 1
			}
		}
		
		# add double quotes to a field that includes a comma (this shouldn't be needed)
		out_field = ctry_names[i]
		if(str_detect(out_field, ",")) {
 			out_field = paste("\"", out_field, "\"", sep = "")
 		}		
		# output per country values to csv file
		cat("\n", ctry_codes[i], ",", ctry_abbrs[i], ",", out_field, ",", prod_ctry[i, j], ",", prod_ctry_orig[i, j], ",",
			prod_ctry_fao[i, j], ",", prod_diff_ctry_orig[i], ",", prod_diff_ctry_fao[i], ",", prod_diff_ctry_orig_fao[i], ",",
			round(prod_pctdiff_ctry_orig[i], digits = 2), ",",
			round(prod_pctdiff_ctry_fao[i], digits = 2), ",", round(prod_pctdiff_ctry_orig_fao[i], digits = 2), sep = "",
			file = oname_prod, append = TRUE)
		cat("\n", ctry_codes[i], ",", ctry_abbrs[i], ",", out_field, ",", area_ctry[i, j], ",", area_ctry_orig[i, j], ",",
			area_ctry_fao[i, j], ",", area_diff_ctry_orig[i], ",",
			area_diff_ctry_fao[i], ",", area_diff_ctry_orig_fao[i], ",", round(area_pctdiff_ctry_orig[i], digits = 2), ",",
			round(area_pctdiff_ctry_fao[i], digits = 2), ",", round(area_pctdiff_ctry_orig_fao[i], digits = 2), sep = "",
			file = oname_area, append = TRUE)
	}	# end for i loop over country
	
	# test distributions and generate histograms
	
	if(gtap) {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_gtap", ptag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_gtap", ptag_ctry, sep="")
	} else {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_orig", ptag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_orig", ptag_ctry, sep="")
	}

	# get the valid values of the individual data sets
	
	prod_ctry_valid = prod_ctry[,j][prod_inds]
	prod_ctry_orig_valid = prod_ctry_orig[,j][prod_orig_inds]
	prod_ctry_fao_valid = prod_ctry_fao[,j][prod_fao_inds]

	area_ctry_valid = area_ctry[,j][area_inds]
	area_ctry_orig_valid = area_ctry_orig[,j][area_orig_inds]
	area_ctry_fao_valid = area_ctry_fao[,j][area_fao_inds]
	
	# first use kolmogorov-smirnov to test whether data could be sampled from the same continuous distribution
	# write these results to a separate csv file
	
	# production
	if(length(prod_ctry_valid) > 0 & length(prod_ctry_orig_valid) > 0) {
		ks_prod_new_orig_ctry = ks.test(prod_ctry_valid, prod_ctry_orig_valid)
		ds_prod_new_orig_ctry = round(ks_prod_new_orig_ctry$statistic, digits = 4)
		pv_prod_new_orig_ctry = round(ks_prod_new_orig_ctry$p.value, digits = 4)
	}else {
		ds_prod_new_orig_ctry = -1
		pv_prod_new_orig_ctry = -1
	}
	if(length(prod_ctry_valid) > 0 & length(prod_ctry_fao_valid) > 0) {
		ks_prod_new_fao_ctry = ks.test(prod_ctry_valid, prod_ctry_fao_valid)
		ds_prod_new_fao_ctry = round(ks_prod_new_fao_ctry$statistic, digits = 4)
		pv_prod_new_fao_ctry = round(ks_prod_new_fao_ctry$p.value, digits = 4)
	}else {
		ds_prod_new_fao_ctry = -1
		pv_prod_new_fao_ctry = -1
	}
	if(length(prod_ctry_orig_valid) > 0 & length(prod_ctry_fao_valid) > 0) {
		ks_prod_orig_fao_ctry = ks.test(prod_ctry_orig_valid, prod_ctry_fao_valid)
		ds_prod_orig_fao_ctry = round(ks_prod_orig_fao_ctry$statistic, digits = 4)
		pv_prod_orig_fao_ctry = round(ks_prod_orig_fao_ctry$p.value, digits = 4)
	}else {
		ds_prod_orig_fao_ctry = -1
		pv_prod_orig_fao_ctry = -1
	}
	
	# area
	if(length(area_ctry_valid) > 0 & length(area_ctry_orig_valid) > 0) {
		ks_area_new_orig_ctry = ks.test(area_ctry_valid, area_ctry_orig_valid)
		ds_area_new_orig_ctry = round(ks_area_new_orig_ctry$statistic, digits = 4)
		pv_area_new_orig_ctry = round(ks_area_new_orig_ctry$p.value, digits = 4)
	}else {
		ds_area_new_orig_ctry = -1
		pv_area_new_orig_ctry = -1
	}
	if(length(area_ctry_valid) > 0 & length(area_ctry_fao_valid) > 0) {
		ks_area_new_fao_ctry = ks.test(area_ctry_valid, area_ctry_fao_valid)
		ds_area_new_fao_ctry = round(ks_area_new_fao_ctry$statistic, digits = 4)
		pv_area_new_fao_ctry = round(ks_area_new_fao_ctry$p.value, digits = 4)
	}else {
		ds_area_new_fao_ctry = -1
		pv_area_new_fao_ctry = -1
	}
	if(length(area_ctry_orig_valid) > 0 & length(area_ctry_fao_valid) > 0) {
		ks_area_orig_fao_ctry = ks.test(area_ctry_orig_valid, area_ctry_fao_valid)
		ds_area_orig_fao_ctry = round(ks_area_orig_fao_ctry$statistic, digits = 4)
		pv_area_orig_fao_ctry = round(ks_area_orig_fao_ctry$p.value, digits = 4)
	}else {
		ds_area_orig_fao_ctry = -1
		pv_area_orig_fao_ctry = -1
	}
	
	cat("\n", crop_names[j], ",", ds_prod_new_orig_ctry, ",", pv_prod_new_orig_ctry, ",", ds_prod_new_fao_ctry, ",", pv_prod_new_fao_ctry, ",",
		ds_prod_orig_fao_ctry, ",", pv_prod_orig_fao_ctry, sep = "", file = ksname_prod, append = TRUE)
	cat("\n", crop_names[j], ",", ds_area_new_orig_ctry, ",", pv_area_new_orig_ctry, ",", ds_area_new_fao_ctry, ",", pv_area_new_fao_ctry, ",",
		ds_area_orig_fao_ctry, ",", pv_area_orig_fao_ctry, sep = "", file = ksname_area, append = TRUE)
	
	# production histograms
	
	if(length(c(prod_ctry_valid, prod_ctry_orig_valid, prod_ctry_fao_valid)) > 0) {
		xmax = ceiling(max(prod_ctry_valid, prod_ctry_orig_valid, prod_ctry_fao_valid) / 10000.0) * 10000
	} else {
		xmax = 0
	}
	if(xmax == 0) {
		breaks = c(0:1)
	} else {
		breaks = c(0:(xmax / 10000)) * 10000
	}
	
	pdf(file=oname_prod,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	title = paste(crop_names[j], "Moirai")
	num_vals = length(prod_ctry_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		prod_ctry_valid_hist = hist(prod_ctry_valid, main = title, sub = sub, xlab = "Production (t)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "Original")
	num_vals = length(prod_ctry_orig_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		prod_ctry_orig_valid_hist = hist(prod_ctry_orig_valid, main = title, sub = sub, xlab = "Production (t)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "FAO")
	num_vals = length(prod_ctry_fao_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		prod_ctry_fao_valid_hist = hist(prod_ctry_fao_valid, main = title, sub = sub, xlab = "Production (t)",
			breaks = breaks)
	}
	
	# differences
	
	if(length(c(prod_diff_ctry_orig[prod_diff_orig_inds], prod_diff_ctry_fao[prod_diff_fao_inds],
			prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds])) > 0) {
		xmax = ceiling(max(prod_diff_ctry_orig[prod_diff_orig_inds], prod_diff_ctry_fao[prod_diff_fao_inds],
			prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds]) / 1000.0) * 1000
		xmin = floor(min(prod_diff_ctry_orig[prod_diff_orig_inds], prod_diff_ctry_fao[prod_diff_fao_inds],
			prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds]) / 1000.0) * 1000
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 1000):(xmax / 1000)) * 1000
	}
	
	title = paste(crop_names[j], "Moirai - Original")
	num_vals = length(prod_diff_ctry_orig[prod_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_diff_ctry_orig[prod_diff_orig_inds]), digits = 2)
		stddev = round(sd(prod_diff_ctry_orig[prod_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", prod_num_new_no_orig, "; # of missing new values", prod_num_orig_no_new)
		prod_diff_ctry_orig_valid_hist = hist(prod_diff_ctry_orig[prod_diff_orig_inds], main = title, xlab = "Production difference (t)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Moirai - FAO")
	num_vals = length(prod_diff_ctry_fao[prod_diff_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_diff_ctry_fao[prod_diff_fao_inds]), digits = 2)
		stddev = round(sd(prod_diff_ctry_fao[prod_diff_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", prod_num_new_no_fao, "; # of missing new values", prod_num_orig_no_new)
		prod_diff_ctry_fao_valid_hist = hist(prod_diff_ctry_fao[prod_diff_fao_inds], main = title, xlab = "Production difference (t)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - FAO")
	num_vals = length(prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds]), digits = 2)
		stddev = round(sd(prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", prod_num_orig_no_fao, "; # of missing original values", prod_num_fao_no_orig)
		prod_diff_ctry_orig_fao_valid_hist = hist(prod_diff_ctry_orig_fao[prod_diff_orig_fao_inds], main = title, xlab = "Production difference (t)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# percent differences
	
	if(length(prod_pctdiff_ctry_orig[prod_diff_orig_inds]) > 0) {
		xmax = ceiling(max(prod_pctdiff_ctry_orig[prod_diff_orig_inds]) / 2.0) * 2
		xmin = floor(min(prod_pctdiff_ctry_orig[prod_diff_orig_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "Moirai - Original")
	num_vals = length(prod_pctdiff_ctry_orig[prod_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_pctdiff_ctry_orig[prod_diff_orig_inds]), digits = 2)
		stddev = round(sd(prod_pctdiff_ctry_orig[prod_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", prod_num_new_no_orig, "; # of missing new values", prod_num_orig_no_new)
		prod_pctdiff_ctry_orig_valid_hist = hist(prod_pctdiff_ctry_orig[prod_diff_orig_inds], main = title, xlab = "% Production difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	if(length(c(prod_pctdiff_ctry_fao[prod_diff_fao_inds], prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds])) > 0) {
		xmax = ceiling(max(prod_pctdiff_ctry_fao[prod_diff_fao_inds],
			prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds]) / 2.0) * 2
		xmin = floor(min(prod_pctdiff_ctry_fao[prod_diff_fao_inds],
			prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "Moirai - FAO")
	num_vals = length(prod_pctdiff_ctry_fao[prod_diff_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_pctdiff_ctry_fao[prod_diff_fao_inds]), digits = 2)
		stddev = round(sd(prod_pctdiff_ctry_fao[prod_diff_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", prod_num_new_no_fao, "; # of missing new values", prod_num_orig_no_new)
		prod_pctdiff_ctry_fao_valid_hist = hist(prod_pctdiff_ctry_fao[prod_diff_fao_inds], main = title, xlab = "% Production difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - FAO")
	num_vals = length(prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds]), digits = 2)
		stddev = round(sd(prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", prod_num_orig_no_fao, "; # of missing original values", prod_num_fao_no_orig)
		prod_pctdiff_ctry_orig_fao_valid_hist = hist(prod_pctdiff_ctry_orig_fao[prod_diff_orig_fao_inds], main = title, xlab = "% Production difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	dev.off()	# close production histogram file
	
	# harvested area histogram file
	
	if(length(c(area_ctry_valid, area_ctry_orig_valid, area_ctry_fao_valid)) > 0) {
		xmax = ceiling(max(area_ctry_valid, area_ctry_orig_valid, area_ctry_fao_valid) / 10000.0) * 10000
	} else {
		xmax = 0
	}
	if(xmax == 0) {
		breaks = c(0:1)
	} else {
		breaks = c(0:(xmax / 10000)) * 10000
	}
	
	pdf(file=oname_area,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	title = paste(crop_names[j], "Moirai")
	num_vals = length(area_ctry_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		area_ctry_valid_hist = hist(area_ctry_valid, main = title, sub = sub, xlab = "Harvested area (ha)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "Original")
	num_vals = length(area_ctry_orig_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		area_ctry_orig_valid_hist = hist(area_ctry_orig_valid, main = title, sub = sub, xlab = "Harvested area (ha)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "FAO")
	num_vals = length(area_ctry_fao_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		area_ctry_fao_valid_hist = hist(area_ctry_fao_valid, main = title, sub = sub, xlab = "Harvested area (ha)",
			breaks = breaks)
	}
	
	# differences
	
	if(length(c(area_diff_ctry_orig[area_diff_orig_inds], area_diff_ctry_fao[area_diff_fao_inds],
			area_diff_ctry_orig_fao[area_diff_orig_fao_inds])) > 0) {
		xmax = ceiling(max(area_diff_ctry_orig[area_diff_orig_inds], area_diff_ctry_fao[area_diff_fao_inds],
			area_diff_ctry_orig_fao[area_diff_orig_fao_inds]) / 1000.0) * 1000
		xmin = floor(min(area_diff_ctry_orig[area_diff_orig_inds], area_diff_ctry_fao[area_diff_fao_inds],
			area_diff_ctry_orig_fao[area_diff_orig_fao_inds]) / 1000.0) * 1000
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 1000):(xmax / 1000)) * 1000
	}
	
	title = paste(crop_names[j], "Moirai - Original")
	num_vals = length(area_diff_ctry_orig[area_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(area_diff_ctry_orig[area_diff_orig_inds]), digits = 2)
		stddev = round(sd(area_diff_ctry_orig[area_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", area_num_new_no_orig, "; # of missing new values", area_num_orig_no_new)
		area_diff_ctry_orig_valid_hist = hist(area_diff_ctry_orig[area_diff_orig_inds], main = title, xlab = "Harvested area difference (ha)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Moirai - FAO")
	num_vals = length(area_diff_ctry_fao[area_diff_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(area_diff_ctry_fao[area_diff_fao_inds]), digits = 2)
		stddev = round(sd(area_diff_ctry_fao[area_diff_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", area_num_new_no_fao, "; # of missing new values", area_num_orig_no_new)
		area_diff_ctry_fao_valid_hist = hist(area_diff_ctry_fao[area_diff_fao_inds], main = title, xlab = "Harvested area difference (ha)",
			breaks = breaks)
			title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - FAO")
	num_vals = length(area_diff_ctry_orig_fao[area_diff_orig_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(area_diff_ctry_orig_fao[area_diff_orig_fao_inds]), digits = 2)
		stddev = round(sd(area_diff_ctry_orig_fao[area_diff_orig_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", area_num_orig_no_fao, "; # of missing original values", area_num_fao_no_orig)
		area_diff_ctry_orig_fao_valid_hist = hist(area_diff_ctry_orig_fao[area_diff_orig_fao_inds], main = title,
			xlab = "Harvested area difference (ha)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# percent differences
	
	if(length(area_pctdiff_ctry_orig[area_diff_orig_inds]) > 0) {
		xmax = ceiling(max(area_pctdiff_ctry_orig[area_diff_orig_inds]) / 2.0) * 2
		xmin = floor(min(area_pctdiff_ctry_orig[area_diff_orig_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "Moirai - Original")
	num_vals = length(area_pctdiff_ctry_orig[area_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(area_pctdiff_ctry_orig[area_diff_orig_inds]), digits = 2)
		stddev = round(sd(area_pctdiff_ctry_orig[area_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", area_num_new_no_orig, "; # of missing new values", area_num_orig_no_new)
		area_pctdiff_ctry_orig_valid_hist = hist(area_pctdiff_ctry_orig[area_diff_orig_inds], main = title, xlab = "% Harvested area difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	if(length(c(area_pctdiff_ctry_fao[area_diff_fao_inds],
			area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds])) > 0) {
		xmax = ceiling(max(area_pctdiff_ctry_fao[area_diff_fao_inds],
			area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds]) / 2.0) * 2
		xmin = floor(min(area_pctdiff_ctry_fao[area_diff_fao_inds],
			area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "Moirai - FAO")
	num_vals = length(area_pctdiff_ctry_fao[area_diff_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(area_pctdiff_ctry_fao[area_diff_fao_inds]), digits = 2)
		stddev = round(sd(area_pctdiff_ctry_fao[area_diff_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", area_num_new_no_fao, "; # of missing new values", area_num_orig_no_new)
		area_pctdiff_ctry_fao_valid_hist = hist(area_pctdiff_ctry_fao[area_diff_fao_inds], main = title, xlab = "% Harvested area difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - FAO")
	num_vals = length(area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds])
	if(num_vals != 0) {
		avg = round(mean(area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds]), digits = 2)
		stddev = round(sd(area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing fao values", area_num_orig_no_fao, "; # of missing original values", area_num_fao_no_orig)
		area_pctdiff_ctry_orig_fao_valid_hist = hist(area_pctdiff_ctry_orig_fao[area_diff_orig_fao_inds], main = title,
		xlab = "% Harvested area difference", breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	dev.off()	# close harvest area histogram file
	
	#########################
	
	# generate histogram comparison plots
	
	if(gtap) {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_gtap_comp", ptag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_gtap_comp", ptag_ctry, sep="")
	} else {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_orig_comp", ptag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_orig_comp", ptag_ctry, sep="")
	}
	
	# production histogram comparison
	
	pdf(file=oname_prod,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	# plot all 3 absolute value histograms on a linear-log plot with fao as solid line and new and orig as broken lines
	
	new_x = log10(prod_ctry_valid_hist$mids)
	new_y = prod_ctry_valid_hist$counts
	orig_x = log10(prod_ctry_orig_valid_hist$mids)
	orig_y = prod_ctry_orig_valid_hist$counts
	fao_x = log10(prod_ctry_fao_valid_hist$mids)
	fao_y = prod_ctry_fao_valid_hist$counts
	num_bins = length(fao_x)
	bin_widths = log10(prod_ctry_fao_valid_hist$breaks)[3:(num_bins + 1)] - log10(prod_ctry_fao_valid_hist$breaks)[2:num_bins]
	bin_widths = c(fao_x[2] - 0.5 *bin_widths[1], bin_widths)
	
	ymax = max(new_y, orig_y, fao_y)
	xmax = max(new_x, orig_x, fao_x)
	xmin = min(new_x, orig_x, fao_x)
	
	maintitle = paste(crop_names[j], " production histogram comparison", sep = "")
	xlab = "Production (t)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	#barplot(width = bin_widths, height = fao_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "FAO")
	lines(fao_x, fao_y, lty = 1, col = "black", lwd = 2)
	lines(orig_x, orig_y, lty = 4, col = "blue3", lwd = 2)
	lines(new_x, new_y, lty = 2, col = "red3", lwd = 2)
	if(gtap) {
		legend(x = "topright", lty = c(4, 2, 1), legend = c("GTAP", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
	} else {
		legend(x = "topright", lty = c(4, 2, 1), legend = c("Original GLUs", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
	}
	par(new=FALSE)
	
	# plot the above comparison using CDFs
	
	new_x = log10(prod_ctry_valid_hist$mids)
	div = sum(prod_ctry_valid_hist$counts)
	if(div == 0) {div = 1}
	new_y = cumsum(prod_ctry_valid_hist$counts) / div
	orig_x = log10(prod_ctry_orig_valid_hist$mids)
	div = sum(prod_ctry_orig_valid_hist$counts)
	if(div == 0) {div = 1}
	orig_y = cumsum(prod_ctry_orig_valid_hist$counts) / div
	fao_x = log10(prod_ctry_fao_valid_hist$mids)
	div = sum(prod_ctry_fao_valid_hist$counts)
	if(div == 0) {div = 1}
	fao_y = cumsum(prod_ctry_fao_valid_hist$counts) / div
	
	ymax = max(new_y, orig_y, fao_y)
	ymin = 0.0
	xmax = max(new_x, orig_x, fao_x)
	xmin = min(new_x, orig_x, fao_x)
	
	maintitle = paste(crop_names[j], " production cumulative distribution comparison", sep = "")
	xlab = "Production (t)"
	ylab = "Cumulative probability"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	
	if(papergray) {
		lines(fao_x, fao_y, lty = 1, col = "black", lwd = 2)
		lines(orig_x, orig_y, lty = 4, col = "gray60", lwd = 2)
		lines(new_x, new_y, lty = 2, col = "gray30", lwd = 2)
		if(gtap) {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Moirai GLUs", "FAO"), col = c("gray60", "gray30", "black"), lwd = 2)
		} else {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("Original GLUs", "Moirai GLUs", "FAO"), col = c("gray60", "gray30", "black"), lwd = 2)
		}		
	} else {
		lines(fao_x, fao_y, lty = 1, col = "black", lwd = 2)
		lines(orig_x, orig_y, lty = 4, col = "blue3", lwd = 2)
		lines(new_x, new_y, lty = 2, col = "red3", lwd = 2)
		if(gtap) {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
		} else {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("Original GLUs", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
		}
	}
	
	par(new=FALSE)
	
	# plot the difference histograms of new-fao and orig-fao together on linear-log axes
	
	# need to deal with the negatives
	# should skip this for now
	# this blows up for sugar cane j=141???????
	if(TRUE) {
	
	# there shouldn't be any zero mid-bin values
	posinds = which(prod_diff_ctry_orig_valid_hist$mids > 0)
	neginds = which(prod_diff_ctry_orig_valid_hist$mids < 0)
	nod_x = c(-log10(-prod_diff_ctry_orig_valid_hist$mids[neginds]), log10(prod_diff_ctry_orig_valid_hist$mids[posinds]))
	nod_y = prod_diff_ctry_orig_valid_hist$counts
	posinds = which(prod_diff_ctry_fao_valid_hist$mids > 0)
	neginds = which(prod_diff_ctry_fao_valid_hist$mids < 0)
	new_x = c(-log10(-prod_diff_ctry_fao_valid_hist$mids[neginds]), log10(prod_diff_ctry_fao_valid_hist$mids[posinds]))
	new_x_labels = prod_diff_ctry_fao_valid_hist$mids
	new_y = prod_diff_ctry_fao_valid_hist$counts
	posinds = which(prod_diff_ctry_orig_fao_valid_hist$mids > 0)
	neginds = which(prod_diff_ctry_orig_fao_valid_hist$mids < 0)
	orig_x = c(-log10(-prod_diff_ctry_orig_fao_valid_hist$mids[neginds]), log10(prod_diff_ctry_orig_fao_valid_hist$mids[posinds]))
	orig_y = prod_diff_ctry_orig_fao_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(crop_names[j], " production difference histogram comparison", sep = "")
	xlab = "Production difference (t)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = new_x_labels)
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GENAEZECON - FAO")
	lines(new_x, new_y, lty = 1, col = "red3", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "blue3", lwd = 2)
	if(gtap) {
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
	}
	par(new=FALSE)
	

	
	# plot the % difference histograms of new-fao and orig-fao together on linear axes
	
	new_x = prod_pctdiff_ctry_fao_valid_hist$mids
	new_y = prod_pctdiff_ctry_fao_valid_hist$counts
	orig_x = prod_pctdiff_ctry_orig_fao_valid_hist$mids
	orig_y = prod_pctdiff_ctry_orig_fao_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(crop_names[j], " % production difference histogram comparison", sep = "")
	xlab = "% production difference"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	par(new=TRUE)
	if (papergray) {
		lines(new_x, new_y, lty = 1, col = "gray30", lwd = 2)
		lines(orig_x, orig_y, lty = 2, col = "gray60", lwd = 2)
		if(gtap) {
			legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Moirai GLUs - FAO"), col = c("gray60", "gray30"), lwd = 2)
		} else {
			legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
		}
	} else {
		#barplot(width = rep(2, num_bins), height = new_y, yaxt = "n", xaxt = "n",
		#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GENAEZECON - FAO")
		lines(new_x, new_y, lty = 1, col = "red3", lwd = 2)
		lines(orig_x, orig_y, lty = 2, col = "blue3", lwd = 2)
		if(gtap) {
			legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
		} else {
			legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
		}
	}
	
	par(new=FALSE)
	
	}	 # end if(j != 141)
	
	dev.off()	# close production histogram comparison file

	# harvested area histogram comparison
	
	pdf(file=oname_area,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	# plot all 3 absolute value histograms on a linear-log plot with fao as solid line and new and orig as broken lines
	
	new_x = log10(area_ctry_valid_hist$mids)
	new_y = area_ctry_valid_hist$counts
	orig_x = log10(area_ctry_orig_valid_hist$mids)
	orig_y = area_ctry_orig_valid_hist$counts
	fao_x = log10(area_ctry_fao_valid_hist$mids)
	fao_y = area_ctry_fao_valid_hist$counts
	num_bins = length(fao_x)
	
	ymax = max(new_y, orig_y, fao_y)
	xmax = max(new_x, orig_x, fao_x)
	xmin = min(new_x, orig_x, fao_x)
	
	maintitle = paste(crop_names[j], " harvested area histogram comparison", sep = "")
	xlab = "Harvested area (ha)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
		axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = fao_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "FAO")
	lines(fao_x, fao_y, lty = 1, col = "black", lwd = 2)
	lines(orig_x, orig_y, lty = 4, col = "blue3", lwd = 2)
	lines(new_x, new_y, lty = 2, col = "red3", lwd = 2)
	if(gtap) {
		legend(x = "topright", lty = c(4, 2, 1), legend = c("GTAP", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
	} else {
		legend(x = "topright", lty = c(4, 2, 1), legend = c("Original GLUs", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
	}
	par(new=FALSE)
	
	# plot the above comparison using CDFs
	
	new_x = log10(area_ctry_valid_hist$mids)
	div = sum(area_ctry_valid_hist$counts)
	if(div == 0) {div = 1}
	new_y = cumsum(area_ctry_valid_hist$counts) / div
	orig_x = log10(area_ctry_orig_valid_hist$mids)
	div = sum(area_ctry_orig_valid_hist$counts)
	if(div == 0) {div = 1}
	orig_y = cumsum(area_ctry_orig_valid_hist$counts) / div
	fao_x = log10(area_ctry_fao_valid_hist$mids)
	div = sum(area_ctry_fao_valid_hist$counts)
	if(div == 0) {div = 1}
	fao_y = cumsum(area_ctry_fao_valid_hist$counts) / div
	
	ymax = max(new_y, orig_y, fao_y)
	ymin = 0.0
	xmax = max(new_x, orig_x, fao_x)
	xmin = min(new_x, orig_x, fao_x)
	
	maintitle = paste(crop_names[j], " harvested area cumulative distribution comparison", sep = "")
	xlab = "Harvested area (ha)"
	ylab = "Cumulative probability"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)

	if(papergray) {
		lines(fao_x, fao_y, lty = 1, col = "black", lwd = 2)
		lines(orig_x, orig_y, lty = 4, col = "gray60", lwd = 2)
		lines(new_x, new_y, lty = 2, col = "gray30", lwd = 2)
		if(gtap) {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Moirai GLUs", "FAO"), col = c("gray60", "gray30", "black"), lwd = 2)
		} else {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("Original GLUs", "Moirai GLUs", "FAO"), col = c("gray60", "gray30", "black"), lwd = 2)
		}		
	} else {
		lines(fao_x, fao_y, lty = 1, col = "black", lwd = 2)
		lines(orig_x, orig_y, lty = 4, col = "blue3", lwd = 2)
		lines(new_x, new_y, lty = 2, col = "red3", lwd = 2)
		if(gtap) {	
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
		} else {
			legend(x = "bottomright", lty = c(4, 2, 1), legend = c("Original GLUs", "Moirai GLUs", "FAO"), col = c("blue3", "red3", "black"), lwd = 2)
		}
	}
	par(new=FALSE)
	
	# plot the difference histograms of new-fao and orig-fao together on linear-log axes
	
	# need to deal with the negatives
	# should skip this for now
	if(TRUE) {
	
	# there shouldn't be any zero mid-bin values
	posinds = which(area_diff_ctry_orig_valid_hist$mids > 0)
	neginds = which(area_diff_ctry_orig_valid_hist$mids < 0)
	nod_x = c(-log10(-area_diff_ctry_orig_valid_hist$mids[neginds]), log10(area_diff_ctry_orig_valid_hist$mids[posinds]))
	nod_y = area_diff_ctry_orig_valid_hist$counts
	posinds = which(area_diff_ctry_fao_valid_hist$mids > 0)
	neginds = which(area_diff_ctry_fao_valid_hist$mids < 0)
	new_x = c(-log10(-area_diff_ctry_fao_valid_hist$mids[neginds]), log10(area_diff_ctry_fao_valid_hist$mids[posinds]))
	new_x_labels = area_diff_ctry_fao_valid_hist$mids
	new_y = area_diff_ctry_fao_valid_hist$counts
	posinds = which(area_diff_ctry_orig_fao_valid_hist$mids > 0)
	neginds = which(area_diff_ctry_orig_fao_valid_hist$mids < 0)
	orig_x = c(-log10(-area_diff_ctry_orig_fao_valid_hist$mids[neginds]), log10(area_diff_ctry_orig_fao_valid_hist$mids[posinds]))
	orig_y = area_diff_ctry_orig_fao_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(crop_names[j], " harvested area difference histogram comparison", sep = "")
	xlab = "Harvested area (ha)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = new_x_labels)
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GENAEZECON - FAO")
	lines(new_x, new_y, lty = 1, col = "red3", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "blue3", lwd = 2)
	if(gtap) {
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
	}
	par(new=FALSE)
	
	
	
	# plot the % difference histograms of new-orig and new-fao and orig-fao together on linear axes
	
	new_x = area_pctdiff_ctry_fao_valid_hist$mids
	new_y = area_pctdiff_ctry_fao_valid_hist$counts
	orig_x = area_pctdiff_ctry_orig_fao_valid_hist$mids
	orig_y = area_pctdiff_ctry_orig_fao_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(crop_names[j], " % harvested area difference histogram comparison", sep = "")
	xlab = "% harvested area difference"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	par(new=TRUE)
	#barplot(width = rep(2, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GENAEZECON - FAO")
	lines(new_x, new_y, lty = 1, col = "red3", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "blue3", lwd = 2)
	if(gtap) {
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"), lwd = 2)
	}
	par(new=FALSE)
	
	}	 # end if(FALSE)
	
	dev.off()	# close harvested area histogram comparison file
	
	######################################################
	# scatter plots across the pairs of datasets for production and area
	
	if(gtap) {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_gtap_scatter", ptag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_gtap_scatter", ptag_ctry, sep="")
	} else {
		oname_prod = paste(outdir, crop_names[j], "_prod_t_orig_scatter", ptag_ctry, sep="")
		oname_area = paste(outdir, crop_names[j], "_harvarea_ha_orig_scatter", ptag_ctry, sep="")
	}
	
	########################################################
	# scatter plots of production across the three data sets
	
	pdf(file=oname_prod,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	new_orig_prod_inds = intersect(prod_inds, prod_orig_inds)
	if(length(new_orig_prod_inds) > 0) {
	
		title = paste("New vs Orig", crop_names[j], "production")
		xlab = "Orig Production (t)"
		ylab = "New Production (t)"
		xmin = min(prod_ctry_orig[new_orig_prod_inds,j], prod_ctry[new_orig_prod_inds,j])
		xmax = max(prod_ctry_orig[new_orig_prod_inds,j], prod_ctry[new_orig_prod_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = prod_ctry_orig[new_orig_prod_inds,j], y = prod_ctry[new_orig_prod_inds,j], type = "n",
			xlab = xlab, main = title, ylab = ylab,
			ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(prod_ctry_orig[new_orig_prod_inds,j], prod_ctry[new_orig_prod_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
	
		if (xmin < min_neworig_prod) { min_neworig_prod = xmin }
		if (xmax > max_neworig_prod) { max_neworig_prod = xmax }
	
		# regression
		if(length(new_orig_prod_inds) > 1)
		{
			n = length(new_orig_prod_inds)
			yvals = prod_ctry[new_orig_prod_inds,j]
			xvals = prod_ctry_orig[new_orig_prod_inds,j]
			fit = lm(yvals ~ xvals, na.action = na.omit)
			confint = confint(fit, level = 0.95)
			smry = summary(fit, correlation = TRUE, signif.stars = TRUE)
			reg_lab = paste("y =", round(fit$coefficients[1], digits = 2), "+",
				round(fit$coefficients[2], digits = 3), "* x", sep=" ")
			p_lab = paste("Zero slope p-value = ",
				signif(smry$coefficients[2,4], digits = 4))
			adj_r2_lab = paste("Adj. r^2 = ", round(smry$adj.r.squared, digits = 3))
			r2_lab = paste("r^2 = ", round(smry$r.squared, digits = 3))
			n_lab = paste("n = ", round(n))
			fit_line = fit$coefficients[1] + fit$coefficients[2] * xvals
			one_lab = "One to one line"
	
			# calculate the t-value and p-value for slope = 1
			# t = (estimated slope - 1) / standard error of slope estimate
			# pt(abs(t),lower.tail=FALSE) returns the one-sided prob > |t|
			slope = fit$coefficients[2]
			slope_stderr = smry$coefficients[2,2]
			t1 = (slope - 1) / slope_stderr
			p1 = pt(abs(t1), df=(n - 2), lower.tail=FALSE) * 2
			p1_lab = paste("One slope p-value = ", signif(p1, digits = 4))
						
			# store the regression model
			prod_intercept[j, neworigind] = smry$coefficients[1,1]
			prod_intercepterr[j, neworigind] = smry$coefficients[1,2]
			prod_slope[j, neworigind] = smry$coefficients[2,1]
			prod_slopeerr[j, neworigind] = smry$coefficients[2,2]
			prod_r2[j, neworigind] = smry$r.squared
			prod_pval_slope1[j, neworigind] = p1
			prod_n[j, neworigind] = n
		}
		else
		{
			reg_lab = NA
			p_lab = NA
			adj_r2_lab = NA
			r2_lab = NA
			n_lab = NA
			fit_line = rep(0, n)
			one_lab = "One to one line"
			p1_lab = NA
						
			prod_intercept[j, neworigind] = NA
			prod_intercepterr[j, neworigind] = NA
			prod_slope[j, neworigind] = NA
			prod_slopeerr[j, neworigind] = NA
			prod_r2[j, neworigind] = NA
			prod_pval_slope1[j, neworigind] = NA
			prod_n[j, neworigind] = NA
		} # end regression
	
	} # end if data exist to plot
	
	new_fao_prod_inds = intersect(prod_inds, prod_fao_inds)
	if(length(new_fao_prod_inds) > 0) {
	
		title = paste("New vs FAO", crop_names[j], "production")
		xlab = "FAO Production (t)"
		ylab = "New Production (t)"
		xmin = min(prod_ctry_fao[new_fao_prod_inds,j], prod_ctry[new_fao_prod_inds,j])
		xmax = max(prod_ctry_fao[new_fao_prod_inds,j], prod_ctry[new_fao_prod_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = prod_ctry_fao[new_fao_prod_inds,j], y = prod_ctry[new_fao_prod_inds,j], type = "n",
			xlab = xlab, main = title, ylab = ylab,
			ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(prod_ctry_fao[new_fao_prod_inds,j], prod_ctry[new_fao_prod_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
		
		# regression
		if(length(new_fao_prod_inds) > 1)
		{
			n = length(new_fao_prod_inds)
			yvals = prod_ctry[new_fao_prod_inds,j]
			xvals = prod_ctry_fao[new_fao_prod_inds,j]
			fit = lm(yvals ~ xvals, na.action = na.omit)
			confint = confint(fit, level = 0.95)
			smry = summary(fit, correlation = TRUE, signif.stars = TRUE)
			reg_lab = paste("y =", round(fit$coefficients[1], digits = 2), "+",
				round(fit$coefficients[2], digits = 3), "* x", sep=" ")
			p_lab = paste("Zero slope p-value = ",
				signif(smry$coefficients[2,4], digits = 4))
			adj_r2_lab = paste("Adj. r^2 = ", round(smry$adj.r.squared, digits = 3))
			r2_lab = paste("r^2 = ", round(smry$r.squared, digits = 3))
			n_lab = paste("n = ", round(n))
			fit_line = fit$coefficients[1] + fit$coefficients[2] * xvals
			one_lab = "One to one line"
	
			# calculate the t-value and p-value for slope = 1
			# t = (estimated slope - 1) / standard error of slope estimate
			# pt(abs(t),lower.tail=FALSE) returns the one-sided prob > |t|
			slope = fit$coefficients[2]
			slope_stderr = smry$coefficients[2,2]
			t1 = (slope - 1) / slope_stderr
			p1 = pt(abs(t1), df=(n - 2), lower.tail=FALSE) * 2
			p1_lab = paste("One slope p-value = ", signif(p1, digits = 4))
						
			# store the regression model
			prod_intercept[j, newfaoind] = smry$coefficients[1,1]
			prod_intercepterr[j, newfaoind] = smry$coefficients[1,2]
			prod_slope[j, newfaoind] = smry$coefficients[2,1]
			prod_slopeerr[j, newfaoind] = smry$coefficients[2,2]
			prod_r2[j, newfaoind] = smry$r.squared
			prod_pval_slope1[j, newfaoind] = p1
			prod_n[j, newfaoind] = n
		}
		else
		{
			reg_lab = NA
			p_lab = NA
			adj_r2_lab = NA
			r2_lab = NA
			n_lab = NA
			fit_line = rep(0, n)
			one_lab = "One to one line"
			p1_lab = NA
						
			prod_intercept[j, newfaoind] = NA
			prod_intercepterr[j, newfaoind] = NA
			prod_slope[j, newfaoind] = NA
			prod_slopeerr[j, newfaoind] = NA
			prod_r2[j, newfaoind] = NA
			prod_pval_slope1[j, newfaoind] = NA
			prod_n[j, newfaoind] = NA
		} # end regression
	
	} # end if data exist to plot
	
	orig_fao_prod_inds = intersect(prod_orig_inds, prod_fao_inds)
	if(length(orig_fao_prod_inds) > 0) {
	
		title = paste("FAO vs Orig", crop_names[j], "production")
		ylab = "FAO Production (t)"
		xlab = "Orig Production (t)"
		xmin = min(prod_ctry_fao[orig_fao_prod_inds,j], prod_ctry_orig[orig_fao_prod_inds,j])
		xmax = max(prod_ctry_fao[orig_fao_prod_inds,j], prod_ctry_orig[orig_fao_prod_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = prod_ctry_orig[orig_fao_prod_inds,j], y = prod_ctry_fao[orig_fao_prod_inds,j], type = "n",
			xlab = xlab, main = title, ylab = ylab,
			ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(prod_ctry_orig[orig_fao_prod_inds,j], prod_ctry_fao[orig_fao_prod_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
	
		if (xmin < min_faoorig_prod) { min_faoorig_prod = xmin }
		if (xmax > max_faoorig_prod) { max_faoorig_prod = xmax }
	
		# regression
		if(length(orig_fao_prod_inds) > 1)
		{
			n = length(orig_fao_prod_inds)
			xvals = prod_ctry_orig[orig_fao_prod_inds,j]
			yvals = prod_ctry_fao[orig_fao_prod_inds,j]
			fit = lm(yvals ~ xvals, na.action = na.omit)
			confint = confint(fit, level = 0.95)
			smry = summary(fit, correlation = TRUE, signif.stars = TRUE)
			reg_lab = paste("y =", round(fit$coefficients[1], digits = 2), "+",
				round(fit$coefficients[2], digits = 3), "* x", sep=" ")
			p_lab = paste("Zero slope p-value = ",
				signif(smry$coefficients[2,4], digits = 4))
			adj_r2_lab = paste("Adj. r^2 = ", round(smry$adj.r.squared, digits = 3))
			r2_lab = paste("r^2 = ", round(smry$r.squared, digits = 3))
			n_lab = paste("n = ", round(n))
			fit_line = fit$coefficients[1] + fit$coefficients[2] * xvals
			one_lab = "One to one line"
	
			# calculate the t-value and p-value for slope = 1
			# t = (estimated slope - 1) / standard error of slope estimate
			# pt(abs(t),lower.tail=FALSE) returns the one-sided prob > |t|
			slope = fit$coefficients[2]
			slope_stderr = smry$coefficients[2,2]
			t1 = (slope - 1) / slope_stderr
			p1 = pt(abs(t1), df=(n - 2), lower.tail=FALSE) * 2
			p1_lab = paste("One slope p-value = ", signif(p1, digits = 4))
						
			# store the regression model
			prod_intercept[j, faoorigind] = smry$coefficients[1,1]
			prod_intercepterr[j, faoorigind] = smry$coefficients[1,2]
			prod_slope[j, faoorigind] = smry$coefficients[2,1]
			prod_slopeerr[j, faoorigind] = smry$coefficients[2,2]
			prod_r2[j, faoorigind] = smry$r.squared
			prod_pval_slope1[j, faoorigind] = p1
			prod_n[j, faoorigind] = n
		}
		else
		{
			reg_lab = NA
			p_lab = NA
			adj_r2_lab = NA
			r2_lab = NA
			n_lab = NA
			fit_line = rep(0, n)
			one_lab = "One to one line"
			p1_lab = NA
						
			prod_intercept[j, faoorigind] = NA
			prod_intercepterr[j, faoorigind] = NA
			prod_slope[j, faoorigind] = NA
			prod_slopeerr[j, faoorigind] = NA
			prod_r2[j, faoorigind] = NA
			prod_pval_slope1[j, faoorigind] = NA
			prod_n[j, faoorigind] = NA
		} # end regression
	
	} # end if data exist to plot
	
	dev.off()
	
	############################################################
	# scatter plots of harvested area across the three data sets
	
	pdf(file=oname_area,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	new_orig_area_inds = intersect(area_inds, area_orig_inds)
	if(length(new_orig_area_inds) > 0) {
	
		title = paste("New vs Orig", crop_names[j], "harvested area")
		xlab = "Orig Harvested area (ha)"
		ylab = "New Harvested area (ha)"
		xmin = min(area_ctry_orig[new_orig_area_inds,j], area_ctry[new_orig_area_inds,j])
		xmax = max(area_ctry_orig[new_orig_area_inds,j], area_ctry[new_orig_area_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = area_ctry_orig[new_orig_area_inds,j], y = area_ctry[new_orig_area_inds,j], type = "n",
			xlab = xlab, main = title, ylab = ylab,
			ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(area_ctry_orig[new_orig_area_inds,j], area_ctry[new_orig_area_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
	
		if (xmin < min_neworig_area) { min_neworig_area = xmin }
		if (xmax > max_neworig_area) { max_neworig_area = xmax }
		
		# regression
		if(length(new_orig_area_inds) > 1)
		{
			n = length(new_orig_area_inds)
			yvals = area_ctry[new_orig_area_inds,j]
			xvals = area_ctry_orig[new_orig_area_inds,j]
			fit = lm(yvals ~ xvals, na.action = na.omit)
			confint = confint(fit, level = 0.95)
			smry = summary(fit, correlation = TRUE, signif.stars = TRUE)
			reg_lab = paste("y =", round(fit$coefficients[1], digits = 2), "+",
				round(fit$coefficients[2], digits = 3), "* x", sep=" ")
			p_lab = paste("Zero slope p-value = ",
				signif(smry$coefficients[2,4], digits = 4))
			adj_r2_lab = paste("Adj. r^2 = ", round(smry$adj.r.squared, digits = 3))
			r2_lab = paste("r^2 = ", round(smry$r.squared, digits = 3))
			n_lab = paste("n = ", round(n))
			fit_line = fit$coefficients[1] + fit$coefficients[2] * xvals
			one_lab = "One to one line"
	
			# calculate the t-value and p-value for slope = 1
			# t = (estimated slope - 1) / standard error of slope estimate
			# pt(abs(t),lower.tail=FALSE) returns the one-sided prob > |t|
			slope = fit$coefficients[2]
			slope_stderr = smry$coefficients[2,2]
			t1 = (slope - 1) / slope_stderr
			p1 = pt(abs(t1), df=(n - 2), lower.tail=FALSE) * 2
			p1_lab = paste("One slope p-value = ", signif(p1, digits = 4))
						
			# store the regression model
			area_intercept[j, neworigind] = smry$coefficients[1,1]
			area_intercepterr[j, neworigind] = smry$coefficients[1,2]
			area_slope[j, neworigind] = smry$coefficients[2,1]
			area_slopeerr[j, neworigind] = smry$coefficients[2,2]
			area_r2[j, neworigind] = smry$r.squared
			area_pval_slope1[j, neworigind] = p1
			area_n[j, neworigind] = n
		}
		else
		{
			reg_lab = NA
			p_lab = NA
			adj_r2_lab = NA
			r2_lab = NA
			n_lab = NA
			fit_line = rep(0, n)
			one_lab = "One to one line"
			p1_lab = NA
						
			area_intercept[j, neworigind] = NA
			area_intercepterr[j, neworigind] = NA
			area_slope[j, neworigind] = NA
			area_slopeerr[j, neworigind] = NA
			area_r2[j, neworigind] = NA
			area_pval_slope1[j, neworigind] = NA
			area_n[j, neworigind] = NA
		} # end regression
	
	} # end if data exist to plot
	
	new_fao_area_inds = intersect(area_inds, area_fao_inds)
	if(length(new_fao_area_inds) > 0) {
	
		title = paste("New vs FAO", crop_names[j], "harvested area")
		xlab = "FAO Harvested area (ha)"
		ylab = "New Harvested area (ha)"
		xmin = min(area_ctry_fao[new_fao_area_inds,j], area_ctry[new_fao_area_inds,j])
		xmax = max(area_ctry_fao[new_fao_area_inds,j], area_ctry[new_fao_area_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = area_ctry_fao[new_fao_area_inds,j], y = area_ctry[new_fao_area_inds,j], type = "n",
			xlab = xlab, main = title, ylab = ylab,
			ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(area_ctry_fao[new_fao_area_inds,j], area_ctry[new_fao_area_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
		
		# regression
		if(length(new_fao_area_inds) > 1)
		{
			n = length(new_fao_area_inds)
			yvals = area_ctry[new_fao_area_inds,j]
			xvals = area_ctry_fao[new_fao_area_inds,j]
			fit = lm(yvals ~ xvals, na.action = na.omit)
			confint = confint(fit, level = 0.95)
			smry = summary(fit, correlation = TRUE, signif.stars = TRUE)
			reg_lab = paste("y =", round(fit$coefficients[1], digits = 2), "+",
				round(fit$coefficients[2], digits = 3), "* x", sep=" ")
			p_lab = paste("Zero slope p-value = ",
				signif(smry$coefficients[2,4], digits = 4))
			adj_r2_lab = paste("Adj. r^2 = ", round(smry$adj.r.squared, digits = 3))
			r2_lab = paste("r^2 = ", round(smry$r.squared, digits = 3))
			n_lab = paste("n = ", round(n))
			fit_line = fit$coefficients[1] + fit$coefficients[2] * xvals
			one_lab = "One to one line"
	
			# calculate the t-value and p-value for slope = 1
			# t = (estimated slope - 1) / standard error of slope estimate
			# pt(abs(t),lower.tail=FALSE) returns the one-sided prob > |t|
			slope = fit$coefficients[2]
			slope_stderr = smry$coefficients[2,2]
			t1 = (slope - 1) / slope_stderr
			p1 = pt(abs(t1), df=(n - 2), lower.tail=FALSE) * 2
			p1_lab = paste("One slope p-value = ", signif(p1, digits = 4))
						
			# store the regression model
			area_intercept[j, newfaoind] = smry$coefficients[1,1]
			area_intercepterr[j, newfaoind] = smry$coefficients[1,2]
			area_slope[j, newfaoind] = smry$coefficients[2,1]
			area_slopeerr[j, newfaoind] = smry$coefficients[2,2]
			area_r2[j, newfaoind] = smry$r.squared
			area_pval_slope1[j, newfaoind] = p1
			area_n[j, newfaoind] = n
		}
		else
		{
			reg_lab = NA
			p_lab = NA
			adj_r2_lab = NA
			r2_lab = NA
			n_lab = NA
			fit_line = rep(0, n)
			one_lab = "One to one line"
			p1_lab = NA
						
			area_intercept[j, newfaoind] = NA
			area_intercepterr[j, newfaoind] = NA
			area_slope[j, newfaoind] = NA
			area_slopeerr[j, newfaoind] = NA
			area_r2[j, newfaoind] = NA
			area_pval_slope1[j, newfaoind] = NA
			area_n[j, newfaoind] = NA
		} # end regression
	
	} # end if data exist to plot
	
	orig_fao_area_inds = intersect(area_orig_inds, area_fao_inds)
	if(length(orig_fao_area_inds) > 0) {
	
		title = paste("FAO vs Orig", crop_names[j], "harvested area")
		ylab = "FAO Harvested area (ha)"
		xlab = "Orig Harvested area (ha)"
		xmin = min(area_ctry_fao[orig_fao_area_inds,j], area_ctry_orig[orig_fao_area_inds,j])
		xmax = max(area_ctry_fao[orig_fao_area_inds,j], area_ctry_orig[orig_fao_area_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = area_ctry_orig[orig_fao_area_inds,j], y = area_ctry_fao[orig_fao_area_inds,j], type = "n",
			xlab = xlab, main = title, ylab = ylab,
			ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(area_ctry_orig[orig_fao_area_inds,j], area_ctry_fao[orig_fao_area_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
	
		if (xmin < min_faoorig_area) { min_faoorig_area = xmin }
		if (xmax > max_faoorig_area) { max_faoorig_area = xmax }
	
		# regression
		if(length(orig_fao_area_inds) > 1)
		{
			n = length(orig_fao_area_inds)
			xvals = area_ctry_orig[orig_fao_area_inds,j]
			yvals = area_ctry_fao[orig_fao_area_inds,j]
			fit = lm(yvals ~ xvals, na.action = na.omit)
			confint = confint(fit, level = 0.95)
			smry = summary(fit, correlation = TRUE, signif.stars = TRUE)
			reg_lab = paste("y =", round(fit$coefficients[1], digits = 2), "+",
				round(fit$coefficients[2], digits = 3), "* x", sep=" ")
			p_lab = paste("Zero slope p-value = ",
				signif(smry$coefficients[2,4], digits = 4))
			adj_r2_lab = paste("Adj. r^2 = ", round(smry$adj.r.squared, digits = 3))
			r2_lab = paste("r^2 = ", round(smry$r.squared, digits = 3))
			n_lab = paste("n = ", round(n))
			fit_line = fit$coefficients[1] + fit$coefficients[2] * xvals
			one_lab = "One to one line"
	
			# calculate the t-value and p-value for slope = 1
			# t = (estimated slope - 1) / standard error of slope estimate
			# pt(abs(t),lower.tail=FALSE) returns the one-sided prob > |t|
			slope = fit$coefficients[2]
			slope_stderr = smry$coefficients[2,2]
			t1 = (slope - 1) / slope_stderr
			p1 = pt(abs(t1), df=(n - 2), lower.tail=FALSE) * 2
			p1_lab = paste("One slope p-value = ", signif(p1, digits = 4))
						
			# store the regression model
			area_intercept[j, faoorigind] = smry$coefficients[1,1]
			area_intercepterr[j, faoorigind] = smry$coefficients[1,2]
			area_slope[j, faoorigind] = smry$coefficients[2,1]
			area_slopeerr[j, faoorigind] = smry$coefficients[2,2]
			area_r2[j, faoorigind] = smry$r.squared
			area_pval_slope1[j, faoorigind] = p1
			area_n[j, faoorigind] = n
		}
		else
		{
			reg_lab = NA
			p_lab = NA
			adj_r2_lab = NA
			r2_lab = NA
			n_lab = NA
			fit_line = rep(0, n)
			one_lab = "One to one line"
			p1_lab = NA
						
			area_intercept[j, faoorigind] = NA
			area_intercepterr[j, faoorigind] = NA
			area_slope[j, faoorigind] = NA
			area_slopeerr[j, faoorigind] = NA
			area_r2[j, faoorigind] = NA
			area_pval_slope1[j, faoorigind] = NA
			area_n[j, faoorigind] = NA
		} # end regression
	
	} # end if data exist to plot
	
	dev.off()
	
}	# end for j loop over crop

# calculate and write the mean slope, intercept, r2, and pvalue for slope=1 across the crops
if(gtap) {
	oname_prod = paste(outdir, "all_prod_t_gtap_regression", ctag_ctry, sep="")
	oname_area = paste(outdir, "all_harvarea_ha_gtap_regression", ctag_ctry, sep="")
} else {
	oname_prod = paste(outdir, "all_prod_t_orig_regression", ctag_ctry, sep="")
	oname_area = paste(outdir, "all_harvarea_ha_orig_regression", ctag_ctry, sep="")
}

cat("comp,crop,n,slope,slope_stderr,intercept,intercept_stderr,r2,p1val\n", file = oname_prod)
cat("comp,crop,n,slope,slope_stderr,intercept,intercept_stderr,r2,p1val\n", file = oname_area)

prod_mean_neworig_inds = which(prod_n[,neworigind] >= min_crop_n)
prod_mean_newfao_inds = which(prod_n[,newfaoind] >= min_crop_n)
prod_mean_faoorig_inds = which(prod_n[,faoorigind] >= min_crop_n)
prod_mean_inds = intersect(intersect(prod_mean_neworig_inds, prod_mean_newfao_inds), prod_mean_faoorig_inds)

area_mean_neworig_inds = which(area_n[,neworigind] >= min_crop_n)
area_mean_newfao_inds = which(area_n[,newfaoind] >= min_crop_n)
area_mean_faoorig_inds = which(area_n[,faoorigind] >= min_crop_n)
area_mean_inds = intersect(intersect(area_mean_neworig_inds, area_mean_newfao_inds), area_mean_faoorig_inds)

# output the number of selected crops for means
cat("\nCrop n for regression means: min_crop_n =", min_crop_n, "\n")
cat("\t# of prod_mean_neworig_inds =", length(prod_mean_neworig_inds), "\n")
cat("\t# of prod_mean_faoorig_inds =", length(prod_mean_faoorig_inds), "\n")
cat("\t# of prod_mean_newfao_inds =", length(prod_mean_newfao_inds), "\n")
cat("\t# of prod_mean_inds =", length(prod_mean_inds), "\n")
cat("\t# of area_mean_neworig_inds =", length(area_mean_neworig_inds), "\n")
cat("\t# of area_mean_faoorig_inds =", length(area_mean_faoorig_inds), "\n")
cat("\t# of area_mean_newfao_inds =", length(area_mean_newfao_inds), "\n")
cat("\t# of area_mean_inds =", length(area_mean_inds), "\n")

# start with the new vs orig

prod_neworig_mean_n = mean(prod_n[prod_mean_inds,neworigind], na.rm = TRUE)
prod_neworig_mean_slope = mean(prod_slope[prod_mean_inds,neworigind], na.rm = TRUE)
prod_neworig_mean_slopeerr = mean(prod_slopeerr[prod_mean_inds,neworigind], na.rm = TRUE)
prod_neworig_mean_intercept = mean(prod_intercept[prod_mean_inds,neworigind], na.rm = TRUE)
prod_neworig_mean_intercepterr = mean(prod_intercepterr[prod_mean_inds,neworigind], na.rm = TRUE)
prod_neworig_mean_r2 = mean(prod_r2[prod_mean_inds,neworigind], na.rm = TRUE)
prod_neworig_mean_pval_slope1 = mean(prod_pval_slope1[prod_mean_inds,neworigind], na.rm = TRUE)

area_neworig_mean_n = mean(area_n[area_mean_inds,neworigind], na.rm = TRUE)
area_neworig_mean_slope = mean(area_slope[area_mean_inds,neworigind], na.rm = TRUE)
area_neworig_mean_slopeerr = mean(area_slopeerr[area_mean_inds,neworigind], na.rm = TRUE)
area_neworig_mean_intercept = mean(area_intercept[area_mean_inds,neworigind], na.rm = TRUE)
area_neworig_mean_intercepterr = mean(area_intercepterr[area_mean_inds,neworigind], na.rm = TRUE)
area_neworig_mean_r2 = mean(area_r2[area_mean_inds,neworigind], na.rm = TRUE)
area_neworig_mean_pval_slope1 = mean(area_pval_slope1[area_mean_inds,neworigind], na.rm = TRUE)

cat("New_Orig,select_mean", prod_neworig_mean_n, prod_neworig_mean_slope, prod_neworig_mean_slopeerr, prod_neworig_mean_intercept, prod_neworig_mean_intercepterr,
	prod_neworig_mean_r2, prod_neworig_mean_pval_slope1, file = oname_prod, append = TRUE, sep = ",")
cat("New_Orig,select_mean", area_neworig_mean_n, area_neworig_mean_slope, area_neworig_mean_slopeerr, area_neworig_mean_intercept, area_neworig_mean_intercepterr,
	area_neworig_mean_r2, area_neworig_mean_pval_slope1, file = oname_area, append = TRUE, sep = ",")

for	(j in 1:num_crop) {
	cat("\nNew_Orig", crop_names[j], prod_n[j,neworigind], prod_slope[j,neworigind], prod_slopeerr[j,neworigind], prod_intercept[j,neworigind], prod_intercepterr[j,neworigind],
		prod_r2[j,neworigind], prod_pval_slope1[j,neworigind], file = oname_prod, append = TRUE, sep = ",")
	cat("\nNew_Orig", crop_names[j], area_n[j,neworigind], area_slope[j,neworigind], area_slopeerr[j,neworigind], area_intercept[j,neworigind], area_intercepterr[j,neworigind],
		area_r2[j,neworigind], area_pval_slope1[j,neworigind], file = oname_area, append = TRUE, sep = ",")
}

# new vs fao

prod_newfao_mean_n = mean(prod_n[prod_mean_inds,newfaoind], na.rm = TRUE)
prod_newfao_mean_slope = mean(prod_slope[prod_mean_inds,newfaoind], na.rm = TRUE)
prod_newfao_mean_slopeerr = mean(prod_slopeerr[prod_mean_inds,newfaoind], na.rm = TRUE)
prod_newfao_mean_intercept = mean(prod_intercept[prod_mean_inds,newfaoind], na.rm = TRUE)
prod_newfao_mean_intercepterr = mean(prod_intercepterr[prod_mean_inds,newfaoind], na.rm = TRUE)
prod_newfao_mean_r2 = mean(prod_r2[prod_mean_inds,newfaoind], na.rm = TRUE)
prod_newfao_mean_pval_slope1 = mean(prod_pval_slope1[prod_mean_inds,newfaoind], na.rm = TRUE)

area_newfao_mean_n = mean(area_n[area_mean_inds,newfaoind], na.rm = TRUE)
area_newfao_mean_slope = mean(area_slope[area_mean_inds,newfaoind], na.rm = TRUE)
area_newfao_mean_slopeerr = mean(area_slopeerr[area_mean_inds,newfaoind], na.rm = TRUE)
area_newfao_mean_intercept = mean(area_intercept[area_mean_inds,newfaoind], na.rm = TRUE)
area_newfao_mean_intercepterr = mean(area_intercepterr[area_mean_inds,newfaoind], na.rm = TRUE)
area_newfao_mean_r2 = mean(area_r2[area_mean_inds,newfaoind], na.rm = TRUE)
area_newfao_mean_pval_slope1 = mean(area_pval_slope1[area_mean_inds,newfaoind], na.rm = TRUE)

cat("\nNew_FAO,select_mean", prod_newfao_mean_n, prod_newfao_mean_slope, prod_newfao_mean_slopeerr, prod_newfao_mean_intercept, prod_newfao_mean_intercepterr,
	prod_newfao_mean_r2, prod_newfao_mean_pval_slope1, file = oname_prod, append = TRUE, sep = ",")
cat("\nNew_FAO,select_mean", area_newfao_mean_n, area_newfao_mean_slope, area_newfao_mean_slopeerr, area_newfao_mean_intercept, area_newfao_mean_intercepterr,
	area_newfao_mean_r2, area_newfao_mean_pval_slope1, file = oname_area, append = TRUE, sep = ",")

for	(j in 1:num_crop) {
	cat("\nNew_FAO", crop_names[j], prod_n[j,newfaoind], prod_slope[j,newfaoind], prod_slopeerr[j,newfaoind], prod_intercept[j,newfaoind], prod_intercepterr[j,newfaoind],
		prod_r2[j,newfaoind], prod_pval_slope1[j,newfaoind], file = oname_prod, append = TRUE, sep = ",")
	cat("\nNew_FAO", crop_names[j], area_n[j,newfaoind], area_slope[j,newfaoind], area_slopeerr[j,newfaoind], area_intercept[j,newfaoind], area_intercepterr[j,newfaoind],
		area_r2[j,newfaoind], area_pval_slope1[j,newfaoind], file = oname_area, append = TRUE, sep = ",")
}

# fao vs orig

prod_faoorig_mean_n = mean(prod_n[prod_mean_inds,faoorigind], na.rm = TRUE)
prod_faoorig_mean_slope = mean(prod_slope[prod_mean_inds,faoorigind], na.rm = TRUE)
prod_faoorig_mean_slopeerr = mean(prod_slopeerr[prod_mean_inds,faoorigind], na.rm = TRUE)
prod_faoorig_mean_intercept = mean(prod_intercept[prod_mean_inds,faoorigind], na.rm = TRUE)
prod_faoorig_mean_intercepterr = mean(prod_intercepterr[prod_mean_inds,faoorigind], na.rm = TRUE)
prod_faoorig_mean_r2 = mean(prod_r2[prod_mean_inds,faoorigind], na.rm = TRUE)
prod_faoorig_mean_pval_slope1 = mean(prod_pval_slope1[prod_mean_inds,faoorigind], na.rm = TRUE)

area_faoorig_mean_n = mean(area_n[area_mean_inds,faoorigind], na.rm = TRUE)
area_faoorig_mean_slope = mean(area_slope[area_mean_inds,faoorigind], na.rm = TRUE)
area_faoorig_mean_slopeerr = mean(area_slopeerr[area_mean_inds,faoorigind], na.rm = TRUE)
area_faoorig_mean_intercept = mean(area_intercept[area_mean_inds,faoorigind], na.rm = TRUE)
area_faoorig_mean_intercepterr = mean(area_intercepterr[area_mean_inds,faoorigind], na.rm = TRUE)
area_faoorig_mean_r2 = mean(area_r2[area_mean_inds,faoorigind], na.rm = TRUE)
area_faoorig_mean_pval_slope1 = mean(area_pval_slope1[area_mean_inds,faoorigind], na.rm = TRUE)

cat("\nFAO_orig,select_mean", prod_faoorig_mean_n, prod_faoorig_mean_slope, prod_faoorig_mean_slopeerr, prod_faoorig_mean_intercept, prod_faoorig_mean_intercepterr,
	prod_faoorig_mean_r2, prod_faoorig_mean_pval_slope1, file = oname_prod, append = TRUE, sep = ",")
cat("\nFAO_orig,select_mean", area_faoorig_mean_n, area_faoorig_mean_slope, area_faoorig_mean_slopeerr, area_faoorig_mean_intercept, area_faoorig_mean_intercepterr,
	area_faoorig_mean_r2, area_faoorig_mean_pval_slope1, file = oname_area, append = TRUE, sep = ",")

for	(j in 1:num_crop) {
	cat("\nFAO_orig", crop_names[j], prod_n[j,faoorigind], prod_slope[j,faoorigind], prod_slopeerr[j,faoorigind], prod_intercept[j,faoorigind], prod_intercepterr[j,faoorigind],
		prod_r2[j,faoorigind], prod_pval_slope1[j,faoorigind], file = oname_prod, append = TRUE, sep = ",")
	cat("\nFAO_orig", crop_names[j], area_n[j,faoorigind], area_slope[j,faoorigind], area_slopeerr[j,faoorigind], area_intercept[j,faoorigind], area_intercepterr[j,faoorigind],
		area_r2[j,faoorigind], area_pval_slope1[j,faoorigind], file = oname_area, append = TRUE, sep = ",")
}

########### plot the regression lines new vs orig and fao vs orig on same plot with the error lines as well
if(gtap) {
	oname_prod = paste(outdir, "all_prod_t_gtap_regression", ptag_ctry, sep="")
	oname_area = paste(outdir, "all_harvarea_ha_gtap_regression", ptag_ctry, sep="")
} else {
	oname_prod = paste(outdir, "all_prod_t_orig_regression", ptag_ctry, sep="")
	oname_area = paste(outdir, "all_harvarea_ha_orig_regression", ptag_ctry, sep="")
}

pdf(file=oname_prod,paper="letter")
par(mar = c(6, 4, 4, 2) + 0.1)

title = paste("Production regression mean")
sub = paste("min_crop_n =", min_crop_n)
xlab = "GTAP Production (t)"
ylab = "Production (t)"
ymin = min(min_neworig_prod, min_faoorig_prod)
ymax = max(max_neworig_prod, max_faoorig_prod)
xmin = ymin
xmax = ymax
plot.default(x = c(xmin,xmax), y = c(ymin,ymax), type = "n",
	xlab = xlab, main = title, ylab = ylab, sub = sub,
	ylim = c(ymin, ymax), xlim = c(xmin, xmax))
par(new=TRUE)
lines(c(xmin,xmax), c(ymin,ymax), col = "blue3", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * prod_faoorig_mean_slope + prod_faoorig_mean_intercept, xmax * prod_faoorig_mean_slope + prod_faoorig_mean_intercept), col = "black", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * (prod_faoorig_mean_slope + prod_faoorig_mean_slopeerr) + (prod_faoorig_mean_intercept + prod_faoorig_mean_intercept),
	xmax * (prod_faoorig_mean_slope + prod_faoorig_mean_slopeerr) + (prod_faoorig_mean_intercept + prod_faoorig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * (prod_faoorig_mean_slope - prod_faoorig_mean_slopeerr) + (prod_faoorig_mean_intercept - prod_faoorig_mean_intercept),
	xmax * (prod_faoorig_mean_slope - prod_faoorig_mean_slopeerr) + (prod_faoorig_mean_intercept - prod_faoorig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * prod_neworig_mean_slope + prod_neworig_mean_intercept, xmax * prod_neworig_mean_slope + prod_neworig_mean_intercept), col = "red3", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * (prod_neworig_mean_slope + prod_neworig_mean_slopeerr) + (prod_neworig_mean_intercept + prod_neworig_mean_intercept),
	xmax * (prod_neworig_mean_slope + prod_neworig_mean_slopeerr) + (prod_neworig_mean_intercept + prod_neworig_mean_intercept)), col = "red3", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * (prod_neworig_mean_slope - prod_neworig_mean_slopeerr) + (prod_neworig_mean_intercept - prod_neworig_mean_intercept),
	xmax * (prod_neworig_mean_slope - prod_neworig_mean_slopeerr) + (prod_neworig_mean_intercept - prod_neworig_mean_intercept)), col = "red3", lty = 2, lwd = 1.5)
par(new=FALSE)

lds_text = paste("Moirai: slope = ", round(prod_neworig_mean_slope,digits=2), " +- ", round(prod_neworig_mean_slopeerr,digits=2),
", r^2 = ", round(prod_neworig_mean_r2,digits=2), sep = "")
fao_text = paste("FAO: slope = ", round(prod_faoorig_mean_slope,digits=2), " +- ", round(prod_faoorig_mean_slopeerr,digits=2),
", r^2 = ", round(prod_faoorig_mean_r2,digits=2), sep = "")

if(gtap) {
	legend(x = "bottomright", lty = c(1,1,1), lwd = c(2,2,2), legend = c(lds_text, fao_text, "GTAP: one-to-one line"), col = c("red3", "black", "blue3"))
} else {
	legend(x = "bottomright", lty = c(1,1,1), lwd = c(2,2,2), legend = c(lds_text, fao_text, "Original GLUs: one-to-one line"), col = c("red3", "black", "blue3"))
}

dev.off()

pdf(file=oname_area,paper="letter")
par(mar = c(6, 4, 4, 2) + 0.1)

title = paste("Harvested area regression mean")
sub = paste("min_crop_n =", min_crop_n)
xlab = "GTAP Harvested area (ha)"
ylab = "Harvested area (ha)"
ymin = min(min_neworig_area, min_faoorig_area)
ymax = max(max_neworig_area, max_faoorig_area)
xmin = ymin
xmax = ymax
plot.default(x = c(xmin,xmax), y = c(ymin,ymax), type = "n",
	xlab = xlab, main = title, ylab = ylab, sub = sub,
	ylim = c(ymin, ymax), xlim = c(xmin, xmax))
par(new=TRUE)
lines(c(xmin,xmax), c(ymin,ymax), col = "blue3", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * area_faoorig_mean_slope + area_faoorig_mean_intercept, xmax * area_faoorig_mean_slope + area_faoorig_mean_intercept), col = "black", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * (area_faoorig_mean_slope + area_faoorig_mean_slopeerr) + (area_faoorig_mean_intercept + area_faoorig_mean_intercept),
	xmax * (area_faoorig_mean_slope + area_faoorig_mean_slopeerr) + (area_faoorig_mean_intercept + area_faoorig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * (area_faoorig_mean_slope - area_faoorig_mean_slopeerr) + (area_faoorig_mean_intercept - area_faoorig_mean_intercept),
	xmax * (area_faoorig_mean_slope - area_faoorig_mean_slopeerr) + (area_faoorig_mean_intercept - area_faoorig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * area_neworig_mean_slope + area_neworig_mean_intercept, xmax * area_neworig_mean_slope + area_neworig_mean_intercept), col = "red3", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * (area_neworig_mean_slope + area_neworig_mean_slopeerr) + (area_neworig_mean_intercept + area_neworig_mean_intercept),
	xmax * (area_neworig_mean_slope + area_neworig_mean_slopeerr) + (area_neworig_mean_intercept + area_neworig_mean_intercept)), col = "red3", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * (area_neworig_mean_slope - area_neworig_mean_slopeerr) + (area_neworig_mean_intercept - area_neworig_mean_intercept),
	xmax * (area_neworig_mean_slope - area_neworig_mean_slopeerr) + (area_neworig_mean_intercept - area_neworig_mean_intercept)), col = "red3", lty = 2, lwd = 1.5)
par(new=FALSE)

lds_text = paste("Moirai: slope = ", round(area_neworig_mean_slope,digits=2), " +- ", round(area_neworig_mean_slopeerr,digits=2),
", r^2 = ", round(area_neworig_mean_r2,digits=2), sep = "")
fao_text = paste("FAO: slope = ", round(area_faoorig_mean_slope,digits=2), " +- ", round(area_faoorig_mean_slopeerr,digits=2),
", r^2 = ", round(area_faoorig_mean_r2,digits=2), sep = "")

if(gtap) {
	legend(x = "bottomright", lty = c(1,1,1), lwd = c(2,2,2), legend = c(lds_text, fao_text, "GTAP: one-to-one line"), col = c("red3", "black", "blue3"))
} else {
	legend(x = "bottomright", lty = c(1,1,1), lwd = c(2,2,2), legend = c(lds_text, fao_text, "Original GLUs: one-to-one line"), col = c("red3", "black", "blue3"))
}
dev.off()


cat("finished plot_moirai_crop_ctry.r at ",date(), "\n")
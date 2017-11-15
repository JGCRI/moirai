######
# plot_lds_crop_raez.r
#
# calc stats and make plots of the GCAM raez (aez by region) level production and harvested area outputs from lds and genaez
#	harvested area is in ha: LDS_ag_HA_ha.csv
#	production is in metric tonnes: LDS_ag_prod_t.csv
#
#	these inputs are rounded to the integer
#	the base directory is:
#	 lds-workspace/input/gcam-data-system/aglu-processing-code/lds/outputs/
#
# compare the LDS original raez data with GENAEZECON original raez data and reigonalized GTAP data
#  
# all files should not have any missing or NA values
# input zeros represent no data
#
# GENAEZECON and GTAP data format
# there are six header lines
# there are 226 countries and 175 crops, with crop varying faster
# first column is country abbreviation
# second column is crop name
# next 18 columns are the climate aezs in order 1-18
#
# LDS data format
# six header lines (the sixth contains the column labels)
# no zero values
# four colums: reglr_iso, glu_code, use_sector, value

# NOTE:
#  ks tests and difference stats are invalid when the aez numbers do not match
#  there are somewhat useful when looking at small shifts between the same number and numbering scheme of AEZs

# LDS is dependent on the gcam region set for some diagnostics, including the region mapping file used here

# modified oct 2015 to read in LDS output as new aezs
# modified 19 may 2015 to generate some grayscale plots for the paper

# this script takes about 20 minutes to run for 32reg and 10 minutes to run for 14 regions, with 18 aezs
# with 235 basins it took about 4 hours for 32 regions

library(stringr)

cat("started plot_lds_crop_raez.r at ",date(), "\n")

setwd("./")

GTAP = TRUE
REG32 = TRUE

papergray = FALSE

# put different region sets in different folders
#outdir = "./AEZ_orig_lds_14reg_2015_stats_raez/"
#outdir = "./AEZ_orig_lds_32reg_2015_stats_raez/"
outdir = "./basins235_32reg_test_stats_raez/"

# input data files

#prodname = "./AEZ_orig_lds_14reg_2015/LDS_ag_prod_t.csv"
#areaname = "./AEZ_orig_lds_14reg_2015/LDS_ag_HA_ha.csv"
#prodname = "./AEZ_orig_lds_32reg_2015/LDS_ag_prod_t.csv"
#areaname = "./AEZ_orig_lds_32reg_2015/LDS_ag_HA_ha.csv"
prodname = "./basins235_32reg_test/LDS_ag_prod_t.csv"
areaname = "./basins235_32reg_test/LDS_ag_HA_ha.csv"
prodname_orig = "./AEZ_orig_newanlr_2015/GENAEZECON_ag_prod_t.csv"
areaname_orig = "./AEZ_orig_newanlr_2015/GENAEZECON_ag_HA_ha.csv"
prodname_gtap = "../indata/maybe/GTAP_ag_prod_t.csv"
areaname_gtap = "../indata/maybe/GTAP_ag_HA_ha.csv"

# input mapping files

# five columns, one header line:
# this has the code, the abbreviation, the name, and the GCAM ctry87 code and the abbr.
country_gtap_fname = "../indata/maybe/GTAP_ctry_GCAM_ctry87.csv"

# read ctry2regioncodes.txt for gcam regions in lds input iso order
# this file must correspond with the REG32 flag above (region number must match!)
# no header line, 1 column
#ldsgcamregion_fname = "./AEZ_orig_lds_14reg_2015/ctry2regioncodes_gcam.txt"
#ldsgcamregion_fname = "./AEZ_orig_lds_32reg_2015/ctry2regioncodes_gcam.txt"
ldsgcamregion_fname = "./basins235_32reg_test/ctry2regioncodes_gcam.txt"

# the crop names are here, fourth column, one header line
cropfname = "../indata/SAGE_gtap_fao_crop2use.csv"

if (GTAP) {
    num_region = 226
    # output names
    ptag_raez = "_raez_gtap.pdf"
    ctag_raez = "_raez_gtap.csv"
} else {
    if (REG32) {
        num_region = 32
        # 2 columns, 4 header lines:
        region_gcam_fname = "../indata/GCAM_region_names_32reg.csv"
        # output names
        ptag_raez = "_raez_32reg.pdf"
        ctag_raez = "_raez_32reg.csv"
    } else {
        num_region = 14
        # 2 columns, 4 header lines:
        region_gcam_fname = "../indata/GCAM_region_names_14reg.csv"
        # output names
        ptag_raez = "_raez_14reg.pdf"
        ctag_raez = "_raez_14reg.csv"
    }
}

# five columns, one header line
# this file determines which countries are in the LDS output file
countryname_lds = "../indata/FAO_ctry_GCAM_ctry87.csv"

num_ctry = 231			# number of countries in the lds outputs
num_ctry_gtap = 226
num_crop = 175
num_aez = 235
num_aez_gtap = 18
num_raez = num_region * num_aez
num_recs_gtap = num_ctry_gtap * num_crop
num_recs = num_ctry * num_crop
num_colskip = 2

intype = as.list(character(20))
nhead = 6
nhead_lds = 5   # don't skip the 6th line because it is the column labels used for subsetting
intype_crop = as.list(character(8))
nhead_crop = 1

# 3d array for in values
prod = array(dim=c(num_ctry, num_crop, num_aez))
area = array(dim=c(num_ctry, num_crop, num_aez))
prod_orig = array(dim=c(num_ctry, num_crop, num_aez))
area_orig = array(dim=c(num_ctry, num_crop, num_aez))
prod_gtap = array(dim=c(num_ctry, num_crop, num_aez))
area_gtap = array(dim=c(num_ctry, num_crop, num_aez))
prod[,,] = 0.0
area[,,] = 0.0
prod_orig[,,] = 0.0
area_orig[,,] = 0.0
prod_gtap[,,] = 0.0
area_gtap[,,] = 0.0

# 2d arrays of values; aez by region
prod_raez = array(dim=c(num_region, num_aez))
area_raez = array(dim=c(num_region, num_aez))
prod_raez_orig = array(dim=c(num_region, num_aez))
area_raez_orig = array(dim=c(num_region, num_aez))
prod_raez_gtap = array(dim=c(num_region, num_aez))
area_raez_gtap = array(dim=c(num_region, num_aez))

# 1d arrays of differences
prod_diff_raez_orig = array(dim = num_raez)
area_diff_raez_orig = array(dim = num_region, num_aez)
prod_diff_raez_gtap = array(dim = num_raez)
area_diff_raez_gtap = array(dim = num_raez)
prod_diff_raez_orig_gtap = array(dim = num_raez)
area_diff_raez_orig_gtap = array(dim = num_raez)

# 1d arrays of percent differences
prod_pctdiff_raez_orig = array(dim = num_raez)
area_pctdiff_raez_orig = array(dim = num_raez)
prod_pctdiff_raez_gtap = array(dim = num_raez)
area_pctdiff_raez_gtap = array(dim = num_raez)
prod_pctdiff_raez_orig_gtap = array(dim = num_raez)
area_pctdiff_raez_orig_gtap = array(dim = num_raez)

# production output
prod_in <- read.csv(prodname, skip = nhead_lds)
#prod_in<-scan(prodname, what=intype, skip=nhead, sep = ",", quote = "\"")
prod_in_orig<-scan(prodname_orig, what=intype, skip=nhead, sep = ",", quote = "\"")
prod_in_gtap<-scan(prodname_gtap, what=intype, skip=nhead, sep = ",", quote = "\"")
# harvested area output
area_in <- read.csv(areaname, skip = nhead_lds)
#area_in<-scan(areaname, what=intype, skip=nhead, sep = ",", quote = "\"")
area_in_orig<-scan(areaname_orig, what=intype, skip=nhead, sep = ",", quote = "\"")
area_in_gtap<-scan(areaname_gtap, what=intype, skip=nhead, sep = ",", quote = "\"")

# crop names
crop_in = scan(cropfname, what=intype_crop, skip=nhead_crop, sep = ",", quote = "\"")
crop_names = str_trim(unlist(crop_in[4])[1:num_crop])

# gtap country info
temp<-scan(country_gtap_fname, what=as.list(character(5)), skip=1, sep = ",", quote = "\"")
countrycode_gtap = as.integer(unlist(temp[1]))
countryabbr_gtap = unlist(temp[2])
countryname_gtap = unlist(temp[3])

# lds iso country info
# not all of the countries in this mapping file are in the lds output
temp<-scan(countryname_lds, what=as.list(character(5)), skip=1, sep = ",", quote = "\"")
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

# region codes and names
if (GTAP) {
    region_codes = countrycode_gtap
    region_names = countryname_gtap
    # get the full list of lds input country gtap country codes
    regioncode_gcam = array(dim=length(countrycode_lds_all))
    regioncode_gcam[] = NA
    for (i in 1:length(countrycode_lds_all)) {
       	for ( j in 1:num_ctry_gtap) {
       	    if (countryabbr_lds_all[i] == countryabbr_gtap[j]) {
       	        regioncode_gcam[i] = countrycode_gtap[j]
       	        break
       	    } else {
       	        regioncode_gcam[i] = -1
       	    }
       	}
    }	# end for all ctry codes
} else {
    temp<-scan(region_gcam_fname, what=as.list(character(2)), skip=4, sep = ",", quote = "\"")
    region_codes = as.integer(unlist(temp[1]))
    region_names = unlist(temp[2])
    # get the full list of lds input country to gcam region codes
    temp<-scan(ldsgcamregion_fname, what=as.list(character(1)), skip=0, sep = ",", quote = "\"")
    regioncode_gcam = as.integer(unlist(temp[1]))
}

ctry_reg_codes = array(dim = num_ctry)
ctry_reg_codes[] = NA

# now finish extracting and mapping the relevant countries to regions
ctry_index = 0
for(i in 1:length(countrycode_lds_all)) {
	if(ctry87code_lds_all[i] != -1) {
		ctry_index = ctry_index + 1
		ctry_codes[ctry_index] = countrycode_lds_all[i]
		ctry_abbrs[ctry_index] = countryabbr_lds_all[i]
		ctry_names[ctry_index] = countryname_lds_all[i]
		ctry_reg_codes[ctry_index] = regioncode_gcam[i]
	}
}

# put the data in a 3d array

cat("filling lds 3d arrays\n")
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

cat("filling gtap/genaezecon 3d arrays\n")
for(i in 1:num_aez_gtap) {
	prod_col_orig = as.double(unlist(prod_in_orig[i+num_colskip]))
	area_col_orig = as.double(unlist(area_in_orig[i+num_colskip]))
	prod_col_gtap = as.double(unlist(prod_in_gtap[i+num_colskip]))
	area_col_gtap = as.double(unlist(area_in_gtap[i+num_colskip]))
	for(j in 1:num_ctry_gtap) {
		out_index = 0
		# get the out lds country index
		for(k in 1:num_ctry) {
			if(countryabbr_gtap[j] == ctry_abbrs[k]) {
				out_index = k
			}
		}
		if(out_index != 0) {
			for(k in 1:num_crop) {
				in_index = (j - 1) * num_crop + k
				prod_orig[out_index, k, i] = prod_col_orig[in_index]
				area_orig[out_index, k, i] = area_col_orig[in_index]
				prod_gtap[out_index, k, i] = prod_col_gtap[in_index]
				area_gtap[out_index, k, i] = area_col_gtap[in_index]		
			}	# end for k loop over number of crops
		} else {
			cat("could not find out index for gtap ctry", countryabbr_gtap[j], "\n")
		}
	}	# end for j loop over number of gtap countries
}	# end for i loop over aez columns

# start the ks test result files here
ksname_prod = paste(outdir, "ks_prod_t", ctag_raez, sep="")
ksname_area = paste(outdir, "ks_harvarea_ha", ctag_raez, sep="")
cat("crop_prod,ds_new_orig,pv_new_orig,ds_new_gtap,pv_new_gtap,ds_orig_gtap,pv_orig_gtap", sep = "", file = ksname_prod)
cat("crop_harvarea,ds_new_orig,pv_new_orig,ds_new_gtap,pv_new_gtap,ds_orig_gtap,pv_orig_gtap", sep = "", file = ksname_area)

# aggregate to GCAM region and calc stats by crop
for(j in 1:num_crop) {
	cat("processing crop", j, crop_names[j], "\n")
	oname_prod = paste(outdir, crop_names[j], "_prod_t", ctag_raez, sep="")
	oname_area = paste(outdir, crop_names[j], "_harvarea_ha", ctag_raez, sep="")
	cat("region,aez,new,orig,gtap,new-orig,new-gtap,orig-gtap,(new-orig)/orig*100,(new-gtap)/gtap*100,(orig-gtap)/gtap*100", sep = "", file = oname_prod)
	cat("region,aez,new,orig,gtap,new-orig,new-gtap,orig-gtap,(new-orig)/orig,(new-gtap)/gtap,(orig-gtap)/gtap", sep = "", file = oname_area)
	
	# initialize arrays to store values for this crop
	prod_raez[,] = 0.0
	area_raez[,] = 0.0
	prod_raez_orig[,] = 0.0
	area_raez_orig[,] = 0.0
	prod_raez_gtap[,] = 0.0
	area_raez_gtap[,] = 0.0
	prod_diff_raez_orig[] = 0.0
	area_diff_raez_orig[] = 0.0
	prod_diff_raez_gtap[] = 0.0
	area_diff_raez_gtap[] = 0.0
	prod_diff_raez_orig_gtap[] = 0.0
	area_diff_raez_orig_gtap[] = 0.0
	prod_pctdiff_raez_orig[] = 0.0
	area_pctdiff_raez_orig[] = 0.0
	prod_pctdiff_raez_gtap[] = 0.0
	area_pctdiff_raez_gtap[] = 0.0
	prod_pctdiff_raez_orig_gtap[] = 0.0
	area_pctdiff_raez_orig_gtap[] = 0.0
	
	# arrays to store valid indices for this crop
	prod_inds = NULL
	prod_orig_inds = NULL
	prod_gtap_inds = NULL
	prod_diff_orig_inds = NULL
	prod_diff_gtap_inds = NULL
	prod_diff_orig_gtap_inds = NULL
	area_inds = NULL
	area_orig_inds = NULL
	area_gtap_inds = NULL
	area_diff_orig_inds = NULL
	area_diff_gtap_inds = NULL
	area_diff_orig_gtap_inds = NULL
	
	# count variables for this crop
	prod_num_new_no_orig = 0
	prod_num_gtap_no_orig = 0
	prod_num_new_no_gtap = 0
	prod_num_orig_no_gtap = 0
	prod_num_orig_no_new = 0
	prod_num_gtap_no_new = 0
	prod_num_no_new_no_orig = 0
	prod_num_no_new_no_gtap = 0
	prod_num_no_orig_no_gtap = 0
	area_num_new_no_orig = 0
	area_num_gtap_no_orig = 0
	area_num_new_no_gtap = 0
	area_num_orig_no_gtap = 0
	area_num_orig_no_new = 0
	area_num_gtap_no_new = 0
	area_num_no_new_no_orig = 0
	area_num_no_new_no_gtap = 0
	area_num_no_orig_no_gtap = 0
	
	# loop over countries to coalesce regions
	for(i in 1:num_ctry) {
		if(ctry_reg_codes[i] != -1 && !is.na(ctry_reg_codes[i])) {
			# do this only if there is a valid region mapping for this country
			region_index = 0
			for(k in 1:num_region) {
				if(ctry_reg_codes[i] == region_codes[k]) {
					region_index = k
					break
				}
			}
			prod_raez[region_index,] = prod_raez[region_index,] + prod[i, j, ]
			prod_raez_orig[region_index,] = prod_raez_orig[region_index,] + prod_orig[i, j, ]
			prod_raez_gtap[region_index,] = prod_raez_gtap[region_index,] + prod_gtap[i, j, ]
			area_raez[region_index,] = area_raez[region_index,] + area[i, j, ]
			area_raez_orig[region_index,] = area_raez_orig[region_index,] + area_orig[i, j, ]
			area_raez_gtap[region_index,] = area_raez_gtap[region_index,] + area_gtap[i, j, ]
		}
	}	# end i loop over countries to coalesce regions
	
	# flatten the arrays so that aez varies faster then region
	#  i.e. the order is region 1 aezs, then region 2 aezs, etc
	prod_raez_flat = t(prod_raez)
	dim(prod_raez_flat) = NULL
	prod_raez_orig_flat = t(prod_raez_orig)
	dim(prod_raez_orig_flat) = NULL
	prod_raez_gtap_flat = t(prod_raez_gtap)
	dim(prod_raez_gtap_flat) = NULL
	area_raez_flat = t(area_raez)
	dim(area_raez_flat) = NULL
	area_raez_orig_flat = t(area_raez_orig)
	dim(area_raez_orig_flat) = NULL
	area_raez_gtap_flat = t(area_raez_gtap)
	dim(area_raez_gtap_flat) = NULL
	
	# now loop over raezs
	for(i in 1:num_raez) {
		
		# find valid indices of absolute values
		# find valid difference indices (both data sets are valid)
		# count the number of mismatched data entries (one data set is zero and the other is not)
		# count the number of both zero data entries
		
		# production
		if(prod_raez_flat[i] != 0) { # new
			prod_inds = c(prod_inds, i)
			if(prod_raez_orig_flat[i] != 0) { # new and orig
				prod_orig_inds = c(prod_orig_inds, i)
				prod_diff_raez_orig[i] = prod_raez_flat[i] - prod_raez_orig_flat[i]
				prod_pctdiff_raez_orig[i] = prod_diff_raez_orig[i] / prod_raez_orig_flat[i] * 100
				prod_diff_orig_inds = c(prod_diff_orig_inds, i)
				if(prod_raez_gtap_flat[i] != 0) { # orig and gtap (when new)
					prod_diff_raez_orig_gtap[i] = prod_raez_orig_flat[i] - prod_raez_gtap_flat[i]
					prod_pctdiff_raez_orig_gtap[i] = prod_diff_raez_orig_gtap[i] / prod_raez_gtap_flat[i] * 100
					prod_diff_orig_gtap_inds = c(prod_diff_orig_gtap_inds, i)
				} else { # orig and no gtap (when new)
					prod_num_orig_no_gtap = prod_num_orig_no_gtap + 1
					prod_diff_raez_orig_gtap[i] = 0
					prod_pctdiff_raez_orig_gtap[i] = 0
				}
			} else { # new and no orig
				prod_num_new_no_orig = prod_num_new_no_orig + 1
				prod_diff_raez_orig[i] = 0
				prod_pctdiff_raez_orig[i] = 0
				prod_diff_raez_orig_gtap[i] = 0
				prod_pctdiff_raez_orig_gtap[i] = 0
				if(prod_raez_gtap_flat[i] != 0) { # no orig and gtap (when new)
					prod_num_gtap_no_orig = prod_num_gtap_no_orig + 1
				} else { # no orig and no gtap (when new)
					prod_num_no_orig_no_gtap = prod_num_no_orig_no_gtap + 1
				}	
			}
			if(prod_raez_gtap_flat[i] != 0) { # new and gtap
				prod_gtap_inds = c(prod_gtap_inds, i)
				prod_diff_raez_gtap[i] = prod_raez_flat[i] - prod_raez_gtap_flat[i]
				prod_pctdiff_raez_gtap[i] = prod_diff_raez_gtap[i] / prod_raez_gtap_flat[i] * 100
				prod_diff_gtap_inds = c(prod_diff_gtap_inds, i)
			} else { # new and no gtap
				prod_num_new_no_gtap = prod_num_new_no_gtap + 1
				prod_diff_raez_gtap[i] = 0
				prod_pctdiff_raez_gtap[i] = 0
			}
		} else { # no new
			prod_diff_raez_orig[i] = 0
			prod_pctdiff_raez_orig[i] = 0
			prod_diff_raez_gtap[i] = 0
			prod_pctdiff_raez_gtap[i] = 0
			if(prod_raez_orig_flat[i] != 0) { # no new and orig
				prod_orig_inds = c(prod_orig_inds, i)
				prod_num_orig_no_new = prod_num_orig_no_new + 1
				if(prod_raez_gtap_flat[i] != 0) { # orig and gtap (when no new)
					prod_diff_raez_orig_gtap[i] = prod_raez_orig_flat[i] - prod_raez_gtap_flat[i]
					prod_pctdiff_raez_orig_gtap[i] = prod_diff_raez_orig_gtap[i] / prod_raez_gtap_flat[i] * 100
					prod_diff_orig_gtap_inds = c(prod_diff_orig_gtap_inds, i)
				} else { # orig an no gtap (when no new)
					prod_num_orig_no_gtap = prod_num_orig_no_gtap + 1
					prod_diff_raez_orig_gtap[i] = 0
					prod_pctdiff_raez_orig_gtap[i] = 0
				}
			} else { # no new and no orig
				prod_num_no_new_no_orig = prod_num_no_new_no_orig + 1
				prod_diff_raez_orig_gtap[i] = 0
				prod_pctdiff_raez_orig_gtap[i] = 0
				if(prod_raez_gtap_flat[i] != 0) { # no orig and gtap (when no new)
					prod_num_gtap_no_orig = prod_num_gtap_no_orig + 1
				} else { # no orig and no gtap (when no new)
					prod_num_no_orig_no_gtap = prod_num_no_orig_no_gtap + 1
				}
			}
			if(prod_raez_gtap_flat[i] != 0) { # no new and gtap
				prod_gtap_inds = c(prod_gtap_inds, i)
				prod_num_gtap_no_new = prod_num_gtap_no_new + 1
			} else { # no new and no gtap
				prod_num_no_new_no_gtap = prod_num_no_new_no_gtap + 1
			}
		}
		
		# area
		if(area_raez_flat[i] != 0) { # new
			area_inds = c(area_inds, i)
			if(area_raez_orig_flat[i] != 0) { # new and orig
				area_orig_inds = c(area_orig_inds, i)
				area_diff_raez_orig[i] = area_raez_flat[i] - area_raez_orig_flat[i]
				area_pctdiff_raez_orig[i] = area_diff_raez_orig[i] / area_raez_orig_flat[i] * 100
				area_diff_orig_inds = c(area_diff_orig_inds, i)
				if(area_raez_gtap_flat[i] != 0) { # orig and gtap (when new)
					area_diff_raez_orig_gtap[i] = area_raez_orig_flat[i] - area_raez_gtap_flat[i]
					area_pctdiff_raez_orig_gtap[i] = area_diff_raez_orig_gtap[i] / area_raez_gtap_flat[i] * 100
					area_diff_orig_gtap_inds = c(area_diff_orig_gtap_inds, i)
				} else { # orig and no gtap (when new)
					area_num_orig_no_gtap = area_num_orig_no_gtap + 1
					area_diff_raez_orig_gtap[i] = 0
					area_pctdiff_raez_orig_gtap[i] = 0
				}
			} else { # new and no orig
				area_num_new_no_orig = area_num_new_no_orig + 1
				area_diff_raez_orig[i] = 0
				area_pctdiff_raez_orig[i] = 0
				area_diff_raez_orig_gtap[i] = 0
				area_pctdiff_raez_orig_gtap[i] = 0
				if(area_raez_gtap_flat[i] != 0) { # no orig and gtap (when new)
					area_num_gtap_no_orig = area_num_gtap_no_orig + 1
				} else { # no orig and no gtap (when new)
					area_num_no_orig_no_gtap = area_num_no_orig_no_gtap + 1
				}	
			}
			if(area_raez_gtap_flat[i] != 0) { # new and gtap
				area_gtap_inds = c(area_gtap_inds, i)
				area_diff_raez_gtap[i] = area_raez_flat[i] - area_raez_gtap_flat[i]
				area_pctdiff_raez_gtap[i] = area_diff_raez_gtap[i] / area_raez_gtap_flat[i] * 100
				area_diff_gtap_inds = c(area_diff_gtap_inds, i)
			} else { # new and no gtap
				area_num_new_no_gtap = area_num_new_no_gtap + 1
				area_diff_raez_gtap[i] = 0
				area_pctdiff_raez_gtap[i] = 0
			}
		} else { # no new
			area_diff_raez_orig[i] = 0
			area_pctdiff_raez_orig[i] = 0
			area_diff_raez_gtap[i] = 0
			area_pctdiff_raez_gtap[i] = 0
			if(area_raez_orig_flat[i] != 0) { # no new and orig
				area_orig_inds = c(area_orig_inds, i)
				area_num_orig_no_new = area_num_orig_no_new + 1
				if(area_raez_gtap_flat[i] != 0) { # orig and gtap (when no new)
					area_diff_raez_orig_gtap[i] = area_raez_orig_flat[i] - area_raez_gtap_flat[i]
					area_pctdiff_raez_orig_gtap[i] = area_diff_raez_orig_gtap[i] / area_raez_gtap_flat[i] * 100
					area_diff_orig_gtap_inds = c(area_diff_orig_gtap_inds, i)
				} else { # orig an no gtap (when no new)
					area_num_orig_no_gtap = area_num_orig_no_gtap + 1
					area_diff_raez_orig_gtap[i] = 0
					area_pctdiff_raez_orig_gtap[i] = 0
				}
			} else { # no new and no orig
				area_num_no_new_no_orig = area_num_no_new_no_orig + 1
				area_diff_raez_orig_gtap[i] = 0
				area_pctdiff_raez_orig_gtap[i] = 0
				if(area_raez_gtap_flat[i] != 0) { # no orig and gtap (when no new)
					area_num_gtap_no_orig = area_num_gtap_no_orig + 1
				} else { # no orig and no gtap (when no new)
					area_num_no_orig_no_gtap = area_num_no_orig_no_gtap + 1
				}
			}
			if(area_raez_gtap_flat[i] != 0) { # no new and gtap
				area_gtap_inds = c(area_gtap_inds, i)
				area_num_gtap_no_new = area_num_gtap_no_new + 1
			} else { # no new and no gtap
				area_num_no_new_no_gtap = area_num_no_new_no_gtap + 1
			}
		}	# end else no new
		
		# get the region and aez indices and output fields
		region_index = ceiling(i / num_aez)
		aez_index = i - (region_index - 1) * num_aez
		region_field = region_names[region_index]
		if(aez_index < 10) {aez_field = paste("AEZ0", aez_index, sep = "")
		} else {aez_field = paste("AEZ", aez_index, sep = "")}
			
		# output per country values to csv file
		cat("\n", region_field, ",", aez_field, ",", prod_raez_flat[i], ",", prod_raez_orig_flat[i], ",", prod_raez_gtap_flat[i], ",",
			prod_diff_raez_orig[i], ",", prod_diff_raez_gtap[i], ",", prod_diff_raez_orig_gtap[i], ",",
			round(prod_pctdiff_raez_orig[i], digits = 2), ",", round(prod_pctdiff_raez_gtap[i], digits = 2), ",",
			round(prod_pctdiff_raez_orig_gtap[i], digits = 2), sep = "", file = oname_prod, append = TRUE)
		cat("\n", region_field, ",", aez_field, ",", area_raez_flat[i], ",", area_raez_orig_flat[i], ",", area_raez_gtap_flat[i], ",",
			area_diff_raez_orig[i], ",", area_diff_raez_gtap[i], ",", area_diff_raez_orig_gtap[i], ",",
			round(area_pctdiff_raez_orig[i], digits = 2), ",", round(area_pctdiff_raez_gtap[i], digits = 2), ",",
			round(area_pctdiff_raez_orig_gtap[i], digits = 2), sep = "", file = oname_area, append = TRUE)

	}	# end for i loop over raez
	
	# test distributions and generate histograms

	oname_prod = paste(outdir, crop_names[j], "_prod_t", ptag_raez, sep="")
	oname_area = paste(outdir, crop_names[j], "_harvarea_ha", ptag_raez, sep="")
	
	# get the valid values
	
	# production
	prod_raez_valid = prod_raez_flat[prod_inds]
	prod_raez_orig_valid = prod_raez_orig_flat[prod_orig_inds]
	prod_raez_gtap_valid = prod_raez_gtap_flat[prod_gtap_inds]
	
	# harvested area
	area_raez_valid = area_raez_flat[area_inds]
	area_raez_orig_valid = area_raez_orig_flat[area_orig_inds]
	area_raez_gtap_valid = area_raez_gtap_flat[area_gtap_inds]
	
	# first use kolmogorov-smirnov to test whether data could be sampled from the same continuous distribution
	# write these results to a separate csv file
	
	# production
	if(length(prod_raez_valid) > 0 & length(prod_raez_orig_valid) > 0) {
		ks_prod_new_orig_raez = ks.test(prod_raez_valid, prod_raez_orig_valid)
		ds_prod_new_orig_raez = round(ks_prod_new_orig_raez$statistic, digits = 4)
		pv_prod_new_orig_raez = round(ks_prod_new_orig_raez$p.value, digits = 4)
	}else {
		ds_prod_new_orig_raez = -1
		pv_prod_new_orig_raez = -1
	}
	if(length(prod_raez_valid) > 0 & length(prod_raez_gtap_valid) > 0) {
		ks_prod_new_gtap_raez = ks.test(prod_raez_valid, prod_raez_gtap_valid)
		ds_prod_new_gtap_raez = round(ks_prod_new_gtap_raez$statistic, digits = 4)
		pv_prod_new_gtap_raez = round(ks_prod_new_gtap_raez$p.value, digits = 4)
	}else {
		ds_prod_new_gtap_raez = -1
		pv_prod_new_gtap_raez = -1
	}
	if(length(prod_raez_orig_valid) > 0 & length(prod_raez_gtap_valid) > 0) {
		ks_prod_orig_gtap_raez = ks.test(prod_raez_orig_valid, prod_raez_gtap_valid)
		ds_prod_orig_gtap_raez = round(ks_prod_orig_gtap_raez$statistic, digits = 4)
		pv_prod_orig_gtap_raez = round(ks_prod_orig_gtap_raez$p.value, digits = 4)
	}else {
		ds_prod_orig_gtap_raez = -1
		pv_prod_orig_gtap_raez = -1
	}
	
	# area
	if(length(area_raez_valid) > 0 & length(area_raez_orig_valid) > 0) {
		ks_area_new_orig_raez = ks.test(area_raez_valid, area_raez_orig_valid)
		ds_area_new_orig_raez = round(ks_area_new_orig_raez$statistic, digits = 4)
		pv_area_new_orig_raez = round(ks_area_new_orig_raez$p.value, digits = 4)
	}else {
		ds_area_new_orig_raez = -1
		pv_area_new_orig_raez = -1
	}
	if(length(area_raez_valid) > 0 & length(area_raez_gtap_valid) > 0) {
		ks_area_new_gtap_raez = ks.test(area_raez_valid, area_raez_gtap_valid)
		ds_area_new_gtap_raez = round(ks_area_new_gtap_raez$statistic, digits = 4)
		pv_area_new_gtap_raez = round(ks_area_new_gtap_raez$p.value, digits = 4)
	}else {
		ds_area_new_gtap_raez = -1
		pv_area_new_gtap_raez = -1
	}
	if(length(area_raez_orig_valid) > 0 & length(area_raez_gtap_valid) > 0) {
		ks_area_orig_gtap_raez = ks.test(area_raez_orig_valid, area_raez_gtap_valid)
		ds_area_orig_gtap_raez = round(ks_area_orig_gtap_raez$statistic, digits = 4)
		pv_area_orig_gtap_raez = round(ks_area_orig_gtap_raez$p.value, digits = 4)
	}else {
		ds_area_orig_gtap_raez = -1
		pv_area_orig_gtap_raez = -1
	}
	
	cat("\n", crop_names[j], ",", ds_prod_new_orig_raez, ",", pv_prod_new_orig_raez, ",", ds_prod_new_gtap_raez, ",", pv_prod_new_gtap_raez, ",",
		ds_prod_orig_gtap_raez, ",", pv_prod_orig_gtap_raez, sep = "", file = ksname_prod, append = TRUE)
	cat("\n", crop_names[j], ",", ds_area_new_orig_raez, ",", pv_area_new_orig_raez, ",", ds_area_new_gtap_raez, ",", pv_area_new_gtap_raez, ",",
		ds_area_orig_gtap_raez, ",", pv_area_orig_gtap_raez, sep = "", file = ksname_area, append = TRUE)
	
	# production histograms
	
	if(length(c(prod_raez_valid, prod_raez_orig_valid, prod_raez_gtap_valid)) > 0) {
		xmax = ceiling(max(prod_raez_valid, prod_raez_orig_valid, prod_raez_gtap_valid) / 10000.0) * 10000
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
	
	title = paste(crop_names[j], "LDS")
	num_vals = length(prod_raez_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		prod_raez_valid_hist = hist(prod_raez_valid, main = title, sub = sub, xlab = "Production (t)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "Original")
	num_vals = length(prod_raez_orig_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		prod_raez_orig_valid_hist = hist(prod_raez_orig_valid, main = title, sub = sub, xlab = "Production (t)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "GTAP")
	num_vals = length(prod_raez_gtap_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		prod_raez_gtap_valid_hist = hist(prod_raez_gtap_valid, main = title, sub = sub, xlab = "Production (t)",
			breaks = breaks)
	}
	
	# differences
	
	if(length(c(prod_diff_raez_orig[prod_diff_orig_inds], prod_diff_raez_gtap[prod_diff_gtap_inds],
			prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds])) > 0) {
		xmax = ceiling(max(prod_diff_raez_orig[prod_diff_orig_inds], prod_diff_raez_gtap[prod_diff_gtap_inds],
			prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds]) / 1000.0) * 1000
		xmin = floor(min(prod_diff_raez_orig[prod_diff_orig_inds], prod_diff_raez_gtap[prod_diff_gtap_inds],
			prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds]) / 1000.0) * 1000
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 1000):(xmax / 1000)) * 1000
	}
	
	title = paste(crop_names[j], "LDS - Original")
	num_vals = length(prod_diff_raez_orig[prod_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_diff_raez_orig[prod_diff_orig_inds]), digits = 2)
		stddev = round(sd(prod_diff_raez_orig[prod_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", prod_num_new_no_orig, "; # of missing LDS values", prod_num_orig_no_new)
		prod_diff_raez_orig_valid_hist = hist(prod_diff_raez_orig[prod_diff_orig_inds], main = title, xlab = "Production difference (t)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "LDS - GTAP")
	num_vals = length(prod_diff_raez_gtap[prod_diff_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_diff_raez_gtap[prod_diff_gtap_inds]), digits = 2)
		stddev = round(sd(prod_diff_raez_gtap[prod_diff_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", prod_num_new_no_gtap, "; # of missing LDS values", prod_num_orig_no_new)
		prod_diff_raez_gtap_valid_hist = hist(prod_diff_raez_gtap[prod_diff_gtap_inds], main = title, xlab = "Production difference (t)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - GTAP")
	num_vals = length(prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds]), digits = 2)
		stddev = round(sd(prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", prod_num_orig_no_gtap, "; # of missing original values", prod_num_gtap_no_orig)
		prod_diff_raez_orig_gtap_valid_hist = hist(prod_diff_raez_orig_gtap[prod_diff_orig_gtap_inds], main = title, xlab = "Production difference (t)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# percent differences
	
	if(length(prod_pctdiff_raez_orig[prod_diff_orig_inds]) > 0) {
		xmax = ceiling(max(prod_pctdiff_raez_orig[prod_diff_orig_inds]) / 2.0) * 2
		xmin = floor(min(prod_pctdiff_raez_orig[prod_diff_orig_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "LDS - Original")
	num_vals = length(prod_pctdiff_raez_orig[prod_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_pctdiff_raez_orig[prod_diff_orig_inds]), digits = 2)
		stddev = round(sd(prod_pctdiff_raez_orig[prod_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", prod_num_new_no_orig, "; # of missing LDS values", prod_num_orig_no_new)
		prod_pctdiff_raez_orig_valid_hist = hist(prod_pctdiff_raez_orig[prod_diff_orig_inds], main = title, xlab = "% Production difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	if(length(c(prod_pctdiff_raez_gtap[prod_diff_gtap_inds], prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds])) > 0) {
		xmax = ceiling(max(prod_pctdiff_raez_gtap[prod_diff_gtap_inds],
			prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds]) / 2.0) * 2
		xmin = floor(min(prod_pctdiff_raez_gtap[prod_diff_gtap_inds],
			prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "LDS - GTAP")
	num_vals = length(prod_pctdiff_raez_gtap[prod_diff_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_pctdiff_raez_gtap[prod_diff_gtap_inds]), digits = 2)
		stddev = round(sd(prod_pctdiff_raez_gtap[prod_diff_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", prod_num_new_no_gtap, "; # of missing LDS values", prod_num_orig_no_new)
		prod_pctdiff_raez_gtap_valid_hist = hist(prod_pctdiff_raez_gtap[prod_diff_gtap_inds], main = title, xlab = "% Production difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - GTAP")
	num_vals = length(prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds]), digits = 2)
		stddev = round(sd(prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", prod_num_orig_no_gtap, "; # of missing original values", prod_num_gtap_no_orig)
		prod_pctdiff_raez_orig_gtap_valid_hist = hist(prod_pctdiff_raez_orig_gtap[prod_diff_orig_gtap_inds], main = title,
			xlab = "% Production difference", breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	dev.off()	# close production histogram file
	
	# harvested area histogram file
	
	if(length(c(area_raez_valid, area_raez_orig_valid, area_raez_gtap_valid)) > 0) {
		xmax = ceiling(max(area_raez_valid, area_raez_orig_valid, area_raez_gtap_valid) / 10000.0) * 10000
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
	
	title = paste(crop_names[j], "LDS")
	num_vals = length(area_raez_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		area_raez_valid_hist = hist(area_raez_valid, main = title, sub = sub, xlab = "Harvested area (ha)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "Original")
	num_vals = length(area_raez_orig_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		area_raez_orig_valid_hist = hist(area_raez_orig_valid, main = title, sub = sub, xlab = "Harvested area (ha)",
			breaks = breaks)
	}
	
	title = paste(crop_names[j], "GTAP")
	num_vals = length(area_raez_gtap_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		area_raez_gtap_valid_hist = hist(area_raez_gtap_valid, main = title, sub = sub, xlab = "Harvested area (ha)",
			breaks = breaks)
	}
	
	# differences
	
	if(length(c(area_diff_raez_orig[area_diff_orig_inds], area_diff_raez_gtap[area_diff_gtap_inds],
			area_diff_raez_orig_gtap[area_diff_orig_gtap_inds])) > 0) {
		xmax = ceiling(max(area_diff_raez_orig[area_diff_orig_inds], area_diff_raez_gtap[area_diff_gtap_inds],
			area_diff_raez_orig_gtap[area_diff_orig_gtap_inds]) / 1000.0) * 1000
		xmin = floor(min(area_diff_raez_orig[area_diff_orig_inds], area_diff_raez_gtap[area_diff_gtap_inds],
			area_diff_raez_orig_gtap[area_diff_orig_gtap_inds]) / 1000.0) * 1000
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 1000):(xmax / 1000)) * 1000
	}
	
	title = paste(crop_names[j], "LDS - Original")
	num_vals = length(area_diff_raez_orig[area_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(area_diff_raez_orig[area_diff_orig_inds]), digits = 2)
		stddev = round(sd(area_diff_raez_orig[area_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", area_num_new_no_orig, "; # of missing LDS values", area_num_orig_no_new)
		area_diff_raez_orig_valid_hist = hist(area_diff_raez_orig[area_diff_orig_inds], main = title, xlab = "Harvested area difference (ha)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "LDS - GTAP")
	num_vals = length(area_diff_raez_gtap[area_diff_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(area_diff_raez_gtap[area_diff_gtap_inds]), digits = 2)
		stddev = round(sd(area_diff_raez_gtap[area_diff_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", area_num_new_no_gtap, "; # of missing LDS values", area_num_orig_no_new)
		area_diff_raez_gtap_valid_hist = hist(area_diff_raez_gtap[area_diff_gtap_inds], main = title, xlab = "Harvested area difference (ha)",
			breaks = breaks)
			title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - GTAP")
	num_vals = length(area_diff_raez_orig_gtap[area_diff_orig_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(area_diff_raez_orig_gtap[area_diff_orig_gtap_inds]), digits = 2)
		stddev = round(sd(area_diff_raez_orig_gtap[area_diff_orig_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", area_num_orig_no_gtap, "; # of missing original values", area_num_gtap_no_orig)
		area_diff_raez_orig_gtap_valid_hist = hist(area_diff_raez_orig_gtap[area_diff_orig_gtap_inds], main = title,
			xlab = "Harvested area difference (ha)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# percent differences
	
	if(length(area_pctdiff_raez_orig[area_diff_orig_inds]) > 0) {
		xmax = ceiling(max(area_pctdiff_raez_orig[area_diff_orig_inds]) / 2.0) * 2
		xmin = floor(min(area_pctdiff_raez_orig[area_diff_orig_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "LDS - Original")
	num_vals = length(area_pctdiff_raez_orig[area_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(area_pctdiff_raez_orig[area_diff_orig_inds]), digits = 2)
		stddev = round(sd(area_pctdiff_raez_orig[area_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", area_num_new_no_orig, "; # of missing LDS values", area_num_orig_no_new)
		area_pctdiff_raez_orig_valid_hist = hist(area_pctdiff_raez_orig[area_diff_orig_inds], main = title, xlab = "% Harvested area difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	if(length(c(area_pctdiff_raez_gtap[area_diff_gtap_inds],
			area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds])) > 0) {
		xmax = ceiling(max(area_pctdiff_raez_gtap[area_diff_gtap_inds],
			area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds]) / 2.0) * 2
		xmin = floor(min(area_pctdiff_raez_gtap[area_diff_gtap_inds],
			area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 2):(xmax / 2)) * 2
	}
	
	title = paste(crop_names[j], "LDS - GTAP")
	num_vals = length(area_pctdiff_raez_gtap[area_diff_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(area_pctdiff_raez_gtap[area_diff_gtap_inds]), digits = 2)
		stddev = round(sd(area_pctdiff_raez_gtap[area_diff_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", area_num_new_no_gtap, "; # of missing LDS values", area_num_orig_no_new)
		area_pctdiff_raez_gtap_valid_hist = hist(area_pctdiff_raez_gtap[area_diff_gtap_inds], main = title, xlab = "% Harvested area difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	title = paste(crop_names[j], "Original - GTAP")
	num_vals = length(area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds]), digits = 2)
		stddev = round(sd(area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", area_num_orig_no_gtap, "; # of missing original values", area_num_gtap_no_orig)
		area_pctdiff_raez_orig_gtap_valid_hist = hist(area_pctdiff_raez_orig_gtap[area_diff_orig_gtap_inds], main = title,
		xlab = "% Harvested area difference", breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	dev.off()	# close harvest area histogram file
	
#################################
	
	# generate histogram comparison plots
	
	oname_prod = paste(outdir, crop_names[j], "_prod_t_comp", ptag_raez, sep="")
	oname_area = paste(outdir, crop_names[j], "_harvarea_ha_comp", ptag_raez, sep="")
	
	# production histogram comparison
	
	pdf(file=oname_prod,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	# plot all 3 absolute value histograms on a linear-log plot as lines
	
	new_x = log10(prod_raez_valid_hist$mids)
	new_y = prod_raez_valid_hist$counts
	orig_x = log10(prod_raez_orig_valid_hist$mids)
	orig_y = prod_raez_orig_valid_hist$counts
	gtap_x = log10(prod_raez_gtap_valid_hist$mids)
	gtap_y = prod_raez_gtap_valid_hist$counts
	#num_bins = length(gtap_x)
	#bin_widths = log10(prod_raez_gtap_valid_hist$breaks)[3:(num_bins + 1)] - log10(prod_raez_gtap_valid_hist$breaks)[2:num_bins]
	#bin_widths = c(gtap_x[2] - 0.5 *bin_widths[1], bin_widths)
	
	ymax = max(new_y, orig_y, gtap_y)
	xmax = max(new_x, orig_x, gtap_x)
	xmin = min(new_x, orig_x, gtap_x)
	
	maintitle = paste(crop_names[j], " production histogram comparison", sep = "")
	xlab = "Production (t)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	#barplot(width = bin_widths, height = gtap_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GTAP")
	lines(new_x, new_y, lty = 1, col = "black", lwd = 2)
	lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
	legend(x = "topright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("blue3", "red3", "black"), lwd = 2)
	par(new=FALSE)
	
	# plot the above comparison using CDFs
	
	new_x = log10(prod_raez_valid_hist$mids)
	div = sum(prod_raez_valid_hist$counts)
	if(div == 0) {div = 1}
	new_y = cumsum(prod_raez_valid_hist$counts) / div
	orig_x = log10(prod_raez_orig_valid_hist$mids)
	div = sum(prod_raez_orig_valid_hist$counts)
	if(div == 0) {div = 1}
	orig_y = cumsum(prod_raez_orig_valid_hist$counts) / div
	gtap_x = log10(prod_raez_gtap_valid_hist$mids)
	div = sum(prod_raez_gtap_valid_hist$counts)
	if(div == 0) {div = 1}
	gtap_y = cumsum(prod_raez_gtap_valid_hist$counts) / div
	
	ymax = max(new_y, orig_y, gtap_y)
	ymin = 0.0
	xmax = max(new_x, orig_x, gtap_x)
	xmin = min(new_x, orig_x, gtap_x)
	
	maintitle = paste(crop_names[j], " production cumulative distribution comparison", sep = "")
	xlab = "Production (t)"
	ylab = "Cumulative probability"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	
	if (papergray) {
		lines(new_x, new_y, lty = 1, col = "black", lwd = 2)
		lines(gtap_x, gtap_y, lty = 4, col = "gray60", lwd = 2)
		lines(orig_x, orig_y, lty = 2, col = "gray30", lwd = 2)
		legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "LDS original AEZs", "LDS AEZs"), col = c("gray60", "gray30", "black"), lwd = 2)
	} else {
		lines(new_x, new_y, lty = 1, col = "black", lwd = 2)
		lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
		lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
		legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("blue3", "red3", "black"), lwd = 2)
	}
	
	par(new=FALSE)
	
	#### !!!!!! differences are not valid for raezs because there is not correspondence between the new and old aezs !!!!!!!!!
	
	# plot the difference histograms of new-gtap and orig-gtap together on linear-log axes
	
	# need to deal with the negatives
	# skip this for now
	if(FALSE) {
	
	new_x = log10(prod_diff_raez_gtap_valid_hist$mids)
	new_y = prod_diff_raez_gtap_valid_hist$counts
	orig_x = log10(prod_diff_raez_orig_gtap_valid_hist$mids)
	orig_y = prod_diff_raez_orig_gtap_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(crop_names[j], " production difference histogram comparison", sep = "")
	xlab = "Production difference (t)"
	plot.default(x = new_x, y = new_y, type = "n",
		xlab = xlab, main = maintitle,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "New - GTAP")
	lines(orig_x, orig_y, lty = 2, lwd = 2)
	lines(new_x, new_y, lty = 1, lwd = 2)
	legend(x = "topright", lty = c(2, 1), legend = c("Original - GTAP", "LDS - GTAP"), lwd = 2)
	par(new=FALSE)
	
	}	 # end if(FALSE)
	
	# plot the % difference histograms of new-orig and new-gtap and orig-gtap together on linear axes
	
	# there shouldn't be any zero mid-bin values
	posinds = which(prod_pctdiff_raez_orig_valid_hist$mids > 0)
	neginds = which(prod_pctdiff_raez_orig_valid_hist$mids < 0)
	nod_x = c(-log10(-prod_pctdiff_raez_orig_valid_hist$mids[neginds]), log10(prod_pctdiff_raez_orig_valid_hist$mids[posinds]))
	nod_y = prod_pctdiff_raez_orig_valid_hist$counts
	posinds = which(prod_pctdiff_raez_gtap_valid_hist$mids > 0)
	neginds = which(prod_pctdiff_raez_gtap_valid_hist$mids < 0)
	new_x = c(-log10(-prod_pctdiff_raez_gtap_valid_hist$mids[neginds]), log10(prod_pctdiff_raez_gtap_valid_hist$mids[posinds]))
	new_x_labels = prod_pctdiff_raez_gtap_valid_hist$mids
	new_y = prod_pctdiff_raez_gtap_valid_hist$counts
	posinds = which(prod_pctdiff_raez_orig_gtap_valid_hist$mids > 0)
	neginds = which(prod_pctdiff_raez_orig_gtap_valid_hist$mids < 0)
	orig_x = c(-log10(-prod_pctdiff_raez_orig_gtap_valid_hist$mids[neginds]), log10(prod_pctdiff_raez_orig_gtap_valid_hist$mids[posinds]))
	orig_y = prod_pctdiff_raez_orig_gtap_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(nod_y, new_y, orig_y)
	xmax = max(nod_x, new_x, orig_x)
	xmin = min(nod_x, new_x, orig_x)
	
	maintitle = paste(crop_names[j], " % production difference histogram comparison", sep = "")
	xlab = "% production difference"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = new_x_labels)
	par(new=TRUE)
	#barplot(width = rep(2, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "New - GTAP")
	lines(nod_x, nod_y, lty = 1, col = "black", lwd = 2)
	#lines(new_x, new_y, lty = 4, col = "blue", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
	#legend(x = "topright", lty = c(2, 4, 1), legend = c("Original - GTAP", "New - GTAP", "New - Original"), lwd = 2)
	legend(x = "topright", lty = c(2, 1), legend = c("Original AEZs - GTAP", "LDS AEZs - Original AEZs"), col = c("red3", "black"), lwd = 2)
	par(new=FALSE)
	
	dev.off()	# close production histogram comparison file

	# harvested area histogram comparison
	
	pdf(file=oname_area,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	# plot all 3 absolute value histograms on a linear-log plot as lines
	
	new_x = log10(area_raez_valid_hist$mids)
	new_y = area_raez_valid_hist$counts
	orig_x = log10(area_raez_orig_valid_hist$mids)
	orig_y = area_raez_orig_valid_hist$counts
	gtap_x = log10(area_raez_gtap_valid_hist$mids)
	gtap_y = area_raez_gtap_valid_hist$counts
	num_bins = length(gtap_x)
	
	ymax = max(new_y, orig_y, gtap_y)
	xmax = max(new_x, orig_x, gtap_x)
	xmin = min(new_x, orig_x, gtap_x)
	
	maintitle = paste(crop_names[j], " harvested area histogram comparison", sep = "")
	xlab = "Harvested area (ha)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = gtap_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GTAP")
	lines(new_x, new_y, lty = 1, col = "black", lwd = 2)
	lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
	legend(x = "topright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("blue3", "red3", "black"), lwd = 2)
	par(new=FALSE)
	
	# plot the above comparison using CDFs
	
	new_x = log10(area_raez_valid_hist$mids)
	div = sum(area_raez_valid_hist$counts)
	if(div == 0) {div = 1}
	new_y = cumsum(area_raez_valid_hist$counts) / div
	orig_x = log10(area_raez_orig_valid_hist$mids)
	div = sum(area_raez_orig_valid_hist$counts)
	if(div == 0) {div = 1}
	orig_y = cumsum(area_raez_orig_valid_hist$counts) / div
	gtap_x = log10(area_raez_gtap_valid_hist$mids)
	div = sum(area_raez_gtap_valid_hist$counts)
	if(div == 0) {div = 1}
	gtap_y = cumsum(area_raez_gtap_valid_hist$counts) / div
	
	ymax = max(new_y, orig_y, gtap_y)
	ymin = 0.0
	xmax = max(new_x, orig_x, gtap_x)
	xmin = min(new_x, orig_x, gtap_x)
	
	maintitle = paste(crop_names[j], " harvested area cumulative distribution comparison", sep = "")
	xlab = "Harvested area (ha)"
	ylab = "Cumulative probability"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	lines(new_x, new_y, lty = 1, col = "black", lwd = 2)
	lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
	legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("blue3", "red3", "black"), lwd = 2)
	par(new=FALSE)
	
	#### !!!!!! differences are not valid for raezs because there is not correspondence between the new and old aezs !!!!!!!!!
	
	# plot the difference histograms of new-gtap and orig-gtap together on linear-log axes
	
	# there shouldn't be any zero mid-bin values
	posinds = which(area_diff_raez_orig_valid_hist$mids > 0)
	neginds = which(area_diff_raez_orig_valid_hist$mids < 0)
	nod_x = c(-log10(-area_diff_raez_orig_valid_hist$mids[neginds]), log10(area_diff_raez_orig_valid_hist$mids[posinds]))
	nod_y = area_diff_raez_orig_valid_hist$counts
	posinds = which(area_diff_raez_gtap_valid_hist$mids > 0)
	neginds = which(area_diff_raez_gtap_valid_hist$mids < 0)
	new_x = c(-log10(-area_diff_raez_gtap_valid_hist$mids[neginds]), log10(area_diff_raez_gtap_valid_hist$mids[posinds]))
	new_x_labels = area_diff_raez_gtap_valid_hist$mids
	new_y = area_diff_raez_gtap_valid_hist$counts
	posinds = which(area_diff_raez_orig_gtap_valid_hist$mids > 0)
	neginds = which(area_diff_raez_orig_gtap_valid_hist$mids < 0)
	orig_x = c(-log10(-area_diff_raez_orig_gtap_valid_hist$mids[neginds]), log10(area_diff_raez_orig_gtap_valid_hist$mids[posinds]))
	orig_y = area_diff_raez_orig_gtap_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(nod_y, new_y, orig_y)
	xmax = max(nod_x, new_x, orig_x)
	xmin = min(nod_x, new_x, orig_x)
	
	maintitle = paste(crop_names[j], " harvested area difference histogram comparison", sep = "")
	xlab = "Harvested area (ha)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = new_x_labels)
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "New - GTAP")
	lines(orig_x, orig_y, lty = 3, lwd = 2)
	lines(new_x, new_y, lty = 2, lwd = 2)
	lines(nod_x, nod_y, lty = 1, lwd = 2)
	legend(x = "topright", lty = c(3, 2, 1), legend = c("Original - GTAP", "New - GTAP", "LDS - Original"), lwd = 2)
	par(new=FALSE)
	
	# plot the % difference histograms of new-orig and new-gtap and orig-gtap together on linear axes
	
	# there shouldn't be any zero mid-bin values
	posinds = which(area_pctdiff_raez_orig_valid_hist$mids > 0)
	neginds = which(area_pctdiff_raez_orig_valid_hist$mids < 0)
	nod_x = c(-log10(-area_pctdiff_raez_orig_valid_hist$mids[neginds]), log10(area_pctdiff_raez_orig_valid_hist$mids[posinds]))
	nod_y = area_pctdiff_raez_orig_valid_hist$counts
	posinds = which(area_pctdiff_raez_gtap_valid_hist$mids > 0)
	neginds = which(area_pctdiff_raez_gtap_valid_hist$mids < 0)
	new_x = c(-log10(-area_pctdiff_raez_gtap_valid_hist$mids[neginds]), log10(area_pctdiff_raez_gtap_valid_hist$mids[posinds]))
	new_x_labels = area_pctdiff_raez_gtap_valid_hist$mids
	new_y = area_pctdiff_raez_gtap_valid_hist$counts
	posinds = which(area_pctdiff_raez_orig_gtap_valid_hist$mids > 0)
	neginds = which(area_pctdiff_raez_orig_gtap_valid_hist$mids < 0)
	orig_x = c(-log10(-area_pctdiff_raez_orig_gtap_valid_hist$mids[neginds]), log10(area_pctdiff_raez_orig_gtap_valid_hist$mids[posinds]))
	orig_y = area_pctdiff_raez_orig_gtap_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(nod_y, new_y, orig_y)
	xmax = max(nod_x, new_x, orig_x)
	xmin = min(nod_x, new_x, orig_x)
	
	maintitle = paste(crop_names[j], " % harvested area difference histogram comparison", sep = "")
	xlab = "% harvested area difference"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = new_x_labels)
	par(new=TRUE)
	#barplot(width = rep(2, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "New - GTAP")
	lines(nod_x, nod_y, lty = 1, col = "black", lwd = 2)
	#lines(new_x, new_y, lty = 4, col = "blue", lwd = 2)
	lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
	#legend(x = "topright", lty = c(2, 4, 1), legend = c("Original - GTAP", "New - GTAP", "New - Original"), lwd = 2)
	legend(x = "topright", lty = c(2, 1), legend = c("Original AEZs - GTAP", "LDS AEZs - Original AEZs"), col = c("red3", "black"), lwd = 2)
	par(new=FALSE)
	
	dev.off()	# close harvested area histogram comparison file
	
}	# end for j loop over crop

cat("finished plot_lds_crop_raez.r at ",date(), "\n")
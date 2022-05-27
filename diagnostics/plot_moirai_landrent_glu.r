######
# plot_moirai_landrent_glu.r
#
# There are only six variables that the user should set (lines 101-116)
#   AGGREGATE:	TRUE=group to the # of regions determined by REG32; FALSE=do not group, output at 87-country level
#   REG32:		TRUE=use 32 gcam regions; FALSE=use 14 GCAM regions (this does not matter if GTAP==TRUE)
#						also note that this must match the number of GCAM regions used as input to moirai
#   papergray:	TRUE=plot grayscale figures; FALSE=plot color figures
#   lrname:		The moirai land rent output filename to diagnose
#   outdir:		The directory to write diagnostic figures to
#   num_aez:		The number of output GLUs (235 or 18 or custom to match the outputs in lrname)
#
# note that the Moirai land data system is sometimes labelled as "lds"
#	and "Original" refers to GENAEZECON, the first version of the land data system, which uses only 18 AEZs (the included data are for the original GTAP 18 AEZs)
#
# calc stats and make plots of the GTAP country87 by GLU level land rent outputs from genaez (original) and moirai
#	GTAP land rent is in million usd (year 2001 $): GTAP_value_milUSD.csv
#   GENAEZECON and Moirai land rent is in million usd (year 2001 $): GENAEZECON_value_milUSD.csv
#	GENAEZECON and Moirai diagnostic land rent is in USD
# Need to do this in USD to make the plotting easier for now
#
#	the base directory is:
#	 .../moirai/diagnostics/
#
# also plot the rglu level for both 14 and 32 regions
#
# compare the Moirai GLU data with Original GANAEZECON glu data (original 18 AEZs)
#	do this for ctry87glu, rglu 14 reg, and rglu 32 reg
# all files should not have any missing or NA values
# zeros represent no data
#
# the difference sums should be valid for the country/region level, regardless of matching glus
#
# Note that individual glu difference stats are valid only when Moirai GLUs are identical to gtap/genaezecon 18 AEZs
#	this includes the regressions and scatter plots
#
# Moirai LDS data format
# six header lines (the sixth contains the column labels)
# no zero values
# four colums: reglr_iso, glu_code, use_sector, value
#
# GENAEZECON and GTAP data format
# there are six header lines
# there are 87 countries and 13 use sectors, with use sector varying faster
# first column is country abbreviation
# second column is use sector abbreviation

# the diagnostic file has no header lines, and ctry87 and use codes
# 	GENAEZECON/GTAP: next 18 columns are the climate aezs in order 1-18
#	Moirai: there could be 18 columns or 235 columns for AEZs

# gtap ctry87 to gcam region mapping is inconsistent in these groups:
#  xea, xer, xfa, xna, xsa, xsd, xse, xsm, xss, xsu
#  for the most part, most of the countries in these groups go into one region (32 or 14)
#  a few cases are mixed, and most are consistent across 32 and 14 regions

# NOTE:
#  ks tests and difference stats and scatterplot are invalid when the aez numbers do not match
#  there are somewhat useful when looking at small shifts between the same number and numbering scheme of AEZs

# modified oct 2015 to incorporate Moirai outputs with different numbers of AEZs

# modified 19 may 2015 to produce some grayscale plots for the paper

# note that "aez" in this code now more generally means or refers to "glu"

# this script takes about 5 minutes for 18 moirai lds aezs

# this script takes about 10 minutes for 235 moirai lds aezs

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

cat("started plot_moirai_landrent_glu.r at ",date(), "\n")

# make sure that the working directory is .../moirai/diagnostics
setwd("./")

# three levels of aggregation: none, 14 region, 32 region
AGGREGATE = FALSE
REG32 = TRUE

papergray = FALSE

# use moirai lds output for new aezs
lrname = "../example_outputs/basins2352/MOIRAI_value_milUSD.csv"
#lrname = "../exmaple_outputs/aez_orig/MOIRAI_value_milUSD.csv"

# recommended outdir is in diagnostics because these are comparisons between cases (include final "/")
outdir = paste("./basins235_exmaple_outputs_stats_landrent/", sep="")
#outdir = paste("./aez_orig_example_outputs_stats_landrent/", sep="")

num_aez = 235
#num_aez = 18

dir.create(outdir, recursive = TRUE)

# other input data files

# and compare with the genaez orignal aez output
lrname_orig = "./GENAEZECON_value_milUSD.csv"
# and compare with the gtap data
lrname_gtap = "../indata/GTAP_value_milUSD.csv"

# input mapping files

# this has the GCAM ctry87 code, the abbr, and the name, in alphabetic order of abbreviations (and ascending code) in the first 3 columns
# this is the order of the ctry87 in the land rent output files
# this includes both 32 (columns 4 and 5) and 14 (columns 6 and 7) region mappings; code first, then name
# seven columns, one header line
countryname_gtap = "./GTAP_GCAM_ctry87_32reg.csv"

# number of regions and output names
if (AGGREGATE) {
	if (REG32) {
		ptag_ctry = "_rglu_32reg.pdf"
		ctag_ctry = "_rglu_32reg.csv"
		num_reg = 32
		region_gcam_fname = "../indata/GCAM_region_names_32reg.csv"
	} else {
		ptag_ctry = "_rglu_14reg.pdf"
		ctag_ctry = "_rglu_14reg.csv"
		num_reg = 14
		region_gcam_fname = "../indata/GCAM_region_names_14reg.csv"
	}
} else {
	ptag_ctry = "_ctry87glu.pdf"
	ctag_ctry = "_ctry87glu.csv"
	num_reg = 87
}

num_ctry = 87
num_use = 13
num_aez_gtap = 18
num_recs = num_ctry * num_use
num_ctry87aez = num_ctry * num_aez
num_raez = num_reg * num_aez
num_ctry87aez_gtap = num_ctry * num_aez_gtap
num_raez_gtap = num_reg * num_aez_gtap
num_colskip = 2

# set the number of raezs to process
# just use the maximum of lds and gtap num_aez -> lds should always be >= gtap for num_aez
if (AGGREGATE) {
	if (REG32) {
		num_proc_aez = num_raez
	} else {
		num_proc_aez = num_raez
	}
} else {
	num_proc_aez = num_ctry87aez
}

ONE2MIL = 1 / 1000000.0

nhead_lds = 5   # don't skip the column label row
#intype = as.list(character(num_aez + num_colskip))
intype_gtap = as.list(character(num_aez_gtap + num_colskip))
nhead = 6
ndiaghead = 0

#  country mapping info
temp<-scan(countryname_gtap, what=as.list(character(7)), skip=1, sep = ",", quote = "\"")
countrycode_gtap = as.integer(unlist(temp[1]))
countryabbr_gtap = unlist(temp[2])
countryname_gtap = unlist(temp[3])
if (REG32) {
	regioncode = as.integer(unlist(temp[4]))
	regionname = unlist(temp[5])
} else {
	regioncode = as.integer(unlist(temp[6]))
	regionname = unlist(temp[7])
}


# allocate the full arrays, and use the num_proc_aez variable to determine what part of the arrays to use
# any extra elements in the gtap/genaezecon arrays should just end up as zero or NA

# 3d array for in values
lr = array(dim=c(num_ctry, num_use, num_aez))
lr_orig = array(dim=c(num_ctry, num_use, num_aez))
lr_gtap = array(dim=c(num_ctry, num_use, num_aez))
lr[,,] = 0.0
lr_orig[,,] = 0.0
lr_gtap[,,] = 0.0

# 2d array of uses by country87aez
# the first dimension has ctry87 and aez, with aez changing faster
# second dimension has use
lr_ctry = array(dim=c(num_ctry87aez, num_use))
lr_ctry_orig = array(dim=c(num_ctry87aez, num_use))
lr_ctry_gtap = array(dim=c(num_ctry87aez, num_use))
lr_ctry[,] = 0.0
lr_ctry_orig[,] = 0.0
lr_ctry_gtap[,] = 0.0

# 1d arrays of differences (lds - original and original - gtap) per use
lr_diff_orig = array(dim = num_ctry87aez)
lr_diff_gtap = array(dim = num_ctry87aez)

# 1d arrays of percent differences (lds - original and original - gtap) per use
lr_pctdiff_orig = array(dim = num_ctry87aez)
lr_pctdiff_gtap = array(dim = num_ctry87aez)

# arrays for regression values
neworigind = 1
newgtapind = 2
gtaporigind = 3
lr_intercept = array(dim = c(num_use, 3))
lr_intercepterr = array(dim = c(num_use, 3))
lr_slope = array(dim = c(num_use, 3))
lr_slopeerr = array(dim = c(num_use, 3))
lr_r2 = array(dim = c(num_use, 3))
lr_pval_slope1 = array(dim = c(num_use, 3))
lr_n = array(dim = c(num_use, 3))
min_neworig_lr = 9000000000
max_neworig_lr = -9999
min_gtaporig_lr = 9000000000
max_gtaporig_lr = -9999

# land rent output
lr_in = read.csv(lrname, skip = nhead_lds)
#lr_in<-scan(lrname, what=intype, skip=nhead, sep = ",", quote = "\"")
lr_in_orig<-scan(lrname_orig, what=intype_gtap, skip=nhead, sep = ",", quote = "\"")
lr_in_gtap<-scan(lrname_gtap, what=intype_gtap, skip=nhead, sep = ",", quote = "\"")

# use abbreviations and names
use_abbrs = c("c_b", "ctl", "frs", "gro", "oap", "ocr", "osd", "pdr", "pfb", "rmk", "v_f", "wht", "wol")
use_names = c("SugarCrops", "Ruminants", "Forestry", "CerealGrains", "SwinePoultry", "OtherCrops", "OilCrops", "PaddyRice",
	"PlantFibers", "RawMilk", "VegFruitNut", "Wheat", "AnimalFibers")
use_codes = c(6, 9, 13, 3, 10, 8, 5, 1, 7, 11, 4, 2, 12)

# region codes and names
if(AGGREGATE) {
	temp<-scan(region_gcam_fname, what=as.list(character(2)), skip=4, sep = ",", quote = "\"")
	region_codes = as.integer(unlist(temp[1]))
	region_names = unlist(temp[2])
}

# put the data in 3d arrays and also in the 2d working arrays
# aggregate to gcam raez if applicable
cat("filling genaezecon/gtap 3d arrays\n")
for(i in 1:num_aez_gtap) {
	lr_col_orig = as.double(unlist(lr_in_orig[i+num_colskip]))
	lr_col_gtap = as.double(unlist(lr_in_gtap[i+num_colskip]))
	for(j in 1:num_ctry) {
		# determine the region index depending on aggregation (32vs14 reg determined above)
		if(AGGREGATE) {
			reg_index = regioncode[j]
		} else {
			reg_index = j
		}
		for(k in 1:num_use) {
			in_index = (j - 1) * num_use + k
			lr_orig[reg_index, k, i] = lr_orig[reg_index, k, i] + lr_col_orig[in_index] / ONE2MIL
			lr_gtap[reg_index, k, i] = lr_gtap[reg_index, k, i] + lr_col_gtap[in_index] / ONE2MIL
			
			# the first dimension has ctry87 and aez, with aez changing faster
			# second dimension has use
			twodim_ind = (reg_index - 1) * num_aez + i
			lr_ctry_orig[twodim_ind, k] = lr_orig[reg_index, k, i]
			lr_ctry_gtap[twodim_ind, k] = lr_gtap[reg_index, k, i]
		}	# end for k loop over number of use sectors
	}	# end for j loop over number of countries
}	# end for i loop over aez columns

cat("filling lds 3d arrays\n")

for(j in 1:num_ctry) {
    # determine the region index depending on aggregation (32vs14 reg determined above)
    if(AGGREGATE) {
        reg_index = regioncode[j]
    } else {
        reg_index = j
    }
    for(k in 1:num_use) {
        s = subset(lr_in, lr_in$reglr_iso == countryabbr_gtap[j] & lr_in$use_sector == use_abbrs[k])
        v = s$value / ONE2MIL
        a = s$glu_code
        if(length(v) > 0) {lr[reg_index, k, a] = lr[reg_index, k, a] + v}
        
        # the first dimension has ctry87 and aez, with aez changing faster
        # second dimension has use
        for(i in a) {
            twodim_ind = (reg_index - 1) * num_aez + i
            lr_ctry[twodim_ind, k] = lr[reg_index, k, i]
        }
    }	# end for k loop over number of use sectors
}	# end for j loop over number of countries

# start the ks test result files here

ksname_lr = paste(outdir, "ks_lr_usd", ctag_ctry, sep="")
cat("use_lr,ds_lds_orig,pv_lds_orig,ds_orig_gtap,pv_orig_gtap", sep = "", file = ksname_lr)

# calc stats by use sector for each aez within each country87aez or raez
for(j in 1:num_use) {
	cat("processing use index", j, ": use code", use_codes[j], use_names[j], "\n")
	
	oname_lr = paste(outdir, use_names[j], "_lr_usd", ctag_ctry, sep="")
	cat("ctry/reg,glu,lds,orig,gtap,lds-orig,orig-gtap,(lds-orig)/orig*100,(orig-gtap)/gtap*100", sep = "", file = oname_lr)
	
	# initialize arrays to store differences for this use sector
	lr_diff_orig[] = 0.0
	lr_diff_gtap[] = 0.0
	lr_pctdiff_orig[] = 0.0
	lr_pctdiff_gtap[] = 0.0
	
	# arrays to store valid indices for this use sector
	lr_inds = NULL
	lr_orig_inds = NULL
	lr_gtap_inds = NULL
	lr_diff_orig_inds = NULL
	lr_diff_gtap_inds = NULL
	
	# count variables for this use sector
	lr_num_new_no_orig = 0
	lr_num_orig_no_new = 0
	lr_num_no_new_no_orig = 0
	lr_num_orig_no_gtap = 0
	lr_num_gtap_no_orig = 0
	lr_num_no_orig_no_gtap = 0
	
	for(i in 1:num_proc_aez) {	# num_proc_aez can be num_ctry87aez, or num_raez for 32 or 14 regions
		# find valid indices of absolute values
		# find valid difference indices (both data sets are valid)
		# count the number of mismatched data entries (one data set is zero and the other is not)
		# count the number of both zero data entries
		
		# do this for moirai lds and orig and gtap
		# but keep track only of lds-orig and orig-gtap; do not keep track of lds-gtap
		
		if(lr_ctry[i, j] != 0) { # lds
			lr_inds = c(lr_inds, i)
			if(lr_ctry_orig[i, j] != 0) { # lds and orig
				lr_orig_inds = c(lr_orig_inds, i)
				lr_diff_orig[i] = lr_ctry[i, j] - lr_ctry_orig[i, j]
				lr_pctdiff_orig[i] = lr_diff_orig[i] / lr_ctry_orig[i, j] * 100
				lr_diff_orig_inds = c(lr_diff_orig_inds, i)
				if(lr_ctry_gtap[i, j] != 0) { # orig and gtap (and lds)
					lr_gtap_inds = c(lr_gtap_inds, i)
					lr_diff_gtap[i] = lr_ctry_orig[i, j] - lr_ctry_gtap[i, j]
					lr_pctdiff_gtap[i] = lr_diff_gtap[i] / lr_ctry_gtap[i, j] * 100
					lr_diff_gtap_inds = c(lr_diff_gtap_inds, i)
				} else { # orig and no gtap (and lds)
					lr_num_orig_no_gtap = lr_num_orig_no_gtap + 1
					lr_diff_gtap[i] = 0
					lr_pctdiff_gtap[i] = 0
				}
			} else { # lds and no orig
				lr_num_new_no_orig = lr_num_new_no_orig + 1
				lr_diff_orig[i] = 0
				lr_pctdiff_orig[i] = 0
				lr_diff_gtap[i] = 0
				lr_pctdiff_gtap[i] = 0
				if(lr_ctry_gtap[i, j] != 0) { # no orig and gtap (and lds)
					lr_gtap_inds = c(lr_gtap_inds, i)
					lr_num_gtap_no_orig = lr_num_gtap_no_orig + 1
				} else { # no orig and no gtap (and lds)
					lr_num_no_orig_no_gtap = lr_num_no_orig_no_gtap + 1
				}
			}
		} else { # no lds
			lr_diff_orig[i] = 0
			lr_pctdiff_orig[i] = 0
			if(lr_ctry_orig[i, j] != 0) { # no lds and orig
				lr_orig_inds = c(lr_orig_inds, i)
				lr_num_orig_no_new = lr_num_orig_no_new + 1
				if(lr_ctry_gtap[i, j] != 0) { # orig and gtap (no lds)
					lr_gtap_inds = c(lr_gtap_inds, i)
					lr_diff_gtap[i] = lr_ctry_orig[i, j] - lr_ctry_gtap[i, j]
					lr_pctdiff_gtap[i] = lr_diff_gtap[i] / lr_ctry_gtap[i, j] * 100
					lr_diff_gtap_inds = c(lr_diff_gtap_inds, i)
				} else { # orig and no gtap (no lds)
					lr_num_orig_no_gtap = lr_num_orig_no_gtap + 1
					lr_diff_gtap[i] = 0
					lr_pctdiff_gtap[i] = 0
				}
			} else { # no lds and no orig
				lr_num_no_new_no_orig = lr_num_no_new_no_orig + 1
				lr_diff_gtap[i] = 0
				lr_pctdiff_gtap[i] = 0
				if(lr_ctry_gtap[i, j] != 0) { # no orig and gtap (no lds)
					lr_gtap_inds = c(lr_gtap_inds, i)
					lr_num_gtap_no_orig = lr_num_gtap_no_orig + 1
				} else { # no orig and no gtap (no lds)
					lr_num_no_orig_no_gtap = lr_num_no_orig_no_gtap + 1
				}
			}
		}	# end else no new
		
		cind = as.integer(ceiling(i / num_aez))
		if(i %% num_aez == 0) {
			aval = num_aez
		}else {
			aval = i %% num_aez
		}
		
		if (AGGREGATE) {
			out_field = region_names[cind]
		} else {
			out_field = countryname_gtap[cind]
			# add double quotes to a field that includes a comma
			if(str_detect(out_field, ",")) {
 				out_field = paste("\"", out_field, "\"", sep = "")
 			}
 		}
 		
		# output per country/region values to csv file
		cat("\n", out_field, ",", aval, ",", lr_ctry[i, j], ",", lr_ctry_orig[i, j], ",", lr_ctry_gtap[i, j], ",",
			lr_diff_orig[i], ",", lr_diff_gtap[i], ",",
			round(lr_pctdiff_orig[i], digits = 2), ",", round(lr_pctdiff_gtap[i], digits = 2), sep = "", file = oname_lr, append = TRUE)

		# add the region sum at the end of the region aezs
		if (aval == num_aez) {
			sum_lr_ctry = sum(lr_ctry[(i-num_aez+1):i, j])
			sum_lr_ctry_orig = sum(lr_ctry_orig[(i-num_aez+1):i, j])
			sum_lr_ctry_gtap = sum(lr_ctry_gtap[(i-num_aez+1):i, j])
			cat("\n", out_field, ",", 0, ",", sum_lr_ctry, ",", sum_lr_ctry_orig, ",", sum_lr_ctry_gtap, ",",
			sum_lr_ctry - sum_lr_ctry_orig, ",", sum_lr_ctry_orig - sum_lr_ctry_gtap, ",",
			round(100 * (sum_lr_ctry - sum_lr_ctry_orig) / sum_lr_ctry_orig, digits = 2), ",",
			round(100 * (sum_lr_ctry_orig - sum_lr_ctry_gtap) / sum_lr_ctry_gtap, digits = 2),
			sep = "", file = oname_lr, append = TRUE)
		}

	}	# end for i loop over num_proc_aez
	
	# test distributions and generate histograms
	
	oname_lr = paste(outdir, use_names[j], "_lr_usd", ptag_ctry, sep="")

	# get the valid values
	lr_ctry_valid = lr_ctry[,j][lr_inds]
	lr_ctry_orig_valid = lr_ctry_orig[,j][lr_orig_inds]
	lr_ctry_gtap_valid = lr_ctry_gtap[,j][lr_gtap_inds]
	
	# first use kolmogorov-smirnov to test whether data could be sampled from the same continuous distribution
	# write these results to a separate csv file
	
	# new vs orig
	if(length(lr_ctry_valid) > 0 & length(lr_ctry_orig_valid) > 0) {
		ks_lr_new_orig_ctry = ks.test(lr_ctry_valid, lr_ctry_orig_valid)
		ds_lr_new_orig_ctry = round(ks_lr_new_orig_ctry$statistic, digits = 4)
		pv_lr_new_orig_ctry = round(ks_lr_new_orig_ctry$p.value, digits = 4)
	}else {
		ds_lr_new_orig_ctry = -1
		pv_lr_new_orig_ctry = -1
	}
	
	# orig vs gtap
	if(length(lr_ctry_orig_valid) > 0 & length(lr_ctry_gtap_valid) > 0) {
		ks_lr_orig_gtap_ctry = ks.test(lr_ctry_orig_valid, lr_ctry_gtap_valid)
		ds_lr_orig_gtap_ctry = round(ks_lr_orig_gtap_ctry$statistic, digits = 4)
		pv_lr_orig_gtap_ctry = round(ks_lr_orig_gtap_ctry$p.value, digits = 4)
	}else {
		ds_lr_orig_gtap_ctry = -1
		pv_lr_orig_gtap_ctry = -1
	}

	cat("\n", use_names[j], ",", ds_lr_new_orig_ctry, ",", pv_lr_new_orig_ctry, ",",
		ds_lr_orig_gtap_ctry, ",", pv_lr_orig_gtap_ctry, sep = "", file = ksname_lr, append = TRUE)
	
	# lr histograms
	
	if(length(c(lr_ctry_valid, lr_ctry_orig_valid, lr_ctry_gtap_valid)) > 0) {
		xmax = ceiling(max(lr_ctry_valid, lr_ctry_orig_valid, lr_ctry_gtap_valid) / 10000.0) * 10000
	} else {
		xmax = 0
	}
	if(xmax == 0) {
		breaks = c(0:1)
	} else {
		breaks = c(0:(xmax / 10000)) * 10000
	}
	
	pdf(file=oname_lr,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	title = paste(use_names[j], "Moirai GLUs")
	num_vals = length(lr_ctry_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		lr_ctry_valid_hist = hist(lr_ctry_valid, main = title, sub = sub, xlab = "Land rent (USD)",
			breaks = breaks)
	}
	
	title = paste(use_names[j], "Original GLUs")
	num_vals = length(lr_ctry_orig_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		lr_ctry_orig_valid_hist = hist(lr_ctry_orig_valid, main = title, sub = sub, xlab = "Land rent (USD)",
			breaks = breaks)
	}
	
	title = paste(use_names[j], "GTAP")
	num_vals = length(lr_ctry_gtap_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		lr_ctry_gtap_valid_hist = hist(lr_ctry_gtap_valid, main = title, sub = sub, xlab = "Land rent (USD)",
			breaks = breaks)
	}
	
	# differences
	
	# lds minus original
	if(length(lr_diff_orig[lr_diff_orig_inds]) > 0) {
		xmax = ceiling(max(lr_diff_orig[lr_diff_orig_inds]) / 1000.0) * 1000
		xmin = floor(min(lr_diff_orig[lr_diff_orig_inds]) / 1000.0) * 1000
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 1000):(xmax / 1000)) * 1000
	}
	
	title = paste(use_names[j], "Moirai GLUs - Original GLUs")
	num_vals = length(lr_diff_orig[lr_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(lr_diff_orig[lr_diff_orig_inds]), digits = 2)
		stddev = round(sd(lr_diff_orig[lr_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", lr_num_new_no_orig, "; # of missing lds values", lr_num_orig_no_new)
		lr_diff_orig_valid_hist = hist(lr_diff_orig[lr_diff_orig_inds], main = title, xlab = "Land rent (USD)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# original minus gtap
	if(length(lr_diff_gtap[lr_diff_gtap_inds]) > 0) {
		xmax = ceiling(max(lr_diff_gtap[lr_diff_gtap_inds]) / 1000.0) * 1000
		xmin = floor(min(lr_diff_gtap[lr_diff_gtap_inds]) / 1000.0) * 1000
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		breaks = c((xmin / 1000):(xmax / 1000)) * 1000
	}
	
	title = paste(use_names[j], "Original GLUs - GTAP")
	num_vals = length(lr_diff_gtap[lr_diff_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(lr_diff_gtap[lr_diff_gtap_inds]), digits = 2)
		stddev = round(sd(lr_diff_gtap[lr_diff_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", lr_num_orig_no_gtap, "; # of missing original values", lr_num_gtap_no_orig)
		lr_diff_gtap_valid_hist = hist(lr_diff_gtap[lr_diff_gtap_inds], main = title, xlab = "Land rent (USD)",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# percent differences
	
	# new minus original
	if(length(lr_pctdiff_orig[lr_diff_orig_inds]) > 0) {
		xmax = ceiling(max(lr_pctdiff_orig[lr_diff_orig_inds]) / 2.0) * 2
		# if xmax is too large the plots crash because there are too many breaks
		xmin = floor(min(lr_pctdiff_orig[lr_diff_orig_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		if(xmax > 1000000) {
			# find the second highest value
			mind = which(lr_pctdiff_orig == max(lr_pctdiff_orig[lr_diff_orig_inds]))
			xmax2 = lr_pctdiff_orig[-mind]
			breaks = c((xmin / 2):(xmax2 / 2)) * 2
			breaks = c(breaks, xmax)
		}else {
			breaks = c((xmin / 2):(xmax / 2)) * 2
		}
	}
	
	title = paste(use_names[j], "Moirai GLUs - Original GLUs")
	num_vals = length(lr_pctdiff_orig[lr_diff_orig_inds])
	if(num_vals != 0) {
		avg = round(mean(lr_pctdiff_orig[lr_diff_orig_inds]), digits = 2)
		stddev = round(sd(lr_pctdiff_orig[lr_diff_orig_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing original values", lr_num_new_no_orig, "; # of missing lds values", lr_num_orig_no_new)
		lr_pctdiff_orig_valid_hist = hist(lr_pctdiff_orig[lr_diff_orig_inds], main = title, xlab = "% land rent difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	# original minus gtap
	if(length(lr_pctdiff_gtap[lr_diff_gtap_inds]) > 0) {
		xmax = ceiling(max(lr_pctdiff_gtap[lr_diff_gtap_inds]) / 2.0) * 2
		# if xmax is too large the plots crash because there are too many breaks
		xmin = floor(min(lr_pctdiff_gtap[lr_diff_gtap_inds]) / 2.0) * 2
	} else {
		xmax = 0
		xmin = 0
	}
	if(xmax == xmin) {
		breaks = c(xmin:(xmin + 1))
	} else {
		if(xmax > 1000000) {
			# find the second highest value
			mind = which(lr_pctdiff_gtap == max(lr_pctdiff_gtap[lr_diff_gtap_inds]))
			xmax2 = lr_pctdiff_gtap[-mind]
			breaks = c((xmin / 2):(xmax2 / 2)) * 2
			breaks = c(breaks, xmax)
		}else {
			breaks = c((xmin / 2):(xmax / 2)) * 2
		}
	}
	
	title = paste(use_names[j], "Original GLUs - GTAP")
	num_vals = length(lr_pctdiff_gtap[lr_diff_gtap_inds])
	if(num_vals != 0) {
		avg = round(mean(lr_pctdiff_gtap[lr_diff_gtap_inds]), digits = 2)
		stddev = round(sd(lr_pctdiff_gtap[lr_diff_gtap_inds]), digits = 2)
		sub = paste("# of valid values", num_vals, "; mean =", avg, "; stddev =", stddev, "\n",
			"# of missing gtap values", lr_num_orig_no_gtap, "; # of missing original values", lr_num_gtap_no_orig)
		lr_pctdiff_gtap_valid_hist = hist(lr_pctdiff_gtap[lr_diff_gtap_inds], main = title, xlab = "% land rent difference",
			breaks = breaks)
		title(sub = sub, line = 5)
	}
	
	dev.off()	# close lr histogram file

#############################
	
	# generate histogram comparison plots
	
	oname_lr = paste(outdir, use_names[j], "_lr_usd_comp", ptag_ctry, sep="")
	
	# lr histogram comparison
	
	pdf(file=oname_lr,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	# plot all 3 absolute value histograms on a linear-log plot with new aezs as solid line and new and orig as broken lines
	
	if(length(lr_ctry_orig_valid) > 0) {
		orig_x = log10(lr_ctry_orig_valid_hist$mids)
		orig_y = lr_ctry_orig_valid_hist$counts
	}else {
		orig_x = 0
		orig_y = 0
		cat("No original glu values for use sector", use_names[j], "\n")
	}
	if(length(lr_ctry_valid) > 0) {
		new_x = log10(lr_ctry_valid_hist$mids)
		new_y = lr_ctry_valid_hist$counts
	}else {
		new_x = orig_x
		new_y = orig_y
		cat("No moirai glu values for use sector", use_names[j], "\n")
	}
	if(length(lr_ctry_gtap_valid) > 0) {
		gtap_x = log10(lr_ctry_gtap_valid_hist$mids)
		gtap_y = lr_ctry_gtap_valid_hist$counts
	}else {
		gtap_x = orig_x
		gtap_y = orig_y
		cat("No gtap values for use sector", use_names[j], "\n")
	}
	
	ymax = max(new_y, orig_y, gtap_y)
	xmax = max(new_x, orig_x, gtap_x)
	xmin = min(new_x, orig_x, gtap_x)
	
	maintitle = paste(use_names[j], " land rent histogram comparison", sep = "")
	xlab = "Land rent (USD)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
	lines(new_x, new_y,  lty = 1, col = "black", lwd = 2)	
	lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
	legend(x = "topright", lty = c(4, 2, 1), legend = c("GTAP", "Original GLUs", "Moirai GLUs"), col = c("blue3", "red3", "black"), lwd = 2)
	par(new=FALSE)
	
	# plot the above comparison using CDFs
	
	if(length(lr_ctry_orig_valid) > 0) {
		orig_x = log10(lr_ctry_orig_valid_hist$mids)
		div = sum(lr_ctry_orig_valid_hist$counts)
		if(div == 0) {div = 1}
		orig_y = cumsum(lr_ctry_orig_valid_hist$counts) / div
	}else {
		orig_x = 0
		orig_y = 0
		div = 1
		cat("No original glu values for use sector", use_names[j], "\n")
	}
	if(length(lr_ctry_valid) > 0) {
		new_x = log10(lr_ctry_valid_hist$mids)
		div = sum(lr_ctry_valid_hist$counts)
		if(div == 0) {div = 1}
		new_y = cumsum(lr_ctry_valid_hist$counts) / div
	}else {
		new_x = orig_x
		new_y = orig_y
		cat("No moirai glu values for use sector", use_names[j], "\n")
	}
	if(length(lr_ctry_gtap_valid) > 0) {
		gtap_x = log10(lr_ctry_gtap_valid_hist$mids)
		div = sum(lr_ctry_gtap_valid_hist$counts)
		if(div == 0) {div = 1}
		gtap_y = cumsum(lr_ctry_gtap_valid_hist$counts) / div
	}else {
		gtap_x = orig_x
		gtap_y = orig_y
		div = 1
		cat("No gtap values for use sector", use_names[j], "\n")
	}
	
	ymax = max(new_y, orig_y, gtap_y)
	ymin = 0
	xmax = max(new_x, orig_x, gtap_x)
	xmin = min(new_x, orig_x, gtap_x)
	
	maintitle = paste(use_names[j], " land rent cumulative distribution comparison", sep = "")
	xlab = "Land rent (USD)"
	ylab = "Cumulative probability"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = 10^new_x)
	par(new=TRUE)
	
	if (papergray) {
		lines(gtap_x, gtap_y, lty = 4, col = "gray60", lwd = 2)
		lines(new_x, new_y,  lty = 1, col = "black", lwd = 2)		
		lines(orig_x, orig_y, lty = 2, col = "gray30", lwd = 2)
		legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Original GLUs", "Moirai GLUs"), col = c("gray60", "gray30", "black"), lwd = 2)
	} else {
		lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
		lines(new_x, new_y,  lty = 1, col = "black", lwd = 2)		
		lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
		legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Original GLUs", "Moirai GLUs"), col = c("blue3", "red3", "black"), lwd = 2)
	}
	
	par(new=FALSE)
	
	# plot the difference histograms of orig-gtap and lds-orig together on linear-log axes
	
	# differences between new and original aezs are problematic because of the different number of land units
	# need to deal with the negatives?
	# FAO needs to be replaced with another GENAEZECON output
	# should skip this for now
	if(FALSE) {
	
	# there shouldn't be any zero mid-bin values
	posinds = which(lr_diff_orig_valid_hist$mids > 0)
	neginds = which(lr_diff_orig_valid_hist$mids < 0)
	nod_x = c(-log10(-lr_diff_orig_valid_hist$mids[neginds]), log10(lr_diff_orig_valid_hist$mids[posinds]))
	nod_y = lr_diff_orig_valid_hist$counts
	posinds = which(lr_diff_ctry_fao_valid_hist$mids > 0)
	neginds = which(lr_diff_ctry_fao_valid_hist$mids < 0)
	new_x = c(-log10(-lr_diff_ctry_fao_valid_hist$mids[neginds]), log10(lr_diff_ctry_fao_valid_hist$mids[posinds]))
	new_x_labels = lr_diff_ctry_fao_valid_hist$mids
	new_y = lr_diff_ctry_fao_valid_hist$counts
	posinds = which(lr_diff_orig_fao_valid_hist$mids > 0)
	neginds = which(lr_diff_orig_fao_valid_hist$mids < 0)
	orig_x = c(-log10(-lr_diff_orig_fao_valid_hist$mids[neginds]), log10(lr_diff_orig_fao_valid_hist$mids[posinds]))
	orig_y = lr_diff_orig_fao_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(use_names[j], " land rent difference histogram comparison", sep = "")
	xlab = "Land rent difference (USD)"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xaxt = "n", xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	axis(side = 1, at = new_x, labels = new_x_labels)
	par(new=TRUE)
	#barplot(width = rep(1, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GENAEZECON - FAO")
	lines(new_x, new_y, lty = 1, col = "red3")
	lines(orig_x, orig_y, lty = 2, col = "blue3")
	if(FALSE) {
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Original GLUs - FAO"), col = c("blue3", "red3"))
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"))
	}
	par(new=FALSE)
	
	# plot the % difference histograms of orig-gtap and new-orig together on linear axes
	
	new_x = lr_pctdiff_ctry_fao_valid_hist$mids
	new_y = lr_pctdiff_ctry_fao_valid_hist$counts
	orig_x = lr_pctdiff_orig_fao_valid_hist$mids
	orig_y = lr_pctdiff_orig_fao_valid_hist$counts
	num_bins = length(new_x)
	
	ymax = max(new_y, orig_y)
	xmax = max(new_x, orig_x)
	xmin = min(new_x, orig_x)
	
	maintitle = paste(use_names[j], " % land rent difference histogram comparison", sep = "")
	xlab = "% land rent difference"
	ylab = "Frequency"
	plot.default(x = new_x, y = new_y, type = "n",
		xlab = xlab, main = maintitle, ylab = ylab,
		ylim = c(0, ymax), xlim = c(xmin, xmax))
	par(new=TRUE)
	#barplot(width = rep(2, num_bins), height = new_y, yaxt = "n", xaxt = "n",
	#	col = "gray90", space = 0, axes = FALSE, axisnames = FALSE, add = TRUE, legend.text = "GENAEZECON - FAO")
	lines(new_x, new_y, lty = 1, col = "red3")
	lines(orig_x, orig_y, lty = 2, col = "blue3")
	if(FALSE) {
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Original GLUs - FAO"), col = c("blue3", "red3"))
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original GLUs - FAO", "Moirai GLUs - FAO"), col = c("blue3", "red3"))
	}
	par(new=FALSE)
	
	}	 # end if(FALSE)
	
	dev.off()	# close land rent histogram comparison file
	
	###################################
	#### adding these so need to test to make sure they work
	# don't forget that the close of the j loop is below before regression
	# these work only if the GLUs are identical
	
	# scatter plots of land rent across the three data sets
	
	# pull the two orig outliers and the one gtap outlier from other crops to check it out
	# nothing to note about this
	# but don't pull them out below for the final results
	if(FALSE) {
	#if(j == 6) {
		bad_orig_inds = which(lr_ctry_orig[,j] > 3000000000)
		bad_gtap_inds = which(lr_ctry_gtap[,j] > 3000000000)
		
		cat("\norig outlier 1:\n")
		cat("orig=", lr_ctry_orig[bad_orig_inds[1],j], "\n")
		cat("gtap=", lr_ctry_gtap[bad_orig_inds[1],j], "\n")
		cat("new=", lr_ctry[bad_orig_inds[1],j], "\n")
		cat("\norig outlier 2:\n")
		cat("orig=", lr_ctry_orig[bad_orig_inds[2],j], "\n")
		cat("gtap=", lr_ctry_gtap[bad_orig_inds[2],j], "\n")
		cat("new=", lr_ctry[bad_orig_inds[2],j], "\n")
		cat("\ngtap outlier 1:\n")
		cat("orig=", lr_ctry_orig[bad_gtap_inds[1],j], "\n")
		cat("gtap=", lr_ctry_gtap[bad_gtap_inds[1],j], "\n")
		cat("new=", lr_ctry[bad_gtap_inds[1],j], "\n")
	}
	
	oname_lr = paste(outdir, use_names[j], "_lr_usd_scatter", ptag_ctry, sep="")
	#oname_lr = paste(outdir, use_names[j], "_lr_usd_scatter_cleanoc", ptag_ctry, sep="")
	pdf(file=oname_lr,paper="letter")
	par(mar = c(6, 4, 4, 2) + 0.1)
	
	new_orig_lr_inds = intersect(lr_inds, lr_orig_inds)
	if(j == 6) {
		#new_orig_lr_inds = setdiff(new_orig_lr_inds,bad_orig_inds)
		#new_orig_lr_inds = setdiff(new_orig_lr_inds,bad_gtap_inds)
	}
	
	if(length(new_orig_lr_inds) > 0) {
		
		title = paste("New vs Orig", use_names[j], "land rent")
		xlab = "Orig Land Rent (USD)"
		ylab = "New Land Rent (USD)"
		xmin = min(lr_ctry_orig[new_orig_lr_inds,j], lr_ctry[new_orig_lr_inds,j])
		xmax = max(lr_ctry_orig[new_orig_lr_inds,j], lr_ctry[new_orig_lr_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = lr_ctry_orig[new_orig_lr_inds,j], y = lr_ctry[new_orig_lr_inds,j], type = "n",
		xlab = xlab, main = title, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(lr_ctry_orig[new_orig_lr_inds,j], lr_ctry[new_orig_lr_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
		
		if (xmin < min_neworig_lr) { min_neworig_lr = xmin }
		if (xmax > max_neworig_lr) { max_neworig_lr = xmax }
		
		# regression
		if(length(new_orig_lr_inds) > 1)
		{
			n = length(new_orig_lr_inds)
			yvals = lr_ctry[new_orig_lr_inds,j]
			xvals = lr_ctry_orig[new_orig_lr_inds,j]
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
			lr_intercept[j, neworigind] = smry$coefficients[1,1]
			lr_intercepterr[j, neworigind] = smry$coefficients[1,2]
			lr_slope[j, neworigind] = smry$coefficients[2,1]
			lr_slopeerr[j, neworigind] = smry$coefficients[2,2]
			lr_r2[j, neworigind] = smry$r.squared
			lr_pval_slope1[j, neworigind] = p1
			lr_n[j, neworigind] = n
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
			
			lr_intercept[j, neworigind] = NA
			lr_intercepterr[j, neworigind] = NA
			lr_slope[j, neworigind] = NA
			lr_slopeerr[j, neworigind] = NA
			lr_r2[j, neworigind] = NA
			lr_pval_slope1[j, neworigind] = NA
			lr_n[j, neworigind] = NA
		} # end regression
		
	} # end if data exist to plot
	
	new_gtap_lr_inds = intersect(lr_inds, lr_gtap_inds)
	if(j == 6) {
		#new_gtap_lr_inds = setdiff(new_gtap_lr_inds,bad_orig_inds)
		#new_gtap_lr_inds = setdiff(new_gtap_lr_inds,bad_gtap_inds)
	}
	
	if(length(new_gtap_lr_inds) > 0) {
		
		title = paste("New vs GTAP", use_names[j], "land rent")
		xlab = "GTAP Land Rent (USD)"
		ylab = "New Land Rent (USD)"
		xmin = min(lr_ctry_gtap[new_gtap_lr_inds,j], lr_ctry[new_gtap_lr_inds,j])
		xmax = max(lr_ctry_gtap[new_gtap_lr_inds,j], lr_ctry[new_gtap_lr_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = lr_ctry_gtap[new_gtap_lr_inds,j], y = lr_ctry[new_gtap_lr_inds,j], type = "n",
		xlab = xlab, main = title, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(lr_ctry_gtap[new_gtap_lr_inds,j], lr_ctry[new_gtap_lr_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
		
		# regression
		if(length(new_gtap_lr_inds) > 1)
		{
			n = length(new_gtap_lr_inds)
			yvals = lr_ctry[new_gtap_lr_inds,j]
			xvals = lr_ctry_gtap[new_gtap_lr_inds,j]
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
			lr_intercept[j, newgtapind] = smry$coefficients[1,1]
			lr_intercepterr[j, newgtapind] = smry$coefficients[1,2]
			lr_slope[j, newgtapind] = smry$coefficients[2,1]
			lr_slopeerr[j, newgtapind] = smry$coefficients[2,2]
			lr_r2[j, newgtapind] = smry$r.squared
			lr_pval_slope1[j, newgtapind] = p1
			lr_n[j, newgtapind] = n
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
			
			lr_intercept[j, newgtapind] = NA
			lr_intercepterr[j, newgtapind] = NA
			lr_slope[j, newgtapind] = NA
			lr_slopeerr[j, newgtapind] = NA
			lr_r2[j, newgtapind] = NA
			lr_pval_slope1[j, newgtapind] = NA
			lr_n[j, newgtapind] = NA
		} # end regression
		
	} # end if data exist to plot
	
	orig_gtap_lr_inds = intersect(lr_orig_inds, lr_gtap_inds)
	if(j == 6) {
		#orig_gtap_lr_inds = setdiff(orig_gtap_lr_inds,bad_orig_inds)
		#orig_gtap_lr_inds = setdiff(orig_gtap_lr_inds,bad_gtap_inds)
	}
	
	if(length(orig_gtap_lr_inds) > 0) {
		
		title = paste("Orig vs GTAP", use_names[j], "land rent")
		xlab = "GTAP Land Rent (USD)"
		ylab = "Orig Land Rent (USD)"
		xmin = min(lr_ctry_gtap[orig_gtap_lr_inds,j], lr_ctry_orig[orig_gtap_lr_inds,j])
		xmax = max(lr_ctry_gtap[orig_gtap_lr_inds,j], lr_ctry_orig[orig_gtap_lr_inds,j])
		ymin = xmin
		ymax = xmax
		plot.default(x = lr_ctry_gtap[orig_gtap_lr_inds,j], y = lr_ctry_orig[orig_gtap_lr_inds,j], type = "n",
		xlab = xlab, main = title, ylab = ylab,
		ylim = c(ymin, ymax), xlim = c(xmin, xmax))
		par(new=TRUE)
		points(lr_ctry_gtap[orig_gtap_lr_inds,j], lr_ctry_orig[orig_gtap_lr_inds,j], col = "black", pch = 0, cex = 0.5)
		par(new=FALSE)
		
		if (xmin < min_gtaporig_lr) { min_gtaporig_lr = xmin }
		if (xmax > max_gtaporig_lr) { max_gtaporig_lr = xmax }
		
		# regression
		if(length(orig_gtap_lr_inds) > 1)
		{
			n = length(orig_gtap_lr_inds)
			yvals = lr_ctry_orig[orig_gtap_lr_inds,j]
			xvals = lr_ctry_gtap[orig_gtap_lr_inds,j]
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
			lr_intercept[j, gtaporigind] = smry$coefficients[1,1]
			lr_intercepterr[j, gtaporigind] = smry$coefficients[1,2]
			lr_slope[j, gtaporigind] = smry$coefficients[2,1]
			lr_slopeerr[j, gtaporigind] = smry$coefficients[2,2]
			lr_r2[j, gtaporigind] = smry$r.squared
			lr_pval_slope1[j, gtaporigind] = p1
			lr_n[j, gtaporigind] = n
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
			
			lr_intercept[j, gtaporigind] = NA
			lr_intercepterr[j, gtaporigind] = NA
			lr_slope[j, gtaporigind] = NA
			lr_slopeerr[j, gtaporigind] = NA
			lr_r2[j, gtaporigind] = NA
			lr_pval_slope1[j, gtaporigind] = NA
			lr_n[j, gtaporigind] = NA
		} # end regression
		
	} # end if data exist to plot
	
	dev.off()
	
}	# end for j loop over use sector

################ calculate and write the mean slope, intercept, r2, and pvalue for slope=1 across the use sectors

# do not include uses with n < min_use_n in any of the comparisons, in any of the means
# this is for a more direct comparison across the data sets

min_use_n = 20

lr_mean_neworig_inds = which(lr_n[,neworigind] >= min_use_n)
lr_mean_newgtap_inds = which(lr_n[,newgtapind] >= min_use_n)
lr_mean_gtaporig_inds = which(lr_n[,gtaporigind] >= min_use_n)
lr_mean_inds = intersect(intersect(lr_mean_neworig_inds, lr_mean_newgtap_inds), lr_mean_gtaporig_inds)

# output the number of selected uses for means
cat("\nUse n for regression means: min_use_n =", min_use_n, "\n")
cat("\t# of lr_mean_neworig_inds =", length(lr_mean_neworig_inds), "\n")
cat("\t# of lr_mean_gtaporig_inds =", length(lr_mean_gtaporig_inds), "\n")
cat("\t# of lr_mean_newgtap_inds =", length(lr_mean_newgtap_inds), "\n")
cat("\t# of lr_mean_inds =", length(lr_mean_inds), "\n")

oname_lr = paste(outdir, "all_lr_usd_regression", ctag_ctry, sep="")
#oname_lr = paste(outdir, "all_lr_usd_regression_cleanoc", ctag_ctry, sep="")

cat("comp,crop,n,slope,slope_stderr,intercept,intercept_stderr,r2,p1val\n", file = oname_lr)

# start with the new vs orig

lr_neworig_mean_n = mean(lr_n[lr_mean_inds,neworigind], na.rm = TRUE)
lr_neworig_mean_slope = mean(lr_slope[lr_mean_inds,neworigind], na.rm = TRUE)
lr_neworig_mean_slopeerr = mean(lr_slopeerr[lr_mean_inds,neworigind], na.rm = TRUE)
lr_neworig_mean_intercept = mean(lr_intercept[lr_mean_inds,neworigind], na.rm = TRUE)
lr_neworig_mean_intercepterr = mean(lr_intercepterr[lr_mean_inds,neworigind], na.rm = TRUE)
lr_neworig_mean_r2 = mean(lr_r2[lr_mean_inds,neworigind], na.rm = TRUE)
lr_neworig_mean_pval_slope1 = mean(lr_pval_slope1[lr_mean_inds,neworigind], na.rm = TRUE)

cat("New_Orig,select_mean", lr_neworig_mean_n, lr_neworig_mean_slope, lr_neworig_mean_slopeerr, lr_neworig_mean_intercept, lr_neworig_mean_intercepterr,
lr_neworig_mean_r2, lr_neworig_mean_pval_slope1, file = oname_lr, append = TRUE, sep = ",")

for	(j in 1:num_use) {
	cat("\nNew_Orig", use_names[j], lr_n[j,neworigind], lr_slope[j,neworigind], lr_slopeerr[j,neworigind], lr_intercept[j,neworigind], lr_intercepterr[j,neworigind],
	lr_r2[j,neworigind], lr_pval_slope1[j,neworigind], file = oname_lr, append = TRUE, sep = ",")
}

# new vs gtap

lr_newgtap_mean_n = mean(lr_n[lr_mean_inds,newgtapind], na.rm = TRUE)
lr_newgtap_mean_slope = mean(lr_slope[lr_mean_inds,newgtapind], na.rm = TRUE)
lr_newgtap_mean_slopeerr = mean(lr_slopeerr[lr_mean_inds,newgtapind], na.rm = TRUE)
lr_newgtap_mean_intercept = mean(lr_intercept[lr_mean_inds,newgtapind], na.rm = TRUE)
lr_newgtap_mean_intercepterr = mean(lr_intercepterr[lr_mean_inds,newgtapind], na.rm = TRUE)
lr_newgtap_mean_r2 = mean(lr_r2[lr_mean_inds,newgtapind], na.rm = TRUE)
lr_newgtap_mean_pval_slope1 = mean(lr_pval_slope1[lr_mean_inds,newgtapind], na.rm = TRUE)

cat("\nNew_GTAP,select_mean", lr_newgtap_mean_n, lr_newgtap_mean_slope, lr_newgtap_mean_slopeerr, lr_newgtap_mean_intercept, lr_newgtap_mean_intercepterr,
lr_newgtap_mean_r2, lr_newgtap_mean_pval_slope1, file = oname_lr, append = TRUE, sep = ",")

for	(j in 1:num_use) {
	cat("\nNew_GTAP", use_names[j], lr_n[j,newgtapind], lr_slope[j,newgtapind], lr_slopeerr[j,newgtapind], lr_intercept[j,newgtapind], lr_intercepterr[j,newgtapind],
	lr_r2[j,newgtapind], lr_pval_slope1[j,newgtapind], file = oname_lr, append = TRUE, sep = ",")
}

# this is actually orig vs gtap

lr_gtaporig_mean_n = mean(lr_n[lr_mean_inds,gtaporigind], na.rm = TRUE)
lr_gtaporig_mean_slope = mean(lr_slope[lr_mean_inds,gtaporigind], na.rm = TRUE)
lr_gtaporig_mean_slopeerr = mean(lr_slopeerr[lr_mean_inds,gtaporigind], na.rm = TRUE)
lr_gtaporig_mean_intercept = mean(lr_intercept[lr_mean_inds,gtaporigind], na.rm = TRUE)
lr_gtaporig_mean_intercepterr = mean(lr_intercepterr[lr_mean_inds,gtaporigind], na.rm = TRUE)
lr_gtaporig_mean_r2 = mean(lr_r2[lr_mean_inds,gtaporigind], na.rm = TRUE)
lr_gtaporig_mean_pval_slope1 = mean(lr_pval_slope1[lr_mean_inds,gtaporigind], na.rm = TRUE)

cat("\norig_GTAP,select_mean", lr_gtaporig_mean_n, lr_gtaporig_mean_slope, lr_gtaporig_mean_slopeerr, lr_gtaporig_mean_intercept, lr_gtaporig_mean_intercepterr,
lr_gtaporig_mean_r2, lr_gtaporig_mean_pval_slope1, file = oname_lr, append = TRUE, sep = ",")

for	(j in 1:num_use) {
	cat("\norig_GTAP", use_names[j], lr_n[j,gtaporigind], lr_slope[j,gtaporigind], lr_slopeerr[j,gtaporigind], lr_intercept[j,gtaporigind], lr_intercepterr[j,gtaporigind],
	lr_r2[j,gtaporigind], lr_pval_slope1[j,gtaporigind], file = oname_lr, append = TRUE, sep = ",")
}

### plot the regression means
oname_lr = paste(outdir, "all_lr_usd_regression", ptag_ctry, sep="")
#oname_lr = paste(outdir, "all_lr_usd_regression_cleanoc", ptag_ctry, sep="")

pdf(file=oname_lr,paper="letter")
par(mar = c(6, 4, 4, 2) + 0.1)

########### for the gtap raez level, plot the regression line orig vs gtap with the error lines


title = paste("Land Rent regression mean")
sub = paste("min_use_n =", min_use_n)
xlab = "GTAP Land Rent (USD)"
ylab = "Original Land Rent (USD)"
ymin = min_gtaporig_lr
ymax = max_gtaporig_lr
xmin = ymin
xmax = ymax
plot.default(x = c(xmin,xmax), y = c(ymin,ymax), type = "n",
xlab = xlab, main = title, ylab = ylab, sub = sub,
ylim = c(ymin, ymax), xlim = c(xmin, xmax))
par(new=TRUE)
lines(c(xmin,xmax), c(ymin,ymax), col = "gray50", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * lr_gtaporig_mean_slope + lr_gtaporig_mean_intercept, xmax * lr_gtaporig_mean_slope + lr_gtaporig_mean_intercept), col = "black", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * (lr_gtaporig_mean_slope + lr_gtaporig_mean_slopeerr) + (lr_gtaporig_mean_intercept + lr_gtaporig_mean_intercept),
xmax * (lr_gtaporig_mean_slope + lr_gtaporig_mean_slopeerr) + (lr_gtaporig_mean_intercept + lr_gtaporig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * (lr_gtaporig_mean_slope - lr_gtaporig_mean_slopeerr) + (lr_gtaporig_mean_intercept - lr_gtaporig_mean_intercept),
xmax * (lr_gtaporig_mean_slope - lr_gtaporig_mean_slopeerr) + (lr_gtaporig_mean_intercept - lr_gtaporig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)

par(new=FALSE)
lds_text = paste("Original: slope = ", round(lr_gtaporig_mean_slope,digits=2), " +- ", round(lr_gtaporig_mean_slopeerr,digits=2),
", r^2 = ", round(lr_gtaporig_mean_r2,digits=2), sep = "")
legend(x = "bottomright", lty = c(1,1), lwd = c(2,2), legend = c(lds_text, "GTAP: one-to-one line"), col = c("black", "gray50"))



########### for the gcam raez level, plot the regression line new vs orig with the error lines


title = paste("Land Rent regression mean")
sub = paste("min_use_n =", min_use_n)
xlab = "Original GLU Land Rent (USD)"
ylab = "Moirai GLU Land Rent (USD)"
ymin = min_neworig_lr
ymax = max_neworig_lr
xmin = ymin
xmax = ymax
plot.default(x = c(xmin,xmax), y = c(ymin,ymax), type = "n",
xlab = xlab, main = title, ylab = ylab, sub = sub,
ylim = c(ymin, ymax), xlim = c(xmin, xmax))
par(new=TRUE)
lines(c(xmin,xmax), c(ymin,ymax), col = "gray50", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * lr_neworig_mean_slope + lr_neworig_mean_intercept, xmax * lr_neworig_mean_slope + lr_neworig_mean_intercept), col = "black", lty = 1, lwd = 2)
lines(c(xmin,xmax), c(xmin * (lr_neworig_mean_slope + lr_neworig_mean_slopeerr) + (lr_neworig_mean_intercept + lr_neworig_mean_intercept),
xmax * (lr_neworig_mean_slope + lr_neworig_mean_slopeerr) + (lr_neworig_mean_intercept + lr_neworig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)
lines(c(xmin,xmax), c(xmin * (lr_neworig_mean_slope - lr_neworig_mean_slopeerr) + (lr_neworig_mean_intercept - lr_neworig_mean_intercept),
xmax * (lr_neworig_mean_slope - lr_neworig_mean_slopeerr) + (lr_neworig_mean_intercept - lr_neworig_mean_intercept)), col = "black", lty = 2, lwd = 1.5)

par(new=FALSE)
new_text = paste("Moirai GLUs: slope = ", round(lr_neworig_mean_slope,digits=2), " +- ", round(lr_neworig_mean_slopeerr,digits=2),
", r^2 = ", round(lr_neworig_mean_r2,digits=2), sep = "")
legend(x = "bottomright", lty = c(1,1), lwd = c(2,2), legend = c(new_text, "Original GLUs: one-to-one line"), col = c("black", "gray50"))

dev.off()

cat("finished plot_moirai_landrent_glu.r at ",date(), "\n")
######
# plot_lds_landrent_aez.r
#
# calc stats and make plots of the GTAP country87 by AEZ level land rent outputs from genaez and lds
#	GTAP land rent is in million usd (year 2001 $): GTAP_value_milUSD.csv
#   GENAEZECON/LDS land rent is in million usd (year 2001 $): GENAEZECON_value_milUSD.csv
#	GENAEZECON/LDS diagnostic land rent is in USD
# Need to do this in USD to make the plotting easier for now
#
#	the base directory is:
#	 .../lds/diagnostics/
#
# also plot the raez level for both 14 and 32 regions
#
# compare the LDS aez data with original GANAEZECON aez data
#	do this for ctry87aez, raez 14 reg, and raez 32 reg
# all files should not have any missing or NA values
# zeros represent no data
#
# the difference sums should be valid for the country/region level, regardless of matching aezs
#	but individual aez difference stats are valid only when lds num_aez == gtap/genaezecon num_aez_gtap
#
# LDS data format
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
#	LDS: there could be 18 columns or 235 columns for AEZs

# gtap ctry87 to region mapping is inconsistent in these groups:
#  xea, xer, xfa, xna, xsa, xsd, xse, xsm, xss, xsu
#  for the most part, most of the countries in these groups go into one reigon (32 or 14)
#  a few cases are mixed, and most are consistent across 32 and 14 regions

# NOTE:
#  ks tests and difference stats are invalid when the aez numbers do not match
#  there are somewhat useful when looking at small shifts between the same number and numbering scheme of AEZs

# modified oct 2015 to incorporate LDS outputs with different numbers of AEZs

# modified 19 may 2015 to produce some grayscale plots for the paper

# this script takes about 5 minutes for 18 lds aezs

# this script takes about 5 minutes for 235 lds aezs


library(stringr)

cat("started plot_lds_landrent_aez.r at ",date(), "\n")

setwd("./")

# three levels of aggregation: none, 14 region, 32 region
AGGREGATE = TRUE
REG32 = TRUE

papergray = FALSE

# recommended outdir is in diagnostics because these are comparisons between cases
outdir = paste("./basins235_stats_landrent/", sep="")
dir.create(outdir, recursive = TRUE)

# input data files

# use lds output for new aezs
lrname = "../outputs/basins235/LDS_value_milUSD.csv"
# and compare with the genaez orignal aez output
lrname_orig = "./GENAEZECON_value_milUSD.csv"
# and compare with the gtap data
lrname_gtap = "../indata/GTAP_value_milUSD.csv"

#num_aez = 18
num_aez = 235

# input mapping files

# this has the GCAM ctry87 code, the abbr, and the name, in alphabetic order of abbreviations (and ascending code) in the first 3 columns
# this is the order of the ctry87 in the land rent output files
# this includes both 32 (columns 4 and 5) and 14 (columns 6 and 7) region mappings; code first, then name
# seven columns, one header line
countryname_gtap = "./GTAP_GCAM_ctry87_32reg.csv"

# number of regions and output names
if (AGGREGATE) {
	if (REG32) {
		ptag_ctry = "_raez_32reg.pdf"
		ctag_ctry = "_raez_32reg.csv"
		num_reg = 32
		region_gcam_fname = "../indata/GCAM_region_names_32reg.csv"
	} else {
		ptag_ctry = "_raez_14reg.pdf"
		ctag_ctry = "_raez_14reg.csv"
		num_reg = 14
		region_gcam_fname = "../indata/GCAM_region_names_14reg.csv"
	}
} else {
	ptag_ctry = "_ctry87aez.pdf"
	ctag_ctry = "_ctry87aez.csv"
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
        s = subset(lr_in, reglr_iso == countryabbr_gtap[j] & use_sector == use_abbrs[k])
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
	cat("ctry/reg,aez,lds,orig,gtap,lds-orig,orig-gtap,(lds-orig)/orig*100,(orig-gtap)/gtap*100", sep = "", file = oname_lr)
	
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
		
		# do this for lds and orig and gtap
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
	
	title = paste(use_names[j], "LDS AEZs")
	num_vals = length(lr_ctry_valid)
	if(num_vals != 0) {
		sub = paste("# of valid values", num_vals)
		lr_ctry_valid_hist = hist(lr_ctry_valid, main = title, sub = sub, xlab = "Land rent (USD)",
			breaks = breaks)
	}
	
	title = paste(use_names[j], "Original AEZs")
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
	
	title = paste(use_names[j], "LDS AEZs - Original AEZs")
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
	
	title = paste(use_names[j], "Original AEZs - GTAP")
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
	
	title = paste(use_names[j], "LDS AEZs - Original AEZs")
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
	
	title = paste(use_names[j], "Original AEZs - GTAP")
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
		cat("No original aez values for use sector", use_names[j], "\n")
	}
	if(length(lr_ctry_valid) > 0) {
		new_x = log10(lr_ctry_valid_hist$mids)
		new_y = lr_ctry_valid_hist$counts
	}else {
		new_x = orig_x
		new_y = orig_y
		cat("No lds aez values for use sector", use_names[j], "\n")
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
	legend(x = "topright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("blue3", "red3", "black"), lwd = 2)
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
		cat("No original aez values for use sector", use_names[j], "\n")
	}
	if(length(lr_ctry_valid) > 0) {
		new_x = log10(lr_ctry_valid_hist$mids)
		div = sum(lr_ctry_valid_hist$counts)
		if(div == 0) {div = 1}
		new_y = cumsum(lr_ctry_valid_hist$counts) / div
	}else {
		new_x = orig_x
		new_y = orig_y
		cat("No lds aez values for use sector", use_names[j], "\n")
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
		legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("gray60", "gray30", "black"), lwd = 2)
	} else {
		lines(gtap_x, gtap_y, lty = 4, col = "blue3", lwd = 2)
		lines(new_x, new_y,  lty = 1, col = "black", lwd = 2)		
		lines(orig_x, orig_y, lty = 2, col = "red3", lwd = 2)
		legend(x = "bottomright", lty = c(4, 2, 1), legend = c("GTAP", "Original AEZs", "LDS AEZs"), col = c("blue3", "red3", "black"), lwd = 2)
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
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Original AEZs - FAO"), col = c("blue3", "red3"))
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original AEZs - FAO", "LDS AEZs - FAO"), col = c("blue3", "red3"))
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
		legend(x = "topright", lty = c(2, 1), legend = c("GTAP - FAO", "Original AEZs - FAO"), col = c("blue3", "red3"))
	} else {
		legend(x = "topright", lty = c(2, 1), legend = c("Original AEZs - FAO", "LDS AEZs - FAO"), col = c("blue3", "red3"))
	}
	par(new=FALSE)
	
	}	 # end if(FALSE)
	
	dev.off()	# close land rent histogram comparison file
	
}	# end for j loop over use sector

cat("finished plot_lds_landrent_aez.r at ",date(), "\n")
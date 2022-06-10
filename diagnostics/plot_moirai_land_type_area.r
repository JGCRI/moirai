###### plot_moirai_land_type_area.r
# diagnostitcs for the land type area output
#
# There are only five variables that the user should set (lines 96-109)
#   AEZ:		TRUE=18 aez moirai outputs; FALSE=235 GLU moirai outputs or custom GLU outputs
#   BOTH:   TRUE=override AEZ flag: olddir is aez 18, newdir is 235 GLU (or custom); this enables comparison
#           between two different sets of bouandaries
#   olddir:		The directory containing the old moirai outputs to diagnose (can be either 235 basins or 18 aezs)
#   newdir:		The directory containing the new moirai outputs to diagnose (can be either 235 basins or 18 aezs)
#   outdir:		The directory to write diagnostic figures to
#   If you have a custom glu file then modify num_lds_glu, ctag, and ptag on lines 117-119 accordingly
#             and set AEZ=FALSE or BOTH=TRUE
#
# compare Moirai LDS outputs with the old GIS code
# for 18 original aezs make comparisons at the aez level
# for any geographic land units (glus) make comparisons at country level
# need to compare specific land types
#
# old gis file format (km^2)
#	one header line with column labels
#	4 columns: AEZ_ID, Category, Year, area(km2)
#		AEZ_ID = 0 means that no original countryXaez info exists for those cells
#
# Moirai v3.1 lds land type area file format (ha)
#	6 header lines, with the line 6 having the column labels
#	5 columns: iso, glu_code, land_type, year, value
#	no records exist for zero area or for cells that do not have a country and glu match
#
# Moirai v3.1 land types file format
#	5 header lines, line 5 has the column labels
#	4 columns: Category, LT_SAGE, LT_HYDE, Status
#
# Moirai v3 and previous lds land type area file format (ha)
#	6 header lines, with the line 6 having the column labels
#	5 columns: iso, glu_code, land_type, year, value
#	no records exist for zero area or for cells that do not have a country and glu match
#
# Moirai v3 previous land types file format
#	5 header lines, line 5 has the column labels
#	4 columns: Category, LT_SAGE, LT_HYDE, LT_WDPA
#
# gis country map file format
#	one header line with column labels
#	7 columns: fao_code, fao_name, gtap_code, gtap_name, gis_code, gis_name
#	only columns 1 and 6 are used: fao_code and gis_code
#
# fao iso map file format
#	one header line, with column labels
#	5 columns: fao_code, iso3_abbr, fao_name, vmap0_code, vmap0_name
#	only columns 1 and 2 are used: fao_code and iso3_abbr
#
# the old gis data uses hyde 3.1 (with 2010 added from the new hyde data)
# previous version of moirai lds has output using the hyde 3.1 data (with 2010)
# moirai v3 and v3.1 lds have output using the new hyde 3.2 data for all years
#
# aggregate to only these four hyde land types:
#	hyde_type = c("Cropland", "Pasture", "Urbanland", "Unmanaged")
#	unmanaged includes the "unknown" pot veg type
#
# the gis area data for no country or glu (aez) are excluded from these diagnostics (about 2.1 million sq km)
# the Moirai lds area data for no country or glu (wb235) are not in the output file, but are < ~80,000 sq km
#
# output in km^2
#

# less than 10 minutes
# about 25 min when using "print()" with the glu plots to put them on the screen as they are created

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

library(ggplot2)

cat("started plot_moirai_land_type_area.r at ",date(), "\n")

# make sure working directory is .../moirai/diagnostics/
setwd("./")

# flag to denote whether this is 18 aez or 235 water basin lds output
AEZ = FALSE
BOTH = TRUE

# location of the old moirai lds output files (include final "/")
#olddir = "../example_outputs/basins235/"
olddir = "../example_outputs/aez_orig/"

# location of the new moirai lds output files (include final "/")
newdir = "../example_outputs/basins235/"
#newdir = "../example_outputs/aez_orig/"

# recommended outdir is in diagnostics because these are comparisons between cases
outdir = paste("./basins235_example_outputs_stats_area/", sep="")
#outdir = paste("./aez_orig_example_outputs_stats_area/", sep="")

num_gis_glu = 18

if(AEZ) {
	num_lds_glu = 18
	coldtag = "_aez.csv"
	poldtag = "_aez.pdf"
	cnewtag = "_aez.csv"
	pnewtag = "_aez.pdf"
	cctrytag = "_aez.csv"
	pctrytag = "_aez.pdf"
} else {
	num_lds_glu = 235
	coldtag = "_wb235.csv"
	poldtag = "_wb235.pdf"
	cnewtag = "_wb235.csv"
	pnewtag = "_wb235.pdf"
	cctrytag = "_wb235.csv"
	pctrytag = "_wb235.pdf"
}

if(BOTH) {
	num_lds_glu = 235
	coldtag = "_aez.csv"
	poldtag = "_aez.pdf"
	cnewtag = "_wb235.csv"
	pnewtag = "_wb235.pdf"
	cctrytag = "_aez_wb235.csv"
	pctrytag = "_aez_wb235.pdf"
}

dir.create(outdir, recursive = TRUE)

# input files
# some of these are lds diagnostic outputs
gis_fname = paste("./Sage_Hyde15_Area.csv", sep="")
land_types_orig_fname = "./orig_land_types.csv"
lds_fname = paste(olddir, "Land_type_area_ha.csv", sep="")
land_types_old_fname = paste(olddir, "Moirai_land_types.csv", sep="")
lds_new_fname = paste(newdir, "Land_type_area_ha.csv", sep="")
land_types_new_fname = paste(newdir, "Moirai_land_types.csv", sep="")
gis_ctry_map_fname = paste("./FAO_gtap_gcam_ctry.csv", sep="")
fao_iso_map_fname = paste("../indata/FAO_iso_VMAP0_ctry.csv", sep="")

HA2KMSQ = 1 / 100

num_gis_ctry = 160
num_gis_ctryglu = num_gis_ctry * num_gis_glu

# read the input files
gis = read.csv(gis_fname)
lt_list_orig = read.csv(land_types_orig_fname, skip=4)
lds = read.csv(lds_fname, skip=5)
lds_new = read.csv(lds_new_fname, skip=5)
lt_list = read.csv(land_types_old_fname, skip=4)
lt_list_new = read.csv(land_types_new_fname, skip=4)
gis_ctry = read.csv(gis_ctry_map_fname)
fao_iso = read.csv(fao_iso_map_fname)

# subset the gis data to remove the no-country data
gis = subset(gis, AEZ_ID != 0)

# get some counts
num_gis_map = length(gis_ctry$gis_code)
num_iso_map = length(fao_iso$fao_code)
num_lds_ctry = length(fao_iso$fao_code)
num_lt = length(lt_list$Category)
num_lt_new = length(lt_list_new$Category)
num_lt_orig = length(lt_list_orig$Category)

gis_fao_code = array(dim = c(num_gis_ctryglu))
gis_fao_code[] = -1
gis_iso = array(dim = c(num_gis_ctryglu))
gis_iso[] = NA
gis_ctryglu_code = NULL
gis_glu_code = NULL

# map gis countrieXaez to iso and aezs
for(i in 1:num_gis_ctryglu) {
	ctry = (((i-1) %/% num_gis_glu) + 1)
	if(i %% num_gis_glu == 0) {
		glu = num_gis_glu
	} else {
		glu = i %% num_gis_glu
	}
	ctryglu = 100 * ctry + glu
	gis_ctryglu_code = c(gis_ctryglu_code, ctryglu)
	gis_glu_code = c(gis_glu_code, glu)
	
	for(j in 1:num_gis_map) {
		if(gis_ctry$gis_code[j] == ctry) {
			gis_fao_code[i] = gis_ctry$fao_code[j]
			break;
		}
	}
	if(gis_fao_code[i] == -1) {
		cat("gis_fao_code", i, "not found for gis_ctry_code", gis_ctry_code[i], "\n")
	}
	for(j in 1:num_iso_map) {
		if(fao_iso$fao_code[j] == gis_fao_code[i]) {
			gis_iso[i] = as.character(fao_iso$iso3_abbr[j])
			break;
		}
	}
	if(is.na(gis_iso[i])) {
		cat("gis_iso", i, "not found for gis_fao_code", gis_fao_code[i], "\n")
	}
}

# put the mapping in a data frame and merge it with the gis data
gis_map_codes = data.frame(AEZ_ID = gis_ctryglu_code, iso = gis_iso, glu_code = gis_glu_code)
gis = merge(gis, gis_map_codes)

# rename the Category column to land_type to match the moirai lds output
gis$land_type = 0
gis$land_type = gis$Category
gis$Category = NULL
# rename the area column to value to match the moirai lds output
gis$value = 0
gis$value = gis$Area.km2.
gis$Area.km2. = NULL
# rename the year column to value to match the moirai lds output
gis$year = 0
gis$year = gis$Year
gis$Year = NULL

# convert the moirai lds data to km^2
lds$value = lds$value * HA2KMSQ
lds_new$value = lds_new$value * HA2KMSQ

# set the hyde land type for all data frames
gis = merge(gis, lt_list_orig, by.x = "land_type", by.y = "Category")
lds = merge(lds, lt_list, by.x = "land_type", by.y = "Category")
lds_new = merge(lds_new, lt_list_new, by.x = "land_type", by.y = "Category")

# now aggregate the data to the hyde land types within glu
gis_glu = aggregate(value ~ year + LT_HYDE + glu_code + iso, gis, FUN = "sum", na.rm = TRUE)
lds_glu = aggregate(value ~ year + LT_HYDE + glu_code + iso, lds, FUN = "sum", na.rm = TRUE)
lds_new_glu = aggregate(value ~ year + LT_HYDE + glu_code + iso, lds_new, FUN = "sum", na.rm = TRUE)

# now aggregate the data to the hyde land types within country
gis_ctry = aggregate(value ~ year + LT_HYDE + iso, gis_glu, FUN = "sum", na.rm = TRUE)
lds_ctry = aggregate(value ~ year + LT_HYDE + iso, lds_glu, FUN = "sum", na.rm = TRUE)
lds_new_ctry = aggregate(value ~ year + LT_HYDE + iso, lds_new_glu, FUN = "sum", na.rm = TRUE)

# label each data frame in a column
gis_ctry$source = NA
gis_ctry$source = "GIS"
lds_ctry$source = NA
lds_ctry$source = "Previous"
lds_new_ctry$source = NA
lds_new_ctry$source = "Moirai new"

# now aggregate the data to the hyde land types globally
gis_globe = aggregate(value ~ year + LT_HYDE, gis_ctry, FUN = "sum", na.rm = TRUE)
lds_globe = aggregate(value ~ year + LT_HYDE, lds_ctry, FUN = "sum", na.rm = TRUE)
lds_new_globe = aggregate(value ~ year + LT_HYDE, lds_new_ctry, FUN = "sum", na.rm = TRUE)

# label each data frame in a column
gis_globe$source = NA
gis_globe$source = "GIS"
lds_globe$source = NA
lds_globe$source = "Previous"
lds_new_globe$source = NA
lds_new_globe$source = "Moirai new"

# now bind the global data frames so that they can be plotted together
plot_df = rbind(gis_globe, lds_globe, lds_new_globe)
	
# only plot and write outputs if there are data
if(length(plot_df[,1] > 0)) {
	
	# plot the global level data
	p1 <- ( ggplot( plot_df, aes( year, value, color = source ) ) 
		+ geom_point(aes(shape=source)) + geom_line() 
		+ facet_grid( LT_HYDE~., scales="free" ) 
		+ ggtitle("Global Land Type Area")
		+ ylab( "Area (km^2)" )
		)
	#print(p1)
	ggsave( paste( outdir, "lt_area_globe.pdf", sep="" ), width=7, height=7)
	# write the global level data
	write.csv(plot_df, paste(outdir, "lt_area_globe.csv", sep=""), row.names = FALSE)
} # end plot global area hyde land types

# now aggregate the unmanaged data to the aggregated sage land types globally
gis_sage = aggregate(value ~ year + LT_SAGE, gis[gis$LT_HYDE=="Unmanaged",], FUN = "sum", na.rm = TRUE)
lds_sage = aggregate(value ~ year + LT_SAGE, lds[lds$LT_HYDE=="Unmanaged",], FUN = "sum", na.rm = TRUE)
lds_new_sage = aggregate(value ~ year + LT_SAGE, lds_new[lds_new$LT_HYDE=="Unmanaged",], FUN = "sum", na.rm = TRUE)

# now aggregate unmanaged land types to forest, savanna, grassland, shrubland, and other (tundra, desert, polar desert/rock/ice, unknown)
# unknown is 1-2 orders of magnitude less than the rest, and is relatively constant, so group it with other
forest_names = levels(lds_new$LT_SAGE)[grep("Forest", levels(lds_new$LT_SAGE), fixed = TRUE)]
shrub_names = levels(lds_new$LT_SAGE)[grep("Shrubland", levels(lds_new$LT_SAGE), fixed = TRUE)]
other_names = c("Tundra", "Desert", "PolarDesert/Rock/Ice", "Unknown")

# old gis data
gis_forest = aggregate(value ~ year, gis_sage[gis_sage$LT_SAGE %in% forest_names,], FUN = "sum", na.rm = TRUE)
gis_shrub = aggregate(value ~ year, gis_sage[gis_sage$LT_SAGE %in% shrub_names,], FUN = "sum", na.rm = TRUE)
gis_other = aggregate(value ~ year, gis_sage[gis_sage$LT_SAGE %in% other_names,], FUN = "sum", na.rm = TRUE)
gis_forest$LT_SAGE = NA
gis_shrub$LT_SAGE = NA
gis_other$LT_SAGE = NA
gis_forest$LT_SAGE = "Forest"
gis_shrub$LT_SAGE = "Shurbland"
gis_other$LT_SAGE = "Other"
gis_forest = gis_forest[,order(c("year", "LT_SAGE", "values"))]
gis_shrub = gis_shrub[,order(c("year", "LT_SAGE", "values"))]
gis_other = gis_other[,order(c("year", "LT_SAGE", "values"))]

gis_lts = rbind(gis_sage[gis_sage$LT_SAGE=="Savanna" | gis_sage$LT_SAGE=="Grassland/Steppe",], gis_forest, gis_shrub, gis_other)

# previous data
lds_forest = aggregate(value ~ year, lds_sage[lds_sage$LT_SAGE %in% forest_names,], FUN = "sum", na.rm = TRUE)
lds_shrub = aggregate(value ~ year, lds_sage[lds_sage$LT_SAGE %in% shrub_names,], FUN = "sum", na.rm = TRUE)
lds_other = aggregate(value ~ year, lds_sage[lds_sage$LT_SAGE %in% other_names,], FUN = "sum", na.rm = TRUE)
lds_forest$LT_SAGE = NA
lds_shrub$LT_SAGE = NA
lds_other$LT_SAGE = NA
lds_forest$LT_SAGE = "Forest"
lds_shrub$LT_SAGE = "Shurbland"
lds_other$LT_SAGE = "Other"
lds_forest = lds_forest[,order(c("year", "LT_SAGE", "values"))]
lds_shrub = lds_shrub[,order(c("year", "LT_SAGE", "values"))]
lds_other = lds_other[,order(c("year", "LT_SAGE", "values"))]

lds_lts = rbind(lds_sage[lds_sage$LT_SAGE=="Savanna" | lds_sage$LT_SAGE=="Grassland/Steppe",], lds_forest, lds_shrub, lds_other)

# current data
lds_new_forest = aggregate(value ~ year, lds_new_sage[lds_new_sage$LT_SAGE %in% forest_names,], FUN = "sum", na.rm = TRUE)
lds_new_shrub = aggregate(value ~ year, lds_new_sage[lds_new_sage$LT_SAGE %in% shrub_names,], FUN = "sum", na.rm = TRUE)
lds_new_other = aggregate(value ~ year, lds_new_sage[lds_new_sage$LT_SAGE %in% other_names,], FUN = "sum", na.rm = TRUE)
lds_new_forest$LT_SAGE = NA
lds_new_shrub$LT_SAGE = NA
lds_new_other$LT_SAGE = NA
lds_new_forest$LT_SAGE = "Forest"
lds_new_shrub$LT_SAGE = "Shurbland"
lds_new_other$LT_SAGE = "Other"
lds_new_forest = lds_new_forest[,order(c("year", "LT_SAGE", "values"))]
lds_new_shrub = lds_new_shrub[,order(c("year", "LT_SAGE", "values"))]
lds_new_other = lds_new_other[,order(c("year", "LT_SAGE", "values"))]

lds_new_lts = rbind(lds_new_sage[lds_new_sage$LT_SAGE=="Savanna" | lds_new_sage$LT_SAGE=="Grassland/Steppe",], lds_new_forest, lds_new_shrub, lds_new_other)

gis_lts$source = NA
gis_lts$source = "GIS"
lds_lts$source = NA
lds_lts$source = "Previous"
lds_new_lts$source = NA
lds_new_lts$source = "Moirai new"

# now bind the global data frames so that they can be plotted together
plot_df = rbind(gis_lts, lds_lts, lds_new_lts)

# only plot and write outputs if there are data
if(length(plot_df[,1] > 0)) {
	
	# plot the global land type data
	p1 <- ( ggplot( plot_df, aes( year, value, color = source ) ) 
		+ geom_point(aes(shape=source)) + geom_line() 
		+ facet_grid( LT_SAGE~., scales="free" ) 
		+ ggtitle("Global Unmanaged Land Type Area")
		+ ylab( "Area (km^2)" )
		)
	#print(p1)
	ggsave( paste( outdir, "unmanaged_lt_area_globe.pdf", sep="" ), width=7, height=7)
	# write the global land type level data
	write.csv(plot_df, paste(outdir, "unmanaged_lt_area_globe.csv", sep=""), row.names = FALSE)
} # end plot global area hyde land types

# loop over the countries to make plots and csv files
for(lds_ctry_ind in 1:num_lds_ctry) {
	gis_pdata = subset(gis_ctry, iso == as.character(fao_iso$iso3_abbr[lds_ctry_ind]))
	gis_tdata = subset(gis_glu, iso == as.character(fao_iso$iso3_abbr[lds_ctry_ind]))
	lds_pdata = subset(lds_ctry, iso == as.character(fao_iso$iso3_abbr[lds_ctry_ind]))
	lds_tdata = subset(lds_glu, iso == as.character(fao_iso$iso3_abbr[lds_ctry_ind]))
	lds_new_pdata = subset(lds_new_ctry, iso == as.character(fao_iso$iso3_abbr[lds_ctry_ind]))
	lds_new_tdata = subset(lds_new_glu, iso == as.character(fao_iso$iso3_abbr[lds_ctry_ind]))
	
	# now bind the country data frames so that they can be plotted together
	plot_df = rbind(gis_pdata, lds_pdata, lds_new_pdata)
	
	# only plot and write outputs if there are data
	if(length(plot_df[,1] > 0)) {
	
		# plot the country level data
		p1 <- ( ggplot( plot_df, aes( year, value, color = source ) ) 
			+ geom_point(aes(shape=source)) + geom_line() 
			+ facet_grid( LT_HYDE~., scales="free" ) 
			+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "Land Type Area") )
			+ ylab( "Area (km^2)" )
			)
		#print(p1)
		ggsave( paste( outdir, "lt_area_ctry_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), pctrytag, sep="" ), width=7, height=7)
		# write the ctry level data
		write.csv(plot_df, paste(outdir, "lt_area_ctry_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), cctrytag, sep=""), row.names = FALSE)
	
		if(length(gis_tdata[,1] > 0)) {
			# plot the glu level data for gis
			p1 <- ( ggplot( gis_tdata, aes( year, value, color = as.factor(glu_code) ) ) 
				+ geom_line() 
				+ facet_grid( LT_HYDE~., scales="free" ) 
				+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "GLU Land Type Area") )
				+ ylab( "Area (km^2)" )
				)
			#print(p1)
			ggsave( paste( outdir, "gis_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "_aez.pdf", sep="" ), width=7, height=7 )
			
			# write the glu level data
			write.csv(gis_tdata, paste(outdir, "gis_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "_aez.csv", sep=""), row.names = FALSE)
		}
		
		if(length(lds_tdata[,1] > 0)) {
			# plot the glu level data for moirai lds
			p1 <- ( ggplot( lds_tdata, aes( year, value, color = as.factor(glu_code) ) ) 
				+ geom_line() 
				+ facet_grid( LT_HYDE~., scales="free" ) 
				+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "GLU Land Type Area") )
				+ ylab( "Area (km^2)" )
				)
			#print(p1)
			ggsave( paste( outdir, "previous_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), poldtag, sep="" ), width=7, height=7 )
			
			# write the glu level data
			write.csv(lds_tdata, paste(outdir, "previous_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), coldtag, sep=""), row.names = FALSE)
		}
		
		if(length(lds_new_tdata[,1] > 0)) {
			# plot the glu level data for moirai lds new
			p1 <- ( ggplot( lds_new_tdata, aes( year, value, color = as.factor(glu_code) ) ) 
				+ geom_line() 
				+ facet_grid( LT_HYDE~., scales="free" ) 
				+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "GLU Land Type Area") )
				+ ylab( "Area (km^2)" )
				)
			#print(p1)
			ggsave( paste( outdir, "moirai_new_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), pnewtag, sep="" ), width=7, height=7 )
			
			# write the glu level data
			write.csv(lds_new_tdata, paste(outdir, "moirai_new_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), cnewtag, sep=""), row.names = FALSE)
		}
	
	} # end if there are data at the country level
} # end for loop over moirai lds countries


cat("finished plot_moirai_land_type_area.r at ",date(), "\n")

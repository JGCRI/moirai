###### plot_lds_land_type_area.r
# diagnostitcs for the land type area output
#
# compare LDS outputs with the old GIS code
# for 18 original aezs make comparisons at the aez level
# for any geographic land units (glus) make comparisons at country level
# need to compare specific land types
#
# the old gis file has been copied to the lds dir
#
# old gis file format (km^2)
#	one header line with column labels
#	4 columns: AEZ_ID, Category, Year, area(km2)
#		AEZ_ID = 0 means that no original countryXaez info exists for those cells
#
# lds land type area file format (ha)
#	6 header lines, with the line 6 having the column labels
#	5 columns: iso, glu_code, land_type, year, value
#	no records exist for zero area or for cells that do not have a country and glu match
#
# land types file format
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
# the lds has output using both the hyde 3.1 data (with 2010) and the new hyde data for all years
#
# aggregate to only these four hyde land types:
#	hyde_type = c("Cropland", "Pasture", "Urbanland", "Unmanaged")
#	unmanaged includes the "unknown" pot veg type
#
# the gis area data for no country or glu (aez) are excluded from these diagnostics (about 2.1 million sq km)
# the lds area data for no country or glu (wb235) are not in the output file, but are only ~80,000 sq km
#
# output in km^2
#

# less than 10 minutes on my workstation, without the glu plots
# about 30 min with the glu plots

library(ggplot2)

cat("started plot_lds_land_type_area.r at ",date(), "\n")

# make sure working directory is .../lds/diagnostics/
setwd("./")

# flag to denote whether this is 18 aez or 235 water basin lds output
AEZ = FALSE

# recommended outdir is in diagnostics because these are comparisons between cases
outdir = paste("./basins235_test_stats_area/", sep="")
dir.create(outdir, recursive = TRUE)

# location of the lds output files
ldsdir = "../outputs/basins235_test/"

# input files
# some of these are lds diagnostic outputs
gis_fname = paste("./Sage_Hyde15_Area.csv", sep="")
lds_fname = paste("./Land_type_area_ha_h31new.csv", sep="")
lds_new_fname = paste(ldsdir, "Land_type_area_ha.csv", sep="")
land_types_fname = paste(ldsdir, "LDS_land_types.csv", sep="")
gis_ctry_map_fname = paste("./FAO_gtap_gcam_ctry.csv", sep="")
fao_iso_map_fname = paste("../indata/FAO_iso_VMAP0_ctry.csv", sep="")

num_gis_glu = 18
if(AEZ) {
	num_lds_glu = 18
	ctag = "_aez.csv"
	ptag = "_aez.pdf"
} else {
	num_lds_glu = 235
	ctag = "_wb235.csv"
	ptag = "_wb235.pdf"
}

HA2KMSQ = 1 / 100

num_gis_ctry = 160
num_gis_ctryglu = num_gis_ctry * num_gis_glu

# read the input files
gis = read.csv(gis_fname)
lds = read.csv(lds_fname, skip=5)
lds_new = read.csv(lds_new_fname, skip=5)
lt_list = read.csv(land_types_fname, skip=4)
gis_ctry = read.csv(gis_ctry_map_fname)
fao_iso = read.csv(fao_iso_map_fname)

# subset the gis data to remove the no-country data
gis = subset(gis, AEZ_ID != 0)

# get some counts
num_gis_map = length(gis_ctry$gis_code)
num_iso_map = length(fao_iso$fao_code)
num_lds_ctry = length(fao_iso$fao_code)
num_lt = length(lt_list$Category)

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

# rename the Category column to land_type to match the lds output
gis$land_type = 0
gis$land_type = gis$Category
gis$Category = NULL
# rename the area column to value to match the lds output
gis$value = 0
gis$value = gis$Area.km2.
gis$Area.km2. = NULL
# rename the year column to value to match the lds output
gis$year = 0
gis$year = gis$Year
gis$Year = NULL

# convert the lds data to km^2
lds$value = lds$value * HA2KMSQ
lds_new$value = lds_new$value * HA2KMSQ

# set the hyde land type for all data frames
gis = merge(gis, lt_list, by.x = "land_type", by.y = "Category")
lds = merge(lds, lt_list, by.x = "land_type", by.y = "Category")
lds_new = merge(lds_new, lt_list, by.x = "land_type", by.y = "Category")

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
lds_ctry$source = "LDS"
lds_new_ctry$source = NA
lds_new_ctry$source = "LDS new"

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
		print(p1)
		ggsave( paste( outdir, "lt_area_ctry_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), ptag, sep="" ) )
	
		if(length(gis_tdata[,1] > 0)) {
			# plot the glu level data for gis
			p1 <- ( ggplot( gis_tdata, aes( year, value, color = as.factor(glu_code) ) ) 
				+ geom_line() 
				+ facet_grid( LT_HYDE~., scales="free" ) 
				+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "GLU Land Type Area") )
				+ ylab( "Area (km^2)" )
				)
			print(p1)
			ggsave( paste( outdir, "gis_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "_aez.pdf", sep="" ) )
			
			# write the glu level data
			write.csv(gis_tdata, paste(outdir, "gis_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "_aez.csv", sep=""), row.names = FALSE)
		}
		
		if(length(lds_tdata[,1] > 0)) {
			# plot the glu level data for lds
			p1 <- ( ggplot( lds_tdata, aes( year, value, color = as.factor(glu_code) ) ) 
				+ geom_line() 
				+ facet_grid( LT_HYDE~., scales="free" ) 
				+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "GLU Land Type Area") )
				+ ylab( "Area (km^2)" )
				)
			print(p1)
			ggsave( paste( outdir, "lds_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), ptag, sep="" ) )
			
			# write the glu level data
			write.csv(lds_tdata, paste(outdir, "lds_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), ctag, sep=""), row.names = FALSE)
		}
		
		if(length(lds_new_tdata[,1] > 0)) {
			# plot the glu level data for lds new
			p1 <- ( ggplot( lds_new_tdata, aes( year, value, color = as.factor(glu_code) ) ) 
				+ geom_line() 
				+ facet_grid( LT_HYDE~., scales="free" ) 
				+ ggtitle( paste(as.character(fao_iso$iso3_abbr[lds_ctry_ind]), "GLU Land Type Area") )
				+ ylab( "Area (km^2)" )
				)
			print(p1)
			ggsave( paste( outdir, "lds_new_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), ptag, sep="" ) )
			
			# write the glu level data
			write.csv(lds_new_tdata, paste(outdir, "lds_new_lt_area_km2_glu_", as.character(fao_iso$iso3_abbr[lds_ctry_ind]), ctag, sep=""), row.names = FALSE)
		}
	
	} # end if there are data at the country level
} # end for loop over lds countries


cat("finished plot_lds_land_type_area.r at ",date(), "\n")
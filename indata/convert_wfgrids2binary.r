############
# convert_wfgrids2binary.r
#
# the water footprint data are in arc binary grid format
# using the gdal library in my LDS c code does not recognize the file type for some reason
#
# so this script converts the arc grids to simple, single band binary files, with an easy to read header file
# then the LDS code reads in the simple binary files
#
# the input water footprint data are 5 arcmin resolution, 4320 columns X 1668 rows
# WGS84 lat-lon projection
# with nodata values of -3.40282e+38
# there are 4 variables: blue, green, gray, and total water footprint
#	in average annual mm across each entire grid cell area (1996-2005 average)
# BUT:
# the water footprint data are also off in their defined extent:
#	0.5 arcmin in east-west
#	0.3 armin in north-south
#
# the output rasters are 5 arcmin resolution, extended to the full globe (4320 columns X 2160 rows)
#	with the data shifted using nearest neighbor resampling to align with 5 arcmin boundaries
#	same WGS84 projection
#	the nodata values have been set to -9999
#
# outputs are r raster format:
#	###.gri is the binary file
#	###.grd is the text header file
#
# this script takes ~43 minutes on my workstation

cat("\nStarted convert_wfgrids2binary.r",date(), "\n")

library(raster)

PROJ4_STRING = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"
WF_EXTENT = extent(-180.0, 180.0, -55.916666666667, 83.083333333333)
FULLGLOBE = extent(-180.0,180.0,-90.0,90.0)
NODATA = -9999

# change basedir to point to your instance of Report47-App-IV-RasterMaps/
basedir = "/Users/ladmin/repos/stash/lds/indata/WaterFootprint/Report47-App-IV-RasterMaps/"
hdrtag = "/hdr.adf"
outtag = ".grd"

NUM_VARS = 4
var_names = c("wfbl_mmyr", "wfgn_mmyr", "wfgy_mmyr", "wftot_mmyr")

NUM_CROPS = 18
crop_names = c("Barley", "Cassava", "Coconuts", "Coffee", "Cotton", "Groundnut", "Maize", "Millet", "Oilpalm", "Olives", "Potatoes", "Rapeseed", "Rice", "Sorghum", "Soybean", "Sugarcane", "Sunflower", "Wheat")

for(crop_index in 1:NUM_CROPS) {
	cropdir = paste(basedir, crop_names[crop_index], "/", sep="")

	for(var_index in 1:NUM_VARS) {
		fname = paste(cropdir, var_names[var_index], hdrtag, sep="")
		ofn = paste(cropdir, var_names[var_index], outtag, sep="")

		# create the template for resampling in order to shift the input boundaries
		template = raster(WF_EXTENT, nrows = 1668, ncols = 4320)
		projection(template) <- PROJ4_STRING

		# read in the data, resample it, extend it to the globe, reset the nodata value in the output data
		wf_in = raster(fname)
		wf_rs = resample(wf_in, template, method = "ngb")
		wf_full = extend(wf_rs, FULLGLOBE)
		nainds = Which(is.na(wf_full), cells=TRUE)
		wf_out = wf_full
		wf_out[nainds] = NODATA

		# write the binary raster file
		writeRaster(wf_out, filename=ofn, datatype = "FLT4S", overwrite=TRUE, NAflag = NODATA)
	} # end loop over the variables
} # end loop over the crops

cat("\nFinished convert_wfgrids2binary.r",date(), "\n")

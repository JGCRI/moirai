# Ancillary data for Moirai

Updated for Moirai version 3.1

## Valid Moirai Boundaries (`moirai_valid_boundaries_v31.zip`)

These files delineate the valid area represented by the Moirai outputs. Valid output area is determined during processing based on the intersection of multiple data sets. Diagnostic outputs store this valid area, and then the spatial diagnostics package (`…/moirai/diagnostics/generate_moirai_spatial.R`) is used to generate the raster and vector files stored here for version 3.1 with 235 water basin GLUs. Moirai determines valid data cells by intersecting the following input data:
* HYDE land area
* Country boundaries
* Geographic land unit (GLU) boundaries
* Land rent data 87-country boundaries (determined from the country boundary data and the 87-country grouping as listed in `…/moirai/indata/FAO_ctry_GCAM_ctry87.csv`)

The original HYDE land area data exclude water and permanent ice (`…/moirai/indata/land_area_hyde.bil`), and the current default input data (`…/moirai/indata/hyde_land_plus.bil`) have been augmented by SAGE data to capture land ice.

## Ancillary input data

The country input data combine FAO VMAP0 vector data and East Timor vector data (`countries_land.zip`) with extended country data over inland water (`update_country_raster_water.zip`) to include inland water bodies within countries.

The 235 water basin input data have been determined from a 1/8 degree global watershed data set that has been aggregated to these basins and topologically cleaned (`basin235_input.zip`) before being converted to the current input raster file (`…/moirai/indata/Global235_CLM_5arcmin.bil`).

#### Note that the SAGE-based data sets (crop data, irrigation, water footprint) are aggregated based on the SAGE land mask to the output countries and GLUs. There are some SAGE land pixels that are not within the valid output boundaries, most of which are along the arctic coast of Eurasia. This means that there may or may not be some data included from pixels that are technically outside the valid Moirai output boundaries. This ensures that all these SAGE data are included, even if their base land area does not match exactly with the valid land area.

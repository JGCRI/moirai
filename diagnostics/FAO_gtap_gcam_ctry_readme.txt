Generating a raster file of fao country codes and mapping these to GTAPv6.0 countries (Land Use DB 2.0)
17 May 2013
Alan Di Vittorio

The raster FAO country file is (see the header for more info):
fao_ctry_rast.bil

The .csv country mapping file has seven columns (in order):
fao_code
fao_name
gtap_code
gtap_abr
gtap_name
gis_code
gis_name

There is one header line followed by 246 record lines. There are no duplicates of FAO countries.

Spatial Data:

Downloaded VMAP0 political boundaries shapefile from University of Washington Geomorphological Research Group
Downloaded Timor-Leste administrative boundaries from the Timor-Leste GIG

Added Timor-Leste country boundary to the vmap0 data set:

Dissolve the admin1 district boundaries to get the country boundary
Union the Timor-Leste data with the VMAP0 data (couldn't snap to the VMAP0 boundaries)
Set the extra polygons to be part of Timor-Leste and dissolve the boundaries
There is some coastline that still belongs to indonesia because the boundaries do not line up
Add an FAO_code column to the attribute table and assign the values based on the mapping described below
Create an initial raster file of the FAO codes (coastal boundaries are not included)
Grow the raster by one cell to include coastal boundaries and all associated land area
Write a raster file of the FAO country codes

FAO country data:

Get list of FAO countries with numeric codes. This list has 245 entries, some of which are regions.
Add missing country:  Svalbard and Jan Mayan Islands (260)
Replace special characters for Reunion and Cote d'Ivoire
Delete the regions. Delete anything else?
West Bank (245) is not listed in the FAO PRODSTAT files.
Data for Gaza Strip and West Bank are under Occupied Palestinian Territory
Sudan (former) (206) is where the merged Sudan and South Sudan data are
Serbia and Montenegro have merged data for 2001-2005, and separate data for 2006-2007.

Mapping VMAP0 countries to FAO country IDs:

Macao, Hong Kong, Taiwan, Antarctica, and Spratly Islands do not have FAO codes. Create new codes so that they do not get lumped with other countries.
Macao and Hong Kong are special administrative regions of China.
Taiwan is a separate state.
Spratly Islands are disputed among Brunei, China, Malaysia, Phillipines, Taiwan, and Vietnam, so they have their own code.
Antarctica=300, Macao=301, Hong Kong=302, Taiwan=303, Spratly Islands=304
Sudan is still merged in VMAP0 and in the FAO data.
Serbia and Montenegro will be mapped as a merged entity.
Several territories and islands in VMAP0 were assigned their respective country codes.
Gaza strip and West bank were assigned to Occupied Palestinian Territory.

Mapping GCAM GIS countries to FAO and GTAP countries:

For consistency, if an FAO country does not have a GCAM GIS equivalent the GCAM GIS code is set to -1.
The GCAM GIS country data set will be used in case a cropland pixel falls outside of the raster FAO country map.
If a cropland pixel falls outside of the FAO and GIS data sets, then it is discarded.
This is mainly an issue along coastlines and for islands.
Belgium and Luxembourg are merged in the GIS data. This shouldn't be an issue for the raster cropland data because it isn't on a coastline, but the data will be assigned to Belgium.


Mapping fao countries to GTAP countries:

If an FAO country has been assigned a GTAP code of -1 it is excluded from calculating the new GTAP outputs.
Hong Kong, Macao, Taiwan, and Timor-Leste have been included.
Hong Kong has area and production data in the GTAP database, but no data from FAO (need to think about how to calibrate Hong Kong).
Macao and Taiwan do not have GTAP area and production data, nor FAO data.
Hong Kong and Taiwan do have land rent values in the GTAP database, based on GTAP country VFM and vietnam production and prices.
Timor-Leste has all zero values in GTAP, but there are FAO data.
Serbia and Montenegro are merged in the GTAP data, so they are mapped to the merged GTAP code. They are also merged in the FAO data through 2005.
FAO countries (plus Antarctica and Spratly Islands) that do not have GTAP codes, or do not have data because they represent entities that have been replaced prior to the years contained in these data, are assigned a GTAP code of -1.
Western Sahara is also assigned a GTAP code of -1 because it is not included in the GTAP country list and it might be inconsistent to wrap it into Morocco, and then the rest of the region. 
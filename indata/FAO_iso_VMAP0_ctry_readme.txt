Remapping the countries to iso and VMAP0 so that the outputs are not beholden to the GTAP list
13 Oct 2015
Alan Di Vittorio

The FAO_iso_VMAP0_ctry.csv file was updated June 2018

Started with the FAO_gtap_gcam_ctry.csv as described below

FAO data used here includes 1997-2016, so these mappings may not apply to other periods.

Raster FAO country file: fao_ctry_rast.bil
Based on VMAP0 data, with Timor-Leste added
The raster values are the corresponding FAO integer codes for harvested area and production
Several islands were assigned to their “owner” countries

Some integer codes and territories were added to the FAO list to maintain spatial consistency with VMAP0 and iso:
	antarctica (300, ata)
	hong kong (302, hkg)
	macao (301, mac)
	spratly islands (304, -), disputed archipelago
	taiwan (303, twn)
	these are not actual FAO codes, they are numbers i selected
	the iso3 codes are correct, however

Thus all of the spatial data are represented in the processing and/or outputs in some way.

Spratly islands and antarctica are not included in any outputs because one has no iso3 and the other has no corresponding economic region (in either GCAM or GTAP87)

FAO_iso_VMAP_ctry.csv

Deleted five countries that do not exist after 1994 (and are not in the VMAP0 data):
	Czechoslovakia (51) - split into czech republic and slovakia in 1993
	ethiopia pdr (62) - existed only from 1987-1991, and is now just ethiopia
	pacific islands trust territory (164) - split in 1986 and ended in 1994
	USSR (228) - ended in 1991
	Yugoslav SFR (248) - ended in 1991

Deleted South Sudan (277, ssd) and the current Sudan (276, sdn) because only Sudan (former) (206, sdn) has FAO data (through 2007 at least). South Sudan was founded in 2011. The VMAP0 data has only the merged, former Sudan.

Deleted Gaza Strip (76, -) and West Bank (245, -) because they are mapped to occupied palestinian territory (299, pse) in VMAP0 and in the FAO data.

Deleted the Belgium-Luxembourg entry because they are separate countries.

Deleted the Holy See (94, vat) because there are no spatial data.

Serbia (272, srb) and Montenegro (273, mne) have merged data from 1997 through 2005, then separate from 2006 forward. So the processing and output is merged in this code into Serbia and Montenegro (186, scg). They were the federal republic of yugoslavia from 1992 to 2003, then S&M through 2005, then split.

The VMAP0 regions that were assigned to “owner” countries are included in their respective vmap0_name cells.

Only one entry does not have VMAP0 mappings:
	Serbia and Montenegro (186, scg), because they are separate in the spatial data

Spratly islands and antarctica are both assigned -1 for GTAP87 region and GCAM region and not included in any outputs because spratly has no iso3 (“-“) and antarctica has no corresponding economic region (in either GCAM or GTAP87)

Generating a raster file of fao country codes and mapping these to GTAPv6.0 countries (Land Use DB 2.0)
17 May 2013
Alan Di Vittorio

The raster FAO country file is (see the header for more info):
fao_ctry_rast.bil

The FAO_gtap_gcam_ctry.csv country mapping file has seven columns (in order):
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
*********************************************************************************************************
Global data set of Monthly Irrigated and Rainfed Crop Areas around the year 2000 (MIRCA2000), version 1.1
*********************************************************************************************************

------ CONTENTS

The files in this folder represent annual harvested areas grids of 26 irrigated and rainfed crops
in 5 arc-minute and 30 arc-minute grid cell resolution (data unit: hectare) (set A).

Additionally, the sum of annual harvested areas of all 26 irrigated crops (AHI), of all 26 rainfed crops (AHR),
and of all 26 irrigated and rainfed crops (AHT) are provided also 
in 5 arc-minute and 30 arc-minute grid cell resolution (data unit: hectare) (set B).

The MIRCA2000-dataset is documented at: 
http://www2.uni-frankfurt.de/45218023/MIRCA.

This is version 1.1 of the data set. The difference to version 1.0 is the implementation of a bugfix to
ensure consistency of two major input data sets indicating cropland extent (Ramankutty et al., 2008) 
and harvested crop areas (Monfreda et al., 2008).


------ DATA FORMAT

The files are compressed (gnu-zip format) plain text files
in ESRI-ascii grid format that can be imported by most of the GIS-software
and other software that can handle gridded data, e.g. 
ESRI ArcViewGIS 3.x + Spatial Analyst extension, ESRI ArcGIS, ERDAS Imagine.
Each file contains a header of 6 rows. 
The following 4320 x 2160 grid cells contain the annual harvested area in hectare.
The cell order is from North to South and West to East.

--
Set A includes one file per crop class, separately for irrigated (IRC) and for rainfed crops (RFC):

The 52 x 2 files have the generic file names:
(1) 5 arc-minute grid (x = crop class 1-2 digits):
	ANNUAL_AREA_HARVESTED_IRC_CROPx_HA.ASC(.gz)
	ANNUAL_AREA_HARVESTED_RFC_CROPx_HA.ASC(.gz)

(2) 30 arc-minute grid (XX = crop class 2 digits):
	annual_area_harvested_irc_cropXX_ha.asc(.gz)
	annual_area_harvested_rfc_cropXX_ha.asc(.gz).

For downloading the crop-specific data of the 26 crops in one file, separately for each resolution,
the 52 5 arc-minute and 52 30 arc-minute gnu-zip-files are joined to 2 files in ZIP-format:
harvested_area_grids_26crops_05mn.zip
harvested_area_grids_26crops_30mn.zip.

The harvested areas are given in hectare (ha).

The codes for the 26 crop classes are as follows:
1	Wheat
2	Maize
3	Rice
4	Barley
5	Rye
6	Millet
7	Sorghum
8	Soybeans
9	Sunflower
10	Potatoes
11	Cassava
12	Sugar cane
13	Sugar beet
14	Oil palm
15	Rape seed / Canola
16	Groundnuts / Peanuts
17	Pulses
18	Citrus
19	Date palm
20	Grapes / Vine
21	Cotton
22	Cocoa
23	Coffee
24	Others perennial
25	Fodder grasses
26	Others annual.

--
Set B includes one file per sum (AHI, AHR, AHT), separately for each resolution:

The 3 x 2 files have the file names:
(1) 5 arc-minute grid:
	annual_area_harvested_irrigated_allcrops_ha.asc(.gz)
	annual_area_harvested_rainfed_allcrops_ha.asc(.gz)
	annual_area_harvested_total_allcrops_ha.asc(.gz)

(2) 30 arc-minute grid (XX = crop class 2 digits):
	annual_area_harvested_irrigated_allcrops_ha_30mn.asc(.gz)
	annual_area_harvested_rainfed_allcrops_ha_30mn.asc(.gz)
	annual_area_harvested_total_allcrops_ha_30mn.asc(.gz).

The harvested areas are given in hectare (ha).


------ AUTHORS of the data compilation

Felix Portmann
Institute of Physical Geography (IPG), Goethe University Frankfurt am Main, Germany
portmann@em.uni-frankfurt.de

Stefan Siebert
Institute of Crop Science and Resource Conservation (INRES), University of Bonn, Germany
s.siebert@uni-bonn.de

September 2009
Update September 2013: new URL for MIRCA2000 documentation
README release HYDE version 3.2.000, d.d. October 4th 2016.

HYDE version 3.2.000 is the final release and update from the HYDE 3.2 2016_beta_release (from july 2016). 
Updates concern data for Mexico after 2012 (were missing), and some Grazing estimates for the Asia -stan countries for the BCE period were updated.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Disclaimer: The Creative Commons License (CC BY 3.0) applies to all of the HYDE data. (see https://creativecommons.org/licenses/by/3.0/)
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Reference: please note that this work has not been published yet, please refer to as: Klein Goldewijk et al., 20xx (in prep).
Or in full: Klein Goldewijk, K., A. Beusen, J.Doelman and E. Stehfest, New anthropogenic land use estimates for the Holocene; HYDE 3.2, in prep.

This database presents an update and expansion of the History Database of the Global Environment (HYDE, v 3.2.000). 
HYDE is and internally consistent combination of updated historical population estimates and land use. Categories include cropland, 
with a new distinction into irrigated and rain fed crops (other than rice) and irrigated and rain fed rice. 
Also grazing lands are provided, divided into more intensively used pasture and less intensively used rangeland. 
Population is represented by maps of total, urban, rural population and population density as well as built-up area. 
The period covered is 10 000 BCE to 2015 CE. Spatial resolution is 5 arc minutes (approx. 85 km2 at the equator), the files are in Arcmap asciigrid format.

- Each header of the gridded .asc files looks like:

ncols 4320
nrows 2160
xllcorner -180
yllcorner -90
cellsize 0.0833333
NODATA_value -9999

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) The HYDE 3.2 release contains 3 scenarios (Baseline estimate, Lower estimate, Upper estimate) and a General Files directory.

### Directory /General files ###

This directory contains general files:

garea_cr.asc	map with total gridcell area in km2, spherical Earth
landlake_cr.asc	map with (land-sea mask on 5'), including lakes (land = 1, lakes = 0, rest = -9999)
maxln_cr.asc	map with maximum landarea available per gridcell in km2

iso_cr.asc	map with national iso numerical country codes 
sub_iso_cr.asc	map with sub-national codes (provinces, states)

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
2) Each scenario contains 4 directories: png, zip, txt and anthromes12K

### Directory /png ### 

Contains all figures for each category for all years in graphical .png format.

### Directory /zip ###

Conatains zipped files with the following formats and content:
yearBC_pop.zip (population) and yearBC_lu.zip (landuse) for the BCE period ; yearAD_pop.zip (population) and yearAD_lu.zip (landuse) for the CE period.
Time intervals are 1000 yr for the BCE period, then 100 yr till 1700, 10 yr till 2000, and from 2000 - 2015 1 year timesteps.

- Each xxxx_pop.zip contains: 
popc_year.asc (population counts, in inhabitants/gridcell)
popd_year.asc (population density, in inhabitants/km2 per gridcell)
rurc_year.asc (rural population counts, in inh/gridcell)
urb_year.asc  (urban population counts, in inh/gridcell)
uopp_year.asc (total built-up area, such as towns, cities, etc, in km2 per grid cell)

- Each xxxx_lu.zip contains: 
croplandyear.asc     (total cropland area, in km2 per grid cell), after 1960 identical to FAO's category 'Arable land and permanent crops'.
grazingyear.asc      (total land used for grazing, in km2 per grid cell), after 1960 identical to FAO's category 'Permanent Pasture'.
pastureyear.asc      (total pasture area, in km2 per grid cell), defined as Grazing land with an aridity index > 0.5, assumed to be more intensively managed.
rangelandyear.asc    (total pasture area, in km2 per grid cell), defined as Grazing land with an aridity index > 0.5, assumed to be less or not managed.
ir_noriceyear.asc    (total irrigated rice area, in km2 per grid cell).
rf_noriceyear.asc    (total rainfed area, in km2 per grid cell).
ir_riceyear.asc      (total irrigated other crops are (no rice), in km2 per grid cell).
rf_riceyear.asc      (total rainfed other crops area (no rice), in km2 per grid cell).
tot_irriyear.asc     (total actual irrigated area, in km2 per grid cell).
tot_rainfedyear.asc  (total rainfed area, in km2 per grid cell).
tot_riceyear.asc     (total rice area, in km2 per grid cell).

### Directory /txt
- For each scenario summary files (plain ascii) are presented with the country or IMAGE 3.0 regional breakdown totals (in km2). See also www.pbl.nl/image for regional breakdown.
pop_c.txt        (total population per country over time, in 1000)
pop_r.txt        (total population per region over time, in 1000)
urb_c.txt	 (total urban population per country over time, in 1000)
urb_r.txt	 (total urban population per region over time, in 1000)
rur_c.txt	 (total rural population per country over time, in 1000)
rur_r.txt	 (total rural population per region over time, in 1000)
uopp_c.txt 	 (total built-up area per country over time, in km2 per grid cell)
uopp_r.txt	 (total built-up area per region over time, in km2 per grid cell)
croplandc.txt	 (total cropland area for each seperate country over time, in km2)
croplandr.txt	 (total cropland area for each region over time, in km2)
grazingc.txt	 (total grazingland area for each seperate country over time, in km2)
grazingr.txt 	 (total grazingland area for each region over time, in km2)
rangeland_c.txt  (total rangeland for each seperate country over time, in km2)
rangeland_r.txt  (total rangeland for each region over time, in km2)

rf_rice_r.txt	   rainfed rice for each region over time, in km2)
ir_rice_r.txt 	   irrigated ricefor each region over time, in km2)
rf_norice_r.txt	   rainfed other crops (no rice)for each region over time, in km2)
ir_norice_r.txt	   irrigated other crops (no rice)for each region over time, in km2)
tot_irri_r.txt	   total irrigated area for each region over time, in km2)
tot_rainfed_r.txt  total rainfed area for each region over time, in km2)
rf_rice_c.txt	   rainfed rice for each country over time, in km2)
ir_rice_c.txt 	   irrigated ricefor each country over time, in km2)
rf_norice_c.txt	   rainfed other crops (no rice)for each country over time, in km2)
ir_norice_c.txt	   irrigated other crops (no rice)for each country over time, in km2)
tot_irri_c.txt	   total irrigated area for each country over time, in km2)
tot_rainfed_c.txt  total rainfed area for each country over time, in km2)

### Directory /Anthromes12K ###

Contains a re-classification of HYDE 3.2.000 according to the Ellis and Ramankutty (2008) scheme of Anthropogenic Biomes or Anthromes,
also with /zip, /png and /txt subdirectories with the ascii gridded files, the graphic maps and the summaries txt files for each anthrome and a summary.

The Anthromes classes are:

"11 Urban"
"12 Dense settlements"

"21 Village, Rice"
"22 Village, Irrigated"
"23 Village, Rainfed"
"24 Village, Pastoral"

"31 Croplands, residential irrigated"
"32 Croplands, residential rainfed"
"33 Croplands, populated"
"34 Croplands, pastoral"

"41 Rangeland, residential"
"42 Rangeland, populated"
"43 Rangeland, remote"

"51 Semi-natural woodlands, residential"
"52 Semi-natural woodlands, populated"
"53 Semi-natural woodlands, remote"
"54 Semi-natural treeless and barren lands"

"61 Wild, remote - woodlands"
"62 Wild, remote - treeless & barren"
"63 Wild, remote - ice"

"70 No definition"

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Kees Klein Goldewijk (c.g.m.kleingoldewijk@uu.nl)
October 22, 2016

Utrecht University
Faculty of Geosciences
Copernicus Institute of Sustainable Development
Environmental Sciences Group
Heidelberglaan 2
Utrecht, The Netherlands

and

PBL Netherlands Environmental Assessment Agency
Bezuidenhoudseweg 30
The Hague, The Netherlands

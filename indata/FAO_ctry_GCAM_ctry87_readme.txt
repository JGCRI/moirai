The mapping of spatially distinguished countries to ctry87
21 oct 2015
Alan Di Vittorio

The file FAO_ctry_GCAM_ctry87.csv determines whether a spatially distinguished country is assigned to an economic region and output by LDS.
If a country is not to be mapped to an economic region (gcam or ctry87), then the gcam_ctry87_id is set to -1 and the gcam_ctry87 is set to the “-“ character.
Countries not assigned to a region here are not given a gcam region or output by LDS.

This is the complete list of spatial countries and codes, and corresponds with the FAO_iso_VMAP0_ctry.csv file in order and length, and with the fao raster country file with respect to fao_code.

Countries without an iso are not output (e.g., spratly islands, which are on the map, but disputed and not assigned to a single country).
Antarctica is not output because it is not assigned to an economic region (there actually are not any data for antarctica).
Serbia and Montenegro are not assigned because they are processed and output as a combined country (scg) due to merged fao data through 2005.

These 87 countries/regions are used because they are tied to the GTAP land rent data.
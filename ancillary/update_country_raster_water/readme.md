# Description
The `update_ctry_rast.R` script within this folder was used to update `fao_ctry_rast_nowater.bil` with country ISO values for non-land cells (inland water bodies). This was originally done for moirai 3.1, and in June 2025 `fao_ctry_rast_nowater.bil` was updated to set the remaining namibia_coast pixels to Namibia, instead of South Africa. This R script now writes the final`fao_ctry_rast.bil` file directly, rather than relying on gdal.

The `clean_gcam_water_basins_grass.bsh` script was used in GRASS GIS to update the Namibia pixels and update the water basin input raster for Moirai (`Global235_CLM_5arcmin.bil`). This GRASS script documents the steps taken to update these files, but the requisite GRASS database is not included here.

# Motivation and methodology
The country and water basin data are separate inputs to Moirai to facilitate user-defined geographic land units that provide sub-country resolution outputs. These are raster data that have been derived from different vector data sources, and as such the country and water basin boundary data do not always align with each other when they should. This creates many small spatial features that should not exist in the data. These erroneaous features represent areas within a country that are labelled with the incorrect water basin. In June 2025 we performed additional processing to the source data to reduce the number of these small, erroneous spatial features by adjusting the water basin raster data, and took the opportunity to update the Namibia pixels at the same time.

##Country data
The country raster file produced by GRASS GIS (`fao_ctry_rast_nowater.bil`) does not assign countries to inland water bodies because the data are associated with the land only. This did not matter in earlier versions of Moirai, but now Moirai generates data that identifies inland water versus land. To this end, these non-land cells require country IDs, since moirai is now used to generate shape files for land cells, no-land cells, and a combination of the two. (See the readme file in the diagnostics folder for a detailed description of the same.)

The script utilizes an additional country shapefile `GCAM_32_w_Taiwan.shp`. ISO numbers from this shapefile are mapped to the fao codes using mapping files from within the LDS. This shapefile is then rasterized and combined with the original raster to fill in holes for water bodies.

##Water basin data
The `clean_gcam_water_basins_grass.bsh` script describes the source data processing in GRASS GIS. Spatial features at the countryXbasin level with six or less pixels were adjusted so that the water basin matched the neighboring pixels withing the country. This dramatically reduces the number countryXbasin features from 2238 to 1389, and reduces the number of countryXbasin land units (after aggregating individual spatial features) from 702 to 679.

While many erroneous features have been corrected, some errors still exist. Spatial features larger than six pixels have not been adjusted, even if they are incorrect, to reduce the number of incorrect adjustments. There are many small features that are correct, including ones with six or less pixels. So some errors have been introduced by incorrectly adjusting some small features, but these additional errors are few compared to the valid corrections. Furthermore, a few features with six or less pixels that were known to be correct were excluded from adjustment. The aggregated countryXbasin land units can be manually checked against known boundaries by referencing the  `vmap0_basin_2025_rast_lu_p6_n7.csv` IDs (first column) to the intersection of the `fao_ctry_rast_nowater.bil` and `Global235_CLM_5arcmin.bil` data. These ID values are the GCAM country thematic values * 10000 + the water basin thematic values. The second column in this csv file is the pixel count.

Moiaria data outputs are at the countyXbasin land unit level, and GCAM further aggregates these data to regionXbasin for simulations.



# Validation
We used the land test check functions in `Compare_LDS_area_outputs.R` on Moirai outputs (See the readme file in the diagnostics folder for a detailed discussion of the same) to verify any change in land outputs on account of the updated input file.

For adding the water pixels to the original country data:
No differences were found at the highest level. Small differences were found at the SAGE/HYDE level. However, updating the raster only increased land coverage and no ISO's or GLU's were lost. This increase in land coverage is due to the addition of coastal land area within newly valid country cells that were previously omitted.

Comparing the June 2025 updated country and water basin data to the Moirai v3.2 data:
There are differences, which is expected, but plots show slight visual differences from the 1:1 line for two of the lowest data levels (`year_iso_glu_sage` and `year_iso_glu_hyde_sage`). Note that there are 652 countryXbasin land units in the new data after Moirai processing, which is 50 less than the previous data. This is interesting because no land units were lost during Moirai for the previous data, but 27 were lost during Moirai for the new data. 





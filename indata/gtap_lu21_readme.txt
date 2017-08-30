GTAP Land Use Database v2.1

The following .CSV files have been converted directly from the original .HAR files using the HAR2CSV tool distributed with the GEMPACK bundle13.exe (by Alan Di Vittorio, 28 May 2013). There is one value per line (only where data exists), and there is one header line at the beginning of each set of variable data. The number of fields per line in each csv file matches the number of columns for each variable. The data are stored in the following formats:

DGTM_DATA.CSV
	The variables are in order: BSDT, TMRN, TRNT, TRN6
	BSDT - timberland area (1000 ha) and CO2 stock (metric tonnes) for the 226 country list (not all countries are included)
		7 columns: CATG_DGTM, TREESPECIS, TVINTAGE, AEZ18, TREEMGMT, CTRY, Value
		The lists of items for each column are in the first sheet of DGTM_DATA.XLS
			!!!might need to create csv files of the column items!!!!!!
	TMRN - timberland marginal land rent (yr2000USD per ha per year) for the 226 country list (not all countries are included)
		3 columns: TREEMGMT, CTRY, Value
		The lists of items for each column are in the first sheet of DGTM_DATA.XLS
	TRNT - timberland rentals (million yr2000USD) for the 87 regions (not sure if they are all included), by AEZ18
		6 columns: TREESPECIS, TVINTAGE, AEZ18, TREEMGMT, REG, Value
		The lists of items for each column are in the first sheet of DGTM_DATA.XLS
	TRN6 - timberland rentals (million yr2000USD) for the 87 regions (not sure if they are all included), by AEZ6
		6 columns: TREESPECIS, TVINTAGE, AEZ6, TREEMGMT, REG, Value
		The lists of items for each column are in the first sheet of DGTM_DATA.XLS

SAGE_DATA.CSV
	The variables are in order: SGLH, SGHA, SGPD
	SGLH - land cover area (ha) for the 160 country list (not all countries are included)
		4 columns: LAND_SAGE, AEZ18, CTRY, Value
		The lists of items for each column are in the second sheet of SAGE_DATA.XLS
	SGHA - crop harvested area (ha) for the 160 country list (not all countries are included)
		4 columns: CTRY, AEZ18, CRP175, Value
		The lists of items for each column are in the second sheet of SAGE_DATA.XLS
	SGPD - crop production (metric tonne) for the 160 country list (not all countries are included)
		4 columns: CTRY, AEZ18, CRP175, Value
		The lists of items for each column are in the second sheet of SAGE_DATA.XLS

GTPLU18V60_AGMP_DEC06.CSV (million yr2001USD)
	The variables are in order: EVOA, EVFA, VFM, VFMA
	EVOA - primary factor sales (yr2001USD), at agent's prices, by 87 regions (not sure if they are all included)
		3 columns: AEZ18_ENDW, REG, Value
		The lists of items for each column are in the first sheet of GTPLU18V60_AGMP_DEC06.XLS
	EVFA - primary factor purchases (yr2001USD), at agent's prices, by 87 regions (not sure if they are all included)
		4 columns: AEZ18_ENDW, PROD_COMM, REG, Value
		The lists of items for each column are in the first sheet of GTPLU18V60_AGMP_DEC06.XLS
	VFM - primary factor purchases (yr2001USD), by household, at market prices, by 87 regions (not sure if they are all included)
		4 columns: AEZ18_ENDW, PROD_COMM, REG, Value
		The lists of items for each column are in the first sheet of GTPLU18V60_AGMP_DEC06.XLS
	VFMA - primary factor purchases (yr2001USD), by household, at market prices, by 87 regions (not sure if they are all included), ag and forest only
		This is the variable used as total land rent
		4 columns: AEZ18, AGFRS_GTAP, REG, Value
		The lists of items for each column are in the first sheet of GTPLU18V60_AGMP_DEC06.XLS
			!!! might need to create csv file of the column items!!!!
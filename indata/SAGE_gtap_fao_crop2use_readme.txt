Mapping SAGE crops, GTAP use, and FAO crops
1 Aug 2013

There are 8 columns (in order):
sage_crop_code
sage_file_name (just the base) - this is a short name
sage_crop_name - this is a long name or description
gtap_crop_name - this is the name currently expected by gcam data system, so use it here
gtap_crop_use_code
gtap_crop_use_name (this is a 3 character abbreviation)
fao_crop_code
fao_crop_name

There is one header line.
Fields containing commas are in quotes ("").
If there isn't a match the code is set to -1.

There are 175 SAGE crops. They are in the order listed in the GTAP database, and have been enumerated in that order.
These data are used mainly to map the SAGE crops to the GTAP crop sectors for disaggregating land rent values to AEZs.

The FAO mappings are used only when recalibrating the SAGE data to a different year. Any FAO categories in the FAOSTAT files that do not correspond to the SAGE/GTAP crop categories are discarded from the calibration (they are not even saved on read-in).

SAGE Jute-Like Fibres (24) has been mapped to FAO Other Bastfibres (782) based on the FAO description

SAGE Pimento (116) is mapped to FAO Chilies and peppers, dry (689) because there is a separate green chilies category in the SAGE data, and the FAO description distinguishes between dried and green pimento

SAGE Kapok Fibre (25) and SAGE Kapokseed in Shell (26) have both been mapped to FAO Kapok Fruit (310). The FAO Kapok Fruit includes both the fibre and seed. The SAGE data split the fibre and fruit yield, and the harvested area is duplicated for the two products (it is literally the same area).

SAGE Mixed Grasses&Legumes (40) does not have a corresponding FAO category. For calibration it might be able to be estimated by proxy using FAO categories 641, 640, 639, 642, 643, 636, 638, 637.

Note: the forage crops are output in the FAO prodstat crop files even though they are not listed with the crops, but rather are listed in the 'value of agricultural production' lists.

Note: no other FAO categories are needed because the livestock/animal products sectors' land rent disaggregations to AEZs are based on the 'gro' sector yields and pasture area.

Updated 26 Sep 2014:
Some FAO crops are in the FAO harvested area and production files, but do not obviously map onto any SAGE crops.
Sometimes these FAO crop categories might also be included in more common categories, or are secondary products of an included crop.
These FAO crops are skipped upon read in, to ensure that there is only one FAO crop per SAGE crop.

# makefile for the Land Data System (LDS)
# command line tool for generating land data for GCAM
#
# invoke by issuing command "make" from this directory
#
# manually create the EXEDIR and OBJDIR before running make
#   for some reason I couldn't get the mkdir command to work within this script
#
# NetCDF library and header locations:
#   the defaults are: /usr/local/lib and /usr/local/include
#   change LDFLAGS_GENERIC line below to point to your instance of the library

SRCDIR = ${PWD}
EXEDIR = ${SRCDIR}/bin
OBJDIR = ${SRCDIR}/obj

# link the math library and the netcdf support libraries
LDFLAGS_GENERIC = -lm -L/usr/local/lib -lnetcdf -L/usr/local/include

# For Linux
CFLAGS =  -O3 -std=c99 ${CFLAGS_GENERIC} # Almost fully optimized and using ISO C99 features
# CFLAGS = -fast -std=c99 ${CFLAGS_GENERIC} # Almost fully optimized and using ISO C99 features
# CFLAGS = -O3 -std=c99 -ffloat-store ${CFLAGS_GENERIC} # Use precise IEEE Floating Point
#CFLAGS = -g -Wall -pedantic -std=c99 ${CFLAGS_GENERIC} # debugging with line/file reporting and 'standards' testing flags
# CFLAGS = -fast -Wall -pedantic -std=c99 ${CFLAGS_GENERIC} # testing with line/file reporting and 'standards' testing flags
LDFLAGS = ${LDFLAGS_GENERIC}
CC = gcc

LDS_OBJS = lds_main.o init_lds.o get_in_args.o get_systime.o parse_utils.o read_country_info_all.o read_country87_info.o read_region_info_gcam.o read_aez_new_info.o read_use_info_gtap.o read_lt_info_sage.o read_crop_info.o get_cell_area.o read_land_area_sage.o read_land_area_hyde.o read_aez_new.o read_aez_orig.o read_potveg.o read_country_fao.o get_land_cells.o calc_potveg_area.o write_gcam_lut.o read_mirca.o proc_mirca.o read_protected.o read_lu_hyde.o proc_land_type_area.o read_soil_carbon.o read_veg_carbon.o proc_potveg_carbon.o read_water_footprint.o proc_water_footprint.o read_yield_fao.o read_harvestarea_fao.o read_production_fao.o read_sage_crop.o get_aez_val.o calc_harvarea_prod_out_crop_aez.o aggregate_crop2gcam.o write_harvestarea_crop_aez.o write_production_crop_aez.o read_rent_orig.o read_prodprice_fao.o calc_rent_ag_use_aez.o calc_rent_frs_use_aez.o write_rent_use_aez.o aggregate_use2gcam.o copy_to_destpath.o write_raster_short.o write_raster_int.o write_raster_float.o write_text_int.o write_text_char.o write_csv_float3d.o write_csv_float2d.o

LDS_INCLUDE = lds.h

OBJ = ${patsubst %,${OBJDIR}/%,${LDS_OBJS}}

${OBJDIR}/%.o : %.c ${LDS_INCLUDE}
	@mkdir -p ${OBJDIR}
	${CC} -c $< -o $@ ${CFLAGS}

lds : ${OBJ}
	@mkdir -p ${EXEDIR}
	${CC} -o ${EXEDIR}/$@ ${CFLAGS} ${OBJ} ${LDFLAGS}

clean :
	rm -f ${OBJDIR}/*.o
	rm -f ${EXEDIR}/lds

# makefile for the Moirai Land Data System (Moirai LDS)
# command line tool for generating land data for GCAM
#
# invoke by issuing command "make" from this directory
#
# the EXEDIR is <project direcotry>/bin by default
#	so lds must either be called from the project directory
#	or the paths must be updated in the input file to reflect the calling/working directory
#
# NetCDF library and header locations:
#   these are now set automatically based on the install location
#   if netcdf is not installed the makefile stops and returns an error message

SRCDIR = ${PWD}/src
HDRDIR = ${PWD}/include
EXEDIR = ${PWD}/bin
OBJDIR = ${PWD}/obj

LDS_HDRS = moirai.h

# if netcdf is installed, assign header and library paths and set linker flags; else, exit with error
# LDFLAGS_GENERIC links the math library and the netcdf support libraries
ifneq ("$(wildcard $(shell $$cat which nc-config))", "")
	NCHDRDIR := $(shell $$cat nc-config --includedir)
	NCLIBS := $(shell $$cat nc-config --libs)
	LDFLAGS_GENERIC = -lm $(NCLIBS)
else
	NCERROR = "NetCDF-C library not found. \
			   Please install NetCDF-C library and try again."
	# NOTE:  next line uses spaces to indent instead of tab which indicates a
	#  command which cannot exist outside of a rule
    $(error $(NCERROR))
endif

# include the lds and netcdf header directories
INCDIRS = $(HDRDIR) $(NCHDRDIR)
IFLAGS = $(INCDIRS:%=-I%)

# For Linux
CFLAGS =  -O3 -std=c99 ${CFLAGS_GENERIC} # Almost fully optimized and using ISO C99 features
# CFLAGS = -fast -std=c99 ${CFLAGS_GENERIC} # Almost fully optimized and using ISO C99 features
# CFLAGS = -O3 -std=c99 -ffloat-store ${CFLAGS_GENERIC} # Use precise IEEE Floating Point
#CFLAGS = -g -Wall -pedantic -std=c99 ${CFLAGS_GENERIC} # debugging with line/file reporting and 'standards' testing flags
# CFLAGS = -fast -Wall -pedantic -std=c99 ${CFLAGS_GENERIC} # testing with line/file reporting and 'standards' testing flags
LDFLAGS = ${LDFLAGS_GENERIC}
CC = gcc

# get just the object names for the desired source files
SRC_FILES_ALL_WPATH = ${wildcard ${SRCDIR}/*.c}
SRC_FILES_ALL = ${notdir ${SRC_FILES_ALL_WPATH}}
RMSRC = read_nfert.c proc_nfert.c read_lu_hyde.c
SRC_FILES = ${filter-out ${RMSRC}, ${SRC_FILES_ALL}}
SRC_FILES_WPATH = ${patsubst %,${SRCDIR}/%,${SRC_FILES}}
LDS_OBJS = ${patsubst %.c,%.o,${SRC_FILES}}
#$(info $$LDS_OBJS is [${LDS_OBJS}])

OBJ = ${patsubst %,${OBJDIR}/%,${LDS_OBJS}}
#$(info $$OBJ is [${OBJ}])

LDS_INCLUDE = ${patsubst %,${HDRDIR}/%,${LDS_HDRS}}
#$(info $$LDS_INCLUDE is [${LDS_INCLUDE}])

${OBJDIR}/%.o : ${SRCDIR}/%.c ${LDS_INCLUDE}
	@mkdir -p ${OBJDIR}
	${CC} -c $< -o $@ ${CFLAGS} ${IFLAGS}

moirai : ${OBJ}
	@echo "Path to NetCDF header source directory:  $(NCHDRDIR)"
	@echo "Path to NetCDF library source directory (includes linker flags):  $(NCLIBS)"
	@mkdir -p ${EXEDIR}
	${CC} -o ${EXEDIR}/$@ ${CFLAGS} ${OBJ} ${LDFLAGS} ${IFLAGS}

clean :
	rm -f ${OBJDIR}/*.o
	rm -f ${EXEDIR}/lds

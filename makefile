# makefile for the Land Data System (LDS)
# command line tool for generating land data for GCAM
#
# invoke by issuing command "make" from this directory
#
# the EXEDIR is <project direcotry>/bin by default
#	so lds must either be called from the project directory
#	or the paths must be updated in the input file to reflect the calling/working directory
#
# NetCDF library and header locations:
#   the defaults are: /usr/local/lib and /usr/local/include
#   change LDFLAGS_GENERIC line below to point to your instance of the library

SRCDIR = ${PWD}/src
HDRDIR = ${PWD}/include
EXEDIR = ${PWD}/bin
OBJDIR = ${PWD}/obj

# link the math library and the netcdf support libraries
LDFLAGS_GENERIC = -lm -L/usr/local/lib -lnetcdf -L/usr/local/include

# include the lds header directory
IFLAGS = -I${HDRDIR}

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
RMSRC = read_nfert.c proc_nfert.c
SRC_FILES = ${filter-out ${RMSRC}, ${SRC_FILES_ALL}}
SRC_FILES_WPATH = ${patsubst %,${SRCDIR}/%,${SRC_FILES}}
LDS_OBJS = ${patsubst %.c,%.o,${SRC_FILES}}
$(info $$LDS_OBJS is [${LDS_OBJS}])

OBJ = ${patsubst %,${OBJDIR}/%,${LDS_OBJS}}

LDS_HDRS = lds.h

LDS_INCLUDE = ${patsubst %,${HDRDIR}/%,${LDS_HDRS}}
#$(info $$LDS_INCLUDE is [${LDS_INCLUDE}])

${OBJDIR}/%.o : ${SRCDIR}/%.c ${LDS_INCLUDE}
	@mkdir -p ${OBJDIR}
	${CC} -c $< -o $@ ${CFLAGS} ${IFLAGS}

lds : ${OBJ}
	@mkdir -p ${EXEDIR}
	${CC} -o ${EXEDIR}/$@ ${CFLAGS} ${OBJ} ${LDFLAGS} ${IFLAGS}

clean :
	rm -f ${OBJDIR}/*.o
	rm -f ${EXEDIR}/lds

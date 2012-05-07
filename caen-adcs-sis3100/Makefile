#-*- Makefile -*-
# Makefile for DAS4MPRS

#####################################################################
# This is only for Linux
OSTYPE = $(shell uname)
ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

ifneq ($(OSTYPE),linux)
SYSERR::
	@echo "....";
	@echo "System \"${OSTYPE}\" unsupported! ";
	@echo "...";
endif

# Find MIDAS
MDAQ_CFG := $(shell which mdaq-config 2> /dev/null)
ifdef MDAQ_CFG
MIDAS_CFLAGS := $(shell mdaq-config --cflags)
MIDAS_LIBS   := $(shell mdaq-config --libs)
MIDAS_LIBDIR := $(shell mdaq-config --libdir)
endif


# Find ROOT
ifndef ROOTSYS
ROOT_CONFIG := $(shell which root-config 2> /dev/null)
ifdef ROOT_CONFIG
	ROOTSYS := $(shell root-config --prefix)
else
missroot::
	@echo "..."
	@echo "ROOT system is not found!"
	@echo "..."
endif

else
ROOT_CONFIG := ${ROOTSYS}/bin/root-config
endif

ROOT_CFLAGS := $(shell ${ROOT_CONFIG} --cflags) -DHAVE_ROOT -DUSE_ROOT
ROOT_LIBS   := $(shell ${ROOT_CONFIG} --libs)
ROOT_LIBS   +=  -lThread -Wl,-rpath,$(shell ${ROOT_CONFIG} --libdir)

##--------- Frontend
MIDAS_LIBDIR := $(shell mdaq-config --libdir)
MVME_LIBS    := -lmvme_sis3100 -l_sis3100
FE_CFLAGS    := ${MIDAS_CFLAGS}
FE_LIBS      := ${MIDAS_LIBS} ${MVME_LIBS}

##--------- Analyzer modules --------------
ANA_MODS   := adcraw.o
ANA_CFLAGS := ${MIDAS_CFLAGS} ${ROOT_CFLAGS}
ANA_LIBS   := ${MIDAS_LIBS} ${ROOT_LIBS}

##--------- Analyzer using rootana
#RANA_CFLAGS := -I/opt/DAQ/include/rootana -I/opt/DAQ/include/libNetDirectory
RANA_CFLAGS += -DHAVE_LIBNETDIRECTORY -DOLD_SERVER
RANA_LIBS   := $(shell ${ROOT_CONFIG} --glibs) -lXMLParser -lThread \
	-L/opt/DAQ/lib -lrootana -lNetDirectory

#############################################################
EXES :=  ../bin/cvadcfe ../bin/cvadcana

all: paths ${EXES}

.PHONY: clean paths

clean:
	-rm -f *.o *~ \#*

paths:
	@mkdir -p ../bin

clean-all: clean
	-rm -f ${EXES}

../bin/cvadcfe: cv560.o vadc_caen.o cvadcfe.o ${MIDAS_LIBDIR}/mfe.o
	${CC} -o $@ $^ ${LDFLAGS} ${FE_LIBS}

../bin/cvadcana: ${MIDAS_LIBDIR}/rmana.o cvadcana.o ${ANA_MODS}
	${CXX} -o $@ $^ ${ANA_LIBS}

%.o: %.c experim.h
	${CC} ${CFLAGS} ${FE_CFLAGS} -c -o $@ $<

%.o: %.cpp
	${CXX} ${CXXFLAGS} ${ANA_CFLAGS} -c -o $@ $<


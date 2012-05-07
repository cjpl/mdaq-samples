#####################################################################
#
#  Name:       Makefile
#  Created by: Exaos Lee
#
#  Contents:   Makefile for MIDAS start example using `mdaq-config`
#
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
MIDAS_CFLAGS := $(shell $(MDAQ_CFG) --cflags)
MIDAS_LIBS   := $(shell $(MDAQ_CFG) --libs)
MIDAS_LIBDIR := $(shell $(MDAQ_CFG) --libdir)
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

#-------------------------------------------------------------------
# List of analyzer modules
#
MODULES   = adccalib.o adcsum.o scaler.o

#-------------------------------------------------------------------
# Hardware driver can be (camacnul, kcs2926, kcs2927, hyt1331)
#
DRIVER := $(shell $(MDAQ_CFG) --srcdir)/cnaf_callback.c  \
          $(shell $(MDAQ_CFG) --drvdir)/camac/camacnul.c

#-------------------------------------------------------------------
# Frontend code name defaulted to frontend in this example.
UFE := frontend
UFE_DEPS := $(MIDAS_LIBDIR)/mfe.o $(DRIVER) $(UFE).o

ANA := analyzer
ANA_DEPS := $(MIDAS_LIBDIR)/rmana.o $(ANA).o $(MODULES)

####################################################################

all: $(UFE) $(ANA)

$(UFE): $(UFE_DEPS)
	$(CC) $(CFLAGS) $(MIDAS_CFLAGS) -o $@ $^ $(MIDAS_LIBS)

$(ANA): $(ANA_DEPS)
	$(CXX) $(CXXFLAGS) $(MIDAS_CFLAGS) $(ROOT_CFLAGS) -o $@ $^ $(MIDAS_LIBS) $(ROOT_LIBS)

%.o: %.c experim.h
	$(CC) $(MIDAS_CFLAGS) -o $@ -c $<

%.o: %.cxx experim.h
	$(CXX) $(MIDAS_CFLAGS) $(ROOT_CFLAGS) -o $@ -c $<

clean::
	rm -f *.o *~ \#* $(UFE) $(ANA)

.PHONY: clean


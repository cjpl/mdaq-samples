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
ifndef MIDASSYS
MIDASSYS := /opt/DAQ
endif

MIDAS_CFLAGS := -g -O2 -Wall -I${MIDASSYS}/include/midas
MIDAS_CFLAGS += -DOS_LINUX -Dextname -DHAVE_ZLIB
MIDAS_LIBS   := -lm -lz -lutil -lnsl -lrt -lpthread -L${MIDASSYS}/lib -lmidas

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

##--------- Analyzer modules --------------
ANA_MODS = adccalib.o adcsum.o scaler.o

##--------- Frontend
UFE := frontend
FE_DRV := camacnul
FE_OBJS = ${MIDASSYS}/lib/midas/mfe.o ${FE_DRV}.o cnaf_callback.o evtgen.o
MIDAS_CFLAGS += -I${MIDASSYS}/share/midas/drivers/camac

##--------- Analyzer using rootana
ROOT_GLIBS  := $(shell ${ROOT_CONFIG} --glibs) -lXMLParser -lThread
RANA_CFLAGS := -I/opt/DAQ/include/rootana -I/opt/DAQ/include/libNetDirectory
RANA_CFLAGS += -DHAVE_LIBNETDIRECTORY -DOLD_SERVER
RANA_LIBS   := -L/opt/DAQ/lib -lrootana -lNetDirectory

#############################################################
CFLAGS   += ${MIDAS_CFLAGS}
CXXFLAGS += ${MIDAS_CFLAGS} ${ROOT_CFLAGS}
LDFLAGS  += ${MIDAS_LIBS} ${ROOT_LIBS}
EXES :=  ${UFE} analyzer ana4evt

all: ${EXES}

${UFE}: ${UFE}.o ${FE_OBJS}
	${CC} ${LDFLAGS} -o $@ $^

analyzer: analyzer.o ${MIDASSYS}/lib/midas/rmana.o ${ANA_MODS}
	${CXX} ${LDFLAGS} -o $@ $^

ana4evt: ana4evt.cpp
	${CXX} ${CXXFLAGS} ${RANA_CFLAGS} -g -o $@ $< \
	       ${ROOT_GLIBS} ${MIDAS_LIBS} ${RANA_LIBS}

cnaf_callback.o: ${MIDASSYS}/share/midas/src/cnaf_callback.c
	${CC} ${CFLAGS} -c -o $@ $<

${FE_DRV}.o: ${MIDASSYS}/share/midas/drivers/camac/${FE_DRV}.c
	${CC} ${CFLAGS} -c -o $@ $<

evtgen.o: evtgen.cc evtgen.h
	${CXX} $(shell ${ROOT_CONFIG} --cflags) ${MIDAS_CFLAGS} -c -o $@ $<

%.o: %.c experim.h
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

%.o: %.cc
	${CXX} ${CXXFLAGS} -c -o $@ $<

clean::
	rm -f *.o *~ \#* ${EXES}


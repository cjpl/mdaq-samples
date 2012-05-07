#-*- mode: cmake -*-
cmake_minimum_required(VERSION 2.6)

SET(EXECUTABLE_OUTPUT_PATH ../bin)
SET(NEED_ROOT 1)
include(FindROOT.cmake)

## MIDAS Library
SET(MIDASSYS /opt/DAQ)
include_directories(${MIDASSYS}/include/midas)
set(MIDAS_LIBS   "-lm -lz -lutil -lnsl -lpthread -lodbc")
set(MIDAS_CFLAGS "-DOS_LINUX -Dextname")

################################################
### Executables: frontend
SET(DRIVER camacnul)
set_source_files_properties(frontend.c PROPERTIES  LANGUAGE C)
set_source_files_properties(evtgen.cc  PROPERTIES  LANGUAGE CXX
  COMPILE_FLAGS  "${ROOT_CFLAGS}")

add_executable(frontend frontend.c
  ${MIDASSYS}/share/midas/src/cnaf_callback.c
  ${MIDASSYS}/lib/midas/mfe.o
  ${MIDASSYS}/share/midas/drivers/camac/${DRIVER}.c
  evtgen.cc )
set_target_properties(frontend PROPERTIES LINKER_LANGUAGE C 
  COMPILE_FLAGS "${MIDAS_CFLAGS}"
  LINK_FLAGS "${ROOT_LIBS} ${MIDAS_LIBS}")
target_link_libraries(frontend ${MIDASSYS}/lib/libmidas.a)

################################################
### Executables: analyzer
SET(ANA_MODS  adccalib.cpp  adcsum.cpp  scaler.c)
set_source_files_properties(analyzer.cpp ${ANA_MODS}
  PROPERTIES LANGUAGE CXX
  COMPILE_FLAGS  "${MIDAS_CFLAGS} -DHAVE_ROOT -DUSE_ROOT ${ROOT_CFLAGS}")

add_executable(analyzer  analyzer.cpp
  ${MIDASSYS}/lib/midas/rmana.o ${ANA_MODS})
set_target_properties(analyzer PROPERTIES
  COMPILE_FLAGS "${MIDAS_CFLAGS} ${ROOT_CFLAGS}"
  LINK_FLAGS "${ROOT_LIBS} -lThread ${MIDAS_LIBS}")
target_link_libraries(analyzer ${MIDASSYS}/lib/libmidas.a)

################################################
### Executables: try_evtgen
add_executable(try_evtgen try_evtgen.c evtgen.cc )

set_source_files_properties(try_evtgen.c PROPERTIES LANGUAGE C)
set_target_properties(try_evtgen PROPERTIES LINKER_LANGUAGE C
  LINK_FLAGS "${ROOT_LIBS}")

################################################
### Analyzer using rootana
add_executable(ana4evt ana4evt.cpp)
include_directories(/opt/DAQ/include/rootana /opt/DAQ/include/libNetDirectory)
set_target_properties(ana4evt PROPERTIES
  COMPILE_FLAGS "${MIDAS_CFLAGS} ${ROOT_CFLAGS} -DHAVE_LIBNETDIRECTORY -DOLD_SERVER"
  LINK_FLAGS "${MIDAS_LIBS} -lmidas ${ROOT_LIBS} -lXMLParser -lThread -L/opt/DAQ/lib -lrootana -lNetDirectory")


################################################
### Just for test
#add_library(evtgen evtgen.cc)

* MIDAS test DAQ code using random event generator       -*- mode: outline; -*-

** Author
   Exaos Lee <Exaos.Lee(at)gmail.com>
   +86 10 6935 7544

** Purpose:
   Modified example code from ${MIDAS_PKG}/examples/experiment for testing
   purpose, especially for using an event generator.

** Changlogs

   2009-08-20 -- a. Finished evtgen.cc
      	      	 b. Solve the problem of mixing C and C++ code
		 c. Test evtgen.cc with try_evtgen.c --- OK!
		 d. Modify frontend.c for using evtgen.cc
		 e. Test whether frontend works with evtgen --- OK!
		 f. Add "CMakeLists.txt" and test --- OK!

   2009-08-19 -- a. Try MC event generator using ROOT TRandom -- OK!

   2009-08-18 -- a. Try MC event generator using my own "pdfs.c" -- OK!

** Files

*** Common and build files
    Makefile        --- as named
    CMakeLists.txt  --- CMake support file
    experim.h       --- Common header
    FindROOT.cmake  --- Find system ROOT installation (for CMake)

*** Event generator
    evtgen.[h,cc]   --- Event generator, header and source
    try_evtgen.c    --- Test "C" code invoking "evtgen.h"
    build_try.sh    --- Build the test code

*** Sources for "frontend"
    frontend.c

*** Sources for "analyzer"
    analyzer.[h,c]  --- The analyzer header and source
    adccalib.c      --- ADC calibration module
    adcsum.c        --- ADC sum module
    scaler.c        --- Scaler module

*** Misc
    daq_evn.sh    --- Environment setup
    readme.txt    --- Original "README" from MIDAS
    00README.txt  --- This file
    template.c    --- from MIDAS
    pdfs.c        --- My test PDFs

** Event generator

   Now, the event generator produces a random event by each time call. One
   event contains at most four simulated parameters: [PH, T, V, E]

   	 PH -- The pulse height of a neutron detector response
	 T  -- The neutron time of flight
	 V  -- The neutron volecity
	 E  -- The neutron energy

   The neutron energy is distributed as a PDF h(E):

       h(E) = a_0 \exp\left(a_1 + a_2 x\right) + 
     	   a_3 \exp\left(-\frac{1}{2}\left(\frac{x-a_4}{a_5}\right)^2\right) +
 	   a_6 \exp\left(-\frac{1}{2}\left(\frac{x-a_7}{a_8}\right)^2\right)

   a. PH is a random from [0,E] when E is determined by PDF h(E)
   b. T is determined by PDF gaus(72.3/\sqrt(E), \sigma)
   c. V is determined by PDF gaus(13.83\sqrt(E), \sigma)
   d. E is determined by PDF h(E) as mentioned above



#!/bin/bash
export PATH=/usr/lib/mdaq/bin:$PATH
export MIDASSYS=/usr/lib/mdaq
export MIDAS_EXPTAB=/home/exaos/Workspace/daq/lumidas/samples/mdaq-start/exptab
unset MIDAS_DIR=
cd /home/exaos/Workspace/daq/lumidas/samples/mdaq-start
/usr/lib/mdaq/bin/mhttpd -e mdaq-start -p 8080
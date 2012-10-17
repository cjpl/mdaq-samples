#!/bin/bash
export MIDASSYS=$HOME/Workspace/daq/lumidas/run
unset MIDAS_DIR
export MIDAS_EXPTAB=/home/exaos/Workspace/daq/lumidas/samples/mdaq-start/exptab
export PATH=${MIDASSYS}/bin:$PATH

cd /home/exaos/Workspace/daq/lumidas/samples/mdaq-start

${MIDASSYS}/bin/mserver -D
sleep 1

${MIDASSYS}/bin/mhttpd -D -p 8080
# -e mdaq-start
sleep 1

/home/exaos/Workspace/daq/lumidas/samples/mdaq-start/bin/frontend -D -e mdaq-start
sleep 1

/home/exaos/Workspace/daq/lumidas/samples/mdaq-start/bin/analyzer -D -e mdaq-start
sleep 1

${MIDASSYS}/bin/mlogger -D -e mdaq-start
sleep 1

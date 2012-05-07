#!/bin/sh

[ ! "$MIDASSYS" ] && MIDASSYS=/opt/MIDAS.PSI/Version/Current
[ ! "$HTTPPORT" ] && HTTPPORT=8080
[ ! "$SRVHOST" ]  && SRVHOST=localhost
LOGGER=${MIDASSYS}/bin/mlogger

EXPPATH=/home/das/online/test
CODEPATH=${EXPPATH}/code
LOGGER=${MIDASSYS}/bin/mlogger

PROG_FE=${CODEPATH}/frontend
PROG_ANA=${CODEPATH}/analyzer

if [ ! "$MIDAS_EXPTAB" ]; then
	MIDAS_DIR=${EXPPATH}
else
	MIDAS_EXPT_NAME="test"
fi

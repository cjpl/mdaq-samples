#!/bin/sh

CFLAGS=`mdaq-config --cflags`
g++ -c ${CFLAGS} `root-config --cflags` -o evtgen.o evtgen.cpp
gcc -c ${CFLAGS} -o try_evtgen.o try_evtgen.c
gcc `root-config --libs` -o try_evtgen  try_evtgen.o evtgen.o

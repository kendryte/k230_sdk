#!/bin/sh
cd mpi
doxygen resource/Doxyfile
cd ../mapi
doxygen resource/Doxyfile
cd ../syslink
doxygen resource/Doxyfile

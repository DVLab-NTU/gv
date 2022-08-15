#!/bin/bash

# make "abc" static library
cd engine/abc; make -j12; make libabc.a;

# copy "abc" static library to /gv_src
mkdir gv_src; mv libabc.a gv_src;

# make "yosys" shared library 
cd ../yosys; make config-gcc; make -j12; make install;

# copy "yosys" shared library to /gv_src
mv libyosys.so gv_src/;

# create "yosys" static library to /gv_src
ar rc gv_src/libyosys.a backends/*/*.o frontends/*/*.o kernel/*.o libs/*/*.o passes/*/*.o techlibs/*/*.o ext-sim/*.o;

# get V3 source
cd .. ; git clone https://github.com/HHHUUUGGGOOO/V3-fixed.git;

# clean exe file
cd V3-fixed/; make; rm -rf src/main/main.o; rm -rf .git;

# copy "V3" header files and static library to /gv_src
mkdir gv_src; cp ./src/*/*.h gv_src; ar rc gv_src/libv3.a src/*/*.o;

# back to /gv0
cd ../..;

# make GV and get start
make clean; make; make; clear; ./gv;


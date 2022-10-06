#!/bin/bash

# make "abc" static library
cd engine/abc; make -j12; make libabc.a;

# copy "abc" static library to /gv_src
mkdir gv_src; mv libabc.a gv_src;

# include all hierarchical folder with only "header file" under gv_src/
cp -r src/* gv_src/
cd gv_src/
rm *.c 
find ./ -name "*.c" | xargs rm 
find ./ -name "*.o" | xargs rm
find ./ -name "*.d" | xargs rm
find ./ -name "*.make" | xargs rm

# make "yosys" shared library 
cd ../../yosys; make config-gcc; make -j12; make install;

# copy "yosys" shared library to /gv_src
mv libyosys.so gv_src/;

# create "yosys" static library to /gv_src
ar rc gv_src/libyosys.a backends/*/*.o frontends/*/*.o kernel/*.o libs/*/*.o passes/*/*.o techlibs/*/*.o ext-sim/*.o;

# back to /gv0
cd ../..;

# make GV and get start
make clean; make; make; clear; ./gv;


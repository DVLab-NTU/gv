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
#ln -s  ./engine/boolector/build/lib ./src/eng/boolector

#clone boolector
cd engine/; git clone https://github.com/Boolector/boolector.git;
cd boolector ;
#build dependend tool
./contrib/setup-lingeling.sh ;
./contrib/setup-btor2tools.sh ;
#build boolector
./configure.sh --no-minisat ; cd build; make;
#copy lib&header to src/eng
cd ../../../
#
#cp ./engine/
#cp ./engine/boolector/build/lib/libboolector.a  ./src/eng/boolector/
#cp ./engine/boolector/src/*.h  ./src/eng/boolector/
#cp ./engine/boolector/src/*.h  ./src/eng/boolector/
# make GV and get start
make clean; make; make; clear; ./gv;


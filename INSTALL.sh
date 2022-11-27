#!/bin/bash

# go to engine/
cd engine/;

# install "abc"
./abc.script;
# install "yosys"
./yosys.script;

# back to gv0/
cd ../;

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
cp ./engine/boolector/build/lib/libboolector.a  ./src/eng/boolector/
cp ./engine/boolector/src/boolector.h  ./src/eng/boolector/
cp ./engine/boolector/src/btortypes.h  ./src/eng/boolector/

mkdir ./src/eng/btor2parser
mkdir ./src/eng/lgl
cp ./engine/boolector/deps/install/lib/libbtor2parser.a  ./src/eng/btor2parser/
cp ./engine/boolector/deps/install/lib/liblgl.a  ./src/eng/lgl

# make GV and get started
make clean; make; make; clear; ./gv;


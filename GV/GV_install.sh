#!/bin/bash

# install abc and v3 in engine
cd engine; chmod +x install.script; ./install.script;

# make static library
cd abc; make libabc.a; mkdir gv_src;

# copy library link
cp libabc.a gv_src

# back to GV engine
cd ../../src/eng

# symbolink
rm -rf abc; ln -s ../../engine/abc/gv_src/ abc;

# clean exe file
cd ../../engine/V3-fixed/src/main; rm -rf main.o;

# make static library
cd ../..; mkdir gv_src; cp ./src/*/*.h gv_src;

# maintain library archive
ar rc gv_src/libv3.a src/*/*.o;

# make GV and get start
cd ../../; make clean; make; clear; ./gv;
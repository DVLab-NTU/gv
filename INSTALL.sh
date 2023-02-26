#!/bin/bash

# go to engine/
cd engine/

# install "abc"
./abc.script
# install "yosys"
./yosys.script
# install "boolector"
./boolector.script

# back to gv0/
cd ../

# make GV
make

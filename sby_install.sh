#!/bin/bash

# prerequisites
sudo apt-get install build-essential clang bison flex libreadline-dev
sudo apt-get install build-essential clang bison flex libreadline-dev
sudo apt-get install gawk tcl-dev libffi-dev git mercurial graphviz
sudo apt-get install xdot pkg-config python python3 libftdi-dev gperf
sudo apt-get install libboost-program-options-dev autoconf libgmp-dev
sudo apt-get update
sudo apt-get install --fix-missing
sudo apt-get install cmake curl

# (MUST) Yosys, Yosys-SMTBMC, abc 
git clone https://github.com/YosysHQ/yosys.git yosys
cd yosys
make -j$(nproc)
sudo make install
cd ..

# (MUST) SymbiYosys 
git clone https://github.com/YosysHQ/SymbiYosys.git SymbiYosys
cd SymbiYosys
sudo make install
cd ..

# (OPTIONAL) Yices2
git clone https://github.com/SRI-CSL/yices2.git yices2
cd yices2
autoconf
./configure
make -j$(nproc)
sudo make install
cd ..

# (MUST, but takes time) Z3
git clone https://github.com/Z3Prover/z3.git z3
cd z3
python scripts/mk_make.py
cd build
make -j$(nproc)
sudo make install
cd ..

# (OPTIONAL, havn't successfull installed) super_prove


# (OPTIONAL) Avy
git clone https://bitbucket.org/arieg/extavy.git
cd extavy
git submodule update --init
mkdir build; cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo cp avy/src/{avy,avybmc} /usr/local/bin/
cd ..

# (OPTIONAL) Boolector
git clone https://github.com/boolector/boolector
cd boolector
./contrib/setup-btor2tools.sh
./contrib/setup-lingeling.sh
./configure.sh
make -C build -j$(nproc)
sudo cp build/bin/{boolector,btor*} /usr/local/bin/
sudo cp deps/btor2tools/bin/btorsim /usr/local/bin/
cd ..

git clone https://github.com/boolector/btor2tools
cd btor2tools
./configure.sh
cmake . -DBUILD_SHARED_LIBS=OFF
make -j$(nproc)
sudo make install
cd ..
#!/bin/bash

# Install prerequisites for building yosys
sudo apt-get install build-essential clang bison flex \
	libreadline-dev gawk tcl-dev libffi-dev git \
	graphviz xdot pkg-config python3 libboost-system-dev \
	libboost-python-dev libboost-filesystem-dev zlib1g-dev


if [ -f "/usr/local/lib/yosys/libyosys.so" ]; then
    echo "File /usr/local/lib/yosys/libyosys.so exists."
else
    echo "File /usr/local/lib/yosys/libyosys.so does not exists."
    # Make install yosys
    cd engine/yosys;
    sudo make install;
fi

# Install GTKWave package
sudo apt-get install gtkwave;

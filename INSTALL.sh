cd engine/abc; mkdir gv_src;make -j12;make libabc.a; mv libabc.a gv_src;
cd ../yosys; 
make config-gcc;make -j12; make install;
mv libyosys.so gv_src/;
ar rc gv_src/libyosys.a backends/*/*.o frontends/*/*.o kernel/*.o libs/*/*.o passes/*/*.o techlibs/*/*.o ext-sim/*.o;
cd .. ; git clone https://github.com/HHHUUUGGGOOO/V3-fixed.git;
cd V3-fixed/; make; rm -rf src/main/main.o; rm -rf .git;
mkdir gv_src; cp ./src/*/*.h gv_src; ar rc gv_src/libv3.a src/*/*.o;
cd ../..;
make clean;make;make;
clear;
./gv;


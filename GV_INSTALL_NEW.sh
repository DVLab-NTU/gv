cd GV/engine/abc; mkdir gv_src;make -j12;make libabc.a; mv libabc.a gv_src;
cd ../yosys; 
make config-gcc;make -j12; make install;
ar rc gv_src/libyosys.a backends/*/*.o frontends/*/*.o kernel/*.o libs/*/*.o passes/*/*.o techlibs/*/*.o ext-sim/*.o;
cd ../..;
make clean;make;make;
clear;
./gv;


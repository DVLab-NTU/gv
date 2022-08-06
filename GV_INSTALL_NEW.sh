cd GV/engine/abc; mkdir gv_src;sudo make -j12;sudo make libabc.a; mv libabc.a gv_src;
cd ../yosys; 
make config-clang;sudo make -j12; sudo make install;
ar rc gv_src/libyosys.a backends/*/*.o frontends/*/*.o kernel/*.o libs/*/*.o passes/*/*.o techlibs/*/*.o;
cd ../..;
sudo make clean;sudo make;./gv

# sudo apt-get install git
# sudo apt-get install wget
# sudo apt-get install libreadline-dev
# sudo apt-get install graphviz

git clone https://github.com/chengyinwu/V3
rm -rf V3/engine
cp -rf v3_needed/SoCV_v3_engine V3/engine
cp v3_needed/Makefile.in.diff V3/src
cd V3/engine/; chmod +x boolector.script; chmod +x minisat.script; chmod +x quteRTL.script
./boolector.script; ./minisat.script; ./quteRTL.script; cd ../
cd src; patch Makefile.in < Makefile.in.diff; cd ..; make;

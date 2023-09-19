# gv0
Verification research for general RTL/system-level designs (take 0.0)

# GV Installation
```json=
git clone git@github.com:ric2k1/gv0.git
cd gv0/
./SETUP.sh
./INSTALL.sh 
```

- **For using GV tool interface, type after installation:**
```json=
cd GV/
./gv
```

# GV's Third-Party Tools
- **word-level** -> [yosys](https://github.com/YosysHQ/yosys) / [boolector](https://github.com/Boolector/boolector)
- **gate-level** -> [berkeley-abc](https://github.com/berkeley-abc/abc) 
- **file format converter** -> yosys
- **formal verification engine** -> berkeley-abc 
- **simulator** -> yosys 

# GV's API & Tutorial
- For GV usage, please check the document in [gv0/doc/GV_tutorial.pdf](https://github.com/ric2k1/gv0/tree/main/doc) (GV_tutorial.pdf):

# GV's Development 
- If one would like to develop features on GV, please check the document in [gv0/doc/README.md](https://github.com/ric2k1/gv0/tree/main/doc) (README.md):

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

# GV's Architecture
<img width="386" alt="Screen Shot 2023-09-19 at 3 19 13 PM" src="https://github.com/ric2k1/gv0/assets/45988775/19ea0c69-2bc5-4646-bb06-bd3f7b29a119">

# GV's API & Tutorial
- For GV usage, please check the document in [gv0/doc/GV_tutorial.pdf](https://github.com/ric2k1/gv0/tree/main/doc) 

# GV's Development 
- If one would like to develop features on GV, please check the document in [gv0/doc/README.md](https://github.com/ric2k1/gv0/tree/main/doc) 

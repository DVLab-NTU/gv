# GV
- **GV** is a general-purposed verification framework (take version 0.0)
- **GV** serves as the bridge between multiple engines, meaning that developers who require several engines can implement their algorithms using only **"GV"**

# GV Installation
```bash=
git clone git@github.com:DVLab-NTU/gv.git
cd gv/
make
```
- ** Run unit tests
```bash=
make test
```

- **For using GV tool interface, type after installation:**
```bash=
./gv
```

# GV's Third-Party Tools
- **word-level** -> [yosys](https://github.com/YosysHQ/yosys) / [boolector](https://github.com/Boolector/boolector)
- **gate-level** -> [berkeley-abc](https://github.com/berkeley-abc/abc) 
- **file format converter** -> yosys
- **formal verification engine** -> berkeley-abc 
- **simulator** -> yosys 

# GV's Architecture
- **Parser**
  - Read in DUV 
  - e.g. berkeley-abc (GIA, which is the improved data structure of AIG) -> convert into GV's AIG data structure
  - e.g. yosys (RTL data structure) -> convert into GV's word-level data structure (TBD)
- **Engine**
  - Wrap API from off-the-shelf engine
  - e.g. [miniSAT](https://github.com/niklasso/minisat) -> SAT solver 
  - e.g. [RicBDD](https://github.com/ric2k1/RicBDD) -> BDD
- **Application**
  - e.g. Engineering Change Order, Model Checking, Equivalence Checking
<img width="300" alt="Screen Shot 2023-09-19 at 3 19 13 PM" src="https://github.com/ric2k1/gv0/assets/45988775/19ea0c69-2bc5-4646-bb06-bd3f7b29a119">

# GV's API & Tutorial
- For GV usage, please check the document in [gv0/doc/GV_tutorial.pdf](https://github.com/ric2k1/gv0/tree/main/doc) 

# Develop on GV
- If one would like to develop features on GV, please check the document in [gv0/doc/README.md](https://github.com/ric2k1/gv0/tree/main/doc) 

# gv0
Verification research for general RTL/system-level designs (take 0.0)

# GV tool installation
```json=
git clone git@github.com:ric2k1/gv0.git
cd gv0/
# General Users
sudo ./INSTALL.sh 
# On cthulhu
./INSTALL.sh
```

- **For using GV tool interface, type after installation:**
```json=
cd GV/
./gv
```

- **Please notice that, if the shell script has no permission, type:**
```json=
chmod +x <shell_script_filename>
```

# GV tool's third-party tools
- **parser** --> [yosys](https://github.com/YosysHQ/yosys), [berkeley-abc](https://github.com/berkeley-abc/abc), [V3](https://github.com/chengyinwu/V3)
- **file format converter** --> yosys, V3
- **formal verification engine** --> berkeley-abc 
- **simulator** --> yosys

# GV tool's API 
- Below are current supporting APIs, please check the document above to get detailed tutorials in [gv0/doc/](https://github.com/ric2k1/gv0/tree/main/doc) (GV-API.pdf): 
    - **common** 
    ```json=
    GV Help
    GV History 
    GV Usage
    GV Dofile
    GV Quit
    ```
    
    - **setup design** 
    ```json=
    GV Set System Mode
    GV Set Engine
    GV Read Design
    GV Print Info
    ```

    - **formal verification**
    ```json=
    GV Verilog2Aig
    GV Formal Verify
    ```

    - **simulation**
    ```json=
    GV Random Simulate
    ```

# GV tool's set-system-mode flow
- Please check the document above to get detailed tutorials in [gv0/doc/](https://github.com/ric2k1/gv0/tree/main/doc) (Set_System_Mode_Flow.pdf):

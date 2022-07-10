# gv0
Verification research for general RTL/system-level designs (take 0.0)

# Reference 
- **overall progress** : https://reurl.cc/QLv7Q5
- **V3** : https://reurl.cc/DyVvVO
- **V3 + ABC-Berkeley** : https://reurl.cc/p1lzex

# How to install yosys
- **Follow the official tutorial for different environment**
    - yosys github src code: https://github.com/YosysHQ/yosys
- **Other useful link** 
    - Writing your own VCD: https://reurl.cc/k76dQq 
    - yosys manual document: https://reurl.cc/e6rxgM (google drive link) 
- **Function as** 
    - "Parser" and "Simulator"

# How to install V3 (Linux/WSL)
```json=
chmod +x v3_install.sh
./v3_install.sh
```

# How to install ABC-Berkeley (Linux/WSL)
```json=
chmod +x abc_install.sh
./abc_install.sh
```

# Test pattern generation flow
- How to simulate the circuit ?
    - **Read HDL file** (-sv means system verilog)
    ```json=
    read -sv <.v_file> 
    ----------------------------------------- (e.g.)
    read -sv tests/RISCV/Baseline/src/Final.v
    ```
    - **Preset top module**
    ```json=
    hierarchy -top <top_module_name>
    prep -top <top_module_name>
    sim -vcd [output_vcd_file] 
    -------------------------------- (e.g.)
    hierarchy -top Final
    prep -top Final
    sim -vcd tests/temp/Final.fst
    ```       

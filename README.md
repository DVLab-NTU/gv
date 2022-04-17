# gv0
Verification research for general RTL/system-level designs (take 0.0)

# About yosys
- **Link**
    - yosys github src code: https://github.com/YosysHQ/yosys 
    - Writing your own VCD: https://reurl.cc/k76dQq 
    - yosys manual document: https://reurl.cc/e6rxgM (google drive link) 
- **Function as** 
    - "Parser" and "Simulator"

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
    sim -v
    ----------------------------------------- (e.g.)
    hierarchy -top Final
    prep -top Final
    sim -v
    ```       
    
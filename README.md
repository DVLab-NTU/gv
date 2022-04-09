# gv0
Verification research for general RTL/system-level designs (take 0.0)

# About yosys
- **Link**
    - yosys github src code: https://github.com/YosysHQ/yosys 
    - Writing your own VCD: https://reurl.cc/k76dQq 
    - yosys manual document: https://reurl.cc/e6rxgM (google drive link) 
- **Function as** 
    - "Parser" and "Simulator"
- **Basic command**
    - Transform **.fst** file to **.tb** file
    ```json=
    fst2tb -tb CHIP_tb -r CHIP.fst -scope CHIP -clock clk CHIP
    ```

# Test pattern generation flow
- How to generate input patterns ?
    - First, handcraft (or use a driver) to generate a vcd file.
    - Install **gtkwave** by the following command
    ```json=
    sudo apt install gtkwave
    ```
    - Use **vcd2fst** to convert the **.vcd** file into an **.fst** file.
- How to simulate the circuit ?
    - **Read HDL file** (-sv means system verilog)
    ```json=
    read -sv <.v_file> 
    ```
    - **Preset top module**
    ```json=
    hierarchy -top <top_module_name>
    prep -top <top_module_name>
    sim -v
    ```        
# gv0
Verification research for general RTL/system-level designs (take 0.0)
# about yosys
Link
yosys github src code: https://github.com/YosysHQ/yosys 
writing your own VCD: https://reurl.cc/k76dQq 
yosys manual document: https://reurl.cc/e6rxgM (google drive link) 
# yosys command
cmd 
cmd > fst2tb -tb CHIP_tb -r CHIP.fst -scope CHIP -clock clk CHIP
transform “.fst” to “.tb”
要給他 input, 也要給他 output, 兩邊不一樣就噴 error
# test pattern generation flow
How to generate input patterns ?
(1) First handcraft (or use a driver) to generate a vcd file.
(How to write a vcd file : https://zipcpu.com/blog/2017/07/31/vcd.html)
(2) Use vcd2fst (you might have to install gtkwave through “sudo apt install gtkwave”) to convert the vcd file into an fst file.
How to simulate the circuit?
(1) read -sv [your.v file] (-sv means system verilog)
(2) hierarchy -top [your top module name]
(3) prep -top [your top module name]
(4) sim -vcd [output vcd file name] -n [number of cycle to sim] -r [the previously generated fst file] -scope [your top module]

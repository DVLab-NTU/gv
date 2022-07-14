### COMPILATIOn
```bash
$ cd <root directory of GV>
$ make
$ ./gv
```

### COMMANDS
```bash
# Common Commands
DOfile:             Execute the commands in the dofile.
HELp:               Print this help message.
HIStory:            Print command history.
Quit:               Quit the execution.
USAGE:              Report resource usage.

# Verilfy Commands
Formal Verify:      Use options to execute specific formal engine.

# Simulate Commands
RAndom Sim:         Conduct random simulation and print the results.

# Network Commands
PRint Info:         Print circuit information extracted by our parser.
REad Design:        Read RTL (Verilog) Designs.
VErilog2 Aig:       Convert verilog file into AIG. 
```

### HOW TO ADD COMMANDS
1. Select your directory under src (ntk for example).
2. Define you command name in gvNtkCmd.h.
   ```bash
   GV_COMMAND(GVReadDesignCmd, CMD_TYPE_NETWORK); 
   ```
3. Add the new command into the initialization function for recognition.
4. Define exec(), usage(), help() functions under gvNtkCmd.cpp.
5. If you add new header file under the directory, add it into the Makefile.
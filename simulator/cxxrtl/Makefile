
EXE_FILE = tb

CFLAGS := -g -std=c++14
INCLUDE := `yosys-config --datdir`/include
# SRCS := testbench.cpp
SRCS := testbench_sim.cpp
CXXRTL := g++ $(CFLAGS) -I $(INCLUDE) $(SRCS)

all: sim run

sim:
	$(CXXRTL) -o $(EXE_FILE)

run: 
	./tb

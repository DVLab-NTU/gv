
EXE_FILE = cxxrtl_tb 

CFLAGS := -g -std=c++14
CFLAGS += -DPATTERN_FILE=$(GV_PATH)/$(PATTERN_FILE)
CFLAGS += -DCYCLE=$(CYCLE)
CFLAGS += -DVCD_FILE=$(VCD_FILE)
INCLUDE := ../../build/engines/src/engine-yosys/
SRCS := testbench.cpp 
HEADERS := simulator.hpp interface.hpp utility.hpp
CXXRTL := g++ $(CFLAGS) -I$(INCLUDE) $(SRCS)

all: $(EXE_FILE)

$(EXE_FILE): $(SRCS) $(HEADERS)
	$(CXXRTL) -o $(EXE_FILE)

run: 
	./cxxrtl_tb

clean:
	rm ./cxxrtl_tb

.PHONY: clean

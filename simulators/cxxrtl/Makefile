# Executable name
EXE_FILE := cxxrtl_tb
# Directories and sources
SRC_DIR := src
SRCS := $(SRC_DIR)/testbench.cpp
# Include directories
INCLUDE := ../../build/engines/src/engine-yosys/
# Compiler and flags
CXX := g++
CXXFLAGS := -g -std=c++14
CXXFLAGS += -I$(INCLUDE) -I$(SRC_DIR)
CXXFLAGS += -DPATTERN_FILE=$(GV_PATH)/$(PATTERN_FILE)
CXXFLAGS += -DCYCLE=$(CYCLE)
CXXFLAGS += -DVCD_FILE=$(VCD_FILE)
CXXFLAGS += -DMODE=$(MODE)
# Headers
HEADERS := $(SRC_DIR)/simulator.hpp \
           $(SRC_DIR)/interface.hpp \
           $(SRC_DIR)/utility.hpp

# Targets
all: $(EXE_FILE)

$(EXE_FILE): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXE_FILE)

run: $(EXE_FILE)
	./$(EXE_FILE)

clean:
	rm -f $(EXE_FILE)

.PHONY: all run clean

VERILATOR := verilator
VRLT_PATH=$(GV_PATH)/simulators/verilator/

DESIGN_PATH := $(GV_PATH)/$(DESIGN)
DESIGN_NAME :=$(notdir $(basename $(DESIGN_PATH)))
SRC_DIR := $(VRLT_PATH)

VFLAGS += --cc --public --trace --assert -O3

VFLAGS += -CFLAGS -O3
VFLAGS += -CFLAGS -DCYCLE=$(CYCLE)
VFLAGS += -CFLAGS -DVCD_FILE=$(GV_PATH)/$(VCD_FILE)
VFLAGS += -CFLAGS -DMODE=$(MODE)
VFLAGS += -CFLAGS -DPATTERN_FILE=$(GV_PATH)/$(PATTERN_FILE)
VFLAGS += -CFLAGS -I$(VRLT_PATH)include


VFLAGS += -y build
VFLAGS += --Mdir build/obj_dir 
VFLAGS += -Wno-WIDTHEXPAND -Wno-WIDTHTRUNC  -Wno-LATCH
VFLAGS += --exe $(SRC_DIR)testbench.cpp

VERILATE=$(VERILATOR) $(VFLAGS)
SUBMAKE := $(MAKE) --no-print-directory --directory=build/obj_dir -f
VRLT_MK := Vdesign_under_test.mk

all: symbolic_link_design run_vrlt_sim

run_vrlt_sim: compile_vrlt_mk

compile_vrlt_mk: gen_vrlt_mk
	@echo Make the Verilator Makefile ...
# 	@echo $(SUBMAKE) $(VRLT_MK) 
	$(SUBMAKE) $(VRLT_MK) 

gen_vrlt_mk: 
	@echo Generating the Verilator Makefile ...
# 	@echo $(VERILATE) $(VRLT_PATH)design_under_test.v
	$(VERILATE) $(VRLT_PATH)design_under_test.v

symbolic_link_design:
	@echo Symbolic linking the design ...
#	@echo ln -fs $(DESIGN_PATH) $(VRLT_PATH)design_under_test.v
	ln -fs $(DESIGN_PATH) $(VRLT_PATH)design_under_test.v

run:
	./build/obj_dir/Vdesign_under_test
	@echo $(DESIGN_NAME)

create_build_dir: clean
	mkdir build

clean:
	rm -rf build/

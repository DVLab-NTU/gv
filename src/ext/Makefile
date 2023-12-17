CSRCS	= $(wildcard *.cc)
CSOBJS	= $(addsuffix .so, $(basename $(CSRCS)))
CDEPS	= $(addsuffix .d, $(basename $(CSRCS)))

YOSYSCONF	= ../../yosys-config -I../../include --build
YOSYSCONF = ../../yosys-config --exec --cxx --cxxflags -I../../include  --ldflags -o

all:	$(CSOBJS)

%.so:	%.cc 
	@echo "Compiling extension $@"
	@echo "$(YOSYSCONF) $@ $< "
	$(YOSYSCONF) $@ -shared $< --ldlibs 

clean:	
	@rm -f $(CSOBJS)

.SILENT:

.PHONY:	all clean





CSRCS	= $(wildcard *.cc)
CSOBJS	= $(addsuffix .so, $(basename $(CSRCS)))
CDEPS	= $(addsuffix .d, $(basename $(CSRCS)))

YOSYSCONF	= ../../yosys-config -I../../include --build


all:	$(CSOBJS)

%.so:	%.cc 
	@echo "Compiling extension $@"
	@echo "$(YOSYSCONF) $@ $< "
	$(YOSYSCONF) $@ $< 

clean:	
	@rm -f $(CSOBJS)

.SILENT:

.PHONY:	all clean





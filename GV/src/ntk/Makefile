PKGFLAG   =
EXTHDRS   = gvNtkCmd.h

CSRCS	= $(wildcard *.cpp)
CHDRS	= $(wildcard *.h)
COBJS	= $(addsuffix .o, $(basename $(CSRCS)))


INCDIR	= ../../include
DEPDIR	= -I. -I$(INCDIR)
LIBDIR	= ../../lib
LIBDEP	= $(LIBDIR)/lib.d
EXTLINK	= $(PKGNAME).d

CC	= g++
AR	= ar cr

ENGFLGS = -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS

YOSYS_CONFIG=yosys-config
CFLAGS ?= -O
ALL_CFLAGS=-fPIC -I$(INCDIR) $(CFLAGS)

#CFLAGS	= $(ENGFLGS) $(PKGFLAG) -pg -g -Wall -Wno-parentheses
CFLAGS	= $(ENGFLGS) $(PKGFLAG) -O3 -DNDEBUG -Wall -Wno-parentheses -Wno-unused-function
#CFLAGS	= $(ENGFLGS) $(PKGFLAG) -g -Wall -Wno-parentheses

.PHONY: depend extheader

%.o: %.cpp
	@echo "> compiling: $<"
	@$(YOSYS_CONFIG) --exec --cxx --cxxflags --ldflags --ldlibs -c -o  $@ $< $(ALL_CFLAGS)


top:	$(EXTLINK) target
	@echo -n

clean:
	@rm -f $(COBJS)



## Make dependencies
depend: .depend.mak
.depend.mak: $(CSRCS) $(CHDRS)
	@echo Making dependencies ...
	@$(CC) -MM $(DEPDIR) $(CSRCS) > $@

## Linking external headers
extheader: .extheader.mak
.extheader.mak: $(EXTHDRS)
	@echo Linking external header files...
	@rm -f $@
	@echo -n "$(EXTLINK): " > $@
	@for hdr in $(EXTHDRS); \
	do \
		echo -n "$(INCDIR)/$$hdr " >> $@; \
		rm -f $(INCDIR)/$$hdr; \
	done
	@echo >> $@
	@for hdr in $(EXTHDRS); \
	do \
		echo "$(INCDIR)/$$hdr: $$hdr" >> $@; \
		echo "	@rm -f $(INCDIR)/$$hdr" >> $@; \
		echo "	@ln -fs ../src/$(PKGNAME)/$$hdr $(INCDIR)/$$hdr" >> $@; \
	done


include .depend.mak
include .extheader.mak
include ../Makefile.lib
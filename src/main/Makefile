PKGFLAG	= $(DEBUG_FLAG)
EXTHDRS	= 

include ../Makefile.in

TARGET	   = ../../$(EXEC)
TOPPATH    = $(abspath $(firstword $(MAKEFILE_LIST)))
LIBABSPATH = $(patsubst %src/main/Makefile, %lib/,$(TOPPATH))
GVABSPATH  = $(patsubst %src/main/Makefile, %,$(TOPPATH))
LIBABSPATH = $(patsubst %src/main/Makefile, %lib/,$(TOPPATH))
MACLIB     = /opt/homebrew/lib
OS         = $(shell uname -s)

target: $(TARGET)
	
$(TARGET): $(COBJS) $(LIBDEP)
	@echo "> building $(EXEC)..."
#	@echo $(TOPPATH);
#	@echo $(LIBABSPATH);
#	@echo $(GVABSPATH);

ifeq ($(OS), Darwin)
#	echo "This is MacOS"; 
	@$(CC) $(CFLAGS) -I$(INCDIR) $(COBJS) $(LIBPATH) -Wl,-rpath $(LIBABSPATH) -Wl,-rpath $(GVABSPATH) -L$(LIBDIR) -L$(MACLIB) $(EXTLIB)  -lpthread -o $@
else
#	echo "This is Linux";
	@$(CC) $(CFLAGS) -I$(INCDIR) $(COBJS) $(LIBPATH) -Wl,-rpath $(LIBABSPATH) -Wl,-rpath $(GVABSPATH) -L$(LIBDIR) $(EXTLIB)  -lpthread -o $@
endif
	
	@echo ""
	@echo "  Build Successful !!"

#	@$(YOSYSCONF) -o $@  --cxxflags -rdynamic -I$(INCDIR) $(COBJS) -L$(LIBDIR) --ldlibs -lstdc++ $(EXTLIB) -pthread -lyosys
#	@$(CC) $(CFLAGS) -I$(INCDIR) $(COBJS) -L$(LIBDIR) $(EXTLIB) -static -lpthread -o $@
#	@$(CC) $(CFLAGS) -I$(INCDIR) $(COBJS) -L$(LIBDIR) $(EXTLIB) -lpthread -o $@

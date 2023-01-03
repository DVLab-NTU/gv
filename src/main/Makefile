PKGFLAG	=
EXTHDRS	= 

include ../Makefile.in

TARGET	= ../../$(EXEC)

target: $(TARGET)
TOPPATH    = $(abspath $(firstword $(MAKEFILE_LIST)))
LIBABSPATH = $(patsubst %src/main/Makefile, %lib/,$(TOPPATH))

$(TARGET): $(COBJS) $(LIBDEP)
	@echo "> building $(EXEC)..."
	@echo $(TOPPATH);
	@echo $(LIBABSPATH);

	$(CC) $(CFLAGS) -no-pie -I$(INCDIR) $(COBJS) $(LIBPATH) -Wl,-rpath $(LIBABSPATH) -L$(LIBDIR)  $(EXTLIB) -lyosys -lpthread -o $@

#	@$(YOSYSCONF) -o $@  --cxxflags -rdynamic -I$(INCDIR) $(COBJS) -L$(LIBDIR) --ldlibs -lstdc++ $(EXTLIB) -pthread -lyosys
#	@$(CC) $(CFLAGS) -I$(INCDIR) $(COBJS) -L$(LIBDIR) $(EXTLIB) -static -lpthread -o $@
#	@$(CC) $(CFLAGS) -I$(INCDIR) $(COBJS) -L$(LIBDIR) $(EXTLIB) -lpthread -o $@

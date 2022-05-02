CSRCS     = $(wildcard *.cpp)
CHDRS     = $(wildcard *.h)
COBJS     = $(addsuffix .o, $(basename $(CSRCS)))

CXX       = g++
CFLAGS    = -g -Wall
CFLAGS    = -O3 -Wall
EXTINCDIR = 
LIBDIR    = 
ECHO      = /bin/echo

EXEC      = parser random_mc AM


# obj:=main.o a.o b.o c.o
all : 
# $(EXEC): $(COBJS)
# 	@$(ECHO) "> building: $@"
# 	@$(CXX) -o $@ $(CFLAGS) $(COBJS)
	@$(ECHO) "> building: random_mc.cpp"
	@$(CXX) -o random_mc random_mc.cpp
	@$(ECHO) "> building: parser.cpp"
	@$(CXX) -o parser    parser.cpp
	@$(ECHO) "> building: A-to-M.cpp"
	@$(CXX) -o AM        A-to-M.cpp

.PHONY : clean

clean:
	@$(ECHO) "> cleaning $(COBJS) $(EXEC)"
	@rm -f $(COBJS) $(EXEC)

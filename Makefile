SRCPKGS  = util cmd mod abc ntk sim vrf ext
LIBPKGS  = util cmd mod abc ntk sim vrf

MAIN     = main

EXTLIBS	 = -lm -lz -lreadline -ltermcap -ldl -lstdc++ -ltcl -lffi -lgmp
SRCLIBS  = $(addprefix -l, $(LIBPKGS)) $(addprefix -l, $(ENGPKGS))


#ENGPKGS	 += quteRTL
ENGPKGS	 += boolector
ENGPKGS	 += lgl
ENGPKGS	 += btor2parser
#ENGPKGS	 += minisat
ENGPKGS	 += abcc
ENGPKGS	 += yosys

ENGSSRC	 = eng

EXEC     = gv
.PHONY : all debug

all : EXEC     = gv
debug : EXEC     = gv.debug

all:  DEBUG_FLAG =
debug:DEBUG_FLAG = -DGV_DEBUG

LIB	     = libgv.a


all debug:	srcLib
	@echo "Checking $(MAIN)..."
	@cd src/$(MAIN); make --no-print-directory EXTLIB="$(SRCLIBS) $(EXTLIBS)" EXEC=$(EXEC); cd ../.. ;

srcLib:	engLib 
	@cd include; ln -fs ../src/*/*.h ./;
	@for pkg in $(SRCPKGS); \
	do \
		echo "Checking $$pkg..."; \
		cd src/$$pkg; make --no-print-directory DEBUG_FLAG=$(DEBUG_FLAG) PKGNAME=$$pkg || exit;  \
		cd ../.. ; \
	done

engLib:	
	@for pkg in $(ENGPKGS);	\
	do \
		cd include ; ln -fs ../src/$(ENGSSRC)/$$pkg/* .; cd .. ; \
		cd lib ; ln -fs ../src/$(ENGSSRC)/$$pkg/*.a .; ln -fs ../src/$(ENGSSRC)/$$pkg/*.so .; cd .. ; \
	done
	@rm -f include/*.a

main:
	@echo "Checking $(MAIN)..."
	@cd src/$(MAIN); \
		make --no-print-directory EXTLIB="$(SRCLIBS) $(EXTLIBS)" EXEC=$(EXEC);
	@ln -fs bin/$(EXEC) .
#	@strip bin/$(EXEC)

clean:	
	@for pkg in $(SRCPKGS); \
	do \
		echo "Cleaning $$pkg..."; \
		cd src/$$pkg; make --no-print-directory PKGNAME=$$pkg clean; \
		cd ../.. ; \
	done
	@echo "Cleaning $(MAIN)..."
	@cd src/$(MAIN); make --no-print-directory clean
	@echo "Removing $(EXEC)..."
	@rm -f $(EXEC) 

ctags:	
	@rm -f src/tags
	@for pkg in $(SRCPKGS); \
	do \
		echo "Tagging $$pkg..."; \
		cd src; ctags -a $$pkg/*.cpp $$pkg/*.h; cd ..; \
	done
	@echo "Tagging $(MAIN)..."
	cd src; ctags -a $(MAIN)/*.cpp
	@echo "Tagging $(GUI)..."
	cd src; ctags -a $(GUI)/*.cpp

rmdep:	
	@for pkg in $(SRCPKGS); \
	do \
		rm src/$$pkg/.*.mak; \
	done
	@rm src/main/.*.mak;

linux18 linux16 mac:
	@for pkg in $(REFPKGS); \
	do \
	        cd lib; ln -sf lib$$pkg-$@.a lib$$pkg.a; cd ../..; \
	done
	@cd ref; ln -sf $(EXEC)-$@ $(EXEC);

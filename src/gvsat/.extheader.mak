gvsat.d: ../../include/gvSat.h 
../../include/gvSat.h: gvSat.h
	@rm -f ../../include/gvSat.h
	@ln -fs ../src/gvsat/gvSat.h ../../include/gvSat.h

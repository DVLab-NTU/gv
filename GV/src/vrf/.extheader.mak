vrf.d: ../../include/gvVrfCmd.h 
../../include/gvVrfCmd.h: gvVrfCmd.h
	@rm -f ../../include/gvVrfCmd.h
	@ln -fs ../src/vrf/gvVrfCmd.h ../../include/gvVrfCmd.h

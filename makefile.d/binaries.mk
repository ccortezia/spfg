
ifeq ($(BUILD_STATIC),y)
	LIBRARIES += $(STAGE_LIB)/libspfg.a
endif

ifeq ($(BUILD_SHARED),y)
	LIBRARIES += $(STAGE_LIB)/libspfg.so
endif

$(STAGE_LIB)/libspfg.so: dirs
	(make -C src libspfg.so)
	(cp src/libspfg.so ${STAGE_LIB})

$(STAGE_LIB)/libspfg.a: dirs
	(make -C src libspfg.a)
	(cp src/libspfg.a ${STAGE_LIB})

$(STAGE_INC)/spfg: dirs
	(cp -r include/spfg ${STAGE_INC})


.PHONY: debug

debug:
	DYLD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} LD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} gdb -tui ${STAGE_BIN}/$(TEST_EXE)

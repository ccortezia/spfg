
ifeq ($(ARCH),wasm)
	EXTRA_CFLAGS += -s WASM=1
	LIBRARIES += $(STAGE_LIB)/libspfg.wasm
	EXECUTABLES = $(STAGE_BIN)/test_spfg.html
endif

$(STAGE_BIN)/test_spfg.html: unity $(HEADERS) $(LIBRARIES)
	(make -C tests test_spfg.html)
	(cp tests/test_spfg.js ${STAGE_BIN})
	(cp tests/test_spfg.wasm ${STAGE_BIN})
	(cp tests/test_spfg.html ${STAGE_BIN})

$(STAGE_LIB)/libspfg.wasm: $(STAGE_LIB)/libspfg.wast
	(cp src/libspfg.wasm ${STAGE_LIB})

$(STAGE_LIB)/libspfg.wast: $(STAGE_LIB)/libspfg.js
	(cp src/libspfg.wast ${STAGE_LIB})

$(STAGE_LIB)/libspfg.js: $(STAGE_LIB)/libspfg.bc
	(make -C src libspfg.js)
	(cp src/libspfg.js ${STAGE_LIB})

$(STAGE_LIB)/libspfg.bc: dirs
	(make -C src libspfg.bc)
	(cp src/libspfg.bc ${STAGE_LIB})

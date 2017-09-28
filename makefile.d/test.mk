
TEST_EXE = test_spfg

$(STAGE_BIN)/${TEST_EXE}: unity $(HEADERS) $(LIBRARIES)
	(make -C tests ${TEST_EXE})
	(cp tests/${TEST_EXE} ${STAGE_BIN})


.PHONY: test unity

unity: dirs
	(mkdir -p ${STAGE_INC}/unity)
	(cp lib/Unity/src/*.h ${STAGE_INC}/unity)
	(cp lib/Unity/extras/fixture/src/*.h ${STAGE_INC}/unity)

test:
	@echo ========= Running Test Cases ==============
	DYLD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} LD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} ${STAGE_BIN}/${TEST_EXE} ${ARGS}


$(STAGE_BIN)/test: unity $(HEADERS) $(LIBRARIES)
	(make -C tests test)
	(cp tests/test ${STAGE_BIN})


.PHONY: test unity

unity: dirs
	(mkdir -p ${STAGE_INC}/unity)
	(cp lib/Unity/src/*.h ${STAGE_INC}/unity)
	(cp lib/Unity/extras/fixture/src/*.h ${STAGE_INC}/unity)

test:
	@echo ========= Running Test Cases ==============
	DYLD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} LD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} ${STAGE_BIN}/test ${ARGS}

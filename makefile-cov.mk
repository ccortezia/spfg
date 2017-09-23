
ifeq ($(BUILDCOV),y)
	EXTRA_CFLAGS += --coverage
	EXTRA_LDFLAGS += --coverage
endif

ifeq ($(BUILDPROF),y)
	EXTRA_CFLAGS += -pg
	EXTRA_LDFLAGS += -pg
endif

gcov: src/*.gcno src/*.gcda tests/*.gcno tests/*.gcno
	(make -C src gcov)
	(make -C tests gcov)

htmlcov: src/*.gcno src/*.gcda tests/*.gcno tests/*.gcno
	(make -C src spfg.info)
	(make -C tests test.info)
	(genhtml -o htmlcov src/spfg.info tests/test.info)


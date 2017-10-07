
gcov: src/*.gcno src/*.gcda tests/*.gcno tests/*.gcno
	(make -C src gcov)
	(make -C tests gcov)

src/spfg.info:
	(make -C src spfg.info)

tests/$(TEST_EXE).info:
	(make -C tests $(TEST_EXE).info)

htmlcov: src/spfg.info tests/$(TEST_EXE).info
	(genhtml -o htmlcov $^)


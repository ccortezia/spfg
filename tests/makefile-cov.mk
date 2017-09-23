COVERAGE = $(NAME).info

GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(wildcard *.gcov)

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${TARGET}" -o ${COVERAGE} -c -d . --no-external)
	(gcov *.c)

.PHONY: cov_clean
cov_clean:
	$(RM) $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE)

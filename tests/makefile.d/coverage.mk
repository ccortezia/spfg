COVERAGE = $(NAME).info

GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(wildcard *.gcov)
ARTIFACTS += $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE)

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${TARGET}" -o ${COVERAGE} -c -d . --no-external)
	(gcov *.c)

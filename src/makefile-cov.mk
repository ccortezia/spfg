COVERAGE = $(NAME).info

GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(shell find . -name *.gcov)

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${SHARED_LIB}" -o ${COVERAGE} -c -d .)
	(gcov *.c)

.PHONY: cov_clean
cov_clean:
	$(RM) $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE)

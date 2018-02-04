COVERAGE = $(NAME).info

GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(wildcard *.gcov)
ARTIFACTS += $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE)

ifeq ($(BUILDCOV),y)
	override EXTRA_CFLAGS += --coverage
	override EXTRA_LDFLAGS += --coverage
endif

ifeq ($(BUILDPROF),y)
	override EXTRA_CFLAGS += -pg
	override EXTRA_LDFLAGS += -pg
endif

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${SHARED_LIB}" -o ${COVERAGE} -c -d .)
	(gcov *.c)

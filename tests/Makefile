NAME = test
NATIVE_EXE = $(NAME)
WASM_RUNNER = $(NAME).html
COVERAGE = $(NAME).info

CC = $(CROSS_COMPILE)gcc
LINK = $(CROSS_COMPILE)gcc
WFLAGS = -s WASM=1 -s EXPORT_ALL=1
CFLAGS = -I${ROOT_PATH}/${STAGE_INC} -I${ROOT_PATH}/${STAGE_INC}/unity -g3 -O0 -std=c99 $(EXTRA_CFLAGS)
LDLIBS = -lspfg
LDFLAGS = -g3 -L${ROOT_PATH}/${STAGE_LIB} $(EXTRA_LDFLAGS)
EXTRA_SOURCES = ${ROOT_PATH}/lib/Unity/src/unity.c ${ROOT_PATH}/lib/Unity/extras/fixture/src/unity_fixture.c

SOURCES = $(shell find . -name '*.c') $(EXTRA_SOURCES)
OBJECTS = $(SOURCES:%.c=%.o)
GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(shell find . -name '*.gcov')
WASMS = $(SOURCES:%.c=%.wasm)
WASTS = $(SOURCES:%.c=%.wast)
HTMLS = $(SOURCES:%.c=%.html)
JSS = $(SOURCES:%.c=%.js)


$(NATIVE_EXE): $(OBJECTS)
	$(CC) $^ ${LDFLAGS} ${LDLIBS} -o $@

$(WASM_RUNNER): $(SOURCES)
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^ ${ROOT_PATH}/${STAGE_LIB}/libspfg.bc --shell-file skel.html

$(OBJECTS): $(SOURCES)

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${TARGET}" -o ${COVERAGE} -c -d . --no-external)
	(gcov *.c)

.PHONY: clean

clean:
	$(RM) $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE) $(OBJECTS) $(TARGET) $(WASMS) $(WASTS) $(HTMLS) $(JSS)

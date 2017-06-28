NAME = spfg
JS_LIB = lib$(NAME).js
BC_LIB = lib$(NAME).bc
SHARED_LIB = lib$(NAME).so
STATIC_LIB = lib$(NAME).a
COVERAGE = $(NAME).info

ifeq ($(BUILD_SHARED),y)
	EXTRA_CFLAGS += -fPIC
	EXTRA_LDFLAGS += -shared
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
WFLAGS = -s WASM=1 -s EXPORTED_FUNCTIONS="['_spfg_init', '_spfg_finish', '_spfg_dp_create', '_spfg_gr_create', '_spfg_fn_create', '_spfg_run_cycle', '_spfg_reset_cycle', '_spfg_gr_export_schema']"
CFLAGS = -I${ROOT_PATH}/include -Wall -g3 -O0 -std=gnu99 $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_LDFLAGS)
ARFLAGS = rcs

SOURCES = $(shell find . -name *.c)
OBJECTS = $(SOURCES:%.c=%.o)
GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(shell find . -name *.gcov)

WASMS = *.wasm
WASTS = *.wast
BCS = *.bc
JSS = *.js
TMP_MAIN = _main.c


all: $(SHARED_LIB) $(STATIC_LIB)

$(JS_LIB): $(SOURCES)
	echo '#include "spfg/spfg.h"' > ${TMP_MAIN}
	echo 'int main(void){spfg_init(); spfg_finish(); spfg_dp_create(0, 0, 0, 0); spfg_gr_create(0, 0); spfg_fn_create(0, 0, 0, 0, 0, 0, 0, 0, 0); spfg_run_cycle(0, 0, 0, 0); spfg_reset_cycle(0); spfg_gr_export_schema(0, 0, 0);}' >> ${TMP_MAIN}
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^ ${TMP_MAIN}
	rm -f ${TMP_MAIN}

$(BC_LIB): $(SOURCES)
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^

$(STATIC_LIB): $(OBJECTS)
	$(AR) ${ARFLAGS} $@ $^

$(SHARED_LIB): $(OBJECTS)
	$(CC) ${LDFLAGS} -o $@ $^

$(OBJECTS): $(SOURCES)

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${SHARED_LIB}" -o ${COVERAGE} -c -d .)
	(gcov *.c)


.PHONY: clean

clean:
	$(RM) $(TMP_MAIN) $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE) $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB) $(JSS) $(BCS) $(WASMS) $(WASTS)

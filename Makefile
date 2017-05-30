export ARCH ?= x86_64
export CROSS_COMPILE ?=
export PREFIX ?= build
export EXTRA_CFLAGS ?=
export EXTRA_LDFLAGS ?=
export BUILDCOV ?=
export BUILDPROF ?=
export ROOT_PATH = $(CURDIR)
export BUILD_SHARED ?= y
export CI_TESTS ?= y

# ----

# Support constants.
export ARDUINO_ARCHS = atmega328 atmega168

# ----

STAGEBASE = build
STAGE = $(STAGEBASE)/$(ARCH)
export STAGE_BIN = $(STAGE)/bin
export STAGE_LIB = $(STAGE)/lib
export STAGE_INC = $(STAGE)/include

# ----

LIBRARIES = $(STAGE_LIB)/libspfg.a

ifeq ($(BUILD_SHARED),y)
	LIBRARIES += $(STAGE_LIB)/libspfg.so
	EXTRA_CFLAGS += -fPIC
endif

# ----

# Set default CROSS_COMPILER for arm architecture.
ifeq ($(ARCH),arm)
	CROSS_COMPILE = arm-linux-gnueabihf-
endif

# Set default CROSS_COMPILER for arduino architectures.
ifneq ($(filter $(ARCH),$(ARDUINO_ARCHS)),)
	CROSS_COMPILE = avr-
endif

# ----

ifeq ($(BUILDCOV),y)
	EXTRA_CFLAGS += --coverage
	EXTRA_LDFLAGS += --coverage
endif

ifeq ($(BUILDPROF),y)
	EXTRA_CFLAGS += -pg
	EXTRA_LDFLAGS += -pg
endif

ifeq ($(CROSS_COMPILE),avr-)
	EXTRA_CFLAGS += -mmcu=${ARCH}
	EXTRA_LDFLAGS += -mmcu=${ARCH}
endif

# ----


all: $(STAGE_BIN)/test


$(STAGE_BIN)/test: unity $(STAGE_INC)/spfg $(LIBRARIES)
	(make -C tests)
	(cp tests/test ${STAGE_BIN})

$(STAGE_LIB)/libspfg.so: dirs
	(make -C src libspfg.so)
	(cp src/libspfg.so ${STAGE_LIB})

$(STAGE_LIB)/libspfg.a: dirs
	(make -C src libspfg.a)
	(cp src/libspfg.a ${STAGE_LIB})

$(STAGE_INC)/spfg: dirs
	(cp -r include/spfg ${STAGE_INC})

gcov: src/*.gcno src/*.gcda tests/*.gcno tests/*.gcno
	(make -C src gcov)
	(make -C tests gcov)

htmlcov: src/*.gcno src/*.gcda tests/*.gcno tests/*.gcno
	(make -C src spfg.info)
	(make -C tests test.info)
	(genhtml -o htmlcov src/spfg.info tests/test.info)

htmldoc:
	(doxygen)

# ----

.PHONY: all dirs sclean clean test devel unity

unity: dirs
	(mkdir -p ${STAGE_INC}/unity)
	(cp lib/Unity/src/*.h ${STAGE_INC}/unity)
	(cp lib/Unity/extras/fixture/src/*.h ${STAGE_INC}/unity)


test:
	DYLD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} LD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} ${STAGE_BIN}/test

debug:
	DYLD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} LD_LIBRARY_PATH=${ROOT_PATH}/${STAGE_LIB} gdb -tui ${STAGE_BIN}/test

devel:
	(BUILDCOV=y make)
	(make test)
	(make -C src clean)
	(make -C tests clean)

ci:
	(make clean)
	(BUILDCOV=y make)
	if test "${CI_TESTS}" = "y"; then make test && make htmlcov && make htmldoc; fi

dirs:
	(mkdir -p ${STAGE_LIB})
	(mkdir -p ${STAGE_BIN})
	(mkdir -p ${STAGE_INC})

sclean:
	(cd src; make clean)
	(cd tests; make clean)

clean:
	(make sclean)
	(rm -rf ${STAGE} htmlcov htmldoc)

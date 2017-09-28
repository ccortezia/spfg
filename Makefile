export ARCH ?= x86_64
export CROSS_COMPILE ?=
export PREFIX ?= build
export EXTRA_CFLAGS ?=
export EXTRA_LDFLAGS ?=
export BUILDCOV ?=
export BUILDPROF ?=
export ROOT_PATH = $(CURDIR)
export BUILD_SHARED ?= y
export BUILD_STATIC ?= y

# ----

STAGEBASE ?= build
STAGE ?= $(STAGEBASE)/$(ARCH)
export STAGE_BIN ?= $(STAGE)/bin
export STAGE_LIB ?= $(STAGE)/lib
export STAGE_INC ?= $(STAGE)/include

# ----

LIBRARIES ?=
EXECUTABLES ?= $(STAGE_BIN)/$(TEST_EXE)
HEADERS ?= $(STAGE_INC)/spfg

# ----

all:
include makefile.d/arch-arm.mk
include makefile.d/arch-arduino.mk
include makefile.d/arch-wasm.mk
include makefile.d/binaries.mk
include makefile.d/test.mk
include makefile.d/coverage.mk
include makefile.d/contrib.mk
all: $(EXECUTABLES)

htmldoc:
	(doxygen)

# ----

.PHONY: all dirs sclean clean test devel

devel:
	(BUILDCOV=y make)
	(make test)
	(make sclean)

ci:
	(make clean)
	(BUILDCOV=y make && make test && make htmlcov)
	(make htmldoc)

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

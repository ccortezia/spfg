NAME = spfg

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar

SOURCES = $(wildcard *.c) ${ROOT_PATH}/lib/azjson/src/azjson.c
PRIVATE = spfg_utils.c spfg_index.c spfg_eval.c spfg_build.c
ARTIFACTS =

include makefile.d/binaries.mk
include makefile.d/coverage.mk
include makefile.d/arch-wasm.mk

.PHONY: clean

clean:
	$(RM) $(ARTIFACTS)

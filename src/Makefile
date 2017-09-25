NAME = spfg

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar

SOURCES = $(wildcard *.c) ${ROOT_PATH}/lib/azjson/src/azjson.c
PRIVATE = spfg_utils.c spfg_eval.c spfg_build.c
ARTIFACTS =

include makefile-native.mk
include makefile-wasm.mk
include makefile-cov.mk

.PHONY: clean

clean:
	$(RM) $(ARTIFACTS)

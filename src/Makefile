NAME = spfg

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar

SOURCES = $(wildcard *.c) ${ROOT_PATH}/lib/azjson/src/azjson.c
PRIVATE = spfg_utils.c spfg_eval.c spfg_build.c

include makefile-native.mk
include makefile-wasm.mk
include makefile-cov.mk

.PHONY: clean
clean: native_clean cov_clean wasm_clean

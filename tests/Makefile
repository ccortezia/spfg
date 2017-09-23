NAME = test

CC = $(CROSS_COMPILE)gcc
LINK = $(CROSS_COMPILE)gcc

SOURCES = $(wildcard *.c) \
	${ROOT_PATH}/lib/Unity/src/unity.c \
	${ROOT_PATH}/lib/Unity/extras/fixture/src/unity_fixture.c

include makefile-native.mk
include makefile-wasm.mk
include makefile-cov.mk

.PHONY: clean
clean: native_clean wasm_clean cov_clean

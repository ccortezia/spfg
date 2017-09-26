NAME = test_spfg

CC = $(CROSS_COMPILE)gcc
LINK = $(CROSS_COMPILE)gcc

SOURCES = $(wildcard *.c) \
	${ROOT_PATH}/lib/Unity/src/unity.c \
	${ROOT_PATH}/lib/Unity/extras/fixture/src/unity_fixture.c

ARTIFACTS =

all:

include makefile-native.mk
include makefile-wasm.mk
include makefile-cov.mk

.PHONY: clean
clean:
	$(RM) $(ARTIFACTS)

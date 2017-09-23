
# Set default CROSS_COMPILER for arm architecture.
ifeq ($(ARCH),arm)
	CROSS_COMPILE = arm-linux-gnueabihf-
endif

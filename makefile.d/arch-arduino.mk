
# Support constants.
export ARDUINO_ARCHS = atmega328 atmega168


# Set default CROSS_COMPILER for arduino architectures.
ifneq ($(filter $(ARCH),$(ARDUINO_ARCHS)),)
	CROSS_COMPILE = avr-
endif


ifeq ($(CROSS_COMPILE),avr-)
	EXTRA_CFLAGS += -mmcu=${ARCH}
	EXTRA_LDFLAGS += -mmcu=${ARCH}
	BUILD_SHARED = no
endif

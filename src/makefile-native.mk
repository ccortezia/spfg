SHARED_LIB = lib$(NAME).so
STATIC_LIB = lib$(NAME).a

CFLAGS ?= -I${ROOT_PATH}/include -I${ROOT_PATH}/lib/azjson/include -Wall -g3 -O0 -std=gnu99
ARFLAGS ?= rcs
LDFLAGS ?=

CFLAGS += $(EXTRA_CFLAGS)
ARFLAGS += $(EXTRA_ARFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)
OBJECTS = $(SOURCES:%.c=%.o)

ifeq ($(BUILD_SHARED),y)
	EXTRA_CFLAGS += -fPIC
	EXTRA_LDFLAGS += -shared
endif


$(STATIC_LIB): $(OBJECTS)
	$(AR) ${ARFLAGS} $@ $^

$(SHARED_LIB): $(OBJECTS)
	$(CC) ${LDFLAGS} -o $@ $^


.PHONY: native_clean
native_clean:
	$(RM) $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB)

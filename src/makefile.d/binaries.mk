SHARED_LIB = lib$(NAME).so
STATIC_LIB = lib$(NAME).a

CFLAGS ?= -I${ROOT_PATH}/include -I${ROOT_PATH}/src -I${ROOT_PATH}/lib/azjson/include -Wall -g3 -O0 -std=gnu99
ARFLAGS ?= rcs
LDFLAGS ?=

CFLAGS += $(EXTRA_CFLAGS)
ARFLAGS += $(EXTRA_ARFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)
OBJECTS = $(SOURCES:%.c=%.o)
ARTIFACTS += $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB)

ifeq ($(BUILD_SHARED),y)
	override EXTRA_CFLAGS += -fPIC
	override EXTRA_LDFLAGS += -shared
endif

all: $(SHARED_LIB) $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	$(AR) ${ARFLAGS} $@ $^

$(SHARED_LIB): $(OBJECTS)
	$(CC) ${LDFLAGS} -o $@ $^

# Prevent symbols from these files from being
# exposed by the resulting linked shared library.
$(PRIVATE:%.c=%.o): $(PRIVATE)
	$(CC) $(CFLAGS) -fvisibility=hidden -c $^

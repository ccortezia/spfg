NAME = spfg
SHARED_LIB = lib$(NAME).so
STATIC_LIB = lib$(NAME).a
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CFLAGS = -I${ROOT_PATH}/include -Wall -g3 -O0 -std=gnu99 $(EXTRA_CFLAGS)
LDFLAGS = -shared $(EXTRA_LDFLAGS)
ARFLAGS = rcs
SOURCES = $(shell find . -name *.c)
OBJECTS = $(SOURCES:%.c=%.o)
GCNOS = $(SOURCES:%.c=%.gcno)
GCDAS = $(SOURCES:%.c=%.gcda)
GCOVS = $(shell find . -name *.gcov)
COVERAGE = $(NAME).info

all: $(SHARED_LIB) $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	$(AR) ${ARFLAGS} $@ $^

$(SHARED_LIB): $(OBJECTS)
	$(CC) ${LDFLAGS} -o $@ $^

$(OBJECTS): $(SOURCES)

$(COVERAGE): $(GCNOS) $(GCDAS)
	(lcov -t "Coverage on ${SHARED_LIB}" -o ${COVERAGE} -c -d .)
	(gcov *.c)


.PHONY: clean

clean:
	$(RM) $(GCNOS) $(GCDAS) $(GCOVS) $(COVERAGE) $(OBJECTS) $(SHARED_LIB) $(STATIC_LIB)

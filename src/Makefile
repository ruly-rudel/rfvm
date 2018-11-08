
CC=gcc

LD=$(CC)
AR=ar

COMOPT=-DNOGC
NOOPTIMIZE=-O0 -DNO_INLINE
OPTIMIZE=-O3
PROFILE=-O3 -DNO_INLINE

DEBUG_OPT=-g3 $(NOOPTIMIZE)
RELEASE_OPT=$(OPTIMIZE) -DNDEBUG
COV_OPT=-coverage $(OPTIMIZE) -DNDEBUG
TEST_OPT=$(OPTIMIZE)

#ARCH=-m32
CFLAGS=$(ARCH) $(COMOPT) -Wall $(INCPATHS)
LDFLAGS=$(ARCH) $(LIBPATHS) -L.

LIBSOURCES=allocator.c dict.c rfvm.c
LIBOBJS=$(LIBSOURCES:%.c=%.o)

TARGET=rfvm

.PHONY:	all clean release debug cov

.SUFFIXES: .c .o

release:	OPT=$(RELEASE_OPT)
release:	all

debug:	OPT=$(DEBUG_OPT)
debug:	all

cov:	OPT=$(COV_OPT)
cov:	$(TARGET)
	./rudel ../tests/cov.rud
	gcov *.gcda

all:
all: $(TARGET)

.deps: *.c *.h
	$(CC) $(CFLAGS) -MM *.c > .deps

$(TARGET): %: main.o librfvm.a
	$(LD) $^ -o $@ $(OPT) $(LDFLAGS)

librfvm.a: $(LIBOBJS)
	$(AR) rcs $@ $^

test:	OPT=$(TEST_OPT)
test:	$(TARGET)
	../scr/runtest.py --test-timeout 180 ../tests/tests.rud ./$(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(OPT) -c $< -o $@

clean:
	rm -rf *.o $(TARGET) librfvm.a .deps tags ../tags gmon.out *.gcno *.gcda *.gcov tags

-include .deps


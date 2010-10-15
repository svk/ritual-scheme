BISONFLAGS=

CC=gcc
CFLAGS=-D_XOPEN_SOURCE=500 -std=c99 -Wall -g -IC:/Lib/GMP5/include -LC:/Lib/GMP5/Lib
CCSYSTEM=$(shell $(CC) -dumpmachine)

ifeq ($(CCSYSTEM), mingw32)
LIB_PTHREAD=-lpthreadGC2
else
LIB_PTHREAD=-lpthread
endif

all: test_repl test_hash_table

reflex:
	rm -rf lex.ritual-r5rs.c ritual-r5rs.tab.c ritual-r5rs.tab.h lex.ritual-r5rs.h

veryclean: clean reflex

clean:
	rm -rf test_hash_table
	rm -f *.o
	rm -f *.exe

test_ump: ritual_memory.o test_ump.o
	gcc $(CFLAGS) $^ $(LIB_PTHREAD) -o $@

test_repl: lex.ritual-r5rs.o ritual-r5rs.tab.o parse_context.o test_repl.o ritual_generic.o ritual_basic_types.o ritual_object.o ritual_flo.o ritual_gc.o ritual_instance.o ritual_error.o ritual_env.o ritual_hash_table.o lookup3.o ritual_eval.o ritual_native_proc.o ritual_lambda.o ritual_keyword.o ritual_bignum.o ritual_arithmetic.o ritual_basic_procs.o
	gcc $(CFLAGS) $^ $(LIB_PTHREAD) -lgmp -o $@

test_repl.o: test_repl.c lex.ritual-r5rs.c ritual-r5rs.tab.c

lex.ritual-r5rs.c: ritual-r5rs.l ritual-r5rs.tab.c
	flex --header-file=lex.ritual-r5rs.h -t ritual-r5rs.l > $@

ritual-r5rs.tab.c: ritual-r5rs.y
	bison $(BISONFLAGS) -d $^

test_hash_table: test_hash_table.o ritual_hash_table.o lookup3.o
	gcc $(CFLAGS) -g $^ $(LIB_PTHREAD) -o $@

.PHONY: all reflex veryclean clean

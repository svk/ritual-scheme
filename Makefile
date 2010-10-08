BISONFLAGS=

CC=gcc
CFLAGS=-D_XOPEN_SOURCE=500 -std=c99 -Wall -g
CCSYSTEM=$(shell $(CC) -dumpmachine)

ifeq ($(CCSYSTEM), mingw32)
LIB_PTHREAD=-lpthreadGC2
else
LIB_PTHREAD=-lpthread
endif

all: test_parsing test_hash_table

reflex:
	rm -rf lex.ritual-r5rs.c ritual-r5rs.tab.c ritual-r5rs.tab.h lex.ritual-r5rs.h

veryclean: clean reflex

clean:
	rm -rf test_hash_table
	rm -f *.o

test_parsing: lex.ritual-r5rs.o ritual-r5rs.tab.o parse_context.o test_parsing.o
	gcc $(CFLAGS) $^ -o $@

test_parsing.o: test_parsing.c lex.ritual-r5rs.c ritual-r5rs.tab.c

lex.ritual-r5rs.c: ritual-r5rs.l ritual-r5rs.tab.c
	flex --header-file=lex.ritual-r5rs.h -t ritual-r5rs.l > $@

ritual-r5rs.tab.c: ritual-r5rs.y
	bison $(BISONFLAGS) -d $^

test_hash_table: test_hash_table.o ritual_hash_table.o lookup3.o
	gcc $(CFLAGS) -g $^ $(LIB_PTHREAD) -o $@

.PHONY: all reflex veryclean clean

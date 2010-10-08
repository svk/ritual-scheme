BISONFLAGS=

CC=gcc
CFLAGS=-D_XOPEN_SOURCE=500 -std=c99 -Wall -g
CCSYSTEM=$(shell $(CC) -dumpmachine)

ifeq ($(CCSYSTEM), mingw32)
LIB_PTHREAD=-lpthreadGC2
else
LIB_PTHREAD=-lpthread
endif

all: ritual-r5rs test_hash_table

clean:
	rm -rf lex.ritual-r5rs.c ritual-r5rs.tab.c ritual-r5rs ritual-r5rs.tab.h
	rm -rf test_hash_table
	rm -f *.o

ritual-r5rs: lex.ritual-r5rs.c ritual-r5rs.tab.c parse_context.c
	gcc -g $^ -o $@

lex.ritual-r5rs.c: ritual-r5rs.l
	flex -t $^ > $@

ritual-r5rs.tab.c: ritual-r5rs.y
	bison $(BISONFLAGS) -d $^

test_hash_table: test_hash_table.c ritual_hash_table.c lookup3.c
	gcc $(CFLAGS) -g $^ $(LIB_PTHREAD) -o $@

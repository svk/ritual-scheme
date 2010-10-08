BISONFLAGS=

all: ritual-r5rs

clean:
	rm -rf lex.ritual-r5rs.c ritual-r5rs.tab.c ritual-r5rs ritual-r5rs.tab.h

ritual-r5rs: lex.ritual-r5rs.c ritual-r5rs.tab.c parse_context.c
	gcc -g $^ -o $@

lex.ritual-r5rs.c: ritual-r5rs.l
	flex -t $^ > $@

ritual-r5rs.tab.c: ritual-r5rs.y
	bison $(BISONFLAGS) -d $^

test_hash_table: test_hash_table.c ritual_hash_table.c lookup3.c
	gcc -std=c99 -Wall -g $^ -lpthreadGC2 -o $@

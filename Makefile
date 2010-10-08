BISONFLAGS=

clean:
	rm -rf lex.ritual-r5rs.c ritual-r5rs.tab.c ritual-r5rs ritual-r5rs.tab.h

ritual-r5rs: lex.ritual-r5rs.c ritual-r5rs.tab.c parsectx.c
	gcc -g $^ -o $@

lex.ritual-r5rs.c: ritual-r5rs.l
	flex -t $^ > $@

ritual-r5rs.tab.c: ritual-r5rs.y
	bison $(BISONFLAGS) -d $^

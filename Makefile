alpha: lex.alpha.c alpha.tab.c
	gcc -g $^ -o $@

lex.alpha.c: alpha.l
	flex -t $^ > $@

alpha.tab.c: alpha.y
	bison -d $^

clean:
	rm -rf lex.alpha.c alpha.tab.c alpha alpha.tab.h
	rm -rf lex.ritual-r5rs.c ritual-r5rs.tab.c ritual-r5rs ritual-r5rs.tab.h

ritual-r5rs: lex.ritual-r5rs.c ritual-r5rs.tab.c
	gcc -g $^ -o $@

lex.ritual-r5rs.c: ritual-r5rs.l
	flex -t $^ > $@

ritual-r5rs.tab.c: ritual-r5rs.y
	bison -d $^

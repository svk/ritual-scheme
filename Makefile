alpha: lex.yy.c alpha.tab.c
	gcc -g $^ -o $@

lex.yy.c: alpha.l
	flex $^

alpha.tab.c: alpha.y
	bison -d $^

clean:
	rm -rf lex.yy.c alpha.tab.c alpha alpha.tab.h

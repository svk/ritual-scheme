%define api.pure
%parse-param { struct parse_ctx* ctx }
%lex-param { void* scanner }

%{
#include <stdio.h>
#include <string.h>

#define YYLEX_PARAM ((struct parse_ctx*)ctx)->scanner

struct parse_ctx {
    void* scanner;
};

void yyerror( struct parse_ctx *ctx, const char *str ) {
    fprintf( stderr, "error: %s\n", str );
}

int main(int argc, char *argv[]) {
    struct parse_ctx my;
    yylex_init( &my.scanner );
    yyparse( &my );
    yylex_destroy( my.scanner );
    return 0;
}

%}

%token NUMBER
%%

numbers:
    | numbers number
    ;

number:
    NUMBER
    {
        printf( "encountered number %d\n", $1 );
    }
    ;

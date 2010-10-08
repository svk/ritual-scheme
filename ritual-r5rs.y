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

%token HASH_LPAREN COMMA_AT

%union
{
    int integer;
    char *string;
}
%token <integer> CHARACTER
%token <string> NUMBER
%token <string> STRING
%token <integer> BOOLEAN
%token <string> IDENTIFIER
%%

numbers:
    | numbers number
    ;

number:
    NUMBER {
        printf( "number: %s\n", $1 );
    }
    | BOOLEAN {
        printf( "boolean: %s\n", ($1) ? "#t" : "#f" );
    }
    
    ;

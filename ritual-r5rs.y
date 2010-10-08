%define api.pure
%parse-param { struct parse_ctx* ctx }
%lex-param { void* scanner }

%{
#include <stdio.h>
#include <string.h>
#include "parse_context.h"

#define YYLEX_PARAM ((struct parse_ctx*)ctx)->scanner

void yyerror( struct parse_ctx *ctx, const char *str ) {
    fprintf( stderr, "error: %s\n", str );
}

int main(int argc, char *argv[]) {
    struct parse_context my;
    pctx_init( &my );
    yylex_init( &my.scanner );
    yyset_extra( &my, my.scanner );
    yy_scan_string( "how many roads must a man walk down? (42)", my.scanner );
    yyparse( &my );
    yylex_destroy( my.scanner );
    pctx_destroy( &my );
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

tokens:
    | tokens token
    ;

token:
    NUMBER {
        printf( "number: %s\n", $1 );
    }
    | IDENTIFIER {
        printf( "identifier: %s\n", $1 );
    }
    | STRING {
        printf( "string: %s\n", $1 );
    }
    | CHARACTER {
        printf( "character: %d (%c)\n", ($1), (char) $1);
    }
    | BOOLEAN {
        printf( "boolean: %s\n", ($1) ? "#t" : "#f" );
    }
    | '(' {
        printf( "(\n" );
    }
    | HASH_LPAREN {
        printf( "#(\n" );
    }
    | ')' {
        printf( ")\n" );
    }

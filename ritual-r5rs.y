%define api.pure
%parse-param { struct parse_context* ctx }
%lex-param { void* scanner }

%code requires {
#include "ritual_object.h"
#define YYSTYPE ritual_object_t*
}

%{
#include <stdio.h>
#include <string.h>
#include "parse_context.h"
#include "ritual_basic_types.h"


#define YYLEX_PARAM ((struct parse_context*)ctx)->scanner

void yyerror( struct parse_context *ctx, const char *str ) {
    fprintf( stderr, "error: %s\n", str );
}

%}

%token HASH_LPAREN COMMA_AT CHARACTER NUMBER STRING BOOLEAN IDENTIFIER
%%

tokens:
    | tokens token { pctx_push_back( ctx, $2 ); }

improper_list: '(' rest_of_improper_list { $$ = $2; }

quoted: '\'' token { $$ = (ritual_object_t*) ritual_quote_create( ctx->instance, $2 ); }

rest_of_improper_list:
    ')' { $$ = 0; }
    | '.' token ')' { $$ = $2; }
    | token rest_of_improper_list { $$ = (ritual_object_t*) ritual_pair_create( ctx->instance, $1, $2 ); }

token:
    improper_list
    | quoted
    | NUMBER
    | IDENTIFIER
    | STRING
    | CHARACTER
    | BOOLEAN

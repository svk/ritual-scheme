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
    | tokens token
    ;

list: '(' rest_of_list { $$ = $2; }

rest_of_list:
    ')' { $$ = 0; }
    | token rest_of_list { $$ = (ritual_object_t*) ritual_pair_create( $1, $2 ); }

token:
    list {
        struct rflo_filehandle *fhf = rflo_filehandle_create( stdout );
        ritual_print( &fhf->flo, $1 );
        printf( "\n" );
        rflo_filehandle_destroy( fhf );
    }
    | NUMBER
    | IDENTIFIER
    | STRING
    | CHARACTER
    | BOOLEAN

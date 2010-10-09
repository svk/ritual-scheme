%option reentrant bison-bridge
%option noyywrap
%option case-insensitive

%{
#include <stdio.h>
#include "ritual-r5rs.tab.h"
#include "parse_context.h"
#include "ritual_basic_types.h"

#define INST (((struct parse_context*)yyextra)->instance)
%}

letter [a-z]
initial [a-z\!\$\%\&\*\/\:\<\=\>\?\^\_\~]
subsequents [a-z\!\$\%\&\*\/\:\<\=\>\?\^\_\~0-9\+\-\.\@]*

%x QUOTED_STRING

%%
[ \n]           ;
;[^\n]*         ; /* comment */
[0-9]+          { *yylval_param = (ritual_object_t*) ritual_native_int_create( INST, atoi( yytext ) );
                  return NUMBER; }
{initial}{subsequents}  {
                  *yylval_param = (ritual_object_t*) ritual_symbol_create( INST, yytext );
                  return IDENTIFIER; }
"+"             { *yylval_param = (ritual_object_t*) ritual_symbol_create( INST,"+");
                  return IDENTIFIER; }
"-"             { *yylval_param = (ritual_object_t*) ritual_symbol_create( INST,"-");
                  return IDENTIFIER; }
"..."           { *yylval_param = (ritual_object_t*) ritual_symbol_create( INST,"...");
                  return IDENTIFIER; }
",@"            { return COMMA_AT; }
"#("            { return HASH_LPAREN; }
"#t"            { *yylval_param = (ritual_object_t*) ritual_boolean_create( INST, 1 );
                  return BOOLEAN; }
"#f"            { *yylval_param = (ritual_object_t*) ritual_boolean_create( INST, 0 );
                  return BOOLEAN; }
"#\\space"      { *yylval_param = (ritual_object_t*) ritual_ascii_char_create( INST, ' ' );
                  return CHARACTER; }
"#\\newline"    { *yylval_param = (ritual_object_t*) ritual_ascii_char_create( INST, '\n' );
                  return CHARACTER; }
#\\.            { *yylval_param = (ritual_object_t*) ritual_ascii_char_create( INST, yytext[2] );
                  return CHARACTER; }
\"              { pctx_dynstring_init( yyextra ); BEGIN(QUOTED_STRING); }
<QUOTED_STRING>{
    \"          { BEGIN(INITIAL);
                  pctx_dynstring_putc( yyextra, '\0' );
                  *yylval_param = (ritual_object_t*) ritual_ascii_string_create( INST, pctx_dynstring_get( yyextra ) );
                  return STRING; }
    \\\"        { pctx_dynstring_putc( yyextra, '"' ); }
    \\\\        { pctx_dynstring_putc( yyextra, '\\' ); }
    .           { pctx_dynstring_putc( yyextra, yytext[0] ); }
}
.            return yytext[0];
%%


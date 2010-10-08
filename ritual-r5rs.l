%option reentrant bison-bridge
%option noyywrap
%option case-insensitive

%{
#include <stdio.h>
#include <gmp.h>
#include "ritual-r5rs.tab.h"
%}

%%
[ \n]           ;
;[^\n]*         ; /* comment */
[0-9]+          { yylval_param->string = yytext; return NUMBER; }
"#("            { return HASH_LPAREN; }
",@"            { return COMMA_AT; }
"#t"            { yylval_param->integer = 1; return BOOLEAN; }
"#f"            { yylval_param->integer = 0; return BOOLEAN; }
.               { return (int) yytext[0]; }
%%


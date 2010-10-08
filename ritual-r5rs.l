%option reentrant bison-bridge
%option noyywrap
%option case-insensitive

%{
#include <stdio.h>
#include <gmp.h>
#include "ritual-r5rs.tab.h"
#include "parsectx.h"
%}

letter [a-z]
initial [a-z\!\$\%\&\*\/\:\<\=\>\?\^\_\~]
subsequents [a-z\!\$\%\&\*\/\:\<\=\>\?\^\_\~0-9\+\-\.\@]*

%x QUOTED_STRING

%%
[ \n]           ;
;[^\n]*         ; /* comment */
[0-9]+          { yylval_param->string = yytext; return NUMBER; }
{initial}{subsequents}  { yylval_param->string = yytext; return IDENTIFIER; }
"+"             { yylval_param->string = yytext; return IDENTIFIER; }
"-"             { yylval_param->string = yytext; return IDENTIFIER; }
"..."           { yylval_param->string = yytext; return IDENTIFIER; }
",@"            { return COMMA_AT; }
"#("            { return HASH_LPAREN; }
"#t"            { yylval_param->integer = 1; return BOOLEAN; }
"#f"            { yylval_param->integer = 0; return BOOLEAN; }
"#\\space"      { yylval_param->integer = ' '; return CHARACTER; }
"#\\newline"    { yylval_param->integer = '\n'; return CHARACTER; }
#\\.            { yylval_param->integer = yytext[2]; return CHARACTER; }
\"              { initialize_dynstring( yyextra ); BEGIN(QUOTED_STRING); }
<QUOTED_STRING>{
    \"          { BEGIN(INITIAL);
                  accumulate_dynstring( yyextra, '\0' );
                  yylval_param->string = get_dynstring( yyextra );
                  return STRING; }
    \\\"        { accumulate_dynstring( yyextra, '"' ); }
    \\\\        { accumulate_dynstring( yyextra, '\\' ); }
    .           { accumulate_dynstring( yyextra, yytext[0] ); }
}
.            return yytext[0];
%%


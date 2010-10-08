%option reentrant bison-bridge
%option noyywrap

%{
#include <stdio.h>
#include "alpha.tab.h"
%}

%%
[0-9]+          { *yylval_param = atoi( yytext ); return NUMBER; }
\n              ;
[ \t]+          ;
%%


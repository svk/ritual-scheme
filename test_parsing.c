#include "parse_context.h"
#include "ritual-r5rs.tab.h"
#include "lex.ritual-r5rs.h"

extern int yyparse( struct parse_context* );

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


#include "parse_context.h"
#include "ritual-r5rs.tab.h"
#include "lex.ritual-r5rs.h"

#include "ritual_instance.h"

#include "ritual_flo.h"

extern int yyparse( struct parse_context* );

int main(int argc, char *argv[]) {
    struct ritual_instance scheme;
    struct parse_context my;
	struct rflo_filehandle *flo_stdout = rflo_filehandle_create( stdout );
    pctx_init( &my, &scheme );
    yylex_init( &my.scanner );
    yyset_extra( &my, my.scanner );
	while( 1 ) {
		char data[1024];
		fputs( ">>> ", stdout );
		fflush( stdout );
		fgets( data, sizeof data, stdin );

		int len = strlen( data );
		if( data[len-1] != '\n' ) {
			puts( "error: line too long" );
		} else {
			data[len-1] = '\0';
			if( !strlen(data) ) {
				break;
			}
			yy_scan_string( data, my.scanner );
			yyparse( &my );
			while( pctx_has_more( &my ) ) {
				ritual_object_t * object = pctx_pop( &my );
				fputs( "-> ", stdout );
				ritual_print( flo_stdout, object );
				puts( "" );
			}
		}
	}
	rflo_filehandle_destroy( flo_stdout );
    yylex_destroy( my.scanner );
    pctx_destroy( &my );
    return 0;
}


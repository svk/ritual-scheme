#include "parse_context.h"
#include "ritual-r5rs.tab.h"
#include "lex.ritual-r5rs.h"

#include "ritual_instance.h"

#include "ritual_flo.h"

#include "ritual_generic.h"

#include "ritual_error.h"

#include "ritual_eval.h"

extern int yyparse( struct parse_context* );

int main(int argc, char *argv[]) {
    struct ritual_instance scheme;
    struct parse_context my;
	struct rflo_filehandle *flo_stdout; 

    int failure = ritual_initialize_instance( &scheme );
    if( failure ) {
        fprintf(stderr, "fatal error: unable to initialize Scheme instance\n" );
        return 1;
    }
    if( RITUAL_TOP_LEVEL_ERROR( &scheme ) ) {
        fprintf(stderr, "fatal error: %s\n", scheme.error->reason );
        ritual_deinitialize_instance( &scheme );
        return 1;
    }

    flo_stdout = rflo_filehandle_create( stdout );

    pctx_init( &my, &scheme );
    yylex_init( &my.scanner );
    yyset_extra( &my, my.scanner );
	while( 1 ) {
		char data[1024];
		fputs( ">>> ", stdout );
		fflush( stdout );
		void* fgrv = fgets( data, sizeof data, stdin );
        if( !fgrv ) {
            puts("");
            break;
        }

		int len = strlen( data );
		if( data[len-1] != '\n' ) {
			puts( "error: line too long" );
		} else {
			data[len-1] = '\0';
			if( !strlen(data) ) {
				continue;
			}
            YY_BUFFER_STATE buffer = yy_scan_string( data, my.scanner );
			yyparse( &my );
            yy_delete_buffer( buffer, my.scanner );
			while( pctx_has_more( &my ) ) {
				ritual_object_t * object = pctx_pop( &my );
                object = ritual_eval( &scheme, scheme.root, object );
				fputs( "-> ", stdout );
				ritual_print( &scheme, &flo_stdout->flo, object );
				puts( "" );
			}
		}
	}
	rflo_filehandle_destroy( flo_stdout );
    yylex_destroy( my.scanner );
    pctx_destroy( &my );

    ritual_deinitialize_instance( &scheme );
    return 0;
}


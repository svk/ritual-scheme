#include "parse_context.h"
#include "ritual-r5rs.tab.h"
#include "lex.ritual-r5rs.h"

#include "ritual_instance.h"

#include "ritual_flo.h"

#include "ritual_generic.h"

#include "ritual_error.h"

#include "ritual_basic_types.h"

#include "rl3.h"

extern int yyparse( struct parse_context* );

int main(int argc, char *argv[]) {
    struct ritual_instance scheme;
    struct parse_context my;
	struct rflo_filehandle *flo_stdout; 

    ritual_global_initialize();
    ritual_select_instance( &scheme ); // Life's easy when you're single-threaded..

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

    flo_stdout = rflo_filehandle_create( &scheme, stdout );

    pctx_init( &my, &scheme );
    yylex_init( &my.scanner );
    yyset_extra( &my, my.scanner );

    struct rl3_global_context gctx_s;
    struct rl3_global_context *gctx = &gctx_s;
    rl3_initialize(gctx );

    struct rl3_context rl3ctx;

    rl3_context_init( &rl3ctx, gctx, &scheme );

    struct rl3_instr *program = 0;
    struct rl3_instr *jpt = program = rl3_mkinstr( &scheme, gctx->IS_NULL, 0, program );
    program = rl3_mkinstr( &scheme, gctx->BRANCH, 0, program );
    program = rl3_mkinstr( &scheme, gctx->SPLIT_PAIR, 0, program );
    program = rl3_mkinstr( &scheme, gctx->PRINT, 0, program );
    program = rl3_mkinstr( &scheme, gctx->JUMP, &jpt->header, program );
    program = rl3_reverse( program );

    struct rl3_instr *print_forwards = program;

    struct rl3_instr *dsc = rl3_mkinstr( &scheme, gctx->DISCARD, 0, 0);
/*
    program = 0;
    program = rl3_mkinstr( &scheme, gctx->STORE, 0, program );
    jpt = program = rl3_mkinstr( &scheme, gctx->SWAP, 0, program );
    program = rl3_mkinstr( &scheme, gctx->IS_NULL, 0, program );
    program = rl3_mkinstr( &scheme, gctx->BRANCH, &dsc->header, program );
    program = rl3_mkinstr( &scheme, gctx->SPLIT_PAIR, 0, program );
    program = rl3_mkinstr( &scheme, gctx->ROTATE, 0, program );
    program = rl3_mkinstr( &scheme, gctx->SWAP, 0, program );
    program = rl3_mkinstr( &scheme, gctx->CONS, 0, program );
    program = rl3_mkinstr( &scheme, gctx->JUMP, &jpt->header, program );
    program = rl3_reverse( program );
    */
    program = 0;
    struct rl3_instr **writep = &program;
    writep = rl3_seqinstr( &scheme, gctx->STORE, 0, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->SWAP, 0, writep, &jpt );
    writep = rl3_seqinstr( &scheme, gctx->IS_NULL, 0, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->BRANCH, &dsc->header, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->SPLIT_PAIR, 0, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->ROTATE, 0, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->SWAP, 0, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->CONS, 0, writep, 0 );
    writep = rl3_seqinstr( &scheme, gctx->JUMP, &jpt->header, writep, 0 );

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
            if( RITUAL_INTERACTIVE_LEVEL_ERROR( &scheme ) ) {
                fprintf( stderr, "error: %s\n", scheme.error->reason );
            } else {
                YY_BUFFER_STATE buffer = yy_scan_string( data, my.scanner );
                yyparse( &my );
                yy_delete_buffer( buffer, my.scanner );
                while( pctx_has_more( &my ) ) {
                    ritual_object_t * object = pctx_pop( &my );
                    ritual_list_push( &scheme, &rl3ctx.values, object ); 
                    ritual_list_push( &scheme, &rl3ctx.sequences, (void*) program );
                    fprintf(stderr, "\n" );
                    while( rl3_running( &rl3ctx ) ) {
                        fprintf(stderr, "." );
                        rl3_run_one( &rl3ctx );
                    }
                    fprintf(stderr, "\n" );
                    object = ritual_list_next( &scheme, &rl3ctx.values );
                    fputs( "-> ", stdout );
                    ritual_print( &scheme, &flo_stdout->flo, object );
                    puts( "" );
                }
            }
		}
	}

	rflo_filehandle_destroy( &scheme, flo_stdout );
    yylex_destroy( my.scanner );
    pctx_destroy( &my );

    rl3_deinitialize( gctx );

    ritual_deinitialize_instance( &scheme );

    ritual_global_deinitialize();
    return 0;
}



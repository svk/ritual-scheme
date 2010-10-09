#include "ritual_error.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void ritual_error_initialize( struct ritual_error_instance *error ) {
    memset( error, 0, sizeof *error );
}

void ritual_error_jump( struct ritual_error_instance *error ) {
    if( error->top_valid ) {
        longjmp( error->top, 1 );
    } else {
        fprintf( stderr, "warning: fatal error without top-level handler set\n" );
        error->reason[ sizeof error->reason - 1 ] = '\0';
        fprintf( stderr, "last reason was: %s\n", error->reason );
        exit( 1 );
    }
}

void ritual_error_str( struct ritual_instance *inst, const char *s ) {
    memset( inst->error->reason, 0, sizeof inst->error->reason );
    strncpy( inst->error->reason, s, sizeof inst->error->reason - 1 );
    ritual_error_jump( inst->error );
}

void ritual_error( struct ritual_instance *inst, const char *fmt, ... ) {
    va_list args;
    va_start( args, fmt );
    memset( inst->error->reason, 0, sizeof inst->error->reason );
    vsnprintf( inst->error->reason, sizeof inst->error->reason - 1, fmt, args );
    va_end( args );

    ritual_error_jump( inst->error );
}

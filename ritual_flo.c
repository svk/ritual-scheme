#include "ritual_flo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

int rflo_filehandle_write( void *flo, const void *data, int len) {
    struct rflo_filehandle *floh = flo; 
    return fwrite( data, 1, len, floh->f );
}

int rflo_filehandle_read( void *flo, void *data, int len) {
    struct rflo_filehandle *floh = flo; 
    return fread( data, 1, len, floh->f );
}

struct rflo_filehandle * rflo_filehandle_create( struct ritual_instance* inst, FILE *f ) {
    struct rflo_filehandle *rv = ritual_alloc( inst, sizeof *rv);
    rv->flo.write = rflo_filehandle_write;
    rv->flo.read = rflo_filehandle_read;
    rv->f = f;
    return rv;
}

void rflo_filehandle_destroy( struct ritual_instance* inst, struct rflo_filehandle *floh ) {
    ritual_free( inst, floh );
}

void rflo_putdata( struct ritual_flo *flo, const char *data, int len) {
    int written = 0;
    while( written < len ) {
        assert( flo->write ); // file writable
        int wrote = flo->write( flo, &data[written], len - written );
        assert( wrote > 0 );
        written += wrote;
    }
}

void rflo_putstring( struct ritual_flo *flo, const char *s ) {
    rflo_putdata( flo, s, strlen( s ) );
}

void rflo_putchar( struct ritual_flo *flo, char ch ) {
    rflo_putdata( flo, &ch, 1 );
}

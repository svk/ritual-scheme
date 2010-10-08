#include "ritual_basic_types.h"
#include "ritual_flo.h"
#include "ritual_generic.h"

/* This is supposed to leak memory -- the objects are supposed
 * to be GCed but there is no GC running in this test. */

int main(int argc, char *argv[]) {
    void *alpha = ritual_native_int_create( 42 );
    void *beta = ritual_ascii_string_create( "hello world" );
    void *gamma = ritual_pair_create( alpha, beta );
    void *delta = ritual_boolean_create( 0 );
    void *epsilon = ritual_symbol_create( "HELLOWORLD" );
    void *zeta = ritual_ascii_char_create( '\n' );
    void *eta = 0;

    struct rflo_filehandle * fhf = rflo_filehandle_create( stdout );
    struct ritual_flo *f = &fhf->flo;

    ritual_print( f, alpha );
    ritual_print( f, beta );
    ritual_print( f, gamma );
    ritual_print( f, delta );
    ritual_print( f, epsilon );
    ritual_print( f, zeta );
    ritual_print( f, eta );

    return 0;
}

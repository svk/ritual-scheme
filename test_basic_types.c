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

    void *megalist = 0;
    megalist = ritual_pair_create( eta, megalist );
    megalist = ritual_pair_create( zeta, megalist );
    megalist = ritual_pair_create( epsilon, megalist );
    megalist = ritual_pair_create( delta, megalist );
    megalist = ritual_pair_create( gamma, megalist );
    megalist = ritual_pair_create( beta, megalist );
    megalist = ritual_pair_create( alpha, megalist );

    struct rflo_filehandle * fhf = rflo_filehandle_create( stdout );
    struct ritual_flo *f = &fhf->flo;

    ritual_print( f, megalist );

    return 0;
}

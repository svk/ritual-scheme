#include "ritual_basic_types.h"
#include "ritual_flo.h"
#include "ritual_generic.h"

#include "ritual_instance.h"

/* This is supposed to leak memory -- the objects are supposed
 * to be GCed but there is no GC running in this test. */

int main(int argc, char *argv[]) {
    struct ritual_instance inst;

    void *alpha = ritual_native_int_create( &inst, 42 );
    void *beta = ritual_ascii_string_create( &inst, "hello world" );
    void *gamma = ritual_pair_create( &inst, alpha, beta );
    void *delta = ritual_boolean_create( &inst, 0 );
    void *epsilon = ritual_symbol_create( &inst, "HELLOWORLD" );
    void *zeta = ritual_ascii_char_create( &inst, '\n' );
    void *eta = 0;

    void *megalist = 0;
    megalist = ritual_pair_create( &inst, eta, megalist );
    megalist = ritual_pair_create( &inst, zeta, megalist );
    megalist = ritual_pair_create( &inst, epsilon, megalist );
    megalist = ritual_pair_create( &inst, delta, megalist );
    megalist = ritual_pair_create( &inst, gamma, megalist );
    megalist = ritual_pair_create( &inst, beta, megalist );
    megalist = ritual_pair_create( &inst, alpha, megalist );

    struct rflo_filehandle * fhf = rflo_filehandle_create( stdout );
    struct ritual_flo *f = &fhf->flo;

    ritual_print( &inst, f, megalist );

    return 0;
}

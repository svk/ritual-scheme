#include "ritual_generic.h"
#include "ritual_basic_types.h"
#include "ritual_native_proc.h"
#include "ritual_lambda.h"

#include "ritual_env.h"

#include "ritual_bignum.h"

struct ritual_generic_functions {
        /* This is "write", not "display", confusingly (sorry!). */
    void (*print)( struct ritual_instance *,
                   struct ritual_flo*, void * );
        /* Most objects actually don't have destroy methods; they
         * are flat objects that can just be freed. This can then
         * be left zeroed out. */
    void (*destroy)( struct ritual_instance *,
                     void * );
};

const struct ritual_generic_functions ritual_genfun[ RTYPE_NUM_TYPES ] = {
    { 0,
      0 },
    { ritual_print_null,
      0 },
    { ritual_print_pair,
      0 },
    { ritual_print_symbol,
      0 },
    { 0,
      0},
    { ritual_print_boolean,
      0},
    { ritual_print_native_int,
      0 },
    { ritual_print_ascii_char,
      0 },
    { ritual_print_ascii_string,
      0 },
    { 0, // vector
      0 },
    { 0, // port
      0 },
    { ritual_print_native_proc, // native procedure
      0 },
    { ritual_print_quote,
      0 },
    { ritual_print_lambda_proc,
      0 },
    { 0, // keyword
      0 },
    { 0,
      ritual_env_destroy },
    { ritual_print_big_int,
      ritual_big_int_destroy },
    { ritual_print_big_rational,
      ritual_big_rational_destroy }
};

void ritual_print( struct ritual_instance *inst,
                   struct ritual_flo *flo, ritual_object_t *obj ) {
    ritual_type_t type = RITUAL_TYPE( obj );
    if( ritual_genfun[ type ].print ) {
        ritual_genfun[ type ].print( inst, flo, obj );
    }
}

void ritual_destroy( struct ritual_instance *inst,
                     ritual_object_t *obj ) {
    ritual_type_t type = RITUAL_TYPE( obj );
    if( ritual_genfun[ type ].destroy ) {
        ritual_genfun[ type ].destroy( inst, obj );
    }
}

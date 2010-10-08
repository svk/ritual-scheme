#include "ritual_generic.h"
#include "ritual_basic_types.h"

struct ritual_generic_functions {
        /* This is "write", not "display", confusingly (sorry!). */
    void (*print)( struct ritual_flo*, void * );
};

struct ritual_generic_functions ritual_genfun[ RTYPE_NUM_TYPES ] = {
    { 0 },
    { ritual_print_null },
    { ritual_print_pair },
    { ritual_print_symbol },
    { 0 },
    { ritual_print_boolean },
    { ritual_print_native_int },
    { ritual_print_ascii_char },
    { ritual_print_ascii_string },
    { 0 },
    { 0 }
};

void ritual_print( struct ritual_flo *flo, ritual_object_t *obj ) {
    ritual_genfun[ RITUAL_TYPE( obj ) ].print( flo, obj );
}

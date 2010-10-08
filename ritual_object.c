#include "ritual_object.h"

#include <stdlib.h>

/* For now we have no actual garbage collection, just talkie
 * comments. We leak memory like a sieve. */

ritual_object_t * ritual_alloc( int sz ) {
    ritual_object_t * rv = malloc( sz );
    return rv;
}

void ritual_free( ritual_object_t * object ) {
    free( object );
}

/* For convenience. */

void * ritual_alloc_typed_object( ritual_type_t type, int sz ) {
    void *rv = ritual_alloc( sz );
    RITUAL_SET_TYPE( rv, type );
    return rv;
}


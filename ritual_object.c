#include "ritual_object.h"

#include "ritual_generic.h"

#include <stdlib.h>

#include <stdio.h>

/* For now we have no actual garbage collection, just talkie
 * comments. We leak memory like a sieve. */

ritual_object_t * ritual_alloc( struct ritual_instance *inst,
                                int sz ) {
    ritual_object_t * rv = malloc( sz );
    fprintf( stderr, "debug: allocated %p\n" );
    return rv;
}

void ritual_free( struct ritual_instance *inst,
                  ritual_object_t * object ) {
    fprintf( stderr, "debug: deallocated %p\n" );
    ritual_destroy( inst, object );
    free( object );
}

/* For convenience. */

void * ritual_alloc_typed_object( struct ritual_instance *inst,
                                  ritual_type_t type, int sz ) {
    void *rv = ritual_alloc( inst, sz );
    RITUAL_SET_TYPE( rv, type );
    return rv;
}


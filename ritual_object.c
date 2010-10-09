#include "ritual_object.h"

#include "ritual_generic.h"

#include "ritual_error.h"

#include "ritual_gc.h"

#include <stdlib.h>

#include <stdio.h>

#include <assert.h>

/* Current GC: a very naïve implementation of tricolour
 * marking. We keep a linked list of ALL OBJECTS;
 * very wasteful.
 */

ritual_object_t * ritual_alloc_object( struct ritual_instance *inst,
                                       int sz ) {
    ritual_object_t * rv = ritual_alloc( inst, sz );
    if( !rv ) {
        ritual_error( inst, "unable to allocate object of size %d (out of memory)", sz );
    }
    rgc_allocated_object( inst, rv );
    return rv;
}

void * ritual_alloc( struct ritual_instance *inst,
                     int sz ) {
    void *rv = malloc( sz );
    fprintf( stderr, "debug: allocated %p\n", rv );
    return rv;
}

void ritual_free_object( struct ritual_instance *inst,
                         ritual_object_t * object ) {
    ritual_destroy( inst, object );
    ritual_free( inst, object );
}

void ritual_free( struct ritual_instance *inst,
                  void * mem ) {
    fprintf( stderr, "debug: deallocated %p\n", mem );
    free( mem );
}

/* For convenience. */

void * ritual_alloc_typed_object( struct ritual_instance *inst,
                                  ritual_type_t type, int sz ) {
    void *rv = ritual_alloc_object( inst, sz );
    RITUAL_SET_TYPE( rv, type );
    return rv;
}

/* Lists obviously not thread-safe for multiple threads manipulating
 * the same list (needs a mutex). */

void ritual_olist_destroy( struct ritual_instance *inst,
                           struct ritual_onode **list ) {
    while( *list ) {
        ritual_olist_pop( inst, list );
    }
}

ritual_object_t * ritual_olist_pop( struct ritual_instance *inst,
                                    struct ritual_onode ** list ) {
    struct ritual_onode *node = *list;
    ritual_object_t *rv = 0;
    if( node ) {
        *list = node->next;
        rv = node->object;
        ritual_free( inst, node );
    }
    return rv;
}

void ritual_olist_push( struct ritual_instance *inst,
                        struct ritual_onode ** list,
                        ritual_object_t * object ) {
    if( object ) {
        struct ritual_onode *node = ritual_alloc( inst, sizeof *node );
        if( !node ) {
            ritual_error_str( inst, "unable to allocate olist node (out of memory)" );
        }
        node->object = object;
        node->next = *list;
        *list = node;
    }
}


const char * ritual_typename( const ritual_object_t * object ) {
    static const char typenames[RTYPE_NUM_TYPES][256] = {
        "(invalid - zero)",
        "empty list",
        "pair",
        "symbol",
        "procedure",
        "boolean",
        "native integer",
        "ascii character",
        "ascii string",
        "vector",
        "port"
    };
    static const char out_of_range[] = "(invalid - out of range)";
    int typeid = RITUAL_TYPE( object );
    if( typeid < 0 || typeid >= RTYPE_NUM_TYPES ) {
        return out_of_range;
    }
    return typenames[ typeid ];

}


#include "ritual_object.h"

#include "ritual_generic.h"

#include "ritual_error.h"

#include "ritual_gc.h"

#include <stdlib.h>

#include <stdio.h>

#include <assert.h>

int ritual_typepred[RTYPE_NUM_TYPES] = {
    RTYPEPRED_NONE,
    RTYPEPRED_NONE,
    RTYPEPRED_PAIR,
    RTYPEPRED_SYMBOL,
    RTYPEPRED_PROCEDURE,
    RTYPEPRED_BOOLEAN,
    RTYPEPRED_NUMBER,
    RTYPEPRED_CHAR,
    RTYPEPRED_STRING,
    RTYPEPRED_VECTOR,
    RTYPEPRED_PORT,
    RTYPEPRED_PROCEDURE,
    RTYPEPRED_NONE, // quote -- ?
    RTYPEPRED_PROCEDURE,
    RTYPEPRED_NONE, // keyword
    RTYPEPRED_NONE, // environment
    RTYPEPRED_NUMBER,
    RTYPEPRED_NUMBER,
    RTYPEPRED_PROCEDURE,
    RTYPEPRED_PROCEDURE
};

ritual_object_t * ritual_object_satisfies_typepred(
        struct ritual_instance *inst,
        ritual_object_t *object,
        int predicate ) {
    if( ritual_typepred[ RITUAL_TYPE( object ) ] == predicate ) {
        return inst->scheme_true;
    }
    return inst->scheme_false;
}

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
    if( rv ) {
        inst->total_bytes_allocated += sz;
    }
    return rv;
}

void ritual_free_object( struct ritual_instance *inst,
                         ritual_object_t * object ) {
    ritual_destroy( inst, object );
    ritual_free( inst, object );
}

void ritual_free( struct ritual_instance *inst,
                  void * mem ) {
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


const char * ritual_typename( const void* p) {
    const ritual_object_t *object = (const ritual_object_t*) p;
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
        "port",
        "native procedure",
        "quote",
        "lambda procedure",
        "keyword",
        "environment",
        "big integer",
        "big rational",
        "native procedure (easy)",
        "native procedure (easy tail)"
    };
    static const char out_of_range[] = "(invalid - out of range)";
    int typeid = RITUAL_TYPE( object );
    if( typeid < 0 || typeid >= RTYPE_NUM_TYPES ) {
        return out_of_range;
    }
    return typenames[ typeid ];

}

ritual_object_t * rconvto_object( void*p ) {
    return p;
}

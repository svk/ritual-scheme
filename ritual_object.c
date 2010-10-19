#include "ritual_object.h"

#include "ritual_generic.h"

#include "ritual_error.h"

#include "ritual_gc.h"

#include <stdlib.h>

#include <stdio.h>

#include <assert.h>

#include <pthread.h>
#include <string.h>

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
    RTYPEPRED_PROCEDURE,
    RTYPEPRED_PROCEDURE,
    RTYPEPRED_NONE // rl3 ins.
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

void * ritual_trivial_realloc( struct ritual_instance *inst, void *old, int oldsz, int sz ) {
    // The trivial implementation. Anything that actually relies
    // heavily on ritual_realloc() and is meant to be efficient
    // should use another set of memory functions that map onto
    // the real malloc(), realloc(), free() after some
    // quota-checking etc.
    void *rv = ritual_alloc( inst, sz );
    if( !rv ) {
        return 0;
    }
    int minsize = (oldsz < sz) ? oldsz : sz;
    memcpy( rv, old, minsize );
    ritual_free( inst, old );
    return rv;
}

/* For convenience. */

void * ritual_alloc_typed_object( struct ritual_instance *inst,
                                  ritual_type_t type, int sz ) {
    void *rv = ritual_alloc_object( inst, sz );
    RITUAL_SET_TYPE( rv, type );
    if( rv ) {
        inst->typed_objects_allocated[ type ]++;
        inst->typed_objects_bytes_allocated[ type ] += sz;
    }
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

        inst->olist_allocated += sizeof *node;
    }
}

const char * ritual_typename_abstract(int typeid) {
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
        "native procedure (easy tail)",
        "native procedure (envproc)",
        "RL3 instruction"
    };
    static const char out_of_range[] = "(invalid - out of range)";
    if( typeid < 0 || typeid >= RTYPE_NUM_TYPES ) {
        return out_of_range;
    }
    return typenames[ typeid ];
}

ritual_object_t * ritual_preevaluate( struct ritual_instance *inst, ritual_object_t* obj, int *success ) {
    // Evaluation that can be done without an environment (and so,
    // implicitly, also without any other evaluations (except if
    // those are also preevaluate-able!))
    // Getting _really_ smart about this edges into actually
    // compiling code. However, another part of compiling code
    // would be "simplify", which is a generalization of this:
    // change expression to something equivalent, that still
    // gets evaluated afterwards. E.g.: arithmetic on literals:
    // (* 2 3.14 r r) => (* 6.28 r r)
    // Using such an operation, this is simply doing that and
    // then returning success iff the result is self-evaluating.
    // (Compiling can become more complex -- the most obvious
    //  way is through awareness of the environment. However,
    //  this is harder than it looks because Scheme makes it so
    //  easy to shadow and change things. If the above said
    //  "pi" rather than 3.14, could we ever really know that
    //  "pi" was a constant and did not need an environment
    //  lookup?)
    *success = 1;
    if( ritual_selfevaluatingp( inst, obj ) ) {
        return obj;
    }
    switch( RITUAL_TYPE( obj ) ) {
        case RTYPE_QUOTE:
            {
                struct ritual_quote *quote = (struct ritual_quote*) value;
                return quote->quoted;
            }
    }
    *success = 0;
    return 0;
}

int ritual_selfevaluatingp( struct ritual_instance *inst, ritual_object_t* obj ) {
    switch( RITUAL_TYPE( obj ) ) {
        case RTYPE_NULL:
        case RTYPE_ASCII_STRING:
        case RTYPE_PROCEDURE:
        case RTYPE_BOOLEAN:
        case RTYPE_NATIVE_INTEGER:
        case RTYPE_ASCII_CHAR:
        case RTYPE_VECTOR:
        case RTYPE_PORT:
        case RTYPE_BIG_INTEGER:
        case RTYPE_BIG_RATIONAL:
        case RTYPE_EASY_PROC:
        case RTYPE_EASY_TAIL_PROC:
            return 1;
        default:
            return 0; // this is the safe default -- no harm in evaluating too much
    }
}

const char * ritual_typename( const void* p) {
    const ritual_object_t *object = (const ritual_object_t*) p;
    int typeid = RITUAL_TYPE( object );
    return ritual_typename_abstract( typeid );
}

ritual_object_t * rconvto_object( void*p ) {
    return p;
}

void * ritual_alloc_tls( size_t sz ) {
    struct ritual_instance *inst = ritual_get_selected_instance();
    return ritual_alloc( inst, sz );
}

void ritual_free_tls( void* p ) {
    struct ritual_instance *inst = ritual_get_selected_instance();
    return ritual_free( inst, p );
}

void * ritual_xalloc_tls( size_t sz ) {
    void *rv = ritual_alloc_tls( sz );
    if( !rv ) {
        struct ritual_instance *inst = ritual_get_selected_instance();
        ritual_error( inst, "completely out of memory in xalloc - was allocating %d bytes", sz );
    }
    return rv;
}

void * ritual_xrealloc_tls_size ( void* p, size_t oldsz, size_t sz) {
    struct ritual_instance *inst = ritual_get_selected_instance();
    void *rv = ritual_trivial_realloc( inst, p, oldsz, sz );
    if( !rv ) {
        ritual_error( inst, "completely out of memory in xrealloc - was allocating %d bytes", sz );
    }
    return rv;
}

void ritual_free_tls_size( void* p, size_t sz ) {
    return ritual_free_tls( p );

}

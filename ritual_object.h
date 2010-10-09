#ifndef H_RITUAL_OBJECT
#define H_RITUAL_OBJECT

/* This is begining to have shades of: "Any sufficiently complicated
 * C implementation of half of Common Lisp contains an ad-hoc,
 * informally-specified, bug-ridden, slow implementation of half
 * of C++." */

typedef enum ritual_type {
    RTYPE_INVALID = 0,
    RTYPE_NULL,
    RTYPE_PAIR,
    RTYPE_SYMBOL,
    RTYPE_PROCEDURE,
    RTYPE_BOOLEAN,
    RTYPE_NATIVE_INTEGER,
    RTYPE_ASCII_CHAR,
    RTYPE_ASCII_STRING,
    RTYPE_VECTOR,
    RTYPE_PORT,
    RTYPE_NUM_TYPES
} ritual_type_t;

struct ritual_object {
    ritual_type_t type;
};

/* Less radically, we might want to shrink this from
 * a probably-padded struct to just an integer of the
 * size required. (Or an integer array if we require
 * more than 32 bits (!).) */
typedef struct ritual_object ritual_object_t;


#include "ritual_instance.h"

/* Is this model theoretically sound without doing something like
 * wrapping everything in a union? Yes. Quoth the C standard:
 *      "A pointer to a structure object, suitably converted,
 *       points to its initial member (or if that member is a
 *       bit-field, then to the unit in which it resides),
 *       and vice versa. There may be unnamed padding within
 *       a structure object, but not at its beginning."
 */

#include <stdint.h>


/* Alternative for consideration: use aligned memory.
 * This allows storing a tag in the low bits of the
 * pointer itself, e.g. if all valid pointers are
 * 16-aligned, then by use of masks we can have
 * 16 different pointers that refer to the same
 * object. Since we only need one we can store
 * 4 bits of information by choosing which pointer to
 * use.
 * Implementing this NOW would be premature
 * optimization but it's a neat idea (not mine). */

    /* Memory for objects must be managed by our own
     * functions since we need garbage collection. */
    /* ritual_alloc_object() should never return NULL,
     * but may trigger an error (hopefully eventually
     * an internal scheme error). */
ritual_object_t * ritual_alloc_object( struct ritual_instance*, int );
void ritual_free_object( struct ritual_instance*, ritual_object_t * );

void * ritual_alloc_typed_object( struct ritual_instance*, ritual_type_t, int );

void * ritual_alloc( struct ritual_instance*, int );
void ritual_free( struct ritual_instance*, void * );

#define RITUAL_SET_TYPE(x,t) {((struct ritual_object*)(x))->type = (t);}
#define RITUAL_TYPE(x) ((x) ? ((struct ritual_object*)(x))->type : RTYPE_NULL)

struct ritual_onode {
    ritual_object_t *object;
    struct ritual_onode *next;
};

void ritual_olist_destroy( struct ritual_instance *,
                           struct ritual_onode ** );
ritual_object_t * ritual_olist_pop( struct ritual_instance *,
                                    struct ritual_onode ** );
void ritual_olist_push( struct ritual_instance *,
                        struct ritual_onode **,
                        ritual_object_t * );

const char * ritual_typename( const ritual_object_t * );

#endif

#include "ritual_gc.h"

/* For now we have the simplest of all possible "garbage
 * collection" schemes: we just make sure that we release
 * all memory when the instance shuts down. Internally
 * we still leak all memory. */

void rgc_initialize( struct ritual_instance *inst,
                     struct ritual_gc_instance * gc ) {
    gc->all = 0;
    gc->gray = 0;
}

void rgc_deinitialize( struct ritual_instance *inst,
                       struct ritual_gc_instance * gc ) {
    ritual_olist_destroy( inst, &gc->gray );
    while( gc->all ) {
        ritual_object_t * object = ritual_olist_pop( inst, &gc->all );
        ritual_free_object( inst, object );
    }
}

void rgc_allocated_object( struct ritual_instance *inst,
                           ritual_object_t *object ) {
    ritual_olist_push( inst, &inst->gc->all, object );
}

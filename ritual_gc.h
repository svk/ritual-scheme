#ifndef H_RITUAL_GC
#define H_RITUAL_GC

#include "ritual_instance.h"
#include "ritual_object.h"

struct ritual_gc_instance {
    struct ritual_onode *all;
    struct ritual_onode *gray;
};

void rgc_initialize( struct ritual_instance*,
                     struct ritual_gc_instance * );
void rgc_deinitialize( struct ritual_instance*,
                       struct ritual_gc_instance * );

void rgc_allocated_object( struct ritual_instance *,
                           ritual_object_t * );

#endif

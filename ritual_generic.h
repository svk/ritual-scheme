#ifndef H_RITUAL_GENERIC
#define H_RITUAL_GENERIC

#include "ritual_object.h"
#include "ritual_flo.h"

void ritual_print( struct ritual_instance *,
                   struct ritual_flo *, ritual_object_t * );
void ritual_destroy( struct ritual_instance *,
                     ritual_object_t * );

#endif

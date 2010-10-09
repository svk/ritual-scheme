#ifndef H_RITUAL_EVAL
#define H_RITUAL_EVAL

#include "ritual_env.h"
#include "ritual_object.h"

ritual_object_t * ritual_eval( struct ritual_instance *,
                               struct ritual_env *,
                               ritual_object_t * );

#endif

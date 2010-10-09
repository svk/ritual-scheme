#ifndef H_RITUAL_EVAL
#define H_RITUAL_EVAL

#include "ritual_env.h"
#include "ritual_object.h"
#include "ritual_instance.h"
#include "ritual_basic_types.h"


ritual_object_t * ritual_apply_proc( struct ritual_instance *,
                                     struct ritual_env *,
                                     ritual_object_t *,
                                     struct ritual_pair * );

ritual_object_t * ritual_eval( struct ritual_instance *,
                               struct ritual_env *,
                               ritual_object_t * );

#endif

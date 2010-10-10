#ifndef H_RITUAL_LAMBDA
#define H_RITUAL_LAMBDA

#include "ritual_object.h"
#include "ritual_instance.h"
#include "ritual_env.h"
#include "ritual_basic_types.h"

struct ritual_lambda_proc {
    ritual_object_t header;
    struct ritual_pair *argsyms; // list of symbols, possibly improper (which also means it might just be one symbol), possibly null
};

struct ritual_env * ritual_lambda_subenv( struct ritual_instance *,
                                          struct ritual_env *,
                                          struct ritual_lambda_proc *,
                                          struct ritual_pair * );


#endif

#ifndef H_RITUAL_LAMBDA
#define H_RITUAL_LAMBDA

#include "ritual_object.h"
#include "ritual_instance.h"
#include "ritual_env.h"
#include "ritual_basic_types.h"

/* Do we need a notion of "current environment", to avoid being
 * dependent on the C stack? (For TCO.) At that point, TCO would
 * consist in -- for the last expression -- creating a new
 * subenvironment of the PARENT of the current environment
 * instead of the current environment itself (and then setting
 * this as current environment before evaluating the statements
 * in the body). */
/* Do we need a trampoline function to avoid growing the C stack?
 * Input:
 *     an environment stack
 *     a body of expressions to evaluate
 * Execute:
 * ... is that trampoline function supposed to be eval? */

#include "ritual_env.h"

struct ritual_lambda_proc {
    ritual_object_t header;
    struct ritual_env *parent;
    ritual_object_t *argsyms; // list of symbols, possibly improper (which also means it might just be one symbol), possibly null
    struct ritual_pair *body;
};

struct ritual_env * ritual_lambda_env( struct ritual_instance *,
                                       struct ritual_env *,
                                       struct ritual_env *,
                                       ritual_object_t *,
                                       struct ritual_pair * );

void ritual_print_lambda_proc(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );

struct ritual_lambda_proc * ritual_lambda_create(
    struct ritual_instance *,
    struct ritual_env *,
    ritual_object_t *,
    struct ritual_pair * );


#endif

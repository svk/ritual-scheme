#include "ritual_native_proc.h"
#include "ritual_object.h"
#include "ritual_env.h"
#include "ritual_eval.h"
#include "ritual_error.h"

#include "ritual_lambda.h"

void ritual_print_native_proc( struct ritual_instance *inst,
                               struct ritual_flo *flo,
                               void *obj ) {
    rflo_putstring( flo, "#<native procedure>" );
}

struct ritual_native_proc * ritual_native_proc_create( struct ritual_instance *inst,
                                                       ritual_native_procedure_t proc) {
    struct ritual_native_proc *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_NATIVE_PROC, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->procedure = proc;
    return rv;
}

void ritual_define_native_proc( struct ritual_instance * inst,
                                struct ritual_env * env,
                                const char * name,
                                ritual_native_procedure_t proc ) {
    struct ritual_native_proc * pobj = ritual_native_proc_create( inst, proc );
    RITUAL_ASSERT( inst, pobj, "object allocation failure should not return" ); 
    ritual_env_define( inst, env, name, (ritual_object_t*) pobj );
}

ritual_object_t * rnp_eqp( struct ritual_instance *inst,
                           struct ritual_env *env,
                           struct ritual_pair * uneval_args ) {
    ritual_object_t *first, *second;
    if( !uneval_args ||
        !uneval_args->cdr ||
        RITUAL_TYPE( uneval_args->cdr ) != RTYPE_PAIR ||
        ((struct ritual_pair *)uneval_args->cdr)->cdr) {
        ritual_error( inst, "eq? requires exactly two arguments" );
    }
    struct ritual_pair *nextpair = (struct ritual_pair*) uneval_args->cdr;
    first = ritual_eval( inst, env, uneval_args->car );
    second = ritual_eval( inst, env, nextpair->car );
    if( first == second ) {
        return inst->scheme_true;
    } else {
        return inst->scheme_false;
    }
}

ritual_object_t * rnp_ritual_print_diagnostics( struct ritual_instance *inst,
                                                struct ritual_env *env,
                                                struct ritual_pair * args ) {
    if( args ) {
        ritual_error( inst, "ritual-print-diagnostics takes no arguments" );
    }
    printf( "Total bytes allocated: %d\n", inst->total_bytes_allocated );
    return 0;
}

ritual_object_t * rnp_define( struct ritual_instance *inst,
                              struct ritual_env *env,
                              struct ritual_pair *args ) {
        /* Also, we need to restrict the contexts in which this
         * is valid; top level or FIRST in a body. */
    static const char bail[] = "syntax error: define";
    if( !args  ) {
        ritual_error( inst, bail );
    }
    if( RITUAL_TYPE( args->car ) == RTYPE_PAIR ) {
        struct ritual_pair *first = (struct ritual_pair*) args->car;
        if( RITUAL_TYPE( first->car ) != RTYPE_SYMBOL ) {
            ritual_error( inst, "procedure name has wrong type" );
        }
        if( RITUAL_TYPE( args->cdr ) != RTYPE_PAIR ) {
            ritual_error( inst, "body has wrong type" );
        }
        struct ritual_symbol *procname = (struct ritual_symbol*) first->car;
        void *formals = first->cdr;
        struct ritual_pair * body = (struct ritual_pair*) args->cdr;
        struct ritual_lambda_proc * lambda = ritual_lambda_create( inst,
                                                                   env,
                                                                   formals,
                                                                   body );
        ritual_env_define( inst, env, procname->name, (ritual_object_t*) lambda );
    } else if( RITUAL_TYPE( args->car ) == RTYPE_SYMBOL ) {
        struct ritual_symbol *sym = (struct ritual_symbol*) args->car;
        if( !args->cdr || RITUAL_TYPE( args->cdr ) != RTYPE_PAIR ) {
            ritual_error( inst, bail );
        }
        struct ritual_pair *nextpair = (struct ritual_pair*) args->cdr;
        ritual_env_define( inst, env, sym->name, ritual_eval( inst, env, nextpair->car) );
    } else {
        ritual_error( inst, bail );
    }
    return 0; // should return "nothing"

}

ritual_object_t * rnp_lambda( struct ritual_instance *inst,
                              struct ritual_env *env,
                              struct ritual_pair *args ) {
    return (ritual_object_t*) ritual_lambda_create( inst, env,
        args->car,
        (struct ritual_pair*) args->cdr );
}

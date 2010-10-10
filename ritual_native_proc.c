#include "ritual_native_proc.h"
#include "ritual_object.h"
#include "ritual_env.h"
#include "ritual_eval.h"
#include "ritual_error.h"

void ritual_print_native_proc( struct ritual_instance *inst,
                               struct ritual_flo *flo,
                               void *obj ) {
    rflo_putstring( flo, "#<procedure>" );
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

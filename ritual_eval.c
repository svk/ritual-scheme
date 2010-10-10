#include "ritual_eval.h"

#include "ritual_error.h"
#include "ritual_basic_types.h"
#include "ritual_native_proc.h"

    /* Receives the procedure _evaluated_
     * and the argument list (Scheme list)
     * _unevaluated_. Remember to map by
     * ritual_eval! */
ritual_object_t * ritual_apply_proc( struct ritual_instance *inst,
                                     struct ritual_env *env,
                                     ritual_object_t *proc,
                                     struct ritual_pair *uneval_args ) {
    switch( RITUAL_TYPE( proc ) ) {
        case RTYPE_NATIVE_PROC:
            {
                struct ritual_native_proc *native_proc = (struct ritual_native_proc*) proc;
                return native_proc->procedure( inst, env, uneval_args );
            }
    }
    ritual_error( inst, "expected procedure, got \"%s\" (%d)", ritual_typename( proc ), RITUAL_TYPE(proc) );
    return 0; // impossible
}


ritual_object_t * ritual_eval( struct ritual_instance *inst,
                               struct ritual_env *env,
                               ritual_object_t *value ) {
    switch( RITUAL_TYPE(value) ) {
        case RTYPE_NULL:
        case RTYPE_ASCII_STRING:
        case RTYPE_PROCEDURE:
        case RTYPE_BOOLEAN:
        case RTYPE_NATIVE_INTEGER:
        case RTYPE_ASCII_CHAR:
        case RTYPE_VECTOR:
        case RTYPE_PORT:
            return value;
        case RTYPE_QUOTE:
            {
                struct ritual_quote *quote = (struct ritual_quote*) value;
                return quote->quoted;
            }
        case RTYPE_SYMBOL:
            {
                struct ritual_symbol *symbol = (struct ritual_symbol*) value;
                return ritual_env_lookup( inst, env, symbol->name );
            }
        case RTYPE_PAIR:
            {
                struct ritual_pair *pair = (struct ritual_pair*) value;
                if( pair->cdr && RITUAL_TYPE( pair->cdr ) != RTYPE_PAIR ) {
                    ritual_error( inst, "function application: expected argument list, got \"%s\"", ritual_typename( pair->cdr ) );
                }
                return ritual_apply_proc( inst, env,
                                          ritual_eval( inst, env, pair->car ),
                                          (struct ritual_pair*) pair->cdr );
            }
    }
    ritual_error( inst, "evaluation of type \"%s\" not implemented", ritual_typename( value ) );
    return 0; // impossible
}

struct ritual_pair * ritual_mapeval( struct ritual_instance *inst,
                                     struct ritual_env *env,
                                     struct ritual_pair * l ) {
    struct ritual_pair *rv = 0;
    struct ritual_pair **rrv = &rv;
    while( l ) {
        if( l->cdr && RITUAL_TYPE( l->cdr ) != RTYPE_PAIR ) {
            ritual_error( inst, "mapping over improper list" );
        }
        *rrv = ritual_pair_create( inst, ritual_eval( inst, env, l->car ), 0 );
        rrv = (struct ritual_pair **) &(*rrv)->cdr;

    }
    return rv;
}

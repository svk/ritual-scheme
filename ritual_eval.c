#include "ritual_eval.h"

#include "ritual_error.h"
#include "ritual_basic_types.h"

    /* Receives the procedure _evaluated_
     * and the argument list (Scheme list)
     * _unevaluated_. Remember to map by
     * ritual_eval! */
ritual_object_t * ritual_apply_proc( struct ritual_instance *inst,
                                     struct ritual_env *env,
                                     ritual_object_t *proc,
                                     ritual_object_t *uneval_args ) {
    switch( RITUAL_TYPE( proc ) ) {
    }
    ritual_error( inst, "expected procedure, got \"%s\"", ritual_typename( proc ) );
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
        case RTYPE_SYMBOL:
            {
                struct ritual_symbol *symbol = (struct ritual_symbol*) value;
                return ritual_env_lookup( inst, env, symbol->name );
            }
        case RTYPE_PAIR:
            {
                struct ritual_pair *pair = (struct ritual_pair*) value;
                return ritual_apply_proc( inst, env,
                                          ritual_eval( inst, env, pair->car ),
                                          pair->cdr );
            }
    }
    ritual_error( inst, "evaluation of type \"%s\" not implemented", ritual_typename( value ) );
    return 0; // impossible
}

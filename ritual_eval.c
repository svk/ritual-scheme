#include "ritual_eval.h"

#include "ritual_error.h"
#include "ritual_basic_types.h"

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
        default:
            ritual_error( inst, "evaluation of type \"%s\" not implemented", ritual_typename( value ) );
    }
    ritual_error( inst, "unreachable state reached in ritual_eval()" );
}

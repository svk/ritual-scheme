#include "ritual_eval.h"

#include "ritual_error.h"

ritual_object_t * ritual_eval( struct ritual_instance *inst,
                               struct ritual_env *env,
                               ritual_object_t *value ) {
    switch( RITUAL_TYPE(value) ) {
        case RTYPE_NULL:
        case RTYPE_ASCII_STRING:
            return value;
        default:
            ritual_error( inst, "evaluation of type \"%s\" not implemented", ritual_typename( value ) );
    }
    ritual_error( inst, "unreachable state reached in ritual_eval()" );
}

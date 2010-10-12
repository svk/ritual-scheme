#include "ritual_basic_procs.h"

#include "ritual_basic_types.h"
#include "ritual_basic_types.h"

/* In this file we'll try to do things properly; no casts
 * of data coming from users. Use conversion functions
 * (they're safer). Casting stuff you've created within
 * the same context is okay, I suppose. */

ritual_object_t* rep_list( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    return rconvto_object( args );
}

ritual_object_t* rep_cons( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    ritual_object_t *first = ritual_list_next( inst, &args );
    ritual_object_t *second = ritual_list_next( inst, &args );
    ritual_list_assert_end( inst, args );
    return rconvto_object( ritual_pair_create( inst, first, second ) );
}

ritual_object_t* rep_car( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    ritual_object_t *first = ritual_list_next( inst, &args );
    ritual_list_assert_end( inst, args );
    struct ritual_pair *pair = rconvto_pair( inst, first );
    return rconvto_object( pair->car );
}

ritual_object_t* rep_not( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    ritual_object_t *first = ritual_list_next( inst, &args );
    ritual_list_assert_end( inst, args );
    if( RITUAL_AS_BOOLEAN( inst, first ) ) {
        return rconvto_object( inst->scheme_false );
    }
    return rconvto_object( inst->scheme_true );
}

ritual_object_t* rep_cdr( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    ritual_object_t *first = ritual_list_next( inst, &args );
    ritual_list_assert_end( inst, args );
    struct ritual_pair *pair = rconvto_pair( inst, first );
    return rconvto_object( pair->cdr );
}

ritual_object_t* rep_nullp( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair *args ) {
    if( args ) {
        return inst->scheme_false;
    }
    return inst->scheme_true;
}

ritual_object_t* rep_length( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    int rv = 0;
    while( args ) {
        ritual_list_next( inst, &args );
        ++rv;
    }
    return rconvto_object( ritual_native_int_create( inst, rv ) );
}

ritual_object_t* rep_append( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair* args) {
    struct ritual_pair *rv = 0;
    ritual_object_t **current = (ritual_object_t**) &rv;
    while( args && args->cdr ) {
        ritual_object_t *next = ritual_list_next( inst, &args );
        struct ritual_pair *nextlist = rconvto_pair( inst, next );
        while( nextlist ) {
            ritual_object_t *nextdata = ritual_list_next( inst, &nextlist );
            *current = rconvto_object( ritual_pair_create( inst, nextdata, 0 ) );
            current = &((struct ritual_pair*)(*current))->cdr;
        }
    }
    if( args ) {
        ritual_object_t *lastarg = ritual_list_next( inst, &args );
        *current = lastarg;
    }
    return rconvto_object( rv );
}

ritual_object_t* retp_apply( struct ritual_instance *inst, struct ritual_env *env, struct ritual_pair *args ) {
    ritual_object_t *first = ritual_list_next( inst, &args );
    ritual_object_t *rest = rep_append( inst, env, args );
    return rconvto_object( ritual_pair_create( inst, first, rest ) );
}


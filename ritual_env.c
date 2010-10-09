#include "ritual_env.h"

#include "ritual_object.h"

#include "ritual_error.h"

/* TODO: alloc with the internal functions inside the HT */

void ritual_env_init_root(struct ritual_instance * inst,
                          struct ritual_env *rv ) {
    RITUAL_ASSERT( inst, rv, "null pointer passed to ritual_env_init_root" );
    rv->parent = 0;

    if( rht_table_init( &rv->table, RITUAL_ENV_SIZE_ROOT ) ) {
        ritual_error( inst, "unable to allocate root environment table" );
    }
}

void ritual_env_init_sub(struct ritual_instance *inst,
                         struct ritual_env *rv,
                         struct ritual_env *parent) {
    RITUAL_ASSERT( inst, rv, "null pointer passed to ritual_env_init_sub" );
    rv->parent = parent;

    if( rht_table_init( &rv->table, RITUAL_ENV_SIZE_SUB ) ) {
        ritual_error( inst, "unable to allocate subenvironment table" );
    }
}

void ritual_env_destroy( struct ritual_instance *inst,
                         struct ritual_env *env ) {
    rht_table_destroy( &env->table );
}

void ritual_env_set( struct ritual_instance *inst,
                     struct ritual_env *env,
                     const char *name,
                     ritual_object_t * value ) {
    ritual_error( inst, "set isn't implemented yet because the hash table needs more features first" );
}

void ritual_env_define( struct ritual_instance *inst,
                        struct ritual_env *env,
                        const char *name,
                        ritual_object_t * value ) {
    rht_set_str( &env->table, name, value );
}

ritual_object_t * ritual_env_lookup( struct ritual_instance * inst,
                                     struct ritual_env * env,
                                     const char * name ) {
    ritual_object_t *rv;
    if( !rht_lookup_str( &env->table, name, &rv ) ) {
        return rv;
    }
    if( env->parent ) {
        return ritual_env_lookup( inst, env->parent, name );
    }
    return 0;
}

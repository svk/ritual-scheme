#include "ritual_env.h"

#include "ritual_object.h"

#include "ritual_eval.h"
#include "ritual_error.h"

#include "ritual_basic_types.h"

void ritual_env_init_root(struct ritual_instance * inst,
                          struct ritual_env *rv ) {
    RITUAL_ASSERT( inst, rv, "null pointer passed to ritual_env_init_root" );
    rv->parent = 0;

    if( rht_table_init( &rv->table, RITUAL_ENV_SIZE_ROOT ) ) {
        ritual_error( inst, "unable to allocate root environment table" );
    }
    rv->table.memory_context = inst;
    rv->table.ht_alloc = (void* (*)(void*,int)) ritual_alloc;
    rv->table.ht_free = (void (*)(void*,void*)) ritual_free;
}

void ritual_env_init_sub(struct ritual_instance *inst,
                         struct ritual_env *rv,
                         struct ritual_env *parent) {
    RITUAL_ASSERT( inst, rv, "null pointer passed to ritual_env_init_sub" );
    rv->parent = parent;

    if( rht_table_init( &rv->table, RITUAL_ENV_SIZE_SUB ) ) {
        ritual_error( inst, "unable to allocate subenvironment table" );
    }
    rv->table.memory_context = inst;
    rv->table.ht_alloc = (void* (*)(void*,int)) ritual_alloc;
    rv->table.ht_free = (void (*)(void*,void*)) ritual_free;
}

void ritual_env_destroy( struct ritual_instance *inst,
                         void *venv ) {
    RITUAL_ASSERT( inst, RITUAL_TYPE( venv ) == RTYPE_ENVIRONMENT, "ritual_env_destroy got object of wrong type" );
    struct ritual_env *env = (struct ritual_env*) venv;
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
    void *rv;
    // Can't use the simple lookup because () is perfectly valid
    if( !rht_lookup_str( &env->table, name, &rv ) ) {
        return rv;
    }
    if( env->parent ) {
        return ritual_env_lookup( inst, env->parent, name );
    }
    ritual_error( inst, "lookup failed: \"%s\"", name );
    return 0; // impossible
}

struct ritual_env * ritual_let_env( struct ritual_instance *inst,
                                    struct ritual_env *env,
                                    struct ritual_pair *bindlist ) {
    struct ritual_env *rv = (struct ritual_env*) ritual_alloc_typed_object( inst, RTYPE_ENVIRONMENT, sizeof *rv );
    if( !rv ) {
        ritual_error( inst, "unable to allocate let subenvironment (out of memory)" );
    }
    ritual_env_init_sub( inst, rv, env );

    while( bindlist ) {
        struct ritual_pair * symdeflist = (struct ritual_pair*) ritual_list_next( inst, &bindlist );
        struct ritual_symbol * sym = (struct ritual_symbol*) ritual_list_next( inst, &symdeflist );
        ritual_object_t * def = ritual_list_next( inst, &symdeflist );
        ritual_list_assert_end( inst, symdeflist );
        if( RITUAL_TYPE( sym ) != RTYPE_SYMBOL ) {
            ritual_error( inst, "expected symbol as name of variable, got \"%s\"", ritual_typename( sym ) );
        }

        ritual_env_define( inst, rv, sym->name,
                           ritual_eval( inst, env, def ) );
    }

    return rv;
}

struct ritual_env * ritual_let_star_env( struct ritual_instance *inst,
                                         struct ritual_env *env,
                                         struct ritual_pair *bindlist ) {
    struct ritual_env *rv = (struct ritual_env*) ritual_alloc_typed_object( inst, RTYPE_ENVIRONMENT, sizeof *rv );
    if( !rv ) {
        ritual_error( inst, "unable to allocate let* subenvironment (out of memory)" );
    }
    ritual_env_init_sub( inst, rv, env );

    while( bindlist ) {
        struct ritual_pair * symdeflist = (struct ritual_pair*) ritual_list_next( inst, &bindlist );
        struct ritual_symbol * sym = (struct ritual_symbol*) ritual_list_next( inst, &symdeflist );
        ritual_object_t * def = ritual_list_next( inst, &symdeflist );
        ritual_list_assert_end( inst, symdeflist );
        if( RITUAL_TYPE( sym ) != RTYPE_SYMBOL ) {
            ritual_error( inst, "expected symbol as name of variable, got \"%s\"", ritual_typename( sym ) );
        }

        ritual_env_define( inst, rv, sym->name,
                           ritual_eval( inst, rv, def ) );
    }

    return rv;
}

struct ritual_env * ritual_letrec_env( struct ritual_instance *inst,
                                       struct ritual_env *env,
                                       struct ritual_pair *bindlist ) {
    ritual_error( inst, "letrec not yet implemented" );
    return 0;
}

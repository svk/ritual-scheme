#include "ritual_envproc.h"

#include "ritual_eval.h"
#include "ritual_error.h"

#include <stdarg.h>

#include "ritual_rl3_bridge.h"

#include <assert.h>

static char ritual_sentry_rest[] = "&rest";

const char * RSENT_REST = ritual_sentry_rest;

struct ritual_env * ritual_envproc_bind( struct ritual_instance *inst,
                                         struct ritual_env *env,
                                         ritual_object_t *params,
                                         struct ritual_pair *uneval_values ) {
    struct ritual_env *rv = (struct ritual_env*) ritual_alloc_typed_object( inst, RTYPE_ENVIRONMENT, sizeof *rv );
    if( !rv ) {
        ritual_error( inst, "unable to allocate envproc subenvironment (out of memory)" );
    }
    ritual_env_init_sub( inst, rv, 0 ); // a small environment with no actual root

    ritual_object_t *curpar = params;
    struct ritual_pair *curval = uneval_values;

    while( curpar ) switch( RITUAL_TYPE( curpar ) ) {
        case RTYPE_PAIR:
            {
                struct ritual_pair *pair = rconvto_pair( inst, curpar );
                struct ritual_symbol *sym = rconvto_symbol( inst, pair->car );
                ritual_object_t *val = curval->car;

                curpar = pair->cdr;
                curval = rconvto_list( inst, curval->cdr );

                ritual_env_define( inst, rv, sym->name, ritual_eval(inst, env, val) );

                break;
            }
        case RTYPE_SYMBOL:
            {
                struct ritual_symbol *sym = rconvto_symbol( inst, curpar );

                ritual_env_define( inst, rv, sym->name, rconvfrom_pair( inst, ritual_mapeval( inst, env, curval ) ) );

                curpar = 0;
                curval = 0;

                break;
            }
        case RTYPE_NULL:
            ritual_error( inst, "not enough arguments" );
        default:
            ritual_error( inst, "unexpected type" );
    }
    if( curval ) {
        ritual_error( inst, "too many arguments" );
    }

    return rv;
}

ritual_object_t * renvp_ritual_typenames( struct ritual_instance *inst,
                                          struct ritual_env *env ) {
    struct ritual_pair *pair = rconvto_list( inst, ritual_env_lookup( inst, env, "rest" ) );
    struct ritual_pair *rv = 0;
    struct ritual_pair **tail = &rv;
    while( pair ) {
        ritual_object_t *obj = ritual_list_next( inst, &pair );
        ritual_object_t *name = (ritual_object_t*) ritual_ascii_string_create( inst, ritual_typename( obj ) );
        *tail = ritual_pair_create( inst, name, 0 );
        tail = (struct ritual_pair**) &(*tail)->cdr;
    }
    return (ritual_object_t*) rv;
}

ritual_object_t * renvp_ritual_typename( struct ritual_instance *inst,
                                         struct ritual_env *env ) {
    ritual_object_t * obj = ritual_env_lookup( inst, env, "object" );
    return (ritual_object_t*) ritual_ascii_string_create( inst, ritual_typename( obj ) );
}

void ritual_define_envproc( struct ritual_instance *inst,
                                       struct ritual_env *env,
                                       ritual_envproc_t func,
                                       const char *name,
                                       ... ) {
    struct ritual_envproc *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_ENVPROC, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->procedure = func;

    struct ritual_pair *params = 0, *current = 0;
    va_list args;
    const char *argname;
    va_start( args, name );
    while( (argname = va_arg( args, const char* )) ) {
        if( argname == RSENT_REST ) {
            argname = va_arg( args, const char * );

            ritual_object_t * sym = rconvfrom_symbol( inst, ritual_symbol_create( inst, argname ) );
            if( current ) {
                current->cdr = sym;
            } else {
                params = (struct ritual_pair*) sym; // ugh
            }

            argname = va_arg( args, const char * );
            assert( !argname );
            break;
        }
        ritual_object_t * sym = rconvfrom_symbol( inst, ritual_symbol_create( inst, argname ) );
        struct ritual_pair *moreparams = ritual_pair_create( inst, sym, 0 );
        if( current ) {
            current->cdr = rconvfrom_pair( inst, moreparams );
        } else {
            params = moreparams;
        }
        current = moreparams;
    }
    va_end( args );

    rv->params = (ritual_object_t*) params; // ew

    ritual_rl3_compile_envproc( inst, rv );

    ritual_env_define( inst, env, name, (ritual_object_t*) rv );
}

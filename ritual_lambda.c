// <3 λ

#include "ritual_lambda.h"
#include "ritual_object.h"
#include "ritual_env.h"

#include "ritual_eval.h"

#include "ritual_error.h"

void ritual_print_lambda_proc( struct ritual_instance *inst,
                               struct ritual_flo *flo,
                               void *obj ) {
    rflo_putstring( flo, "#<lambda procedure>" );
}


struct ritual_env * ritual_lambda_env( struct ritual_instance * inst,
                                       struct ritual_env *env,
                                       struct ritual_env *parent,
                                       ritual_object_t *argsyms,
                                       struct ritual_pair * arglist ) {
    struct ritual_env *rv = (struct ritual_env*) ritual_alloc_typed_object( inst, RTYPE_ENVIRONMENT, sizeof *rv );
    if( !rv ) {
        ritual_error( inst, "unable to allocate lambda subenvironment (out of memory)" );
    }
    ritual_env_init_sub( inst, rv, parent );

    void *nextsym = argsyms;
    struct ritual_pair *nextarg = arglist;

    if( nextarg && RITUAL_TYPE( nextarg ) != RTYPE_PAIR ) {
        ritual_env_destroy( inst, rv );
        ritual_free( inst, rv );
        ritual_error( inst, "argument list must be proper list" );
    }

    while( nextsym && nextarg ) {
        switch( RITUAL_TYPE(nextsym) ) {
            case RTYPE_PAIR:
                {
                    struct ritual_pair *currentpair = nextsym;
                    RITUAL_ASSERT( inst, RITUAL_TYPE( currentpair->car ) == RTYPE_SYMBOL, "formals list must consist of symbols" );
                    struct ritual_symbol *currentsym = (struct ritual_symbol*) currentpair->car;

                    fprintf( stderr, "changing definition of %s in %p\n", currentsym->name, rv );
                    ritual_env_define( inst, rv, currentsym->name,
                                       ritual_eval( inst, env, nextarg->car ) );

                    nextsym = currentpair->cdr;
                    if( nextarg->cdr && RITUAL_TYPE( nextarg->cdr ) != RTYPE_PAIR ) {
                        ritual_env_destroy( inst, rv );
                        ritual_free( inst, rv );
                        ritual_error( inst, "argument list must be proper list" );
                    }
                    nextarg = (struct ritual_pair*) nextarg->cdr;
                }
                break;
            case RTYPE_SYMBOL:
                {
                    struct ritual_symbol *finalsym = nextsym;
                    ritual_env_define( inst, rv, finalsym->name,
                                       (ritual_object_t*) ritual_mapeval( inst, env, nextarg ) );
                    nextsym = 0;
                    nextarg = 0;
                }
                break;
            default:
                ritual_env_destroy( inst, rv );
                ritual_free( inst, rv );
                ritual_error( inst, "invalid lambda formals list" );
        }
    }

    if( !nextsym && nextarg ) {
        ritual_env_destroy( inst, rv );
        ritual_free( inst, rv );
        ritual_error( inst, "too many arguments" );
    }

    if( !nextarg && nextsym ) {
        ritual_env_destroy( inst, rv );
        ritual_free( inst, rv );
        ritual_error( inst, "too few arguments" );
    }

    return rv;
}

struct ritual_lambda_proc * ritual_lambda_create( struct ritual_instance *inst,
                                                  struct ritual_env *env,
                                                  ritual_object_t *formals,
                                                  struct ritual_pair *body ) {
    /* TODO verify listproperness etc */
    if( !body ) {
        ritual_error( inst, "cannot define bodiless lambda" );
    }

    struct ritual_lambda_proc *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_LAMBDA_PROC, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    /* Verify all the tricky/costly stuff such as properlistness */
    rv->argsyms = formals;
    rv->body = body;
    rv->parent = env;
    return rv;
                    
}

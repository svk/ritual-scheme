// <3 λ

#include "ritual_lambda.h"
#include "ritual_object.h"
#include "ritual_env.h"

#include "ritual_eval.h"

#include "ritual_error.h"

struct ritual_env * ritual_lambda_subenv( struct ritual_instance * inst,
                                          struct ritual_env *env,
                                          struct ritual_lambda_proc *proc,
                                          struct ritual_pair * arglist ) {
    struct ritual_env *rv = ritual_alloc( inst, sizeof *rv );
    if( !rv ) {
        ritual_error( inst, "unable to allocate lambda subenvironment (out of memory)" );
    }
    ritual_env_init_sub( inst, rv, env );

    void *nextsym = proc->argsyms;
    struct ritual_pair *nextarg = arglist;

    if( RITUAL_TYPE( nextarg ) != RTYPE_PAIR ) {
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

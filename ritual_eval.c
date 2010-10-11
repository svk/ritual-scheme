#include "ritual_eval.h"

#include "ritual_error.h"
#include "ritual_basic_types.h"
#include "ritual_native_proc.h"
#include "ritual_lambda.h"

#include "ritual_env.h"

#include "ritual_keyword.h"

ritual_object_t * ritual_eval( struct ritual_instance *inst,
                               struct ritual_env *env,
                               ritual_object_t *value ) {
    struct ritual_pair * body = 0;
    while( 1 ) {
        /* Note for the future (multithreading): this seems like
         * a great place for a pthread_yield(), and even a great
         * place to stop and check whether we've been suspended
         * (and wait on a condition variable) or terminated.
         * (Of course at that point (internal multithreading)
         * most procedures will need to take a per-thread
         * argument "thread" similar to inst and env.) */

        if( body ) {
            while( body->cdr ) {
                if( RITUAL_TYPE( body->cdr ) != RTYPE_PAIR ) {
                    ritual_error( inst, "body must be proper list" );
                }
                /* TCO is not possible here since this is not a TC;
                 * so using the C stack is fine. Note that we discard
                 * the value; only the side effects are important. */
                ritual_eval( inst, env, body->car );
                body = (struct ritual_pair*) body->cdr;
            }
            value = body->car;
            body = 0;
        }
        /* The remaining is a tail call. If there is more to be
         * evaluated after this switch:
         *   - If there is a list of expressions, assign it to the
         *     body variable.
         *   - If there is a single expression, assign it to the
         *     value variable.
         *   - In either case, if a new environment is required,
         *     generate it as a subenvironment of env->parent
         *     (though using env to e.g. evaluate arguments)
         *     and then assign it to env.
         */
        switch( RITUAL_TYPE(value) ) {
            case RTYPE_NULL:
            case RTYPE_ASCII_STRING:
            case RTYPE_PROCEDURE:
            case RTYPE_BOOLEAN:
            case RTYPE_NATIVE_INTEGER:
            case RTYPE_ASCII_CHAR:
            case RTYPE_VECTOR:
            case RTYPE_PORT:
            case RTYPE_BIG_INTEGER:
            case RTYPE_BIG_RATIONAL:
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
                    ritual_object_t *proc = ritual_eval( inst, env, pair->car );
                    
                    switch( RITUAL_TYPE ( proc ) ) {
                        case RTYPE_KEYWORD:
                            {
                                struct ritual_keyword *keyword = (struct ritual_keyword*) proc;
                                if( keyword->rnp ) {
                                    if( pair->cdr && RITUAL_TYPE( pair->cdr ) != RTYPE_PAIR ) {
                                        ritual_error( inst, "must pass proper list as argument list" );
                                    }
                                    return keyword->rnp( inst, env, (struct ritual_pair*) pair->cdr );
                                } else switch( keyword->value ) {
                                    case RKW_LET:
                                        {
                                            ritual_list_next( inst, &pair );
                                            ritual_object_t *bindings = ritual_list_next( inst, &pair );
                                            env = ritual_let_env( inst, env,
                                                                  rconvto_list( inst, bindings ) );
                                            body = pair;
                                            break;
                                        }
                                    case RKW_LETREC:
                                        {
                                            ritual_list_next( inst, &pair );
                                            ritual_object_t *bindings = ritual_list_next( inst, &pair );
                                            env = ritual_letrec_env( inst, env,
                                                                     rconvto_list( inst, bindings ) );
                                            body = pair;
                                            break;
                                        }
                                    case RKW_LET_STAR:
                                        {
                                            ritual_list_next( inst, &pair );
                                            ritual_object_t *bindings = ritual_list_next( inst, &pair );
                                            env = ritual_let_star_env( inst, env, rconvto_list( inst, bindings ) );
                                            body = pair;
                                            break;
                                        }
                                    case RKW_IF:
                                        {
                                            ritual_list_next( inst, &pair );
                                            ritual_object_t *condition = ritual_list_next( inst, &pair );
                                            ritual_object_t *clause_true = ritual_list_next( inst, &pair );
                                            ritual_object_t *clause_false = 0;
                                            if( pair ) {
                                                clause_false = ritual_list_next( inst, &pair );
                                                ritual_list_assert_end( inst, pair );
                                            }
                                            if( ritual_eval( inst, env, condition ) != inst->scheme_false ) {
                                                value = clause_true;
                                            } else {
                                                value = clause_false;
                                            }
                                            break;
                                        }
                                    case RKW_BEGIN:
                                        body = (struct ritual_pair*) pair->cdr;
                                        break;
                                    case RKW_OR:
                                        {
                                            ritual_list_next( inst, &pair );
                                            if( !pair ) {
                                                return inst->scheme_false;
                                            }
                                            while( ritual_list_has_cdr( inst, pair ) ) {
                                                ritual_object_t *condition = ritual_list_next( inst, &pair );
                                                if( RITUAL_AS_BOOLEAN( inst, condition ) ) {
                                                    return condition;
                                                }
                                            }
                                            value = ritual_list_next( inst, &pair );
                                            ritual_list_assert_end( inst, pair );
                                        }
                                        break;
                                    case RKW_AND:
                                        {
                                            ritual_list_next( inst, &pair );
                                            if( !pair ) {
                                                return inst->scheme_true;
                                            }
                                            while( ritual_list_has_cdr( inst, pair ) ) {
                                                ritual_object_t *condition = ritual_list_next( inst, &pair );
                                                if( !RITUAL_AS_BOOLEAN( inst, condition ) ) {
                                                    return condition;
                                                }
                                            }
                                            value = ritual_list_next( inst, &pair );
                                            ritual_list_assert_end( inst, pair );
                                        }
                                        break;
                                    default:
                                        ritual_error( inst, "unknown keyword" );
                                }
                                break;
                            }
                        case RTYPE_LAMBDA_PROC:
                            {
                                struct ritual_lambda_proc *lambda_proc = (struct ritual_lambda_proc*) proc;
                                body = lambda_proc->body;
                                if( pair->cdr && RITUAL_TYPE( pair->cdr ) != RTYPE_PAIR ) {
                                    ritual_error( inst, "must pass proper list as argument list" );
                                }
                                env = ritual_lambda_env( inst, env,
                                                         lambda_proc->parent,
                                                         lambda_proc->argsyms,
                                                         (struct ritual_pair*) pair->cdr );
                                break;
                            }
                        case RTYPE_NATIVE_PROC:
                            {
                                struct ritual_native_proc *native_proc = (struct ritual_native_proc*) proc;
                                if( pair->cdr && RITUAL_TYPE( pair->cdr ) != RTYPE_PAIR ) {
                                    ritual_error( inst, "must pass proper list as argument list" );
                                }
                                return native_proc->procedure( inst, env, (struct ritual_pair*) pair->cdr );
                            }
                        default:
                            ritual_error( inst, "type \"%s\" is not callable", ritual_typename( proc ) );
                    }
                    break;
                }
            case RTYPE_KEYWORD:
                ritual_error( inst, "syntax error; cannot evaluate keyword" );
            default:
                ritual_error( inst, "evaluation of type \"%s\" not implemented", ritual_typename( value ) );
        }
    }
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
        l = (struct ritual_pair*) l->cdr;

    }
    return rv;
}

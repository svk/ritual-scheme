#include "ritual_arithmetic.h"

#include "ritual_basic_types.h"
#include "ritual_bignum.h"


#include "ritual_error.h"
#include "ritual_eval.h"

#include <gmp.h>
#include <stdint.h>

    /* In retrospect, "simple add" is a bit of an ironic name for this
     * class of functions. (The intention was simple as opposed to
     * complex (but not real, since that means something entirely
     * different in our scheme...)) */

ritual_object_t* rnum_mpq_simple_add(
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpq_t *acc,
        struct ritual_pair *list ) {
    mpq_t tempq;
    mpq_init( tempq );

    while( list ) {
        ritual_object_t * next = ritual_eval( inst, env,
                                              ritual_list_peek(inst, list ) );
        switch( RITUAL_TYPE( next ) ) {
            case RTYPE_NATIVE_INTEGER:
                {
                    struct ritual_native_int *nint = rconvto_native_int( inst, next );
                    mpz_set_si( mpq_numref( tempq ), nint->value );
                    mpq_add( *acc, *acc, tempq );
                    break;
                }
            case RTYPE_BIG_INTEGER:
                {
                    struct ritual_big_int *nint = rconvto_big_int( inst, next );
                    mpz_set( mpq_numref( tempq ), nint->value );
                    mpq_add( *acc, *acc, tempq );
                    break;
                }
            case RTYPE_BIG_RATIONAL:
                {
                    struct ritual_big_rational *rint = rconvto_big_rational( inst, next );
                    mpq_add( *acc, *acc, rint->value );
                    break;
                }
            default:
                ritual_error( inst, "unexpected type for simple arithmetic: \"%s\"", ritual_typename( next ) );
        }
        ritual_list_next( inst, &list );
    }

    mpq_clear( tempq );

    if( mpz_cmp_si( mpq_denref( *acc ), 1 ) == 0 ) {
        if( mpz_cmp_si( mpq_numref( *acc ), RITUAL_NATIVE_INT_MIN ) > 0 &&
            mpz_cmp_si( mpq_numref( *acc ), RITUAL_NATIVE_INT_MAX ) < 0 ) {
            long int si = mpz_get_si( mpq_numref( *acc ) );
            mpq_clear( *acc );
            return (ritual_object_t*) ritual_native_int_create( inst, si );
        }

            // _not_ create_copy!
        ritual_object_t* rv = (ritual_object_t*) ritual_big_int_create( inst, mpq_numref( *acc ) );
        mpq_clear( *acc );
        return rv;
    }
    return (ritual_object_t*) ritual_big_rational_create_copy( inst, acc );
}

ritual_object_t* rnum_mpz_simple_add( 
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpz_t *acc,
        struct ritual_pair *list ) {
    while( list ) {
        ritual_object_t * next = ritual_eval( inst, env,
                                              ritual_list_peek( inst, list ) );
        switch( RITUAL_TYPE( next ) ) {
            case RTYPE_NATIVE_INTEGER:
                {
                    struct ritual_native_int *nint = rconvto_native_int( inst, next );
                    if( nint->value >= 0 ) {
                        mpz_add_ui( *acc, *acc, (unsigned long int) nint->value );
                    } else {
                        mpz_add_ui( *acc, *acc, ((unsigned long int) -((long int) nint->value)) );
                    }
                    break;
                }
            case RTYPE_BIG_INTEGER:
                {
                    struct ritual_big_int *bint = rconvto_big_int( inst, next );
                    mpz_add( *acc, *acc, bint->value );
                    break;
                }
            case RTYPE_BIG_RATIONAL:
                {
                    mpq_t bigrat;
                    mpq_init( bigrat );
                    mpq_set_z( bigrat, *acc );
                    mpz_clear( *acc );
                    return rnum_mpq_simple_add( inst, env, &bigrat, list );
                }
            default:
                ritual_error( inst, "unexpected type for simple arithmetic: \"%s\"", ritual_typename( next ) );

        }
        ritual_list_next( inst, &list );
    }

    if( mpz_cmp_si( *acc, RITUAL_NATIVE_INT_MIN ) > 0 &&
        mpz_cmp_si( *acc, RITUAL_NATIVE_INT_MAX ) < 0 ) {
        long int si = mpz_get_si( *acc );
        mpz_clear( *acc );
        return (ritual_object_t*) ritual_native_int_create( inst, si );
    }
    return (ritual_object_t*) ritual_big_int_create_copy( inst, acc );
}

ritual_object_t* rnum_native_int_simple_add(
        struct ritual_instance *inst,
        struct ritual_env *env,
        int32_t *acc,
        struct ritual_pair *list ) {
    while( list ) {
        ritual_object_t * next = ritual_eval( inst, env,
                                              ritual_list_peek(inst, list ) );
        switch( RITUAL_TYPE( next ) ) {
            case RTYPE_NATIVE_INTEGER:
                {
                    struct ritual_native_int * nint = rconvto_native_int( inst, next );
                    int64_t sum = *acc + nint->value;
                    int32_t value = (int32_t) sum;
                    ritual_list_next( inst, &list );
                    if( value == sum ) {
                        *acc = value;
                    } else {
                        mpz_t bigint;
                        mpz_init_set_si( bigint, sum );
                        return rnum_mpz_simple_add( inst, env, &bigint, list );
                    }
                    break;
                }
            case RTYPE_BIG_INTEGER:
                {
                    mpz_t bigint;
                    mpz_init_set_si( bigint, *acc );
                    return rnum_mpz_simple_add( inst, env, &bigint, list );
                }
            case RTYPE_BIG_RATIONAL:
                {
                    mpq_t bigrat;
                    mpq_init( bigrat );
                    mpq_set_si( bigrat, *acc, 1 );
                    return rnum_mpq_simple_add( inst, env, &bigrat, list );
                }
            default:
                ritual_error( inst, "unexpected type for simple arithmetic: \"%s\"", ritual_typename( next ) );
        }
    }

    return (ritual_object_t*) ritual_native_int_create( inst, *acc );
}

ritual_object_t* rnp_add( struct ritual_instance* inst,
                          struct ritual_env* env,
                          struct ritual_pair * list ) {
    int32_t value = 0;
    return rnum_native_int_simple_add( inst, env, &value, list );
}

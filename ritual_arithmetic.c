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

#define RNUM_ARITHMETIC_BEGIN \
    while( list ) { \
        ritual_object_t * next = ritual_eval( inst, env, \
                                              ritual_list_peek(inst, list ) ); \
        switch( RITUAL_TYPE( next ) ) { \
            default: \
                ritual_error( inst, "unexpected type for simple arithmetic: \"%s\"", ritual_typename( next ) );

#define RNUM_ARITHMETIC_NOEVAL_BEGIN \
    while( list ) { \
        ritual_object_t * next = ritual_list_peek(inst, list );  \
        switch( RITUAL_TYPE( next ) ) { \
            default: \
                ritual_error( inst, "unexpected type for simple arithmetic: \"%s\"", ritual_typename( next ) );


#define RNUM_ARITHMETIC_END \
        } \
        ritual_list_next( inst, &list ); \
    }

#define RNUM_CASE_NATIVE_INTEGER_AS(name) \
        case RTYPE_NATIVE_INTEGER: \
            { \
                struct ritual_native_int *name = rconvto_native_int( inst, next );

#define RNUM_CASE_BIG_INTEGER_AS(name) \
        case RTYPE_BIG_INTEGER: \
            { \
                struct ritual_big_int *name = rconvto_big_int( inst, next );

#define RNUM_CASE_BIG_RATIONAL_AS(name) \
        case RTYPE_BIG_RATIONAL: \
            { \
                struct ritual_big_rational *name = rconvto_big_rational( inst, next );

#define RNUM_CASE_END break; }

ritual_object_t* rnum_mpz_downgrade(struct ritual_instance *inst, mpz_t *acc) {
    if( mpz_cmp_si( *acc, RITUAL_NATIVE_INT_MIN ) > 0 &&
        mpz_cmp_si( *acc, RITUAL_NATIVE_INT_MAX ) < 0 ) {
        long int si = mpz_get_si( *acc );
        mpz_clear( *acc );
        return (ritual_object_t*) ritual_native_int_create( inst, si );
    }
    return (ritual_object_t*) ritual_big_int_create_copy( inst, acc );
}

ritual_object_t* rnum_mpq_downgrade(struct ritual_instance *inst, mpq_t *acc) {
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

    /* Addition */
ritual_object_t* rnum_mpq_simple_add(
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpq_t *acc,
        struct ritual_pair *list ) {
    mpq_t tempq;
    mpq_init( tempq );

    RNUM_ARITHMETIC_BEGIN

        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            mpz_set_si( mpq_numref( tempq ), nint->value );
            mpq_add( *acc, *acc, tempq );
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_set( mpq_numref( tempq ), bint->value );
            mpq_add( *acc, *acc, tempq );
        } RNUM_CASE_END

        RNUM_CASE_BIG_RATIONAL_AS(rint) {
            mpq_add( *acc, *acc, rint->value );
        } RNUM_CASE_END

    RNUM_ARITHMETIC_END

    mpq_clear( tempq );

    return rnum_mpq_downgrade( inst, acc );
}

ritual_object_t* rnum_mpz_simple_add( 
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpz_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            if( nint->value >= 0 ) {
                mpz_add_ui( *acc, *acc, (unsigned long int) nint->value );
            } else {
                mpz_add_ui( *acc, *acc, ((unsigned long int) -((long int) nint->value)) );
            }
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_add( *acc, *acc, bint->value );
        } RNUM_CASE_END

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_z( bigrat, *acc );
            mpz_clear( *acc );
            return rnum_mpq_simple_add( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return rnum_mpz_downgrade( inst, acc );
}

ritual_object_t* rnum_native_int_simple_add(
        struct ritual_instance *inst,
        struct ritual_env *env,
        int32_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            int64_t sum = *acc + nint->value;
            int32_t value = (int32_t) sum;
            if( value == sum ) {
                *acc = value;
            } else {
                mpz_t bigint;
                mpz_init_set_si( bigint, sum );
                ritual_list_next( inst, &list );
                return rnum_mpz_simple_add( inst, env, &bigint, list );
            }
        } RNUM_CASE_END

        case RTYPE_BIG_INTEGER: {
            mpz_t bigint;
            mpz_init_set_si( bigint, *acc );
            return rnum_mpz_simple_add( inst, env, &bigint, list );
        }

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_si( bigrat, *acc, 1 );
            return rnum_mpq_simple_add( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return (ritual_object_t*) ritual_native_int_create( inst, *acc );
}

ritual_object_t* rnp_add( struct ritual_instance* inst,
                          struct ritual_env* env,
                          struct ritual_pair * list ) {
    int32_t value = 0;
    return rnum_native_int_simple_add( inst, env, &value, list );
}

    /* Subtraction */
ritual_object_t* rnum_mpq_simple_sub(
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpq_t *acc,
        struct ritual_pair *list ) {
    mpq_t tempq;
    mpq_init( tempq );

    RNUM_ARITHMETIC_BEGIN

        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            mpz_set_si( mpq_numref( tempq ), nint->value );
            mpq_sub( *acc, *acc, tempq );
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_set( mpq_numref( tempq ), bint->value );
            mpq_sub( *acc, *acc, tempq );
        } RNUM_CASE_END

        RNUM_CASE_BIG_RATIONAL_AS(rint) {
            mpq_sub( *acc, *acc, rint->value );
        } RNUM_CASE_END

    RNUM_ARITHMETIC_END

    mpq_clear( tempq );

    return rnum_mpq_downgrade( inst, acc );
}

ritual_object_t* rnum_mpz_simple_sub( 
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpz_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            if( nint->value >= 0 ) {
                mpz_sub_ui( *acc, *acc, (unsigned long int) nint->value );
            } else {
                mpz_add_ui( *acc, *acc, ((unsigned long int) -((long int) nint->value)) );
            }
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_sub( *acc, *acc, bint->value );
        } RNUM_CASE_END

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_z( bigrat, *acc );
            mpz_clear( *acc );
            return rnum_mpq_simple_sub( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return rnum_mpz_downgrade( inst, acc );
}

ritual_object_t* rnum_native_int_simple_sub(
        struct ritual_instance *inst,
        struct ritual_env *env,
        int32_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            int64_t sum = *acc - nint->value;
            int32_t value = (int32_t) sum;
            if( value == sum ) {
                *acc = value;
            } else {
                mpz_t bigint;
                mpz_init_set_si( bigint, sum );
                ritual_list_next( inst, &list );
                return rnum_mpz_simple_sub( inst, env, &bigint, list );
            }
        } RNUM_CASE_END

        case RTYPE_BIG_INTEGER: {
            mpz_t bigint;
            mpz_init_set_si( bigint, *acc );
            return rnum_mpz_simple_sub( inst, env, &bigint, list );
        }

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_si( bigrat, *acc, 1 );
            return rnum_mpq_simple_sub( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return (ritual_object_t*) ritual_native_int_create( inst, *acc );
}

ritual_object_t* rnp_sub( struct ritual_instance* inst,
                          struct ritual_env* env,
                          struct ritual_pair * list ) {
    if( !ritual_list_has_cdr( inst, list ) ) {
        // (- x) is negation, not subtraction
        int32_t value = 0;
        return rnum_native_int_simple_sub( inst, env, &value, list );
    }
    ritual_object_t* first = ritual_eval( inst, env, ritual_list_next( inst, &list ) );
    switch( RITUAL_TYPE( first ) ) {
        case RTYPE_NATIVE_INTEGER:
            {
                struct ritual_native_int * native_int = rconvto_native_int( inst, first );
                return rnum_native_int_simple_sub( inst, env, &native_int->value, list );
            }
        case RTYPE_BIG_INTEGER:
            {
                struct ritual_big_int * big_int = rconvto_big_int( inst, first );
                mpz_t bigint;
                mpz_init_set( bigint, big_int->value );
                return rnum_mpz_simple_sub( inst, env, &bigint, list );
            }
        case RTYPE_BIG_RATIONAL:
            {
                struct ritual_big_rational * big_rational = rconvto_big_rational( inst, first );
                mpq_t bigrational;
                mpq_init( bigrational );
                mpq_set( bigrational, big_rational->value );
                return rnum_mpq_simple_sub( inst, env, &bigrational, list );
            }
        default:
            ritual_error( inst, "unexpected type: \"%s\"", ritual_typename( first ) );
            return 0;
    }
}

    /* Multiplication */
ritual_object_t* rnum_mpq_simple_mul(
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpq_t *acc,
        struct ritual_pair *list ) {
    mpq_t tempq;
    mpq_init( tempq );

    RNUM_ARITHMETIC_BEGIN

        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            mpz_set_si( mpq_numref( tempq ), nint->value );
            mpq_mul( *acc, *acc, tempq );
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_set( mpq_numref( tempq ), bint->value );
            mpq_mul( *acc, *acc, tempq );
        } RNUM_CASE_END

        RNUM_CASE_BIG_RATIONAL_AS(rint) {
            mpq_mul( *acc, *acc, rint->value );
        } RNUM_CASE_END

    RNUM_ARITHMETIC_END

    mpq_clear( tempq );

    return rnum_mpq_downgrade( inst, acc );
}

ritual_object_t* rnum_mpz_simple_mul( 
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpz_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            if( nint->value >= 0 ) {
                mpz_mul_ui( *acc, *acc, (unsigned long int) nint->value );
            } else {
                mpz_mul_ui( *acc, *acc, ((unsigned long int) -((long int) nint->value)) );
            }
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_mul( *acc, *acc, bint->value );
        } RNUM_CASE_END

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_z( bigrat, *acc );
            mpz_clear( *acc );
            return rnum_mpq_simple_mul( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return rnum_mpz_downgrade( inst, acc );
}

ritual_object_t* rnum_native_int_simple_mul(
        struct ritual_instance *inst,
        struct ritual_env *env,
        int32_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            int64_t sum = *acc * nint->value;
            int32_t value = (int32_t) sum; // verify: is this test still safe?
            if( value == sum ) {
                *acc = value;
            } else {
                mpz_t bigint;
                mpz_init_set_si( bigint, sum );
                ritual_list_next( inst, &list );
                return rnum_mpz_simple_mul( inst, env, &bigint, list );
            }
        } RNUM_CASE_END

        case RTYPE_BIG_INTEGER: {
            mpz_t bigint;
            mpz_init_set_si( bigint, *acc );
            return rnum_mpz_simple_mul( inst, env, &bigint, list );
        }

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_si( bigrat, *acc, 1 );
            return rnum_mpq_simple_mul( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return (ritual_object_t*) ritual_native_int_create( inst, *acc );
}

ritual_object_t* rnp_mul( struct ritual_instance* inst,
                          struct ritual_env* env,
                          struct ritual_pair * list ) {
    int32_t value = 1;
    return rnum_native_int_simple_mul( inst, env, &value, list );
}

    /* Equality */
ritual_object_t* rnum_mpq_simple_eqp(
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpq_t *acc,
        struct ritual_pair *list ) {
    mpq_t tempq;
    mpq_init( tempq );

    RNUM_ARITHMETIC_NOEVAL_BEGIN

        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            mpz_set_si( mpq_numref( tempq ), nint->value );
            if( mpq_cmp( *acc, tempq ) ) {
                mpq_clear( *acc );
                return inst->scheme_false;
            }
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            mpz_set( mpq_numref( tempq ), bint->value );
            if( mpq_cmp( *acc, tempq ) ) {
                mpq_clear( tempq );
                mpq_clear( *acc );
                return inst->scheme_false;
            }
        } RNUM_CASE_END

        RNUM_CASE_BIG_RATIONAL_AS(rint) {
            if( mpq_cmp( *acc, rint->value ) ) {
                mpq_clear( tempq );
                mpq_clear( *acc );
                return inst->scheme_false;
            }
        } RNUM_CASE_END

    RNUM_ARITHMETIC_END

    mpq_clear( tempq );
    mpq_clear( *acc );

    return inst->scheme_true;
}

ritual_object_t* rnum_mpz_simple_eqp( 
        struct ritual_instance *inst,
        struct ritual_env *env,
        mpz_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_NOEVAL_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            if( mpz_cmp_si( *acc, nint->value ) ) {
                mpz_clear( *acc );
                return inst->scheme_false;
            }
        } RNUM_CASE_END

        RNUM_CASE_BIG_INTEGER_AS(bint) {
            if( mpz_cmp( *acc, bint->value ) ) {
                mpz_clear( *acc );
                return inst->scheme_false;
            }
        } RNUM_CASE_END

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_z( bigrat, *acc );
            mpz_clear( *acc );
            return rnum_mpq_simple_eqp( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    mpz_clear( *acc );
    return inst->scheme_true;
}

ritual_object_t* rnum_native_int_simple_eqp(
        struct ritual_instance *inst,
        struct ritual_env *env,
        int32_t *acc,
        struct ritual_pair *list ) {
    RNUM_ARITHMETIC_NOEVAL_BEGIN
        RNUM_CASE_NATIVE_INTEGER_AS(nint) {
            if( *acc != nint->value ) {
                return inst->scheme_false;
            }
        } RNUM_CASE_END

        case RTYPE_BIG_INTEGER: {
            mpz_t bigint;
            mpz_init_set_si( bigint, *acc );
            return rnum_mpz_simple_eqp( inst, env, &bigint, list );
        }

        case RTYPE_BIG_RATIONAL: {
            mpq_t bigrat;
            mpq_init( bigrat );
            mpq_set_si( bigrat, *acc, 1 );
            return rnum_mpq_simple_eqp( inst, env, &bigrat, list );
        }
    RNUM_ARITHMETIC_END

    return inst->scheme_true;
}

ritual_object_t* rnp_number_eqp( struct ritual_instance* inst,
                                 struct ritual_env* env,
                                 struct ritual_pair * list ) {
    struct ritual_pair *evald = ritual_mapeval( inst, env, list );
    ritual_object_t *first = ritual_list_next( inst, &evald );
    ritual_object_t *second = ritual_list_peek( inst, evald );
    if( !second ) {
        ritual_error( inst, "comparison to nothing" );
    }
    switch( RITUAL_TYPE( first ) ) {
        case RTYPE_NATIVE_INTEGER:
            {
                struct ritual_native_int * native_int = rconvto_native_int( inst, first );
                return rnum_native_int_simple_eqp( inst, env, &native_int->value, evald );
            }
        case RTYPE_BIG_INTEGER:
            {
                struct ritual_big_int * big_int = rconvto_big_int( inst, first );
                mpz_t bigint;
                mpz_init_set( bigint, big_int->value );
                return rnum_mpz_simple_eqp( inst, env, &bigint, evald );
            }
        case RTYPE_BIG_RATIONAL:
            {
                struct ritual_big_rational * big_rational = rconvto_big_rational( inst, first );
                mpq_t bigrational;
                mpq_init( bigrational );
                mpq_set( bigrational, big_rational->value );
                return rnum_mpq_simple_eqp( inst, env, &bigrational, evald );
            }
        default:
            ritual_error( inst, "unexpected type: \"%s\"", ritual_typename( first ) );
            return 0;
    }
}

ritual_object_t* rnp_zerop( struct ritual_instance *inst,
                            struct ritual_env *env,
                            struct ritual_pair *list ) {
    struct ritual_pair *evald = ritual_mapeval( inst, env, list );
    ritual_object_t *second = ritual_list_peek( inst, evald );
    if( !second ) {
        ritual_error( inst, "comparison to nothing" );
    }
    int32_t zeroval = 0;
    return rnum_native_int_simple_eqp( inst, env, &zeroval, evald );
}

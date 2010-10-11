#include "ritual_arithmetic.h"

#include "ritual_basic_types.h"
#include "ritual_bignum.h"

#include <gmp.h>
#include <stdint.h>

int rnum_convert_native_int( void *target,
                             ritual_simple_size_t sz,
                             ritual_simple_generality_t gen,
                             void *from ) {
    int32_t *me = (int32_t*) from;
    if( gen == RNUM_INTEGER && sz == RNUM_UNBOUNDED ) {
        mpz_t *them = (mpz_t*) target;
        mpz_set_si( *them, *me );
    } else if( gen == RNUM_RATIONAL && sz == RNUM_UNBOUNDED ) {
        mpq_t *them = (mpq_t*) target;
        mpq_set_si( *them, *me, 1 );
    } else return 1;
    return 0;
}

int rnum_convert_mpz( void *target,
                      ritual_simple_size_t sz,
                      ritual_simple_generality_t gen,
                      void *from ) {
    mpz_t *me = (mpz_t*) from;
    if( gen == RNUM_RATIONAL && sz == RNUM_UNBOUNDED ) {
        mpq_t *them = (mpq_t*) target;
        mpq_set_z( *them, *me );
    } else return 1;
    return 0;
}

int rnum_convert_mpq( void *target,
                      ritual_simple_size_t sz,
                      ritual_simple_generality_t gen,
                      void *from ) {
    mpz_t *me = (mpq_t*) from;
    return 1;
}

int rnum_init_native_int ( void *target ) {
    return 0;
}

int rnum_init_mpz( void *target ) {
    mpz_t *me = (mpz_t*) target;
    *me = malloc(sizeof *me);
    mpz_init( *me );
    return 0;
}

int rnum_init_mpq( void *target ) {
    mpq_t *me = (mpq_t*) target;
    *me = malloc(sizeof *me);
    mpq_init( *me );
    return 0;
}

int rnum_destroy_native_int ( void *target ) {
    return 0;
}

int rnum_destroy_mpz( void *target ) {
    mpz_t *me = (mpz_t*) = target;
    mpz_clear( *me );
    return 0;
}

int rnum_destroy_mpq( void *target ) {
    mpq_t *me = (mpq_t*) = target;
    mpq_clear( *me );
    return 0;
}

int rnum_native_int_copymove( void *target, void *from ) {
    int32_t *targeti = (int32_t*) target;
    int32_t *fromi = (int32_t*) from;
    *targeti = *fromi;
    return 0;
}

int rnum_mpz_copy( void *target, void *from ) {
    mpz_t *targeti = (mpz_t*) target;
    mpz_t *fromi = (mpz_t*) from;
    mpz_set( *targeti, *fromi );
    return 0;
}

int rnum_mpq_copy( void *target, void *from ) {
    mpq_t *targeti = (mpq_t*) target;
    mpq_t *fromi = (mpq_t*) from;
    mpq_set( *targeti, *fromi );
    return 0;
}

/* The idea that a shallow copy should work as a move
 * is supported only by a brief inspection of gmp.h.
 * The GMP manual says:
 *  "Using functions, macros, data types, etc. not
 *   documented in this manual is strongly discouraged.
 *   If you do so your application is guaranteed to be
 *   incompatible with future versions of GMP."
 *  :(
 * (I wonder whether I can sue or something if my
 * application turns out to be compatible after all.) */
/* Can always swap out the move for a copy if that
 * turns out not to be an empty threat. */
int rnum_mpz_move( void *target, void *from ) {
    mpz_t *targeti = (mpz_t*) target;
    mpz_t *fromi = (mpz_t*) from;
    memcpy( targeti, fromi, sizeof *targeti );
    return 0;
}

int rnum_mpq_move( void *target, void *from ) {
    mpq_t *targeti = (mpq_t*) target;
    mpq_t *fromi = (mpq_t*) from;
    memcpy( targeti, fromi, sizeof *targeti );
    return 0;
}

/* Finally some actual operations */

int rnum_native_int_add( void* target, void* from ) {
    int64_t value;
    int32_t *targeti = (int32_t*) target;
    int32_t *fromi = (int32_t*) from;
    value = *targeti + *fromi;
    *targeti = (int32_t) value;
    return value != *targeti;
}

int rnum_mpz_add( void* target, void* from ) {
    mpz_t *targeti = (mpz_t*) target;
    mpz_t *fromi = (mpz_t*) from;
    mpz_add( *targeti, *targeti, *fromi );
    return 0;
}

int rnum_mpq_add( void* target, void* from ) {
    mpq_t *targeti = (mpq_t*) target;
    mpq_t *fromi = (mpq_t*) from;
    mpq_add( *targeti, *targeti, *fromi );
    return 0;
}

ritual_object_t * rnp_add_simple( struct ritual_instance *inst,
                                  struct ritual_env *env,
                                  struct ritual_pair * list ) {
        // warning; that's zero, not a null pointer (semantically)
    void *current = (void*) 0;
    struct ritual_simple_type * current_type = rnum_get_simple_type( RNUM_BOUNDED, RNUM_INTEGER );

    while( list ) {
        ritual_object_t *element = ritual_list_next( &list );
        struct ritual_simple_type * element_type = rnum_get_simple_type_of( element );
        if( element_type == current_type ) {
            if( current_type->add( &current,
        }
        
    }

    return current_type->box_simple( inst, current );
}

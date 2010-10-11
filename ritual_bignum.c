#include "ritual_bignum.h"

#include "ritual_object.h"
#include "ritual_error.h"
#include "ritual_basic_types.h"

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

struct ritual_big_rational * ritual_big_rational_create_copy( struct ritual_instance *inst, mpq_t* value ) {
    struct ritual_big_rational *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BIG_INTEGER, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
        // this is "risky" but should work -- the semantics that may
        // be tricky to keep in mind are that the copied-from value
        // should not now be freed, ever.
        // also, technically not supported in the manual.
    memcpy( &rv->value, value, sizeof rv->value );
    return rv;
}

struct ritual_big_int * ritual_big_int_create_copy( struct ritual_instance *inst, mpz_t* value ) {
    struct ritual_big_int *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BIG_INTEGER, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
        // this is "risky" but should work -- the semantics that may
        // be tricky to keep in mind are that the copied-from value
        // should not now be freed, ever.
        // also, technically not supported in the manual.
    memcpy( &rv->value, value, sizeof rv->value );
    return rv;
}

struct ritual_big_int * ritual_big_int_create( struct ritual_instance *inst, mpz_t value ) {
    struct ritual_big_int *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BIG_INTEGER, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    mpz_init( rv->value );
    mpz_set( rv->value, value );
    return rv;
}

struct ritual_big_int * ritual_big_int_create_decimal( struct ritual_instance *inst, const char *s ) {
    struct ritual_big_int *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BIG_INTEGER, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    if( mpz_init_set_str( rv->value, s, 10 ) ) {
        mpz_clear( rv->value );
        ritual_error( inst, "unable to parse integer \"%s\"", s );
    }
    return rv;
}

void ritual_big_int_destroy( struct ritual_instance *inst, void* p) {
    struct ritual_big_int *bigint = (struct ritual_big_int*) p;
    mpz_clear( bigint->value );
}
void ritual_print_big_int( struct ritual_instance *inst,
                           struct ritual_flo *flo,
                           void *obj ) {
    // simplest way first, though this is even more dirty memory
    struct ritual_big_int *bigint = (struct ritual_big_int*) obj;
    char *s;
    gmp_asprintf( &s, "%Zd", bigint->value );
    rflo_putstring( flo, s );
    free( s ); // note depends on allocator funcs
}


struct ritual_big_rational * ritual_big_rational_create( struct ritual_instance *inst, mpq_t value ) {
    struct ritual_big_rational *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BIG_RATIONAL, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    mpq_init( rv->value );
    mpq_set( rv->value, value );
    return rv;
}

struct ritual_big_rational * ritual_big_rational_create_decimal( struct ritual_instance *inst, const char *s ) {
    struct ritual_big_rational *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BIG_RATIONAL, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    mpq_init( rv->value );
    if( mpq_set_str( rv->value, s, 10 ) ) {
        mpq_clear( rv->value );
        ritual_error( inst, "unable to parse rational \"%s\"", s );
    }
    mpq_canonicalize( rv->value );
    return rv;
}

void ritual_big_rational_destroy( struct ritual_instance *inst, void* p) {
    struct ritual_big_rational *bigint = (struct ritual_big_rational*) p;
    mpq_clear( bigint->value );
}
void ritual_print_big_rational( struct ritual_instance *inst,
                           struct ritual_flo *flo,
                           void *obj ) {
    // simplest way first, though this is even more dirty memory
    struct ritual_big_rational *bigint = (struct ritual_big_rational*) obj;
    char *s;
    gmp_asprintf( &s, "%Qd", bigint->value );
    rflo_putstring( flo, s );
    free( s ); // note depends on allocator funcs
}

ritual_object_t * ritual_string_to_number( struct ritual_instance *inst, const char *s ) {
    if( strchr( s, '/' ) ) {
        return (ritual_object_t*) ritual_big_rational_create_decimal( inst, s );
    } else {
        char *endp;
        // lli is at least 64 bits, and might be overflowed by a 64-bit number (unsigned).
        // such a number can be most compactly represented in Scheme by a hexadecimal repr.
        // that would take at least 16 hexadecimal characters
        // thus we assume that any string of at least 16 characters is too long for native
        // (which might not strictly be true for binary constants later but certainly for dec)
        // and after that assume that the lli is not overflowed.
        const int magic_constant_explained_above = 16;
        if( strlen( s ) > magic_constant_explained_above ) {
            return (ritual_object_t*) ritual_big_int_create_decimal( inst, s );
        } else {
            long long int lli = strtoll( s, &endp, 10 );
            if( *endp != '\0' ) {
                ritual_error( inst, "unable to parse integer \"%s\"", s );
            }
            const long long int llimin = RITUAL_NATIVE_INT_MIN;
            const long long int llimax = RITUAL_NATIVE_INT_MAX;
            if( lli < llimin || lli > llimax ) {
                return (ritual_object_t*) ritual_big_int_create_decimal( inst, s );
            }
            int32_t value = (int32_t) lli;
            return (ritual_object_t*) ritual_native_int_create( inst, value );
        }
    }
}

struct ritual_big_int *rconvto_big_int( struct ritual_instance *inst,
                                  ritual_object_t *obj ) {
    if( RITUAL_TYPE( obj ) != RTYPE_BIG_INTEGER ) {
        ritual_error( inst, "expected big int, got \"%s\"", ritual_typename( obj ) );
    }
    return (struct ritual_big_int*) obj;
}

ritual_object_t * rconvfrom_big_int( struct ritual_instance *inst,
                                        struct ritual_big_int *big_int ) {
    if( !big_int ) {
        ritual_error( inst, "expected big int, got null" );
    }
    return (ritual_object_t*) big_int;
}

struct ritual_big_rational *rconvto_big_rational( struct ritual_instance *inst,
                                  ritual_object_t *obj ) {
    if( RITUAL_TYPE( obj ) != RTYPE_BIG_RATIONAL ) {
        ritual_error( inst, "expected big rational, got \"%s\"", ritual_typename( obj ) );
    }
    return (struct ritual_big_rational*) obj;
}

ritual_object_t * rconvfrom_big_rational( struct ritual_instance *inst,
                                        struct ritual_big_rational *big_rational ) {
    if( !big_rational ) {
        ritual_error( inst, "expected big rational, got null" );
    }
    return (ritual_object_t*) big_rational;
}

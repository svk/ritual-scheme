#ifndef H_RITUAL_BIGNUM
#define H_RITUAL_BIGNUM

#include "ritual_object.h"
#include "ritual_basic_types.h"

#include <stdio.h>
#include <gmp.h>

/* Slightly irritating: GMP is reportedly not safe for longjmp() on
 * out-of-memory, although this is obviously what we need to do.
 * Also, allocators do not take a context argument, so we cannot
 * easily restrict memory to per-instance.
 * XXX for now I've left GMP at its default options; malloc/free
 *     and fatal error on OOM.
 *     longer-term: probably accept the leak and try to minimize
 *     it with garbage collection and such. perhaps explore
 *     opportunities for other bignum libraries (avoid NIH
 *     syndrome). Consider whether there's actually a useful
 *     use case for per-instance memory allocation, or even
 *     for running several instances. Surely the goal is being
 *     able to run different code in different threads with
 *     total independence, nothing more.
 *     also consider that we can probably detect how much memory
 *     we're using after the fact and use that to impose quotas
 *     on sub-instance levels. */
/* Also means we should take care to keep this modular. */
/* Get to work though, the perfect is the enemy of the good. */

struct ritual_big_int {
    ritual_object_t header;
    mpz_t value;
};

struct ritual_big_rational {
    ritual_object_t header;
    mpq_t value;
};

struct ritual_big_int * ritual_big_int_create_copy( struct ritual_instance *, mpz_t* );
struct ritual_big_rational * ritual_big_rational_create_copy( struct ritual_instance *, mpq_t* );

struct ritual_big_int * ritual_big_int_create( struct ritual_instance *, mpz_t );
struct ritual_big_int * ritual_big_int_create_decimal( struct ritual_instance *, const char* );
void ritual_big_int_destroy( struct ritual_instance *, void* );
void ritual_print_big_int( struct ritual_instance *, struct ritual_flo *, void * );

struct ritual_big_rational * ritual_big_rational_create( struct ritual_instance *, mpq_t );
void ritual_big_rational_destroy( struct ritual_instance *, void* );
void ritual_print_big_rational( struct ritual_instance *, struct ritual_flo *, void * );

ritual_object_t * ritual_string_to_number( struct ritual_instance *, const char * ); 

struct ritual_big_int *rconvto_big_int( struct ritual_instance *, ritual_object_t* );
ritual_object_t * rconvfrom_big_int( struct ritual_instance *, struct ritual_big_int * );

struct ritual_big_rational *rconvto_big_rational( struct ritual_instance *, ritual_object_t* );
ritual_object_t * rconvfrom_big_rational( struct ritual_instance *, struct ritual_big_rational * );

void ritual_bignum_initialize(void);
#endif

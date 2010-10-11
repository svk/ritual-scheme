#ifndef H_RITUAL_ARITHMETIC
#define H_RITUAL_ARITHMETIC

#include "ritual_native_proc.h"

/* There is a three-dimensional array of types with axes:
 *   - size (practically I'll restrict this to {bounded,unbounded})
 *   - coefficient generality (integer, rational, real)
 *   - dimensionality (simple, complex)
 * There is no need to supply everything (no plans for nonexact
 * integers, for instance, nor exact reals).
 *
 * Exactness and coefficient generality are heavily related,
 * in fact I see no reason not to build a system where
 *     coefficient generality is real
 *                 <=>
 *     exactness is false
 * which means we can remove the "exactness" axis.
 * 
 * A fourth dimension that would complicate things is
 * quality of approximation.
 *
 * That means we have a well-behaved hierarchy; it is always
 * possible to generalize (but at the implicit loss of exactness,
 * so it should not be done lightly). The "ultimate type" in
 * our system is an inexact complex type with unbounded real
 * coefficients.
 *
 * By convention, an "unbounded inexact real" number will be a
 * fixed-precision floating point type that cannot overflow,
 * but can end up in degenerate states (+inf, -inf, NaN).
 * (Note that the "inexactness" can go so far as to misjudge the
 *  sign of an infinity, e.g.: (limit/2 + limit/2 + limit/2
 *                              - limit/2 - limit/2 - limit/2
 *                              - limit/2 - limit/2 - limit/2)
 *  which will be +inf even though the number is negative,
 *  so this type is hardly unbounded in any real sense.
 *  The obvious alternatives are discarded because:
 *    - unbounded integral part with fixed-precision decimal
 *      part is likely to have worse precision for very small
 *      numbers than floating point
 *    - actual unboundedness is unworkable / probably
 *      impossible; simple operations result in irrational
 *      quantities (as opposed to unbounded integers, where
 *      you generally need to add/mul up a whole lot of numbers
 *      before you reach something that requires significant
 *      memory)
 * 
 * Since we can formulate such a type (we don't need to implement
 * it yet -- throw a NIY error whenever it's required) we can
 * orient our system around the simple operation:
 *   upgrade to "x" along its axis
 *     first, find the target type:
 *        - go to the cell where the desired type would be
 *        - if it's there, stop
 *        - otherwise, if bounded, upgrade size and retry
 *        - otherwise, if not real, upgrade generality by one step and retry
 *        - otherwise, if simple, upgrade dimensionality and retry
 *        - fail with error
 *     convert the coefficients in the original number (both boundedness and generality):
 *        - a conversion to a bounded type fails if the value is out of bounds
 *          (this means we should arrange the array such that legal bounded-to-bounded
 *           autoupgrades cannot result in this)
 *        - 
 */

typedef enum ritual_simple_size {
    RNUM_SIZE_BOUNDED = 0,
    RNUM_SIZE_UNBOUNDED,
    RNUM_SIZES
} ritual_simple_size_t;

typedef enum ritual_simple_generality {
    RNUM_INTEGER,
    RNUM_RATIONAL,
    // ..algebraic? let's not miss an opportunity for complexity here
    RNUM_REAL,
    RNUM_GENERALITIES
} ritual_simple_generality_t;

struct ritual_simple_type {
    ritual_simple_size_t size;
    ritual_simple_generality_t generality;

    int (*convert_to)( void*, ritual_simple_size_t, ritual_simple_generality_t, void* );

    int (*copy)( void*, void* ); // assume target initialized, leave both intact
    int (*move)( void*, void* ); // assume target NOT initialized, destroy source
    int (*init)( void* );
    int (*destroy)( void* );

        // If these operations are defined but fail on bounded
        // types, we will assume the reason is the boundedness.
    int (*add)( void*, void* );
    int (*sub)( void*, void* );
    int (*mul)( void*, void* );
    int (*div)( void*, void* );

    ritual_object_t* (*box_simple)( struct ritual_instance*, void* );
};


#if 0
ritual_object_t * rnp_add( struct ritual_instance *,
                            struct ritual_env *,
                            struct ritual_pair * );
ritual_object_t * rnp_sub( struct ritual_instance *,
                            struct ritual_env *,
                            struct ritual_pair * );
ritual_object_t * rnp_mul( struct ritual_instance *,
                            struct ritual_env *,
                            struct ritual_pair * );
ritual_object_t * rnp_div( struct ritual_instance *,
                            struct ritual_env *,
                            struct ritual_pair * );
#endif

#endif

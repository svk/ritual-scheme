#ifndef H_RITUAL_BASIC_TYPES
#define H_RITUAL_BASIC_TYPES

#include "ritual_object.h"
#include "ritual_flo.h"

/* The _destroy() functions should be called by the GC.
 * Note that the presence of a GC means that no object
 * should assert ownership over any other -- "deep freeing"
 * is strictly to liberate internal fields. Freeing a cons
 * cell is shallow. The final freeing of an object should
 * be done with ritual_free() to allow the GC to process
 * it. */

/* Objects that don't need a destroy function have none
 * defined for now. That'd correspond to a null pointer
 * in a function pointer table eventually. */

/* Next up:
 *   -if, as a "keyword" (not a native proc)
 *   -integral arithmetic
 *   -GMP integers, seamless
 */
/* Those weren't optimizations but correctness.
 * Optimizations to think about, tagged pointers for:
 *   1 bit:
 *      -native integers
 *   2 bits:
 *      -native integers
 *      -ascii characters
 *      -?
 * In anticipation of this, should perhaps begin using
 * a macro like RITUAL_POINTER(type,obj) -> obj, as it'll
 * otherwise be hell to convert. (The type just does an
 * assertion. Actually, this could also do a cast for us!)
 *
 * Note that it is absolutely NECESSARY that any trick we
 * use here works fine with both 32-bit and 64-bit pointers.
 * Frankly, it should work fine for any size >= 32 bits.
 * No funny business.
 */

struct ritual_quote {
    ritual_object_t header;
    ritual_object_t *quoted;
};

struct ritual_pair {
    ritual_object_t header;
    ritual_object_t *car;
    ritual_object_t *cdr;
};

struct ritual_symbol {
    ritual_object_t header;
    char name[1]; // Simple but rather inefficient.
};

struct ritual_ascii_string {
        // null-terminated; can NOT contain binary data!
    ritual_object_t header;
    char data[1];
};

struct ritual_native_int {
    ritual_object_t header;
    int32_t value;
};

struct ritual_ascii_char {
    ritual_object_t header;
    int8_t value;
};

struct ritual_boolean {
    ritual_object_t header;
    uint8_t value; // nonzero?
};

struct ritual_quote * ritual_quote_create(
    struct ritual_instance *,
    ritual_object_t * );
struct ritual_pair * ritual_pair_create(
    struct ritual_instance *,
    ritual_object_t *,
    ritual_object_t * );
struct ritual_symbol * ritual_symbol_create(
    struct ritual_instance *,
    const char * );
struct ritual_ascii_string * ritual_ascii_string_create(
    struct ritual_instance *,
    const char * );
struct ritual_native_int * ritual_native_int_create(
    struct ritual_instance *,
    int32_t );
struct ritual_ascii_char * ritual_ascii_char_create(
    struct ritual_instance *,
    int8_t );
struct ritual_boolean * ritual_boolean_create(
    struct ritual_instance *,
    int);

void ritual_print_null(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_pair(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_ascii_string(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_boolean(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_native_int(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_ascii_char(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_symbol(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );
void ritual_print_quote(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );

#endif

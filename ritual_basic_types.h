#ifndef H_RITUAL_BASIC_TYPES
#define H_RITUAL_BASIC_TYPES

#include "ritual_object.h"

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

struct ritual_pair {
    ritual_object_t header;
    ritual_object_t *car;
    ritual_object_t *cdr;
};

struct ritual_pair * ritual_pair_create( ritual_object_t *,
                                         ritual_object_t * );

struct ritual_symbol {
    ritual_object_t header;
    char name[1]; // Simple but rather inefficient.
}

struct ritual_symbol * ritual_symbol_create( const char * );

struct ritual_ascii_string {
        // null-terminated; can NOT contain binary data!
    ritual_object_t header;
    char data[1];
}

struct ritual_ascii_string * ritual_ascii_string_create( const char * );

struct ritual_native_int {
    ritual_object_t header;
    int32_t value;
}

struct ritual_native_int * ritual_native_int_create( int32_t );

struct ritual_ascii_char {
    ritual_object_t header;
    int8_t value;
}

struct ritual_ascii_char * ritual_ascii_char_create( int8_t );

struct ritual_boolean {
    ritual_object_t header;
    uint8_t value; // nonzero?
}

struct ritual_boolean * ritual_boolean_create(int);

#endif

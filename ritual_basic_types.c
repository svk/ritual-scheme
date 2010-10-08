#include "ritual_basic_types.h"

/* TODO: out of memory conditions are obvious candidates for
 *       ritual_error to longjmp to somewhere. */

struct ritual_pair * ritual_pair_create( ritual_object_t *car,
                                         ritual_object_t *cdr ) {
    struct ritual_pair *rv;
    rv = ritual_alloc_typed_object( RTYPE_PAIR, sizeof *rv );
    assert( rv ); 
    rv->car = car;
    rv->cdr = cdr;
    return rv;
                    
}

struct ritual_symbol * ritual_symbol_create( const char *s ) {
    struct ritual_symbol *rv;
    const int len = strlen( s ),
          size = sizeof *rv + len; // 1 elt (NUL) included in sizeof
    rv = ritual_alloc_typed_object( RTYPE_SYMBOL, size );
    assert( rv );
    memcpy( rv->name, s, len + 1 );
    return rv;
}

struct ritual_ascii_string * ritual_ascii_string_create( const char *s ) {
    struct ritual_ascii_string *rv;
    const int len = strlen( s ),
          size = sizeof *rv + len; // 1 elt (NUL) included in sizeof
    rv = ritual_alloc_typed_object( RTYPE_ASCII_STRING, size );
    assert( rv );
    memcpy( rv->data, s, len + 1 );
    return rv;
}

struct ritual_ascii_char * ritual_ascii_char_create( int8_t value ) {
    struct ritual_ascii_char *rv;
    rv = ritual_alloc_typed_object( RTYPE_ASCII_CHAR, sizeof *rv );
    assert( rv );
    rv->value = value;
    return rv;
}

struct ritual_native_int * ritual_native_int_create( int32_t value ) {
    struct ritual_native_int *rv;
    rv = ritual_alloc_typed_object( RTYPE_NATIVE_INT, sizeof *rv );
    assert( rv );
    rv->value = value;
    return rv;
}

struct ritual_boolean * ritual_boolean_create( int value ) {
    struct ritual_boolean *rv;
    rv = ritual_alloc_typed_object( RTYPE_BOOLEAN, sizeof *rv );
    assert( rv );
    rv->value = (value) ? 1 : 0;
    return rv;
}

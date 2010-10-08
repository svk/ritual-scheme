#include "ritual_basic_types.h"

#include "ritual_generic.h"

#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

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
    rv = ritual_alloc_typed_object( RTYPE_NATIVE_INTEGER, sizeof *rv );
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

void ritual_print_null( struct ritual_flo *flo, void *null ) {
    rflo_putstring( flo, "()" );
}

void ritual_print_pair( struct ritual_flo *flo,
                        void *obj ) {
    struct ritual_pair *pair = obj;
    rflo_putstring( flo, "(" );
    struct ritual_pair *current = pair;
    while( current->cdr &&
           RITUAL_TYPE(current->cdr) == RTYPE_PAIR ) {
        ritual_print( flo, current->car );
        rflo_putchar( flo,  ' ' );
        current = (struct ritual_pair*) current->cdr;
    }
    ritual_print( flo, current->car );
    if( current->cdr ) {
        rflo_putstring( flo,  " . " );
        ritual_print( flo, current->cdr );
    }
    rflo_putstring( flo, ")" );
}

void ritual_print_ascii_string( struct ritual_flo *flo,
                                void *obj ) {
    struct ritual_ascii_string *string = obj;
    /* TODO: check quoting. */
    int len = strlen( string->data );
    rflo_putchar( flo, '"' );
    for(int i=0;i<len;i++) {
        char ch = string->data[i];
        if( ch == '\n' ) {
            rflo_putchar( flo, '\\' );
            rflo_putchar( flo, 'n' );
        } else if( ch == '"' ) {
            rflo_putchar( flo, '\\' );
            rflo_putchar( flo, '"' );
        } else {
            rflo_putchar( flo, ch );
        }
    }
    rflo_putchar( flo, '"' );
}

void ritual_print_symbol( struct ritual_flo *flo,
                          void *obj ) {
    struct ritual_symbol *symbol = obj;
    rflo_putstring( flo, symbol->name );
}

void ritual_print_ascii_char( struct ritual_flo *flo,
                              void *obj ) {
    struct ritual_ascii_char *ascii_char = obj;
    if( ascii_char->value == '\n' ) {
        rflo_putstring( flo, "#\\newline" );
    } else if( ascii_char->value == ' ' ) {
        rflo_putstring( flo, "#\\space" );
    } else if( ascii_char->value == '\t' ) {
        rflo_putstring( flo, "#\\tab" );
    } else {
        rflo_putstring( flo, "#\\" );
        rflo_putchar( flo, ascii_char->value );
    }
}

void ritual_print_native_int( struct ritual_flo *flo,
                              void *obj ) {
    struct ritual_native_int * nint = obj;
    char buffer[64];
    sprintf( buffer, "%d", nint->value );
    rflo_putstring( flo, buffer );
}

void ritual_print_boolean( struct ritual_flo *flo,
                           void *obj ) {
    struct ritual_boolean *boolean = obj;
    if( boolean->value ) {
        rflo_putstring( flo, "#t" );
    } else {
        rflo_putstring( flo, "#f" );
    }
}

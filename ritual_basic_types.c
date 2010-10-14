#include "ritual_basic_types.h"

#include "ritual_generic.h"

#include <stdlib.h>
#include <string.h>

#include "ritual_error.h"

#include <stdio.h>
#include <stdlib.h>

#include "ritual_hash_table.h"

struct ritual_quote * ritual_quote_create( struct ritual_instance *inst,
                                           ritual_object_t *quoted ) {
    struct ritual_quote *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_QUOTE, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->quoted = quoted;
    return rv;
}


struct ritual_pair * ritual_pair_create( struct ritual_instance *inst,
                                         ritual_object_t *car,
                                         ritual_object_t *cdr ) {
    struct ritual_pair *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_PAIR, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->car = car;
    rv->cdr = cdr;
    inst->cons_cells_allocated++;
    return rv;
                    
}

struct ritual_symbol * ritual_symbol_create( struct ritual_instance *inst,
                                             const char *s ) {
    const int len = strlen( s );
    struct ritual_symbol *rv = rht_qlookup( inst->symbol_table, s, len );
    if( !rv ) {
        const int size = sizeof *rv + len; // 1 elt (NUL) included in sizeof
        rv = ritual_alloc_typed_object( inst, RTYPE_SYMBOL, size );
        RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
        memcpy( rv->name, s, len + 1 );

        rht_set( inst->symbol_table, s, len, rv );
    }

    return rv;
}

struct ritual_ascii_string * ritual_ascii_string_create( struct ritual_instance *inst,
                                                         const char *s ) {
    struct ritual_ascii_string *rv;
    const int len = strlen( s ),
          size = sizeof *rv + len; // 1 elt (NUL) included in sizeof
    rv = ritual_alloc_typed_object( inst, RTYPE_ASCII_STRING, size );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    memcpy( rv->data, s, len + 1 );
    return rv;
}

struct ritual_ascii_char * ritual_ascii_char_create( struct ritual_instance *inst,
                                                     int8_t value ) {
    if( value >= 0 && value < 256 && inst->scheme_ascii_char[value] ) {
        return inst->scheme_ascii_char[ value ];
    }
    struct ritual_ascii_char *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_ASCII_CHAR, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->value = value;
    return rv;
}

struct ritual_native_int * ritual_native_int_create( struct ritual_instance *inst,
                                                     int32_t value ) {
    struct ritual_native_int *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_NATIVE_INTEGER, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->value = value;
    return rv;
}

struct ritual_boolean * ritual_boolean_create( struct ritual_instance *inst,
                                               int value ) {
    if( !value && inst->scheme_false ) {
        return inst->scheme_false;
    } else if( value && inst->scheme_true ) {
        return inst->scheme_true;
    }
    struct ritual_boolean *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_BOOLEAN, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->value = (value) ? 1 : 0;
    return rv;
}

void ritual_print_null( struct ritual_instance *inst,
                        struct ritual_flo *flo, void *null ) {
    rflo_putstring( flo, "()" );
}

void ritual_print_quote( struct ritual_instance *inst,
                         struct ritual_flo *flo,
                         void *obj ) {
    struct ritual_quote *quote = obj;
    rflo_putchar( flo,  '\'' );
    ritual_print( inst, flo, quote->quoted );
}

void ritual_print_pair( struct ritual_instance *inst,
                        struct ritual_flo *flo,
                        void *obj ) {
    struct ritual_pair *pair = obj;
    rflo_putstring( flo, "(" );
    struct ritual_pair *current = pair;
    while( current->cdr &&
           RITUAL_TYPE(current->cdr) == RTYPE_PAIR ) {
        ritual_print( inst, flo, current->car );
        rflo_putchar( flo,  ' ' );
        current = (struct ritual_pair*) current->cdr;
    }
    ritual_print( inst, flo, current->car );
    if( current->cdr ) {
        rflo_putstring( flo,  " . " );
        ritual_print( inst, flo, current->cdr );
    }
    rflo_putstring( flo, ")" );
}

void ritual_print_ascii_string( struct ritual_instance *inst,
                                struct ritual_flo *flo,
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

void ritual_print_symbol( struct ritual_instance *inst,
                          struct ritual_flo *flo,
                          void *obj ) {
    struct ritual_symbol *symbol = obj;
    rflo_putstring( flo, symbol->name );
}

void ritual_print_ascii_char( struct ritual_instance *inst,
                              struct ritual_flo *flo,
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

void ritual_print_native_int( struct ritual_instance *inst,
                              struct ritual_flo *flo,
                              void *obj ) {
    struct ritual_native_int * nint = obj;
    char buffer[64];
    sprintf( buffer, "%d", nint->value );
    rflo_putstring( flo, buffer );
}

void ritual_print_boolean( struct ritual_instance *inst,
                           struct ritual_flo *flo,
                           void *obj ) {
    struct ritual_boolean *boolean = obj;
    if( boolean->value ) {
        rflo_putstring( flo, "#t" );
    } else {
        rflo_putstring( flo, "#f" );
    }
}

ritual_object_t *ritual_list_peek( struct ritual_instance *inst,
                                   struct ritual_pair * list ) {
    if( !list ) {
        ritual_error( inst, "premature end of list" );
    }
    if( RITUAL_TYPE( list ) != RTYPE_PAIR ) {
        ritual_error( inst, "expected proper list" );
    }
    ritual_object_t *rv = list->car;
    return rv;
}

ritual_object_t *ritual_list_next( struct ritual_instance *inst,
                                   struct ritual_pair ** list ) {
    if( !(*list) ) {
        ritual_error( inst, "premature end of list" );
    }
    if( RITUAL_TYPE( (*list) ) != RTYPE_PAIR ) {
        ritual_error( inst, "expected proper list" );
    }
    ritual_object_t *rv = (*list)->car;
    *list = rconvto_list( inst, (*list)->cdr );
    return rv;
}
void ritual_list_assert_end( struct ritual_instance *inst,
                             struct ritual_pair *p ) {
    if( p ) {
        ritual_error( inst, "expected end of list" );
    }
}

int ritual_list_has_cdr( struct ritual_instance *inst,
                         struct ritual_pair * list ) {
    if( list && RITUAL_TYPE( (list) ) != RTYPE_PAIR ) {
        ritual_error( inst, "expected proper list" );
    }
    return list->cdr != 0;
}

struct ritual_pair *rconvto_pair( struct ritual_instance *inst,
                                  ritual_object_t *obj ) {
    if( RITUAL_TYPE( obj ) != RTYPE_PAIR ) {
        ritual_error( inst, "expected pair, got \"%s\"", ritual_typename( obj ) );
    }
    return (struct ritual_pair*) obj;
}

ritual_object_t * rconvfrom_pair( struct ritual_instance *inst,
                                  struct ritual_pair *pair ) {
    if( !pair ) {
        ritual_error( inst, "expected pair, got null" );
    }
    return (ritual_object_t*) pair;
}

struct ritual_pair *rconvto_list( struct ritual_instance *inst,
                                  ritual_object_t *obj ) {
    if( obj && RITUAL_TYPE( obj ) != RTYPE_PAIR ) {
        ritual_error( inst, "expected list, got \"%s\"", ritual_typename( obj ) );
    }
    return (struct ritual_pair*) obj;
}

ritual_object_t * rconvfrom_list( struct ritual_instance *inst,
                                  struct ritual_pair *list ) {
    return (ritual_object_t*) list;
}

struct ritual_native_int *rconvto_native_int( struct ritual_instance *inst,
                                  ritual_object_t *obj ) {
    if( RITUAL_TYPE( obj ) != RTYPE_NATIVE_INTEGER ) {
        ritual_error( inst, "expected native int, got \"%s\"", ritual_typename( obj ) );
    }
    return (struct ritual_native_int*) obj;
}

ritual_object_t * rconvfrom_native_int( struct ritual_instance *inst,
                                        struct ritual_native_int *native_int ) {
    if( !native_int ) {
        ritual_error( inst, "expected native int, got null" );
    }
    return (ritual_object_t*) native_int;
}

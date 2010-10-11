#include "ritual_instance.h"

#include "ritual_gc.h"
#include "ritual_env.h"
#include "ritual_error.h"

#include "ritual_basic_types.h"

#include "ritual_native_proc.h"

#include "ritual_keyword.h"

#include "ritual_arithmetic.h"

#include "ritual_generic.h"
#include "ritual_flo.h"

#include <string.h>
#include <stdlib.h>

#include <stdio.h>

/* These use normal malloc/free; since they allocate the context
 * within other memory will operate that's natural. */

int ritual_initialize_instance( struct ritual_instance * inst ) {
    memset( inst, 0, sizeof *inst );
    do {
        inst->gc = malloc(sizeof *inst->gc );
        if( !inst->gc ) break;
        rgc_initialize( inst, inst->gc );

        inst->error = malloc(sizeof *inst->error );
        if( !inst->error ) break;
        ritual_error_initialize( inst->error );

        inst->root = (struct ritual_env*) ritual_alloc_typed_object( inst, RTYPE_ENVIRONMENT, sizeof *inst->root );
        if( !inst->root ) break;
        ritual_env_init_root( inst, inst->root );

        inst->symbol_table = malloc( sizeof *inst->root );
        if( !inst->symbol_table ) break;
        if( rht_table_init( inst->symbol_table, RITUAL_SYMBOL_TABLE_SIZE ) ) {
            free( inst->symbol_table );
            inst->symbol_table = 0;
            break;
        }
        inst->symbol_table->memory_context = inst;
        inst->symbol_table->ht_alloc = (void* (*)(void*,int)) ritual_alloc;
        inst->symbol_table->ht_free = (void (*)(void*,void*)) ritual_free;

        /* These objects are GCed on deinitialization as all others
         * and we do not need to free them. On the contrary, we need
         * to make sure they're never GCed automatically when we
         * implement proper GC! */
        inst->scheme_true = ritual_boolean_create( inst, 1 );
        inst->scheme_false = ritual_boolean_create( inst, 0 );
        for(int i=0;i<128;i++) {
            inst->scheme_ascii_char[i] = ritual_ascii_char_create( inst, i );
        }

        ritual_define_keyword( inst, inst->root, "if", RKW_IF );
        ritual_define_keyword( inst, inst->root, "begin", RKW_BEGIN );
        ritual_define_keyword( inst, inst->root, "and", RKW_AND );
        ritual_define_keyword( inst, inst->root, "or", RKW_OR );
        ritual_define_keyword( inst, inst->root, "let", RKW_LET );
        ritual_define_keyword( inst, inst->root, "let*", RKW_LET_STAR );
        ritual_define_keyword( inst, inst->root, "letrec", RKW_LETREC );

        ritual_define_rnp_as_keyword( inst, inst->root, "define", rnp_define );
        ritual_define_rnp_as_keyword( inst, inst->root, "lambda", rnp_lambda );

        ritual_define_native_proc( inst, inst->root, "+", rnp_add );
        ritual_define_native_proc( inst, inst->root, "-", rnp_sub );
        ritual_define_native_proc( inst, inst->root, "*", rnp_mul );

        ritual_define_native_proc( inst, inst->root, "=", rnp_number_eqp );

        ritual_define_native_proc( inst, inst->root, "zero?", rnp_zerop );

        ritual_define_native_proc( inst, inst->root, "boolean?", rnp_booleanp );
        ritual_define_native_proc( inst, inst->root, "symbol?", rnp_symbolp );
        ritual_define_native_proc( inst, inst->root, "number?", rnp_numberp );
        ritual_define_native_proc( inst, inst->root, "char?", rnp_charp );
        ritual_define_native_proc( inst, inst->root, "string?", rnp_stringp );
        ritual_define_native_proc( inst, inst->root, "procedure?", rnp_procedurep );
        ritual_define_native_proc( inst, inst->root, "port?", rnp_portp );
        ritual_define_native_proc( inst, inst->root, "vector?", rnp_vectorp );
        ritual_define_native_proc( inst, inst->root, "pair?", rnp_pairp );

        inst->flo_stdout = (void*) rflo_filehandle_create( inst, stdout );
        inst->flo_stderr = (void*) rflo_filehandle_create( inst, stderr );

        return 0;
    } while(0);
    if( inst->gc ) {
        rgc_deinitialize( inst, inst->gc );
        free( inst->gc );
    }
    if( inst->error ) {
        free( inst->error );
    }
    if( inst->root ) {
        ritual_env_destroy( inst, inst->root );
        free( inst->root );
    }
    return 1;

}

void ritual_deinitialize_instance( struct ritual_instance *inst ) {
    rht_table_destroy( inst->symbol_table );
    free( inst->symbol_table );

    free( inst->error );

    rflo_filehandle_destroy( inst, (void*) inst->flo_stdout );
    rflo_filehandle_destroy( inst, (void*) inst->flo_stderr );

    rgc_deinitialize( inst, inst->gc );
    free( inst->gc );
}

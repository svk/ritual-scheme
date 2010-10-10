#include "ritual_instance.h"

#include "ritual_gc.h"
#include "ritual_env.h"
#include "ritual_error.h"

#include "ritual_basic_types.h"

#include "ritual_native_proc.h"

#include <string.h>
#include <stdlib.h>

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

        inst->root = malloc(sizeof *inst->root );
        if( !inst->root ) break;
        ritual_env_init_root( inst, inst->root );

        /* These objects are GCed on deinitialization as all others
         * and we do not need to free them. On the contrary, we need
         * to make sure they're never GCed automatically when we
         * implement proper GC! */
        inst->scheme_true = ritual_boolean_create( inst, 1 );
        inst->scheme_false = ritual_boolean_create( inst, 0 );

        ritual_define_native_proc( inst, inst->root, "define", rnp_define );
        ritual_define_native_proc( inst, inst->root, "lambda", rnp_lambda );

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
    ritual_env_destroy( inst, inst->root );
    free( inst->root );

    free( inst->error );

    rgc_deinitialize( inst, inst->gc );
    free( inst->gc );


}

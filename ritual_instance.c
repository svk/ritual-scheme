#include "ritual_instance.h"

#include "ritual_gc.h"
#include "ritual_env.h"
#include "ritual_error.h"

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

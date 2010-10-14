#ifndef H_RITUAL_INSTANCE
#define H_RITUAL_INSTANCE

/* Because we aim for multithreading, we can't have nonconst globals,
 * so all that semantically would be global goes here. Notably things
 * like the execution environment and the entire memory model, as well
 * as global locks if we chicken out and put in that.
 *
 * We need an instance to create objects (because the garbage collector
 * and the memory management is associated with an instance, which
 * allows us to e.g. impose quotas, down the line) -- this means that
 * the parser needs access to one in the context. */

struct ritual_instance;

#include "ritual_hash_table.h"
#include "ritual_object.h"

#define RITUAL_SYMBOL_TABLE_SIZE 1024

#include <setjmp.h>

struct ritual_gc_instance;
struct ritual_error_instance;
struct ritual_env;
struct ritual_flo;

struct ritual_instance {
    struct ritual_gc_instance *gc;
    struct ritual_error_instance *error;

    struct ritual_env *root;

    struct rht_table * symbol_table;

    // XXX hax: these being void* 
    void *scheme_true, *scheme_false;
    void *scheme_ascii_char[256];


    // handy for debugging
    struct ritual_flo * flo_stdout;
    struct ritual_flo * flo_stderr;

    // diagnostics
    int total_bytes_allocated;
    int cons_cells_allocated;
    int subenvironments_allocated;
    int bytes_allocated_to_subenvironments;

    int olist_allocated;
    int env_hash_tables_allocated;
    int typed_objects_allocated[ RTYPE_NUM_TYPES ];
    int typed_objects_bytes_allocated[ RTYPE_NUM_TYPES ];
};

int ritual_initialize_instance( struct ritual_instance* );
void ritual_deinitialize_instance( struct ritual_instance * );

#endif

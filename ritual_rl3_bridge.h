#ifndef H_RITUAL_RL3_BRIDGE
#define H_RITUAL_RL3_BRIDGE

#include "rl3.h"

#include "ritual_object.h"

#include "ritual_envproc.h"

struct ritual_rl3_extensions {
    struct rl3_global_context gctx;

    int EVAL, EVAL_DISCARD, TAILEVAL;
    int ENV_CREATE_PUSH, ENV_CREATE_REPLACE, ENV_DISCARD, ENV_BIND;
    int GENERAL_ERROR;
    int CALL_NATIVE;
};

struct ritual_rl3_extended_context {
    struct rl3_context ctx;

    struct ritual_rl3_extensions *ext;

    struct ritual_pair *environments;
};

struct ritual_rl3_procedure {
    ritual_object_t header;
    struct rl3_instr * entry;
        // expect fresh env, pushed values-list
        // handles all bindings etc. itself
        // (curiously, that seems more optimized than the
        //  native procedures...)
};

void ritual_initialize_rl3_extensions( struct ritual_rl3_extensions* );
void ritual_initialize_extended_rl3_context( struct ritual_rl3_extended_context*, struct rl3_global_context*, struct ritual_instance *, struct ritual_env* );

struct rl3_instr ** ritual_rl3_make_arglist_parser( struct ritual_rl3_extended_context *,
                                                    ritual_object_t *,
                                                    struct rl3_instr ** );

void ritual_rl3_compile_envproc( struct ritual_instance *,
                                 struct ritual_envproc * );

void ritual_rl3_clear_context( struct ritual_rl3_extended_context *, ritual_object_t *);
#endif

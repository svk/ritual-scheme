#ifndef H_RITUAL_RL3_BRIDGE
#define H_RITUAL_RL3_BRIDGE

#include "rl3.h"

#include "ritual_object.h"

struct ritual_rl3_extensions {
    int EVAL, EVAL_DISCARD, TAILEVAL;
    int ENV_PUSH, ENV_DISCARD, ENV_REPLACE, ENV_BIND;
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

void ritual_initialize_rl3_extensions( struct rl3_global_context*, struct ritual_rl3_extensions* );

#endif

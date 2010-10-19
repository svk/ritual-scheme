#ifndef H_RL3
#define H_RL3

#include "ritual_object.h"
#include "ritual_basic_types.h"

struct rl3_context;

struct rl3_instr {
    ritual_object_t header;
    int id;
    ritual_object_t *arg;
    struct rl3_instr *next;
};

struct rl3_global_context {
    int size;
    int next_id;
    void (**ptr)(struct rl3_context*, ritual_object_t*);

    int DUP, STORE, DISCARD, SWAP, ROTATE;
    int IS_NULL, IS_PAIR;
    int SPLIT_PAIR, PRINT, CONS;
    int JUMP, CALL, BRANCH;
};

struct rl3_context {
    struct rl3_global_context *global;

    struct ritual_instance *inst;

    struct ritual_pair *sequences; // ip
    struct ritual_pair *values;
};

struct rl3_global_context * rl3_initialize(void);
void rl3_deinitialize(struct rl3_global_context*);
int rl3_register_instruction(struct rl3_global_context*, void (*p)(struct rl3_context*,ritual_object_t*) );

void rl3_context_init( struct rl3_context*, struct rl3_global_context*, struct ritual_instance* );

int rl3_running( struct rl3_context* );
void rl3_run_one( struct rl3_context* );
ritual_object_t * rl3_run( struct rl3_context*, struct rl3_instr * );

struct rl3_instr* rl3_mkinstr( struct ritual_instance*, int, ritual_object_t*, struct rl3_instr* );
struct rl3_instr* rl3_reverse( struct rl3_instr* );

ritual_object_t * rconvfrom_sequence( struct ritual_instance *, struct rl3_instr *);
struct rl3_instr *rconvto_sequence( struct ritual_instance *,ritual_object_t * );

#endif

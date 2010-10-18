#ifndef H_RL3
#define H_RL3

#include "ritual_object.h"
#include "ritual_basic_types.h"

struct rl3_instr {
    ritual_object_t header;
    int id;
    void *arg;
    struct rl3_instr *next;
};

struct rl3_context {
    struct ritual_instance *inst;
    struct ritual_pair *sequences; // ip
    struct ritual_pair *values;
};

void rl3_context_init( struct rl3_context*, struct ritual_instance* );

void rl3_run_one( struct rl3_context* );
ritual_object_t * rl3_run( struct rl3_context*, struct rl3_instr * );

struct rl3_instr* rl3_mkinstr( struct ritual_instance*, int, void*, struct rl3_instr* );
struct rl3_instr* rl3_reverse( struct rl3_instr* );

ritual_object_t * rconvfrom_sequence( struct ritual_instance *, struct rl3_instr *);
struct rl3_instr *rconvto_sequence( struct ritual_instance *,ritual_object_t * );

#endif

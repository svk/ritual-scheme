#ifndef H_RL3
#define H_RL3

#include "ritual_object.h"
#include "ritual_basic_types.h"

struct rl3_context {
    struct ritual_instance *inst;
    struct ritual_pair *sequences; // ip
    struct ritual_pair *values;
};

void rl3_context_init( struct rl3_context*, struct ritual_instance* );

#endif

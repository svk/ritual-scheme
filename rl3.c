#include "rl3.h"

void rl3_context_init( struct rl3_context* ctx, struct ritual_instance *inst ) {
    ctx->inst = inst;
    ctx->sequences = 0;
    ctx->values = 0;
}

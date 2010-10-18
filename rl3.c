#include "rl3.h"

#include "ritual_error.h"

void rl3_context_init( struct rl3_context* ctx, struct ritual_instance *inst ) {
    ctx->inst = inst;
    ctx->sequences = 0;
    ctx->values = 0;
}

struct rl3_instr* rl3_mkinstr( struct ritual_instance* inst, int id, void* arg, struct rl3_instr* next) {
    struct rl3_instr *rv = ritual_alloc_typed_object( inst, RTYPE_RL3INSTRUCTION, sizeof *rv );
    if( !rv ) {
        ritual_error( inst, "unable to allocate memory for RL3 instruction" );
    }
    rv->id = id;
    rv->arg = arg;
    rv->next = next;
    return rv;
}

struct rl3_instr* rl3_reverse( struct rl3_instr* reversed ) {
    struct rl3_instr *current = reversed;
    struct rl3_instr *last = 0;

    while( current ) {
        struct rl3_instr *next = current->next;
        current->next = last;
        last = current;
        current = next;
    }
    return last;
}

struct rl3_instr *rconvto_sequence( struct ritual_instance *inst,
                                  ritual_object_t *obj ) {
    if( obj && RITUAL_TYPE( obj ) != RTYPE_RL3INSTRUCTION ) {
        ritual_error( inst, "expected sequence, got \"%s\"", ritual_typename( obj ) );
    }
    return (struct rl3_instr*) obj;
}

ritual_object_t * rconvfrom_sequence( struct ritual_instance *inst,
                                      struct rl3_instr *obj ) {
    return (ritual_object_t*) obj;
}



ritual_object_t * rl3_run( struct rl3_context* ctx, struct rl3_instr *program ) {
    RITUAL_ASSERT( ctx->inst, !ctx->sequences, "already running RL3 program" );
    ritual_list_push( ctx->inst, &ctx->sequences, rconvfrom_sequence( ctx->inst, program ) );
    while( ctx->sequences ) {
        rl3_run_one( ctx );
    }
    return ritual_list_next( ctx->inst, &ctx->values );
}

void rl3_run_one( struct rl3_context* ctx ) {
    if( !ctx->sequences ) return;

    ritual_object_t *obj = ritual_list_peek( ctx->inst, ctx->sequences );

    while( !obj ) {
        ritual_list_next( ctx->inst, &ctx->sequences );
        if( !ctx->sequences ) return;
        obj = ritual_list_peek( ctx->inst, ctx->sequences );
    }

    struct rl3_instr *ins = rconvto_sequence( ctx->inst, obj );
    ritual_pair_setcar( ctx->inst, ctx->sequences, rconvfrom_sequence( ctx->inst, ins->next ) );
    switch( ins->id ) { // For now, HQ9+ inspired
        case 0: printf("Hello "); break;
        case 1: printf("world!\n"); break;
        case 2: ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_ascii_char['K'] ); break;
        case 3: ritual_list_next( ctx->inst, &ctx->values ); break;
    }
}

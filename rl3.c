#include "rl3.h"

#include "ritual_error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ritual_generic.h"

#include <assert.h>

#define VERBOSE_DEBUG

void rl3_debug_show( struct rl3_context *ctx ) {
    struct ritual_pair *current = ctx->sequences;
    int n = 0;
    printf( "Sequences:\t" );
    while( current ) {
        struct rl3_instr* ins = (struct rl3_instr*) ritual_list_next( ctx->inst, &current );
        if( ins ) {
            printf( "[%d] ", ins->id );
            ++n;
        }
    }
    printf( "(%d)\n", n );

    printf( "Values:\t" );
    current = ctx->values;
    n = 0;
    while( current ) {
        ritual_object_t* obj = ritual_list_next( ctx->inst, &current );
        if( obj ) {
            printf( "[" );
            ritual_print( ctx->inst, ctx->inst->flo_stdout, obj );
            printf( "] " );
            ++n;
        }
    }
    printf( "(%d)\n", n );
}

void rl3_ins_dup(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *top = ritual_list_peek( ctx->inst, ctx->values );
    ritual_list_push( ctx->inst, &ctx->values, top );
}

void rl3_ins_store(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_list_push( ctx->inst, &ctx->values, arg );
}

void rl3_ins_discard(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_list_next( ctx->inst, &ctx->values );
}

void rl3_ins_swap(struct rl3_context *ctx, ritual_object_t *arg) {
    // keep in mind these are "atomic" wrt the GC,
    // so we can afford some tomfoolery
    ritual_object_t **first = &ctx->values->car;
    ritual_object_t **second = &rconvto_list( ctx->inst, ctx->values->cdr )->car;
    ritual_object_t *temp = *first;
    *first = *second;
    *second = temp;
}

void rl3_ins_rotate(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t **first = &ctx->values->car;
    struct ritual_pair *firstp = rconvto_list( ctx->inst, ctx->values->cdr );
    ritual_object_t **second = &firstp->car;
    struct ritual_pair *secondp = rconvto_list( ctx->inst, firstp->cdr );
    ritual_object_t **third = &secondp->car;
    ritual_object_t *temp = *first;
    *first = *third;
    *third = *second;
    *second = temp;
}

void rl3_ins_is_null(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *top = ritual_list_peek( ctx->inst, ctx->values );
    if( !top ) {
        ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_true );
    } else {
        ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_false );
    }
}

void rl3_ins_is_pair_or_null(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *top = ritual_list_peek( ctx->inst, ctx->values );
    if( !top || RITUAL_TYPE(top) == RTYPE_PAIR ) {
        ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_true );
    } else {
        ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_false );
    }
}

void rl3_ins_is_pair(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *top = ritual_list_peek( ctx->inst, ctx->values );
    if( RITUAL_TYPE(top) == RTYPE_PAIR ) {
        ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_true );
    } else {
        ritual_list_push( ctx->inst, &ctx->values, ctx->inst->scheme_false );
    }
}

void rl3_ins_split_pair(struct rl3_context *ctx, ritual_object_t *arg) {
    assert( ctx->values );

    ritual_object_t **first = &ctx->values->car;

    assert( RITUAL_TYPE( *first ) == RTYPE_PAIR );

    struct ritual_pair *pair = rconvto_pair( ctx->inst, *first );
    ritual_object_t *obj = pair->car;
    *first = pair->cdr;

    ritual_list_push( ctx->inst, &ctx->values, obj );
}

void rl3_ins_cons(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *first = ritual_list_next( ctx->inst, &ctx->values );
    ritual_object_t *second = ritual_list_next( ctx->inst, &ctx->values );
    struct ritual_pair *pair = ritual_pair_create( ctx->inst, first, second );
    ritual_list_push( ctx->inst, &ctx->values, rconvfrom_pair( ctx->inst,  pair ) );
}

void rl3_ins_print(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *obj = ritual_list_next( ctx->inst, &ctx->values );
    ritual_print( ctx->inst, ctx->inst->flo_stdout, obj );
}

void rl3_ins_jump(struct rl3_context *ctx, ritual_object_t *arg) {
    ctx->sequences->car = (ritual_object_t*) arg;
}

void rl3_ins_call(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_list_push( ctx->inst, &ctx->sequences, arg );
}

void rl3_ins_branch_not(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *top = ritual_list_next( ctx->inst, &ctx->values );
    if( !RITUAL_AS_BOOLEAN( ctx->inst, top ) ) {
        rl3_ins_jump( ctx, arg );
    }
}


void rl3_ins_branch(struct rl3_context *ctx, ritual_object_t *arg) {
    ritual_object_t *top = ritual_list_next( ctx->inst, &ctx->values );
    if( RITUAL_AS_BOOLEAN( ctx->inst, top ) ) {
        rl3_ins_jump( ctx, arg );
    }
}


struct rl3_global_context * rl3_initialize(void) {
    struct rl3_global_context *gctx = malloc(sizeof *gctx);
    if( !gctx ) return 0;
    do {
        memset( gctx, 0, sizeof *gctx );
        gctx->size = 32;

        gctx->ptr = malloc(sizeof *gctx->ptr * gctx->size);
        if( !gctx->ptr ) break;

        gctx->desc = malloc(sizeof *gctx->desc * gctx->size);
        if( !gctx->desc ) break;

        gctx->next_id = 1;

        gctx->DUP = rl3_register_instruction( gctx, rl3_ins_dup, "DUP" );
        gctx->STORE = rl3_register_instruction( gctx, rl3_ins_store, "STORE" );
        gctx->DISCARD = rl3_register_instruction( gctx, rl3_ins_discard, "DISCARD" );
        gctx->SWAP = rl3_register_instruction( gctx, rl3_ins_swap, "SWAP" );
        gctx->ROTATE = rl3_register_instruction( gctx, rl3_ins_rotate, "ROTATE" );

        gctx->IS_NULL = rl3_register_instruction( gctx, rl3_ins_is_null, "IS-NULL" );
        gctx->IS_PAIR = rl3_register_instruction( gctx, rl3_ins_is_pair, "IS-PAIR" );
        gctx->IS_PAIR_OR_NULL = rl3_register_instruction( gctx, rl3_ins_is_pair_or_null, "IS-PAIR" );

        gctx->SPLIT_PAIR = rl3_register_instruction( gctx, rl3_ins_split_pair, "SPLIT-PAIR" );
        gctx->CONS = rl3_register_instruction( gctx, rl3_ins_cons, "CONS" );
        gctx->PRINT = rl3_register_instruction( gctx, rl3_ins_print, "PRINT" );
//        gctx->READ = rl3_register_instruction( gctx, rl3_ins_read, "READ" );

        gctx->JUMP = rl3_register_instruction( gctx, rl3_ins_jump, "JUMP" );
        gctx->CALL = rl3_register_instruction( gctx, rl3_ins_call, "CALL" );
        gctx->BRANCH = rl3_register_instruction( gctx, rl3_ins_branch, "BRANCH" );
        gctx->BRANCH_NOT = rl3_register_instruction( gctx, rl3_ins_branch_not, "BRANCH-NOT" );

        return gctx;
    } while(0);
    if( gctx->desc ) {
        free( gctx->desc );
    }
    if( gctx->ptr ) {
        free( gctx->ptr );
    }
    return 0;
}

void rl3_deinitialize(struct rl3_global_context* gctx) {
    if( gctx->ptr ) {
        free( gctx->ptr );
    }
    free( gctx );
}

int rl3_register_instruction(struct rl3_global_context *gctx, void (*p)(struct rl3_context*,ritual_object_t*), const char *name ) {
    int id = gctx->next_id++;
    while( id >= gctx->size ) {
        void *mem = realloc( gctx->ptr, sizeof *gctx->ptr * gctx->size * 2);
        if( !mem ) {
            return -1;
        }
        void *mem2 = realloc( gctx->desc, sizeof *gctx->desc * gctx->size * 2);
        if( !mem2 ) {
            free( mem );
            return -1;
        }
        gctx->size *= 2;
        gctx->ptr = mem;
        gctx->desc = mem2;
    }
    gctx->ptr[id - 1] = p;
    gctx->desc[id - 1] = name;
    return id;
}

void rl3_context_init( struct rl3_context* ctx, struct rl3_global_context *gctx, struct ritual_instance *inst ) {
    ctx->global = gctx;
    ctx->inst = inst;
    ctx->sequences = 0;
    ctx->values = 0;
}

struct rl3_instr* rl3_mkinstr( struct ritual_instance* inst, int id, ritual_object_t* arg, struct rl3_instr* next) {
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

#ifdef VERBOSE_DEBUG
    rl3_print_instruction( ctx, ctx->inst->flo_stderr, ins );
    fprintf( stderr, "\n" );
    rl3_debug_show( ctx );
#endif
    ctx->global->ptr[ ins->id - 1 ](ctx, ins->arg );
}

int rl3_running( struct rl3_context* ctx ) {
    return ctx->sequences != 0;
}

struct rl3_instr** rl3_seqinstr( struct ritual_instance* inst, int id, ritual_object_t* arg, struct rl3_instr** last, struct rl3_instr **label) {
    struct rl3_instr *ins = rl3_mkinstr( inst, id, arg, 0 );
    *last = ins;
    if( label ) {
        *label = ins;
    }
    return &ins->next;
}

void rl3_print_instruction( struct rl3_context *ctx,
                            struct ritual_flo *flo,
                            void *obj ) {
    struct rl3_instr *instr = obj;
    rflo_putstring( flo, ctx->global->desc[ instr->id - 1 ] );
    rflo_putchar( flo, ' ' );
    switch( RITUAL_TYPE( instr->arg ) ) {
        case RTYPE_RL3INSTRUCTION:
            rflo_putstring( flo, "<instruction>" );
            break;
        default:
            ritual_print( ctx->inst, flo, instr->arg );
            break;
    }
}

#include "ritual_rl3_bridge.h"

#include "ritual_env.h"
#include "ritual_error.h"

#include "ritual_basic_types.h"

#include "ritual_envproc.h"



#include <assert.h>

void ritual_rl3_compile_envproc( struct ritual_instance *inst,
                                               struct ritual_envproc *envproc ) {
    envproc->entry = 0;
    struct rl3_instr **writep = &envproc->entry;
    writep = ritual_rl3_make_arglist_parser( inst->rl3_ctx, envproc->params, writep );
    writep = rl3_seqinstr( inst, inst->rl3_ext->CALL_NATIVE, (ritual_object_t*) envproc, writep, 0 );
}

struct rl3_instr ** ritual_rl3_make_arglist_parser( struct ritual_rl3_extended_context *ectx,
                                                    ritual_object_t *arglist,
                                                    struct rl3_instr ** last ) {
    struct rl3_global_context *gctx = ectx->ctx.global;
    struct ritual_rl3_extensions *ext = ectx->ext;
    struct ritual_instance *inst = ectx->ctx.inst;

    struct rl3_instr *too_few_arguments = rl3_mkinstr( inst, ext->GENERAL_ERROR, 0, 0 );
    // should probably discard some stuff -- important?

    struct rl3_instr *too_many_arguments = rl3_mkinstr( inst, ext->GENERAL_ERROR, 0, 0 );
    // should probably discard some stuff -- important?
    
    int was_rest = 0;
    
    while( arglist ) {
        switch( RITUAL_TYPE( arglist ) ) {
            case RTYPE_PAIR:
                {
                    struct ritual_pair *pair = rconvto_pair( inst, arglist );
                    struct ritual_symbol *sym = rconvto_symbol( inst, pair->car );
                    arglist = pair->cdr;

                    last = rl3_seqinstr( inst, gctx->IS_PAIR, 0, last, 0 );
                    last = rl3_seqinstr( inst, gctx->BRANCH_NOT, (ritual_object_t*)too_few_arguments, last, 0 );
                    last = rl3_seqinstr( inst, gctx->SPLIT_PAIR, 0, last, 0 );
                    last = rl3_seqinstr( inst, ext->EVAL, 0, last, 0 );
                    last = rl3_seqinstr( inst, ext->ENV_BIND, rconvfrom_symbol( inst, sym ), last, 0 );

                    break;
                }
            case RTYPE_SYMBOL:
                {
                    struct ritual_symbol *sym = rconvto_symbol( inst, arglist );
                    arglist = 0;

                    last = rl3_seqinstr( inst, gctx->IS_PAIR_OR_NULL, 0, last, 0 );
                    last = rl3_seqinstr( inst, gctx->BRANCH_NOT, (ritual_object_t*)too_few_arguments, last, 0 );
                    // TODO: map(eval, args)
                    last = rl3_seqinstr( inst, ext->ENV_BIND, rconvfrom_symbol( inst, sym ), last, 0 );
                    was_rest = 1;
                    break;
                }
            default:
                ritual_error( inst, "invalid argument list" );
        }
    }

    if( !was_rest ) {
            // discard the nil, verifying that it's actually nil
        last = rl3_seqinstr( inst, gctx->IS_NULL, 0, last, 0 );
        last = rl3_seqinstr( inst, gctx->BRANCH_NOT, (ritual_object_t*) too_many_arguments, last, 0 );
        last = rl3_seqinstr( inst, gctx->DISCARD, 0, last, 0 );
    }

    return last;
}

void rl3ext_call_native( struct rl3_context *ctx, ritual_object_t *arg ) {
    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    struct ritual_envproc *envproc = (struct ritual_envproc*) arg;
    assert( RITUAL_TYPE( arg ) == RTYPE_ENVPROC );

    struct ritual_env *env = (struct ritual_env *) ectx->environments->car;
    assert( RITUAL_TYPE(env) == RTYPE_ENVIRONMENT );

    ritual_object_t *rv = envproc->procedure( ctx->inst, env );

    ritual_list_push( ctx->inst, &ctx->values, rv );
}

void ritual_rl3_clear_context( struct ritual_rl3_extended_context *ectx, ritual_object_t *rv ) {
    // Note, better solution for the future: just set an exception flag.
    // The caller of run*() detects this and aborts the execution.
    // It then cleans up in this fashion.
    // This is better because between those two, it can inspect the stacks
    // to give a more informative error message (a backtrace, basically).

    // Back to root environment, but don't discard that.
    while( ectx->environments->cdr ) {
        ritual_list_next( ectx->ctx.inst, &ectx->environments );
    }

    // Discard all scheduled instructions.
    ectx->ctx.sequences = 0;

    // Discard all current values and return rv.
    ectx->ctx.values = 0;
    ritual_list_push( ectx->ctx.inst, &ectx->ctx.values, rv );
}

void rl3ext_general_error( struct rl3_context *ctx, ritual_object_t *arg ) {
    ritual_rl3_clear_context( (struct ritual_rl3_extended_context*) ctx, 0 );

    ritual_error( ctx->inst, "generic unhelpful error!" );
}

void rl3ext_eval( struct rl3_context *ctx, ritual_object_t *arg ) {
//    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ritual_object_t *obj = ritual_list_peek( ctx->inst, ctx->values );

    if( ritual_selfevaluatingp( ctx->inst, obj ) ) {
        return;
    }

    int success;
    obj = ritual_preevaluate( ctx->inst, obj, &success );

    if( !success ) {
        // TODO proper evaluation isn't implemented yet
        rl3ext_general_error(ctx,0);
    } else {
        ctx->values->car = obj;
    }
}

void rl3ext_eval_discard( struct rl3_context *ctx, ritual_object_t *arg ) {
        // TODO unoptimized..
    rl3ext_eval( ctx, 0 );
    rl3_ins_discard( ctx, 0 );
}

void rl3ext_taileval( struct rl3_context *ctx, ritual_object_t *arg ) {
    ritual_object_t *obj = ritual_list_peek( ctx->inst, ctx->values );
    if( RITUAL_TYPE(obj) != RTYPE_PAIR ) {
        rl3ext_eval( ctx, 0 );
        return;
    }
    // TODO proper tail-context evaluation definitely isn't implemented yet
    rl3ext_general_error(ctx,0);
}

void rl3ext_env_push( struct rl3_context *ctx, ritual_object_t *arg ) {
    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ritual_list_push( ctx->inst, &ectx->environments, arg );
}

void rl3ext_env_discard( struct rl3_context *ctx, ritual_object_t *arg ) {
    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ritual_list_next( ctx->inst, &ectx->environments );
}

void rl3ext_env_replace( struct rl3_context *ctx, ritual_object_t *arg ) {
    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ectx->environments->car = arg;
}

void rl3ext_env_bind( struct rl3_context *ctx, ritual_object_t *arg ) {
    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    struct ritual_symbol *sym = rconvto_symbol( ctx->inst, arg );
    struct ritual_env *env = (struct ritual_env*) ectx->environments->car;
    ritual_object_t * object = ritual_list_next( ctx->inst, &ctx->values );
    ritual_env_define( ctx->inst, env, sym->name, object );
}

void ritual_initialize_rl3_extensions( struct rl3_global_context* gctx, struct ritual_rl3_extensions* ext ) {
    ext->EVAL = rl3_register_instruction( gctx, rl3ext_eval, "EVAL" );
    ext->EVAL_DISCARD = rl3_register_instruction( gctx, rl3ext_eval_discard, "EVAL-DISCARD" );
    ext->TAILEVAL = rl3_register_instruction( gctx, rl3ext_taileval, "TAILEVAL" );

    ext->ENV_PUSH = rl3_register_instruction( gctx, rl3ext_env_push, "ENV-PUSH" );
    ext->ENV_DISCARD = rl3_register_instruction( gctx, rl3ext_env_discard, "ENV-DISCARD" );
    ext->ENV_REPLACE = rl3_register_instruction( gctx, rl3ext_env_replace, "ENV-REPLACE" );
    ext->ENV_BIND = rl3_register_instruction( gctx, rl3ext_env_bind, "ENV-BIND" );

    ext->GENERAL_ERROR = rl3_register_instruction( gctx, rl3ext_general_error, "GENERAL-ERROR" );

    ext->CALL_NATIVE = rl3_register_instruction( gctx, rl3ext_call_native, "CALL-NATIVE" );
}

void ritual_initialize_extended_rl3_context( struct ritual_rl3_extended_context *ectx, struct rl3_global_context* gctx, struct ritual_instance *inst, struct ritual_env *root ) {
    rl3_context_init( &ectx->ctx, gctx, inst );

    ectx->ext = inst->rl3_ext;

    ectx->environments = 0;
    ritual_list_push( inst, &ectx->environments, (ritual_object_t*) root );
}

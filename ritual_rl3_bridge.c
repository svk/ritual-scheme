#include "ritual_rl3_bridge.h"

#include "ritual_env.h"
#include "ritual_error.h"

#include "ritual_basic_types.h"

#include "ritual_envproc.h"



#include <assert.h>

struct rl3_instr ** ritual_rl3_make_arglist_parser( struct ritual_rl3_extended_context *ectx,
                                                    ritual_object_t *arglist,
                                                    struct rl3_instr ** last ) {
    struct rl3_global_context *gctx = ectx->ctx.global;
    struct ritual_rl3_extensions *ext = ectx->ext;
    struct ritual_instance *inst = ectx->ctx.inst;

    struct rl3_instr *too_many_arguments = rl3_mkinstr( inst, ext->GENERAL_ERROR, 0, 0 );
    // should probably discard some stuff -- important?
    
    while( arglist ) {
        switch( RITUAL_TYPE( arglist ) ) {
            case RTYPE_PAIR:
                {
                    struct ritual_pair *pair = rconvto_pair( inst, arglist );
                    struct ritual_symbol *sym = rconvto_symbol( inst, pair->car );
                    arglist = pair->cdr;

                    last = rl3_seqinstr( inst, gctx->SPLIT_PAIR, 0, last, 0 );
                    last = rl3_seqinstr( inst, ext->ENV_BIND, rconvfrom_symbol( inst, sym ), last, 0 );

                    break;
                }
            case RTYPE_SYMBOL:
                {
                    struct ritual_symbol *sym = rconvto_symbol( inst, arglist );
                    arglist = 0;

                    last = rl3_seqinstr( inst, ext->ENV_BIND, rconvfrom_symbol( inst, sym ), last, 0 );
                    break;
                }
            default:
                ritual_error( inst, "invalid argument list" );
        }
    }

        // discard the nil, verifying that it's actually nil
    last = rl3_seqinstr( inst, gctx->IS_NULL, 0, last, 0 );
    last = rl3_seqinstr( inst, gctx->BRANCH_NOT, (ritual_object_t*) too_many_arguments, last, 0 );
    last = rl3_seqinstr( inst, gctx->DISCARD, 0, last, 0 );

    return last;
}

void rl3ext_call_native( struct rl3_context *ctx, ritual_object_t *arg ) {
    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    struct ritual_envproc *envproc = (struct ritual_envproc*) arg;
    assert( RITUAL_TYPE( arg ) == RTYPE_ENVPROC );

    struct ritual_env *env = (struct ritual_env *) ectx->environments->car;
    assert( RITUAL_TYPE(env) == RTYPE_ENVIRONMENT );

    envproc->procedure( ctx->inst, env );
}

void rl3ext_general_error( struct rl3_context *ctx, ritual_object_t *arg ) {
    ritual_error( ctx->inst, "generic unhelpful error!" );
}

void rl3ext_eval( struct rl3_context *ctx, ritual_object_t *arg ) {
//    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ritual_error( ctx->inst, "not implemented yet!" );
}

void rl3ext_eval_discard( struct rl3_context *ctx, ritual_object_t *arg ) {
//    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ritual_error( ctx->inst, "not implemented yet!" );
}

void rl3ext_taileval( struct rl3_context *ctx, ritual_object_t *arg ) {
//    struct ritual_rl3_extended_context *ectx = (struct ritual_rl3_extended_context*) ctx;
    ritual_error( ctx->inst, "not implemented yet!" );
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
    ext->EVAL = rl3_register_instruction( gctx, rl3ext_eval );
    ext->EVAL_DISCARD = rl3_register_instruction( gctx, rl3ext_eval_discard );
    ext->TAILEVAL = rl3_register_instruction( gctx, rl3ext_taileval );

    ext->ENV_PUSH = rl3_register_instruction( gctx, rl3ext_env_push );
    ext->ENV_DISCARD = rl3_register_instruction( gctx, rl3ext_env_discard );
    ext->ENV_REPLACE = rl3_register_instruction( gctx, rl3ext_env_replace );
    ext->ENV_BIND = rl3_register_instruction( gctx, rl3ext_env_bind );

    ext->GENERAL_ERROR = rl3_register_instruction( gctx, rl3ext_general_error );

    ext->CALL_NATIVE = rl3_register_instruction( gctx, rl3ext_call_native );
}

void ritual_initialize_extended_rl3_context( struct ritual_rl3_extended_context *ectx, struct rl3_global_context* gctx, struct ritual_instance *inst, struct ritual_env *root ) {
    rl3_context_init( &ectx->ctx, gctx, inst );

    ectx->environments = 0;
    ritual_list_push( inst, &ectx->environments, (ritual_object_t*) root );
}

#include "ritual_rl3_bridge.h"

#include "ritual_env.h"
#include "ritual_error.h"

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
}

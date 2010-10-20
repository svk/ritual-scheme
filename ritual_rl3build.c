#include "ritual_rl3build.h"
#include "ritual_rl3_bridge.h"

/* calling a function; nontail:
 *    [mapeval]
 *    env-create-push func.env
 *    [bindargs]
 *    call func.body
 *    env-discard
 * calling a function; tail:
 *    [mapeval]
 *    env-create-replace
 *    [bindargs]
 *    call func.body
 * assertion: this is the only difference between eval and taileval
 *
 * eval_discard is just a [probably silly] optimization
 *
 * keywords we have so far and will miss:
 *   define
 *   lambda
 *   if     \
 *           \_ these should translate pretty easily
 *   and     /
 *   or     /
 *   let
 *   let*
 *
 *   begin
 *
 * lambda can get arbitrarily complex with ambitions of compilation,
 * but the initial version is simple: (create an object that includes
 * code to:) eval_discard each nonfinal form, taileval the last form
 */

struct rl3_instr* rl3build_mapeval( struct ritual_instance *inst,
                                    struct rl3_global_context *gctx,
                                    struct rl3_instance *** exit_success ) {
        /* Transform (a b c d) to (a' b' c' d') by evaluating
         * in the current environment. */
    struct ritual_rl3_extensions *ext = (struct ritual_rl3_extensions*) gctx;

    struct rl3_instr *rv;
    struct rl3_instr **w = &rv;
    
    struct rl3_instr *finale;
    struct rl3_instr **v = &finale;

    v = rl3_seqinstr(inst, gctx->DISCARD, 0, v, 0 );

    struct rl3_instr *looplabel;

    w = rl3_seqinstr(inst, gctx->STORE, 0, w, 0 );
    w = rl3_seqinstr(inst, gctx->SWAP, 0, w, 0 );

    w = rl3_seqinstr(inst, gctx->IS_NULL, 0, w, &looplabel );
    w = rl3_seqinstr(inst, gctx->BRANCH, (ritual_object_t*) finale, w, 0 );
    w = rl3_seqinstr(inst, gctx->SPLIT_PAIR, 0, w, 0 );
    w = rl3_seqinstr(inst, ext->EVAL, 0, w, 0 );
    w = rl3_seqinstr(inst, gctx->ROTATE, 0, w, 0 );
    w = rl3_seqinstr(inst, gctx->SWAP, 0, w, 0 );
    w = rl3_seqinstr(inst, gctx->CONS, 0, w, 0 );
    w = rl3_seqinstr(inst, gctx->SWAP, 0, w, 0 );
    w = rl3_seqinstr(inst, gctx->JUMP, (ritual_object_t*) looplabel, w, 0 );

    if( exit_success ) {
        *exit_success = v;
    }

    return rv;
}

struct rl3_instr* rl3build_argbind( struct ritual_instance *inst,
                                    struct rl3_global_context *gctx,
                                    ritual_object_t *arglist,
                                    struct rl3_instance *** exit_success,
                                    struct rl3_instance *** exit_too_few,
                                    struct rl3_instance *** exit_too_many,
                                    struct rl3_instance *** exit_broken ) {
    struct rl3_instr *pass_too_few = rl3_mkinstr( inst, gctx->PASS, 0, 0 );
    struct rl3_instr *pass_too_many = rl3_mkinstr( inst, gctx->PASS, 0, 0 );
    struct rl3_instr *pass_broken = rl3_mkinstr( inst, gctx->PASS, 0, 0 );

    if( exit_too_few ) {
        *exit_too_few = &pass_too_few->next;
    }
    if( exit_too_many ) {
        *exit_too_many = &pass_too_many->next;
    }
    if( exit_broken ) {
        *exit_broken = &pass_broken->next;
    }

    struct rl3_instr *rv;
    struct rl3_instr **w = &rv;
    int did_rest = 0;

    while( arglist ) {
        w = rl3_seqinstr(inst, gctx->IS_NULL, 0, w, 0 );
        w = rl3_seqinstr(inst, gctx->BRANCH, (ritual_object_t*) pass_too_few, w, 0 );
        w = rl3_seqinstr(inst, gctx->IS_PAIR, 0, w, 0 );
        w = rl3_seqinstr(inst, gctx->BRANCH_NOT, (ritual_object_t*) pass_broken, w, 0 );

        switch( RITUAL_TYPE( arglist ) ) {
            case RTYPE_PAIR:
                {
                    struct ritual_pair *pair = rconvto_pair( inst, arglist );
                    arglist = pair->cdr;
                    if( RITUAL_TYPE( pair->car ) != RTYPE_SYMBOL ) {
                        ritual_error( inst, "broken parameter list (nonsymbol)" );
                    }

                    w = rl3_seqinstr(inst, gctx->SPLIT_PAIR, 0, w, 0 );
                    w = rl3_seqinstr(inst, ext->ENV_BIND, pair->car, w, 0 );
                }
            case RTYPE_SYMBOL:
                {
                    w = rl3_seqinstr(inst, ext->ENV_BIND, arglist, w, 0 );
                    arglist = 0;
                    did_rest = 1;
                    break;
                }
            default:
                ritual_error( inst, "broken parameter list (nonlist)" );
        }
    }

    if( !did_rest ) {
        w = rl3_seqinstr(inst, gctx->IS_NULL, 0, w, 0 );
        w = rl3_seqinstr(inst, gctx->BRANCH_NOT, (ritual_object_t*) pass_too_many, w, 0 );
    }

    if( exit_success ) {
        *exit_success = w;
    }

    return rv;
}

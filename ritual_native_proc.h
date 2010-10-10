#ifndef H_RITUAL_NATIVE_PROC
#define H_RITUAL_NATIVE_PROC

#include "ritual_object.h"
#include "ritual_flo.h"
#include "ritual_env.h"
#include "ritual_basic_types.h"

/* A native procedure takes an UNEVALUATED Scheme list
 * of arguments. */

/* TODO: Do we need to be concerned about TCO here?
 *       If so, the environment is probably important. */
/* An argument for no: whether TCO can be applied depends
 * on the implementation of the procedure. Since any
 * builtin function is opaque, the user cannot expect
 * any special optimization from them. However, as the
 * implementor I must take care not to use recursion
 * in a way that relies on TCO in my implementation of
 * the native procedures. (Aka, write C-style loops;
 * it's not magically safe to write Scheme-style
 * recursion in C just because you're writing a Scheme
 * procedure.) */

typedef ritual_object_t* (*ritual_native_procedure_t) (
    struct ritual_instance*,
    struct ritual_env*,
    struct ritual_pair* );

struct ritual_native_proc {
    ritual_object_t header;
    ritual_native_procedure_t procedure;
};

struct ritual_native_proc * ritual_native_proc_create(
    struct ritual_instance *,
    ritual_native_procedure_t);

void ritual_define_native_proc( struct ritual_instance *,
                                struct ritual_env *,
                                const char *,
                                ritual_native_procedure_t);

void ritual_print_native_proc(
    struct ritual_instance *,
    struct ritual_flo *,
    void * );

    /* One native procedure as a demo.
     * (As of yet it always returns false, because we have
     *  no way to call it on two equal objects...) */
    /* Actually, we now have booleans for that! */
    /* Only after we reflex :( */
    /* No! (eq? (eq? 1 1) (eq? 0 0)) :P */
ritual_object_t * rnp_eqp( struct ritual_instance *,
                           struct ritual_env *,
                           struct ritual_pair * );

ritual_object_t * rnp_ritual_print_diagnostics( struct ritual_instance *,
                                                struct ritual_env *,
                                                struct ritual_pair * );

    /* Shocker: the Scheme keywords (lambda, define, and so on)
     * are supposed to be redefinable. This means that they're not so
     * much keywords that should be understood as tokens on the bison/flex
     * level as builtin procedures. So here they are (well, not all of
     * them, yet): */

    /* Of course, now it comes in handy that the arglist is unevaluated.
     * I wish that was foresight and not luck! */

ritual_object_t * rnp_define( struct ritual_instance *,
                              struct ritual_env *,
                              struct ritual_pair * );

ritual_object_t * rnp_lambda( struct ritual_instance *,
                              struct ritual_env *,
                              struct ritual_pair * );

#endif

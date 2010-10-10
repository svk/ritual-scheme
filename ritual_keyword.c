#include "ritual_keyword.h"

#include "ritual_error.h"
#include "ritual_instance.h"
#include "ritual_native_proc.h"
#include "ritual_env.h"

struct ritual_keyword * ritual_keyword_create( struct ritual_instance *inst,
                                               int value,
                                               ritual_native_procedure_t rnp) {
    struct ritual_keyword *rv;
    rv = ritual_alloc_typed_object( inst, RTYPE_KEYWORD, sizeof *rv );
    RITUAL_ASSERT( inst, rv, "object allocation failure should not return" ); 
    rv->value = value;
    rv->rnp = rnp;
    RITUAL_ASSERT( inst, rv->value || rv->rnp, "meaningless keyword" );
    RITUAL_ASSERT( inst, !rv->value || !rv->rnp, "double-booked keyword" );
    return rv;
}

void ritual_define_keyword( struct ritual_instance* inst,
                            struct ritual_env* env,
                            const char *name,
                            int value ) {
    struct ritual_keyword *kw = ritual_keyword_create( inst, value, 0 );
    ritual_env_define( inst, env, name, (ritual_object_t*) kw );
}

void ritual_define_rnp_as_keyword( struct ritual_instance* inst,
                                   struct ritual_env* env,
                                   const char *name,
                                   ritual_native_procedure_t proc ) {
    struct ritual_keyword *kw = ritual_keyword_create( inst, 0, proc );
    ritual_env_define( inst, env, name, (ritual_object_t*) kw );
}

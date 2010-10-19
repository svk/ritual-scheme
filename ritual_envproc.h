#ifndef H_RITUAL_ENVPROC
#define H_RITUAL_ENVPROC

#include "ritual_instance.h"
#include "ritual_object.h"
#include "ritual_basic_types.h"

#include "rl3.h"

typedef ritual_object_t* (*ritual_envproc_t) (
        struct ritual_instance*,
        struct ritual_env*);

ritual_object_t * renvp_ritual_typename( struct ritual_instance *,struct ritual_env *);

struct ritual_envproc {
    ritual_object_t header;

    ritual_object_t *params;
    ritual_envproc_t procedure;

    struct rl3_instr *entry;
};

// This is for compatibility for before RL3 is implemented

struct ritual_env * ritual_envproc_bind( struct ritual_instance *,
                          struct ritual_env *,
                          ritual_object_t *,
                          struct ritual_pair * );

void renvp_define_proper( struct ritual_instance *,
                           struct ritual_env *,
                           ritual_envproc_t ,
                           const char *,
                           ... );

#endif

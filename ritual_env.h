#ifndef H_RITUAL_ENV
#define H_RITUAL_ENV

#include "ritual_instance.h"
#include "ritual_object.h"
#include "ritual_hash_table.h"

#include "ritual_basic_types.h"

struct ritual_env {
    ritual_object_t header;
    struct ritual_env *parent;
    struct rht_table table;
};

#define RITUAL_ENV_SIZE_ROOT 1024
#define RITUAL_ENV_SIZE_SUB 8

void ritual_env_init_root(struct ritual_instance *,
                          struct ritual_env *);
void ritual_env_init_sub(struct ritual_instance *,
                         struct ritual_env *,
                         struct ritual_env *);
void ritual_env_destroy( struct ritual_instance *,
                         void * );

void ritual_env_set( struct ritual_instance *,
                     struct ritual_env *,
                     const char *,
                     ritual_object_t * );
void ritual_env_define( struct ritual_instance *,
                        struct ritual_env *,
                        const char *,
                        ritual_object_t * );
ritual_object_t * ritual_env_lookup( struct ritual_instance *,
                                     struct ritual_env *,
                                     const char * );



struct ritual_env * ritual_let_env( struct ritual_instance *,
                                    struct ritual_env *,
                                    struct ritual_pair * );
struct ritual_env * ritual_let_star_env( struct ritual_instance *,
                                         struct ritual_env *,
                                         struct ritual_pair * );
struct ritual_env * ritual_letrec_env( struct ritual_instance *,
                                       struct ritual_env *,
                                       struct ritual_pair * );


struct ritual_env * ritual_env_sub_create( struct ritual_instance *, struct ritual_env * );

#endif

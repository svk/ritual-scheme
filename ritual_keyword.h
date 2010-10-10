#ifndef H_RITUAL_KEYWORD
#define H_RITUAL_KEYWORD

#include "ritual_instance.h"
#include "ritual_native_proc.h"

struct ritual_keyword {
    ritual_object_t header;
    int value;
    ritual_native_procedure_t rnp;
};
struct ritual_keyword * ritual_keyword_create(
    struct ritual_instance *,
    int value,
    ritual_native_procedure_t );

void ritual_define_keyword( struct ritual_instance*,
                            struct ritual_env*,
                            const char *,
                            int );
void ritual_define_rnp_as_keyword( struct ritual_instance*,
                                   struct ritual_env*,
                                   const char *,
                                   ritual_native_procedure_t );

enum {
    RKW_INVALID = 0,
    RKW_IF,
    RKW_BEGIN,
    RKW_AND,
    RKW_OR,
    RKW_LET,
    RKW_LET_STAR,
    RKW_LETREC
};

#endif

#ifndef H_RITUAL_ARITHMETIC
#define H_RITUAL_ARITHMETIC

#include "ritual_native_proc.h"

ritual_object_t* rnp_add( struct ritual_instance*,
                          struct ritual_env*,
                          struct ritual_pair * );
ritual_object_t* rnp_sub( struct ritual_instance*,
                          struct ritual_env*,
                          struct ritual_pair * );
ritual_object_t* rnp_mul( struct ritual_instance*,
                          struct ritual_env*,
                          struct ritual_pair * );


#endif

#ifndef H_RITUAL_RL3BUILD
#define H_RITUAL_RL3BUILD

#include "rl3.h"

/* Convention: return the first generated instruction.
 *             Set each exit point to NIL, but return pointers
 *             to these.
 *             Caller links up (to "prepend") by changing the exit pointer
 *             to point to the current beginning, and then setting the new
 *             beginning as the returned value. */

struct rl3_instr* rl3build_mapeval( struct ritual_instance *,
                                    struct rl3_global_context *,
                                    struct rl3_instance *** );

struct rl3_instr* rl3build_argbind( struct ritual_instance *,
                                    struct rl3_global_context *,
                                    ritual_object_t *,
                                    struct rl3_instance ***,
                                    struct rl3_instance ***,
                                    struct rl3_instance ***,
                                    struct rl3_instance *** );

#endif

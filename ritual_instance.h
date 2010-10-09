#ifndef H_RITUAL_INSTANCE
#define H_RITUAL_INSTANCE

/* Because we aim for multithreading, we can't have nonconst globals,
 * so all that semantically would be global goes here. Notably things
 * like the execution environment and the entire memory model, as well
 * as global locks if we chicken out and put in that.
 *
 * We need an instance to create objects (because the garbage collector
 * and the memory management is associated with an instance, which
 * allows us to e.g. impose quotas, down the line) -- this means that
 * the parser needs access to one in the context. */

struct ritual_instance {
    int dummy;
};

#endif

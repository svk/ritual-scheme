#include "ritual_gc.h"

/* For now we have the simplest of all possible "garbage
 * collection" schemes: we just make sure that we release
 * all memory when the instance shuts down. Internally
 * we still leak all memory. */

/* Thinking about GC:
 *   - C functions may need to declare which objects they
 *     can reach -- basically reference counting -- as
 *     I'm not going to scan through C memory automatically.
 *   - The worst-case scenario to keep in mind is that the
 *     C code basically halts, and the garbage collector
 *     runs an arbitrary number of times before returning
 *     control to C -- its objects must still live.
 *   - Is there a clever solution that does not involve
 *     reference counting?
 *   - C functions that use values:
 *     - C functions that _create_ values.
 *       (Including the reader!)
 *     - Native procedures, of various sorts.
 *     - Eval.
 *   - To be safe, we cannot, e.g. do "grab" on the first
 *     line of a native procedure and "release" on the last.
 *     Something could happen _before_ the first line.
 *     Whatever _calls_ the native proc needs to do any
 *     grabbing and releasing. (This is a relief, since it
 *     means that it won't get drastically more cumbersome
 *     to write native procs.) This means, probably, eval.
 *   - What -- ultimately -- happens to new objects that
 *     are allocated? (Separate possibilities.) Note that
 *     "ultimately" means that, after this point, C should
 *     not touch these variables anymore. This is subtle
 *     enough that we should probably null out the value
 *     to enforce it.
 *        (a) They are assigned to some permanent C structure
 *            (e.g. the instance struct where the constants
 *            and functions reside). This is not a problem;
 *            we can follow references from this structure.
 *            In the general case we need to have the variables
 *            grabbed if we're going to use them; however,
 *            this is the case usually only for constants,
 *            so we can simply leave them grabbed, never
 *            releasing them.
 *        (b) They are assigned to a Scheme environment.
 *            This is the main case, also relatively easy
 *            to handle (as long as we can keep track of
 *            the environments themselves).
 *        (c) They are used internally in C, go out of scope,
 *            and are never used again. Such objects
 *            need explicit "unpinning".
 *   - All objects start off pinned and on the "C objects"
 *     list, and are also immediately added to the current
 *     gray set. The "C objects" list is referenced from
 *     the root set.
 *   - The garbage collector periodically scans the
 *     "C objects" list and removes any objects that are
 *     not flagged as pinned. 
 *     - Generations might work well here, as many objects
 *       are going to be very short-lived.
 *     - At the cost of _two_ pointers in every damn object
 *       instead of a flag, we could straight-up remove
 *       from the list instead of unpinning (XOR linked
 *       list). However, that'd be a questionable move wrt.
 *       conserving memory, which after all is what a GC
 *       is supposed to do. After flag-unpinning, the
 *       GC can in principle get at the object right away
 *       (that is, it won't cause any OOM errors -- as long
 *       as we scan the list completely and then do a fresh
 *       GC before giving up and declaring OOM), so why worry?
 *   - This actually works. The major usage difficulty is making
 *     sure C does not leak objects. This should at least be
 *     testable in each case, as we can write a function to
 *     measure the cobjects list. (That's actually amusing --
 *     could we, accurately? We'd have to lock the list and 
 *     cache the structure in C memory, I think.)
 */  
/* So, practically speaking, what we need:
 *  - the cobjects list, an olist onto which all newly allocated
 *    objects are pushed. (Perhaps _two_, so we can clean up one
 *    while adding to the other? GC acquires both locks and switches
 *    at the end.) Locking for this construct.
 *  - a function to add any object directly referenced from the
 *    root set, including the cobject olist(s), to the gray set.
 *  - a function that, given an object, adds all its outgoing
 *    references to the gray set and blackens the object.
 *    (The best way to build this is with a generic function
 *     which does only the first part -- run a specific
 *     void (*p)(void*,ritual_object_t*) on each outgoing
 *     reference.)
 *  - a way to loop through all allocated objects (UMPs, plus linked
 *    lists of large objects) and apply 

void rgc_initialize( struct ritual_instance *inst,
                     struct ritual_gc_instance * gc ) {
    gc->all = 0;
    gc->gray = 0;
}

void rgc_deinitialize( struct ritual_instance *inst,
                       struct ritual_gc_instance * gc ) {
    ritual_olist_destroy( inst, &gc->gray );
    while( gc->all ) {
        ritual_object_t * object = ritual_olist_pop( inst, &gc->all );
        ritual_free_object( inst, object );
    }
}

void rgc_allocated_object( struct ritual_instance *inst,
                           ritual_object_t *object ) {
    ritual_olist_push( inst, &inst->gc->all, object );
}

#ifndef H_RITUAL_FLO
#define H_RITUAL_FLO

#include <stdio.h>

#include "ritual_instance.h"

/* Note this ugliness: FLOs are, as of now, allocated in
 * non-instance memory (without use of an instance pointer).
 * When they are associated with the Scheme type ports,
 * that'll be a bit strange. For instance, quotas imposed
 * on the instance will not actually upper-bound the amount
 * of memory spent on the heap if we allow allocating
 * objects like this. */

struct ritual_flo {
    int (*write)(void*, const void*, int);
    int (*read)(void*, void*, int);
};

struct rflo_filehandle {
    struct ritual_flo flo;
    FILE *f;
};

    /* Try until you succeed or fail at writing all the
     * data -- don't return until it's done (but do
     * throw an exception). */
void rflo_putdata( struct ritual_flo *, const char *, int );
void rflo_putstring( struct ritual_flo *, const char * );
void rflo_putchar( struct ritual_flo *, char );

    /* Mostly for wrapping stdin, stdout, stderr -- note
     * that it does not deal with closing handles at all */
struct rflo_filehandle * rflo_filehandle_create( struct ritual_instance*, FILE *f );
void rflo_filehandle_destroy( struct ritual_instance*, struct rflo_filehandle * );


#endif

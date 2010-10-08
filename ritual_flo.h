#ifndef H_RITUAL_FLO
#define H_RITUAL_FLO

#include <stdio.h>

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
struct rflo_filehandle * rflo_filehandle_create( FILE *f );
void rflo_filehandle_destroy( struct rflo_filehandle * );


#endif

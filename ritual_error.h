#ifndef H_RITUAL_ERROR
#define H_RITUAL_ERROR

#include <stdarg.h>
#include <setjmp.h>

#include "ritual_instance.h"

#define ERROR_REASON_LENGTH 1024

struct ritual_error_instance {
    char reason[ ERROR_REASON_LENGTH ];

    int top_valid;
    jmp_buf top;
};

void ritual_error_initialize( struct ritual_error_instance * );

void ritual_error_jump( struct ritual_error_instance * );

void ritual_error( struct ritual_instance *, const char *, ... );

void ritual_error_str( struct ritual_instance *, const char * );

#define RITUAL_TOP_LEVEL_ERROR(scheme) \
        ( (scheme)->error->top_valid = 1, setjmp( (scheme)->error->top ) )

/* Note that assertions should NOT be used to test for errors
 * that might actually occur, such as fatal out-of-memory
 * conditions. This is because they'll be stripped out of production
 * builds, leading to inexplicable crashes if the error does occur.
 * We can do better: using ritual_error*() can at the very least
 * perform a longjmp to a top-level error handler that can
 * deinitialize and/or restart the Scheme interpreter and allow
 * the containing application to continue unscathed, assuming
 * the interpreter was a nonessential scripting component. */
/* A valid use of assertions is to check for an error that SHOULD
 * already have been caught by an earlier error checking routine,
 * e.g. verifying that a routine which should never return NULL
 * never does return NULL. */
#ifndef STRIP_ASSERTIONS
#define RITUAL_ASSERT(inst,cond,reason) { \
    if( !(cond) ) { \
        ritual_error_str( (inst), \
         "RITUAL_ASSERT() failed in " __FILE__ " -- " \
         reason ); \
    }}
#else
#define RITUAL_ASSERT(inst,cond,reason) ;
#endif

#endif

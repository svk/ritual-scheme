#include "ritual_memory.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <time.h>
#include <stdlib.h>

int test_gc_all(struct ritual_ump *ump,
                void *data) {
    return 1;
}

int test_gc_even(struct ritual_ump *ump,
                 void *data) {
    int number;
    sscanf( data, "This is number %d", &number );
    return (number % 2) == 0;
}

int test_gc_24601(struct ritual_ump *ump,
                  void *data) {
    const char *s = data;

    if( !strcmp( s, "This is number 24601" ) ) {
        fprintf(stderr, "Freeing #24601 at %p\n", data ); 
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    struct ritual_ump *ump = ritual_ump_create( 100000, 64 );
    int allocated = 0;

    while( 1 ) {
        int result = ritual_ump_alloc(ump);
        if( result < 0 ) break;
        char *s = ritual_ump_id_to_pointer( ump, result );
        sprintf( s, "This is number %d", ++allocated );
    }

    fprintf(stderr, "Created %d cells (expected: 32768)\n", allocated );

    fprintf(stderr, "Now freeing #24601:\n" );
    ritual_ump_clean( ump, test_gc_24601 );

    allocated = 0;
    while( 1 ) {
        int result = ritual_ump_alloc(ump);
        if( result < 0 ) break;
        char *s = ritual_ump_id_to_pointer( ump, result );
        sprintf( s, "This is number %d", ++allocated );
    }
    fprintf(stderr, "Allocated %d cells (expected: 1)\n", allocated );
    
    fprintf(stderr, "Now freeing all even numbers (should be 16385).\n" );
    ritual_ump_clean( ump, test_gc_even );

    allocated = 0;
    while( 1 ) {
        int result = ritual_ump_alloc(ump);
        if( result < 0 ) break;
        char *s = ritual_ump_id_to_pointer( ump, result );
        sprintf( s, "This is number %d", ++allocated );
    }
    fprintf(stderr, "Allocated %d cells (expected: 16385)\n", allocated );
    ritual_ump_clean( ump, test_gc_all );

    clock_t zero = clock(), one;
    int allocs = 0, wipes = 0;

    int arrsize = 32768, i;
    void **pointers = malloc( sizeof *pointers * arrsize );

    memset( pointers, 0, sizeof *pointers * arrsize );
    i = 0;
    fprintf(stderr, "Stress-testing for five seconds.\n" );
    while( ((one = clock()) - zero) < 5000 ) {
        if( pointers[i] ) {
            ritual_ump_clean( ump, test_gc_all );
            memset( pointers, 0, sizeof *pointers * arrsize );
            wipes++;
        }
        pointers[i] = ritual_ump_id_to_pointer( ump, ritual_ump_alloc( ump ) );
        allocs++;
        i = (i+1) % arrsize;
    }
    fprintf(stderr, "%d ticks elapsed; allocs=%d, frees=%d.\n", one - zero, allocs, wipes );

    allocs = wipes = 0;

    zero = clock();

    memset( pointers, 0, sizeof *pointers * arrsize );
    i = 0;
    fprintf(stderr, "Stress-testing malloc()/free() for five seconds.\n" );
    while( ((one = clock()) - zero) < 5000 ) {
        if( pointers[i] ) {
            for(int j=0;j<arrsize;j++) {
                free( pointers[j] );
            }
            memset( pointers, 0, sizeof *pointers * arrsize );
            wipes++;
        }
        pointers[i] = malloc( 64 );
        allocs++;
        i = (i+1) % arrsize;
    }
    fprintf(stderr, "%d ticks elapsed; allocs=%d, frees=%d.\n", one - zero, allocs, wipes );
    ritual_ump_free_tree( ump );

    /* Result (on my machine) -- UMP slightly underperforms compared to
     * malloc() for small values (including 64). However, it's now
     * possible to apply garbage collection easily, without keeping
     * a gargantuan linked list, and we won't suffer from
     * fragmentation.
     *
     * The time to lock/unlock a mutex turns out to be significant,
     * amounting to a 25% or so speed increase if I omit it
     * (which can be done in single-threaded programs but not in
     * multithreaded ones).
     *
     * Yay: with the FUB optimization UMP is significantly faster
     * than even single-byte malloc(), even with locks left in.
     */

    return 0;
}

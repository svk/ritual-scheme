#include "ritual_ump.h"

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

struct ritual_dynump * ritual_dynump_create(int sz) {
    struct ritual_dynump *rv = malloc(sizeof *rv);
    rv->element_size = sz;
    rv->current = ritual_ump_create( sz );
    if( !rv->current ) {
        free( rv );
        return 0;
    }
    if( pthread_mutex_init( &rv->mutex, 0 ) ) {
        ritual_ump_free( rv->current );
        free( rv );
        return 0;
    }
    return rv;
}

void ritual_dynump_free(struct ritual_dynump *dynump) {
    pthread_mutex_destroy( &dynump->mutex );

    while( dynump->current != dynump->current->next ) {
        ritual_ump_free( dynump->current->next );
    }
    ritual_ump_free( dynump->current );

    free( dynump );
}

void * ritual_dynump_alloc(struct ritual_dynump *dynump) {
    const int tries = 5;
    struct ritual_ump *current;
    void *rv;
    for(int i=0;i<tries;i++) {
        current = dynump->current;
        dynump->current = current->next;
        rv = ritual_ump_try_alloc( current );
        if( rv ) {
            return rv;
        }
    }

    pthread_mutex_lock( &dynump->mutex );

    rv = ritual_ump_alloc( dynump->current );
    if( rv ) {
        pthread_mutex_unlock( &dynump->mutex );
        return rv;
    }

    struct ritual_ump *ump = ritual_ump_create( dynump->element_size );
    if( !ump ) {
        // we really are out of memory.. (ritual_ump_create should
        // be using some sort of retrying malloc which runs GC twice
        // to be absolutely sure before returning failure, but still,
        // this can happen.)
        return 0;
    }

    ump->next = dynump->current;
    ump->prev = dynump->current->prev;
    ump->prev->next = ump;
    ump->next->prev = ump;
    dynump->current = ump;

    pthread_mutex_lock( &dynump->current->mutex );
    pthread_mutex_unlock( &dynump->mutex );

    rv = ritual_ump_alloc_unsafe( ump, 1 );
    assert( rv );

    return rv;
}

struct ritual_ump * ritual_ump_create(int sz) {
    struct ritual_ump *rv;

    rv = malloc( sizeof *rv );
    memset( rv, 0, sizeof *rv ); // no need to clear data

    rv->element_size = sz;

    if( pthread_mutex_init( &rv->mutex, 0 ) ) {
        free( rv );
        return 0;
    }

    rv->data = malloc( sz * RITUAL_UMP_SIZE );

    rv->prev = rv->next = rv;

    return rv;
}

void ritual_ump_free(struct ritual_ump *ump) {
    if( ump ) {
        if( ump->prev ) {
            ump->prev->next = ump->next;
        }
        if( ump->next ) {
            ump->next->prev = ump->prev;
        }
        pthread_mutex_destroy( &ump->mutex );
        free( ump->data );
        free( ump );
    }
}

const int ritual_fub_array[] = {
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, -1
};

#define FIRST_UNSET_BIT(x) \
    ( ritual_fub_array[x&0xff] >= 0 ) ? \
        ritual_fub_array[x&0xff] : \
        ( ritual_fub_array[(x>>8)&0xff] >= 0 ) ? \
            ritual_fub_array[(x>>8)&0xff] + 8 : \
            ( ritual_fub_array[(x>>16)&0xff] >= 0 ) ? \
                ritual_fub_array[(x>>16)&0xff] + 16 : \
                ( ritual_fub_array[(x>>24)] >= 0 ) ? \
                    ritual_fub_array[(x>>24)] + 24 : \
                        -1

void * ritual_ump_try_alloc(struct ritual_ump *ump) {
    if( pthread_mutex_trylock( &ump->mutex ) ) {
        return 0;
    }
    return ritual_ump_alloc_unsafe( ump, 1 );
}

void * ritual_ump_alloc(struct ritual_ump *ump) {
    return ritual_ump_alloc_unsafe( ump, 0 );
}

void * ritual_ump_alloc_unsafe(struct ritual_ump *ump, int was_locked) {
    int rv = -1, i, jb, j, k;
    if( !was_locked ) {
        pthread_mutex_lock( &ump->mutex );
    }
    do {
        i = FIRST_UNSET_BIT( ump->level1 );
        if( i < 0 ) break;
        jb = FIRST_UNSET_BIT( ump->level2[i] );
        assert( jb >= 0 );
        j = jb + (i << 5);
        k = FIRST_UNSET_BIT( ump->level3[j] );
        assert( k >= 0 );

        rv = (k + (j <<5));

        ump->level3[j] |= (1 << k);
        if( !~ump->level3[j] ) {
            ump->level2[i] |= (1 << jb);
            if( !~ump->level2[i] ) {
                ump->level1 |= (1 << i);
            }
        }

        ++ump->used;
    } while(0);
    pthread_mutex_unlock( &ump->mutex );
    return (rv < 0) ? 0 : &ump->data[ump->element_size * rv];
}

void ritual_ump_clean(struct ritual_ump *ump,
                      int (*gc)(struct ritual_ump*, void*) ) {
    pthread_mutex_lock( &ump->mutex );
    for(int i=0;i<1024;i++) if( ump->level3[i] ) {
        int cleaned = 0;
        uint32_t bit = 1;
        for(int j=0;j<32;j++) {
            if( bit & ump->level3[i] ) {
                void *data = &ump->data[((i << 5) + j)*ump->element_size];
                if( gc( ump, data ) ) {
                    ump->level3[i] ^= bit;
                    --ump->used;
                    cleaned = 1;
                }
            }
            bit <<= 1;
        }
        if( cleaned ) {
            int j = i / 32;
            bit = 1 << (i % 32);
            ump->level2[j] &= ~bit;
            bit = 1 << j;
            ump->level1 &= ~bit;
        }
    }
    pthread_mutex_unlock( &ump->mutex );
} 

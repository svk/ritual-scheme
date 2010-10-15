#include "ritual_memory.h"

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

struct ritual_ump * ritual_ump_create(int first_id, int sz) {
    struct ritual_ump *rv;

    rv = malloc( sizeof *rv );
    memset( rv, 0, sizeof *rv ); // no need to clear data

    rv->first_id = first_id;
    rv->element_size = sz;

    if( pthread_mutex_init( &rv->mutex, 0 ) ) {
        free( rv );
        return 0;
    }

    rv->data = malloc( sz * RITUAL_UMP_SIZE );

    rv->prev = rv->next = this;

    return rv;
}

void ritual_ump_free_tree(struct ritual_ump *ump) {
    if( ump ) {
        struct ritual_ump *left = ump->left,
                          *right = ump->right;
        pthread_mutex_destroy( &ump->mutex );
        free( ump->data );
        free( ump );
        ritual_ump_free_tree( left );
        ritual_ump_free_tree( right );
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

void * ritual_ump_alloc(struct ritual_ump *ump) {
    int rv = -1, i, jb, j, k;
    pthread_mutex_lock( &ump->mutex );
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

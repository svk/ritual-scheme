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

void * ritual_ump_id_to_pointer(struct ritual_ump *ump, int i) {
    const int ri = i - ump->first_id;
    assert( ri >= 0 && ri < RITUAL_UMP_SIZE );
    return &ump->data[ump->element_size * ri];
}

int first_unset_bit( uint32_t x ) {
    // fun optimization exercise for later.
    uint32_t bit = 1;
    for(int i=0;i<32;++i) {
        if( !(bit & x) ) return i;
        bit <<= 1;
    }
    return -1;
}

int ritual_ump_alloc(struct ritual_ump *ump) {
    int rv = -1, i, jb, j, k;
    pthread_mutex_lock( &ump->mutex );
    do {
        i = first_unset_bit( ump->level1 );
        if( i < 0 ) break;
        jb = first_unset_bit( ump->level2[i] );
        assert( jb >= 0 );
        j = jb + (i << 5);
        k = first_unset_bit( ump->level3[j] );
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
    return (rv < 0) ? rv : (rv + ump->first_id);
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

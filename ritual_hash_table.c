#include "ritual_hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#ifdef USE_JENKINS_HASH_32
/* lookup3.c; implementation and algorithm by Bob Jenkins.
 * Public domain. */
extern uint32_t hashlittle( const void*, size_t, uint32_t );

ritual_hash_t rht_get_hash( const void *s, int l ) {
    return hashlittle( s, l, 0 );
}
#endif

int rht_get_best_size( int lower_bound ) {
    /* Primality does not matter with Jenkins hash. */
    int rv = 1;
    while( rv < lower_bound ) {
        rv <<= 1;
    }
    return rv;
}

int rht_table_init( struct rht_table* table, int initial_size ) {
    memset( table, 0, sizeof *table );

    initial_size = rht_get_best_size( lower_bound );

    table->slot = malloc( sizeof *table->slot * initial_size );
    if( !table->slot ) return 1;

    memset( table->slot, 0, sizeof *table->slot * initial_size );
    table->slots = initial_size;

#ifdef RHT_THREAD_SAFE
    if( pthread_rwlock_init( &table->lock, 0 ) ) {
        free( table->slot );
        return 1;
    }
#endif

    return 0;
}

void rht_table_destroy( struct rht_table* table ) {
    while( table->slot ) {
        rht_entry_delete( &table->slot );
    }
    while( table->old_slot ) {
        rht_entry_delete( &table->old_slot );
    }
#ifdef RHT_THREAD_SAFE
    pthread_rwlock_destroy( &table->lock );
#endif
}

struct rht_entry * rht_entry_create( const void *key, int keylen,
                                     void **value ) {
    struct rht_entry *rv = malloc(sizeof *rv);
    if( !rv ) return 0;

    rv->key = malloc( keylen );
    if( !rv->key ) {
        free( rv );
        return 0;
    }

    memcpy( rv->key, key, keylen );
    rv->keylen = keylen;

    rv->value = value;

    rv->next = 0;

    return rv;
}

struct rht_entry ** rht_find_entry_in_list( struct rht_entry **list,
                                            const void* key, int keylen ) {
    for( struct rht_entry ** rv = list; rv; rv = rv->next ) {
        if( (*rv)->keylen != keylen ) continue;
        if( memcmp( (*rv)->key, key, keylen ) ) continue;
        return rv;
    }
    return 0;
}

struct rht_entry ** rht_find_entry( const struct rht_table* table,
                                    const void* key, int keylen ) {
    ritual_hash_t hash = rht_get_hash( key, keylen );
    struct rht_entry ** rv = 0;

    if( table->old_slot ) {
        int old_index = hash % table->old_slots;
        rv = rht_find_entry_in_list( &table->old_slot[ old_index ],
                                    key, keylen );
    }

    if( !rv ) {
        int index = hash % table->slots;
        rv = rht_find_entry_in_list( &table->slot[ index ], key, keylen );
    }

    return 0;
}

void rht_entry_delete( struct rht_entry **pp ) {
    /* Keys are owned, values are not. */
    if( *pp ) {
        struct rht_entry *next = (*pp)->next;
        free( (*pp)->key );
        free( *pp );
        *pp = next;
    }
}

int rht_lookup( const struct rht_table *table,
                const void *key, int keylen,
                void **value ) {
    int rv;
    struct rht_entry **pp;

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_rdlock( &table->lock );
#endif

    pp = rht_find_entry( table, key, keylen );
    if( pp ) {
        *value = (*pp)->value;
        rv = 0;
    } else {
        *value = 0;
        rv = 1;
    }

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_unlock( &table->lock );
#endif

    return rv;
}

int rht_set( struct rht_table *table,
             const void *key, int keylen,
             void *value ) {
    int rv;

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_wrlock( &table->lock );
#endif

    if( table->old_slot ) {
        for(int i=0;i<MOVES_PER_OPERATION;i++) {
            rht_do_move( table );
        }
    }

    int lfl = (table->entries + 1) * LOAD_FACTOR_DENOM;
    int lfr = REALLOC_ON_LOAD_FACTOR * table->slots;
    if( lfl > lfr ) {
        int newsize = rht_get_best_size( RESIZE_TO( table->slots ) );
        if( rht_realloc_table( table, newsize ) ) {
            /* Ignore allocation error here, continuing with
             * suboptimal load factor. */
            ;
#ifdef DEBUG
            fprintf( stderr, "warning: failed to reallocate table"
                             " of %d entries, %d slots\n",
                             table->entries, table->slots );
#endif
        }
    }
    struct rht_entry **pp = rht_find_entry( table, key, keylen );
    if( !pp ) {
        struct rht_entry * newentry = rht_entry_create( key, keylen, value );
        if( !newentry ) {
            rv = 1;
        } else {
            ritual_hash_t hash = rht_get_hash( key, keylen );
            int index = hash % table->slots;
            newentry->next = table->slot[index];
            table->slot[index] = newentry;
            ++table->entries;
            rv = 0;
        }
    } else {
        if( table->free_value ) {
            table->free_value( (*pp)->value );
        }
        (*pp)->value = value;
    }

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_unlock( &table->lock );
#endif

    return rv;
}

int rht_delete( struct rht_table *table, const void *key, int keylen ) {
    int rv;

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_wrlock( &table->lock );
#endif

    if( table->old_slot ) {
        for(int i=0;i<MOVES_PER_OPERATION;i++) {
            rht_do_move( table );
        }
    }

    pp = rht_find_entry( table, key, keylen );
    if( pp ) {
        rv = 0;
        if( table->free_value ) {
            table->free_value( (*pp)->value );
        }
        rht_entry_delete( pp );
        --table->entries;
    } else {
        rv = 1;
    }

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_unlock( &table->lock );
#endif

    return rv;
}

int rht_realloc_table( struct rht_table *table, int newsize ) {
    assert( !table->old_slot );

    struct rht_entry *newslot = malloc( sizeof *newslot * newsize );
    if( !newslot ) {
#ifdef DEBUG
        fprintf( stderr, "warning: failed to reallocate table\n" );
#endif
        return 1;
    }

    memset( newslot, 0, sizeof *newslot * newsize );
    table->old_slot = table->slot;
    table->old_slots = table->slots;
    table->first_old_index = 0;
    table->slot = newslot;
    table->entries = 0;
    table->slots = newsize;

#ifdef DEBUG_VERBOSE
    fprintf( stderr, "notice: beginning table reallocation"
                     " from %d to %d slots\n",
                     table->old_slots, table->slots );
#endif
    
    return 0;
}

int rht_do_move( struct rht_table *table ) {
    if( table->old_slot ) {
        while( table->first_old_index < table->old_slots &&
               !table->old_slot[ table->first_old_index ] ) {
            ++table->first_old_index;
        }
        
        if( table->first_old_index < table->old_slots ) {
            struct rht_entry **entryp = &table->old_slot[ table->first_old_index ];
            struct rht_entry *entry = *entryp;

            *entryp = entry->next; // delete from old

            ritual_hash_t hash = rht_get_hash( key, keylen );
            int index = hash % table->slots;
            entry->next = table->slot[index];
            table->slot[index] = entry;

            ++table->entries;
        } else {
#ifdef DEBUG_VERBOSE
            fprintf(stderr, "notice: transition completed, freeing old table\n" );
#endif
            free( table->old_slots );
            table->old_slots = 0;
        }

    }
    return 0;
}

int rht_lookup_str( const struct rht_table* table, const char* key, void** value ) {
    return rht_lookup( table, key, strlen( key ), value );
}

int rht_set_str( const struct rht_table* table, const char* key, void* value) {
    return rht_set( table, key, strlen( key ), value );
}

int rht_delete_str( const struct rht_table* table, const char* key) {
    return rht_delete( table, key, strlen( key ) );
}

void * rht_qlookup( const struct rht_table *table, const void *key, int keylen ) {
    void *rv;
    if( rht_lookup( table, key, keylen, &rv ) ) {
        return 0;
    }
    return rv;
}

void * rht_qlookup_str( const struct rht_table *table, const char *key) {
    return rht_qlookup( table, key, strlen( key ) );
}


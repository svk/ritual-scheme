#ifndef H_RITUAL_HASH_TABLE
#define H_RITUAL_HASH_TABLE

#define USE_JENKINS_HASH_32
#define RHT_THREAD_SAFE

#define DEBUG
#define DEBUG_VERBOSE

#include <stdint.h>

typedef uint32_t ritual_hash_t;

#define REALLOC_ON_LOAD_FACTOR 750
#define LOAD_FACTOR_DENOM 1000

#define MOVES_PER_OPERATION 2
#define RESIZE_FACTOR 2
#define RESIZE_TO(x) ((x)*RESIZE_FACTOR)
/* RESIZE_FACTOR >= (MOVES_PER_OPERATION+1)/MOVES_PER_OPERATION */


#ifdef RHT_THREAD_SAFE
/* Only thread-safe for reasonable use; e.g. concurrent lookups,
 * inserts, and deletes. */
#include <pthread.h>
#endif

#define RHT_OWN_VALUES 1

struct rht_entry {
    void *key;
    int keylen;

    void *value;

    struct rht_entry *next;
};

struct rht_table {
    void (*free_value)(void*);

    int slots, entries;
    struct rht_entry ** slot;

    int old_slots;
    int first_old_index;
    struct rht_entry ** old_slot;

#ifdef RHT_THREAD_SAFE
    pthread_rwlock_t lock;
#endif
};

ritual_hash_t rht_get_hash( const void *, int );

int rht_table_init( struct rht_table*, int );
void rht_table_destroy( struct rht_table* );

struct rht_entry * rht_entry_create( const void*, int, void* );
void rht_entry_delete( struct rht_entry **, void (*)(void*) );

struct rht_entry ** rht_find_entry_in_list( struct rht_entry **, const void*, int );
struct rht_entry ** rht_find_entry( const struct rht_table*, const void*, int );
int rht_get_best_size(int);

int rht_realloc_table( struct rht_table *, int );
int rht_do_move( struct rht_table * );

/* Functions after this point guaranteed safe in thread-safe mode. */
/* Return nonzero on failure (e.g. no such key for lookup), 0 on success. */
/* delete returns the value (in case that needs deletion), or 0 on failure. */

int rht_lookup( const struct rht_table*, const void*, int, void** );
int rht_set( struct rht_table*, const void*, int, void* );
int rht_delete( struct rht_table*, const void*, int );

int rht_lookup_str( const struct rht_table*, const char*, void** );
int rht_set_str( struct rht_table*, const char*, void* );
int rht_delete_str( struct rht_table*, const char* );

void * rht_qlookup( const struct rht_table*, const void*, int);
void * rht_qlookup_str( const struct rht_table*, const char*);


#endif

#ifndef H_RITUAL_MEMORY
#define H_RITUAL_MEMORY

#include <stdint.h>
#include <pthread.h>

/* For 32-bit: (1+32+1024)*4 = 4228 bytes on bitfields
 *             32768 C bytes administrated
 *             Wasted memory 422800/(4228+32768C) pct
 * For 64-bit: (1+64+4096)*8 = 33288 bytes on bitfields
 *             262144 C bytes administrated
 *             Wasted memory 3328800/(33288+262144C) pct
 * Important values of C:
 *   sizeof pair = 8 to 20 bytes
 * Almost no difference between 32 and 64 (probably just
 * caused by the constant and linear parts). Waste ranges
 * from around 1.5% at 8 to around 0.64% at 20.
 * For GC it's much more efficient if we can use full
 * structures, so 32 is probably the best choice.
 */

#define RITUAL_UMP_SIZE (32768)

struct ritual_ump { // uniform memory page
    pthread_mutex_t mutex;
        // needed for allocation, deallocation
        // not needed to access pointers

    int first_id;
    int element_size;

    uint32_t level1;
    uint32_t level2[32];
    uint32_t level3[1024];

        // for arranging pages in a tree
    struct ritual_ump *left;
    struct ritual_ump *right;

    uint8_t *data;
};

struct ritual_ump * ritual_ump_create(int, int);
void ritual_ump_free_tree(struct ritual_ump *);

void * ritual_ump_id_to_pointer(struct ritual_ump*, int);

int ritual_ump_alloc(struct ritual_ump*);

void ritual_ump_clean(struct ritual_ump *ump,
                      int (*gc)(struct ritual_ump*, void*) );

#endif

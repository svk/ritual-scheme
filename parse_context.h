#ifndef H_RITUAL_PARSE_CONTEXT
#define H_RITUAL_PARSE_CONTEXT

#define PCTX_DYNSTRING_SIZE 128

#include "ritual_object.h"

#include "ritual_instance.h"

struct object_node {
	ritual_object_t *object;
	struct object_node *next;
};

struct parse_context {
    void* scanner;

    struct ritual_instance *instance;

    char* dynstring;
    int dynstring_size, dynstring_index;

	struct object_node *expressions_tail;
	struct object_node *expressions;
};

void pctx_init(struct parse_context *, struct ritual_instance *);
void pctx_destroy(struct parse_context *);

char *pctx_dynstring_get( struct parse_context*);
int pctx_dynstring_init( struct parse_context*);
int pctx_dynstring_putc( struct parse_context*, char);

int pctx_has_more( struct parse_context* );
int pctx_push_back( struct parse_context*, ritual_object_t * );
ritual_object_t * pctx_pop( struct parse_context* );


#endif

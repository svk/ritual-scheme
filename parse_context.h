#ifndef H_RITUAL_PARSE_CONTEXT
#define H_RITUAL_PARSE_CONTEXT

#define PCTX_DYNSTRING_SIZE 128

struct parse_context {
    void* scanner;

    char* dynstring;
    int dynstring_size, dynstring_index;
};

void pctx_init(struct parse_context *);
void pctx_destroy(struct parse_context *);

char *pctx_dynstring_get( struct parse_context*);
int pctx_dynstring_init( struct parse_context*);
int pctx_dynstring_putc( struct parse_context*, char);

#endif

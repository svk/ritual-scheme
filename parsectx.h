#ifndef H_PARSECTX
#define H_PARSECTX

#define DYNSTRING_SIZE 128

struct parse_ctx {
    void* scanner;

    char* dynstring;
    int dynstring_size, dynstring_index;
};

void init_parse_ctx(struct parse_ctx *ctx);
void destroy_parse_ctx(struct parse_ctx *ctx);

char *get_dynstring( struct parse_ctx* ctx );
void free_dynstring( struct parse_ctx* ctx);
int initialize_dynstring( struct parse_ctx* ctx );
int accumulate_dynstring( struct parse_ctx *ctx, char ch );

#endif

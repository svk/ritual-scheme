#include "parsectx.h"
#include <stdlib.h>
#include <string.h>

#include <assert.h>

void init_parse_ctx(struct parse_ctx *ctx) {
    memset( ctx, 0, sizeof *ctx );
}

void destroy_parse_ctx(struct parse_ctx *ctx) {
    if( ctx->dynstring ) {
        free( ctx->dynstring );
        ctx->dynstring = 0;
    }
}

char *get_dynstring( struct parse_ctx* ctx ) {
    return ctx->dynstring;
}

int initialize_dynstring( struct parse_ctx* ctx ) {
    ctx->dynstring_index = 0;
    if( !ctx->dynstring ) { 
        ctx->dynstring_size = DYNSTRING_SIZE;
        ctx->dynstring = malloc( ctx->dynstring_size );
    }
    return ctx->dynstring == 0;
}

int accumulate_dynstring( struct parse_ctx *ctx, char ch ) {
    if( ctx->dynstring_index >= ctx->dynstring_size ) {
        while( ctx->dynstring_index >= ctx->dynstring_size ) {
            ctx->dynstring_size *= 2;
        }
        char *rv = realloc( ctx->dynstring, ctx->dynstring_size );
        if( !rv ) {
            return 1;
        }
        ctx->dynstring = rv;
    }
    ctx->dynstring[ ctx->dynstring_index++ ] = ch;
}

#include "parse_context.h"

#include <stdlib.h>
#include <string.h>

#include <assert.h>

void pctx_init(struct parse_context *ctx) {
    memset( ctx, 0, sizeof *ctx );
}

void pctx_destroy(struct parse_context *ctx) {
    if( ctx->dynstring ) {
        free( ctx->dynstring );
        ctx->dynstring = 0;
    }
}

char *pctx_dynstring_get( struct parse_context* ctx ) {
    return ctx->dynstring;
}

int pctx_dynstring_init( struct parse_context* ctx ) {
    ctx->dynstring_index = 0;
    if( !ctx->dynstring ) { 
        ctx->dynstring_size = PCTX_DYNSTRING_SIZE;
        ctx->dynstring = malloc( ctx->dynstring_size );
    }
    return ctx->dynstring == 0;
}

int pctx_dynstring_putc( struct parse_context *ctx, char ch ) {
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

#include "parse_context.h"

#include <stdlib.h>
#include <stdio.h>
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
	while( ctx->expressions ) {
		pctx_pop( ctx );
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
    return 0;
}

int pctx_has_more( struct parse_context *ctx ) {
	return ctx->expressions != 0;
}

int pctx_push_back( struct parse_context *ctx, ritual_object_t * object ) {
	struct object_node * rv = malloc( sizeof *rv );
	assert( rv ); // TODO handle OOM
	rv->next = 0;
	rv->object = object;
	if( ctx->expressions_tail ) {
		ctx->expressions_tail->next = rv;
	} else {
		ctx->expressions = rv;
	}
	ctx->expressions_tail = rv;
}

ritual_object_t * pctx_pop( struct parse_context *ctx ) {
	if( !ctx->expressions ) {
		return 0;
	}
	struct object_node *rv = ctx->expressions;
	ctx->expressions = rv->next;
	if( !ctx->expressions ) {
		ctx->expressions_tail = 0;
	}
	ritual_object_t *rrv = rv->object;
	free( rv );
	return rrv;
}


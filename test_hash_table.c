#include "ritual_hash_table.h"

#include <stdio.h>
#include <stdlib.h>

void show( const char *name, const char *data ) {
    fprintf( stderr, "%s=%s\n", name, (data) ? data : "(not set)" );
}

void showkey( struct rht_table *table, const char *key ) {
    show( key, rht_qlookup_str( table, key ) );
}

void wouldfree( void *p ) {
    fprintf(stderr, "would free \"%s\"\n", (char*) p );
}

int main(int argc, char *argv[]) {
    struct rht_table ht;

    rht_table_init( &ht, 1024 );
    ht.free_value = &wouldfree;

    rht_set_str( &ht, "Pravin", "Lal" );
    rht_set_str( &ht, "Miriam", "Godwinson" );
    rht_set_str( &ht, "Deirdre", "Skye" );
    rht_set_str( &ht, "Prokhor", "Zakharov" );
    rht_set_str( &ht, "Sheng-ji", "Yang" );
    rht_set_str( &ht, "Nwabudike", "Morgan" );
    rht_set_str( &ht, "Corazon", "Santiago" );

    showkey( &ht, "Deirdre" );
    showkey( &ht, "Miriam" );
    showkey( &ht, "Pravin" );

    showkey( &ht, "invalid" );

    rht_delete_str( &ht, "Deirdre" );
    rht_delete_str( &ht, "invalid" );

    showkey( &ht, "Deirdre" );
    showkey( &ht, "Miriam" );
    showkey( &ht, "Pravin" );

    showkey( &ht, "invalid" );

    rht_set_str( &ht, "Deirdre", "Skye" );

    showkey( &ht, "Deirdre" );
    showkey( &ht, "Miriam" );
    showkey( &ht, "Pravin" );

    showkey( &ht, "invalid" );

    rht_table_destroy( &ht );

    return 0;
}

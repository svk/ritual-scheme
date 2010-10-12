#ifndef H_RITUAL_BASIC_PROCS
#define H_RITUAL_BASIC_PROCS

#include "ritual_native_proc.h"


ritual_object_t* rep_list( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );
ritual_object_t* rep_cons( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );
ritual_object_t* rep_car( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );
ritual_object_t* rep_cdr( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );

ritual_object_t* rep_not( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );

ritual_object_t* rep_append( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );
ritual_object_t* rep_length( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );
ritual_object_t* rep_nullp( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );

ritual_object_t* retp_apply( struct ritual_instance *, struct ritual_env*, struct ritual_pair* );

/* Missing (sorely):
 *   - map, filter
 *     ^ an internal apply_callable is needed (building cells for apply would be wasteful)
 *   - compose (is there a standard name for the last one?)
 *
 *   (radical):
 *     the REP convention for arglists should be struct ritual_mapped_list*,
 *     or struct ritual_evaluated_list*, which moves through the list as
 *     needed (forward only) and applies the map when values are requested,
 *     as well as when "finalize" is called to move through the rest of the
 *     list. (There is no way to skip a value without evaluating it with the
 *     functions the REP should be using.) Interface analogous to the
 *     ritual_list_* functions that the REP functions are already using for
 *     pairs.
 */

#endif

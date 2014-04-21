#ifndef HASH_H

/*

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

  */

/* a value in a hash table can be any of the following types... values
   are untagged, so the type had better be clear from context */
typedef union {
    int i;
    float f;
    char *s;
    void *p;
} hash_value;

/* a hash_bucket is a linked list of string/value pairs */
typedef struct s_hash_element {
    char *s;				    /* the key */
    hash_value value;			    /* the value */
    struct s_hash_element *next;	    /* and the rest of the list */
} *hash_bucket;				    /* next should not be used */

/* a hash table is a size and an array of hash_buckets */
typedef struct {
    int size;				    /* internals of this structure */
    hash_bucket table[1];		    /* should not be used */
} *hash_table;

/* create a hash table of at least the specified size */
hash_table make_hash_table( /* int size */ );

/* find a bucket containing an entry.  return NULL if no such entry */
hash_bucket find( /* hash_table t, char *s */ );

/* find a bucket containing an entry or create one that does */
hash_bucket find_or_create( /* hash_table t, char *s, hash_value defalt */ );

/* find a value in a table or have a fatal error */
hash_value must_find( /* hash_table t, char *s */ );

/* insert a value under a particular key */
void insert( /* hash_table t, char *s, hash_value v */ );

/* insert a value, but barf if it is already there */
void insert_new( /* hash_table t, char *s, hash_value v */ );

/* convert a char* into a hash_value */
hash_value the_string( /* char *s */ );

/* convert an integer into a hash value */
hash_value the_int( /* int i */ );

/* convert a float into a hash value */
hash_value the_float( /* double f */ );

/* convert a void* into a hash value */
hash_value the_voidp( /* void *vp */ );

/* slightly funky hash table looping construct.  maybe we should just 
   admit that c isn't lisp, but it is handy to have */
#define for_hash(tbl, el, code) {int _i;for(_i=0;_i<(tbl)->size;_i++) for(el=(tbl)->table[_i];el!=NULL;el=el->next) {code;}}

#define HASH_H
#endif

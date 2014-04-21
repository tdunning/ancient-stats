#define DH_NULL (-1)
#define dh_key(base, p) ((p)==DH_NULL?NULL:((char*) ((base->keys->contents)+(p))))
#define dh_ptr(base, p, type) ((p)==DH_NULL?NULL:((type) ((base->ht->contents)+(p))))

#define dh_internalize_ptr(base, p) (((char *)p) - (base)->ht->contents)

typedef int dh_pointer;			    /* offset into the ht part
					       of a dh_table. */

typedef int dh_key;			    /* offset into the keys part
					       of a dh_table*/

typedef union {
    int i;
    float f;
    dh_pointer p;
} dh_value;

typedef struct dh_element {
    dh_key s;				    /* the key */
    dh_value value;			    /* the value */
    dh_pointer next;			    /* and the rest of the list */
} *dh_bucket;				    /* next should not be used */
    
typedef struct {
    int size, count;
    char *contents;
} *dh_buffer;

typedef struct {
    dh_pointer root;
    dh_buffer ht;
    dh_buffer keys;
    int was_mmapped;
} *dh_table;

/* create a hash table of at least the specified size */
dh_table dh_make_table( int size );

/* find a bucket containing an entry.  return NULL if no such entry */
dh_bucket dh_find( dh_table t, char *s );

/* find a bucket containing an entry or create one that does */
dh_bucket dh_find_or_create( dh_table t, char *s, dh_value defalt );

/* find a value in a table or have a fatal error */
dh_value dh_must_find( dh_table t, char *s );

/* insert a value under a particular key */
void dh_insert( dh_table t, char *s, dh_value v );

/* insert a value, but barf if it is already there */
void dh_insert_new( dh_table t, char *s, dh_value v );

void dh_save(FILE *f, dh_table tbl);

dh_table dh_open(char *name);
void dh_close(dh_table tbl);

/* convert an integer into a dh value */
dh_value the_int(int i);

/* convert a float into a dh value */
dh_value the_float(double f);

/* convert a void* into a dh value */
dh_value the_voidp(dh_table tbl, void *vp);

/* slightly funky hash table looping construct.  maybe we should just 
   admit that c isn't lisp, but it is handy to have */
#define for_hash(tbl, el, code) {int _i;for(_i=0;_i<dh_ptr(tbl,0,dh_internal)->size;_i++) for(el=dh_ptr(tbl,dh_ptr(tbl,0,dh_internal)->table[_i],dh_bucket);el!=NULL;el=dh_ptr(tbl, el->next, dh_bucket)) {code;}}



    

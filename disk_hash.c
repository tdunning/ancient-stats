#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <netinet/in.h>


#include "disk_hash.h"
#include "map_file.h"
#include "xvector.h"

#define ALLOCATION_QUANTUM 8

typedef struct {
    int size;				    /* internal details */
    int count;				    /* of this structure */
    dh_pointer table[1];		    /* should not be used */
} *dh_internal;

int dh_roundup(int x, int n)
{
    x = (x+n-1)/n;
    return x*n;
}

static int last_hash;		/* a handy cache for the last hash */

#if 0
/* old hash function from ted's hash package.  not as fast or as good
   as ozan yigit's hash function which follows.

   compute a hash of a string.  this is just one hash function of many
   that has been found to work well for english words */
static unsigned int dh_hash(s)
char *s;
{
    unsigned char *p;		/* unsigned to be 8 bit clean */
    unsigned int r;

    p = (unsigned char *)s;
    for (r=0;*p;p++) {
	r = r*3 + *p;		/* unsigneds should overflow quietly */
    }
    return r;
}
#endif

/*
 * This is INCREDIBLY ugly, but fast.  We break the string up into 8 byte
 * units.  On the first time through the loop we get the "leftover bytes"
 * (strlen % 8).  On every other iteration, we perform 8 HASHC's so we handle
 * all 8 bytes.  Essentially, this saves us 7 cmp & branch instructions.  If
 * this routine is heavily used enough, it's worth the ugly coding.
 *
 * OZ's original sdbm hash
 */
static long hash3(const void *keyarg, register size_t len)
{
	register const unsigned char *key;
	register long loop;
	register long h;

#define HASHC   h = *key++ + 65599 * h

	h = 0;
	key = keyarg;
	if (len > 0) {
		loop = (len + 8 - 1) >> 3;

		switch (len & (8 - 1)) {
		case 0:
			do {
				HASHC;
				/* FALLTHROUGH */
		case 7:
				HASHC;
				/* FALLTHROUGH */
		case 6:
				HASHC;
				/* FALLTHROUGH */
		case 5:
				HASHC;
				/* FALLTHROUGH */
		case 4:
				HASHC;
				/* FALLTHROUGH */
		case 3:
				HASHC;
				/* FALLTHROUGH */
		case 2:
				HASHC;
				/* FALLTHROUGH */
		case 1:
				HASHC;
			} while (--loop);
		}
	}
	return (h);
}

dh_pointer dh_malloc(dh_table pt, int size)
{
    dh_pointer r;

    if (pt->was_mmapped) {
	void *t;

	t = malloc(pt->ht->size);
	memcpy(t, pt->ht->contents, pt->ht->count);
	munmap(pt->ht->contents, pt->ht->size);
	pt->ht->contents = t;
    }

    size = dh_roundup(size, ALLOCATION_QUANTUM);
    x_extend(pt->ht, size);
    r = x_length(pt->ht);

    /* not strictly required for malloc clones, but required for calloc */
    memset(dh_ptr(pt, r, void*), 0, size);

    x_length(pt->ht) += size;
    return r;
}

dh_pointer dh_calloc(dh_table tbl, int count, int size)
{
    return dh_malloc(tbl, count*size);
}

/* create a hash table */
dh_table dh_make_table( int size )
{
    int i;
    dh_table r;
    dh_internal tbl;

    r = malloc(sizeof(*r));
    x_create(r->ht, size*8);
    x_create(r->keys, 100);
    r->was_mmapped = 0;

    r->root = dh_malloc(r, sizeof(*tbl)+(size-1)*sizeof(dh_pointer));
    tbl = dh_ptr(r, r->root, dh_internal);
    tbl->size = size;
    tbl->count = 0;
    for (i=0;i<size;i++) {
	tbl->table[i] = DH_NULL;
    }
    return r;
}

/* push a string into the string table */
static dh_key dh_strdup(dh_table tbl, char *s)
{
    dh_key r = x_length(tbl->keys);
    int n = strlen(s)+1;
    
    x_extend(tbl->keys, n);
    strcpy(tbl->keys->contents + x_length(tbl->keys), s);
    x_length(tbl->keys) += n;
    return r;
}

/* deallocate a disk based hash table */
void dh_free( dh_table tbl )
{
    if (tbl->was_mmapped) {
	munmap(tbl->ht->contents, tbl->ht->size);
	munmap(tbl->keys->contents, tbl->keys->size);
    }
    else {
	free(tbl->ht->contents);
	free(tbl->ht);
	free(tbl->keys->contents);
	free(tbl->keys);
    }
    free(tbl);
}

/* make_bucket()
   allocate, partially fill and link a single bucket entry */
static dh_bucket dh_make_bucket(dh_table t, char *s)
{
    dh_bucket p;
    dh_internal tbl;

    p = dh_ptr(t, dh_malloc(t, sizeof(*p)), dh_bucket);
    p->s = dh_strdup(t, s);

    tbl = dh_ptr(t, t->root, dh_internal);
    tbl->count++;
    p->next = tbl->table[last_hash];
    tbl->table[last_hash] = dh_internalize_ptr(t, p);
    return p;
}

/* find a bucket containing an entry.  return NULL if no such entry */
dh_bucket dh_find( dh_table t, char *s )
{
    char *key;
    dh_bucket p;
    dh_internal tbl;
    tbl = dh_ptr(t, t->root, dh_internal);

    last_hash = hash3(s, strlen(s))%tbl->size;
    if (last_hash < 0) last_hash = -last_hash;

    p = dh_ptr(t, tbl->table[last_hash], dh_bucket);
    
    if (p) key = dh_key(t, p->s);
    while (p && (*s != key[0] || strcmp(s, key) != 0)) {
	p = dh_ptr(t, p->next, dh_bucket);
	if (p) key = dh_key(t, p->s);
    }

    return p;
}

/* find a bucket containing an entry or create one that does */
dh_bucket dh_find_or_create( dh_table t, char *s, dh_value defalt )
{
    dh_bucket p;
    p = dh_find(t, s);

    if (p == NULL) {
	p = dh_make_bucket(t, s);
	p->value = defalt;
    }
    return p;
}    

/* must_find()
   same as find(), but blow out with a fatal error if no entry is
   found */
dh_value dh_must_find( dh_table t, char *s )
{
    dh_bucket p;
    p = dh_find(t, s);

    if (p == NULL) {
	fprintf(stderr, "cannot find key [%s] in table", s);
	abort();
    }
    return p->value;
}

/* insert()
   add a new entry with specified key and value, or if an entry with
   that key already exists, then change the value on that entry.  note
   that no storage is allocated for the key.  this means that later
   calls may create entries which share space for their keys.  unless
   they have the same hash, this has the effect of deleting the first
   entry.  this behavior may be a bad idea, but helps performance. */
void dh_insert( dh_table t, char *s, dh_value v )
{
    dh_bucket p;

    p = dh_find_or_create(t, s, v);
    p->value = v;
}

/* insert a value, but barf if it is already there */
void dh_insert_new( dh_table t, char *s, dh_value v )
{
    dh_bucket p;

    p = dh_find(t, s);
    if (p) {
	fprintf(stderr, "insert_new: element already exists");
	abort();
    }
 
    p = dh_make_bucket(t, s);
    p->value = v;
}

void dh_save(FILE *f, dh_table tbl)
{
    int n;

    n = htons(tbl->keys->size);
    if (fwrite(&n, sizeof(n), 1, f) != 1) {
	fprintf(stderr, "unable to write table\n");
	abort();
    }

    n = htons(tbl->ht->size);
    if (fwrite(&n, sizeof(n), 1, f) != 1) {
	fprintf(stderr, "unable to write table\n");
	abort();
    }
	
    n = htons(tbl->root);
    if (fwrite(&n, sizeof(n), 1, f) != 1) {
	fprintf(stderr, "unable to write table\n");
	abort();
    }
	
    if (fwrite(tbl->keys->contents, tbl->keys->size, 1, f) != 1) {
	fprintf(stderr, "unable to write table\n");
	abort();
    }

    if (fwrite(tbl->ht->contents, tbl->ht->size, 1, f) != 1) {
	fprintf(stderr, "unable to write table\n");
	abort();
    }
}

dh_table dh_open(char *name)
{
    FILE *f;
    dh_table r;
    void *p;
    int size;

    f = fopen(name, "r");
    if (!f) {
	perror("dh_open");
	abort();
    }

    size = file_size(f);
    p = map_file(f, size, 0);
    fclose(f);

    r = malloc(sizeof(*r));
    
    r->keys = malloc(sizeof(*r->keys));
    r->keys->size = r->keys->count = ntohs(*((int *) p));
    r->keys->contents = p + 12;

    r->ht = malloc(sizeof(*r->ht));
    r->ht->size = r->ht->count = ntohs(*((int *) p+4));
    r->ht->contents = p + r->keys->size + 12;

    r->root = ntohs(*((int *) p+8));

    return r;
}

/* the_int()
   convert an int to a dh_value */
dh_value the_int(i)
int i;
{
    dh_value r;

    r.i = i;
    return r;
}

/* the_float()
   convert a float to a dh_value */
dh_value the_float(f)
double f;
{
    dh_value r;

    r.f = f;
    return r;
}

/* the_voidp()
   convert a void* to a dh_value */
dh_value the_voidp(dh_table tbl, void *vp)
{
    dh_value r;
    dh_pointer p;

    p = dh_internalize_ptr(tbl, vp);
    if (p < 0 || p > tbl->ht->size) {
	fprintf(stderr,
		"cannot internalize pointer unless allocated"
		" using dh_malloc\n");
	abort();
    }
    r.p = p;
    return r;
}


#ifdef TEST

#include <string.h>

int main(int argc, char *argv[])
{
    FILE *f;
    char key[100];
    int count;
    dh_table big, little;
    dh_bucket el;

    little = dh_make_table(10);
    big = dh_make_table(1000);

    count = 0;

    while (scanf("%s", key) == 1) {
	dh_insert(little, key, the_int(count));

	dh_insert(big, key, the_int(count));
	count++;
    }

    dh_insert_new(little, "xyzzy", the_int(3));
    dh_insert_new(little, "one", the_int(6));
    printf("%d\n",  dh_must_find(little, "one").i);

    printf("%d\n",  the_int(34).i);
    printf("%f\n", the_float(45.0).f);

    printf("\nlittle\n");
    for_hash(little, el, printf("%s=%d\n", dh_key(little, el->s),
				el->value.i));
    printf("\nbig\n");
    for_hash(big, el, printf("%s=%d\n", dh_key(big, el->s),
			     el->value.i));


    f = fopen("/tmp/little", "w");
    dh_save(f, little);
    fclose(f);
    dh_free(little);

    f = fopen("/tmp/big", "w");
    dh_save(f, big);
    fclose(f);
    dh_free(big);

    little = dh_open("/tmp/little");
    printf("\nlittle\n");
    for_hash(little, el, printf("%s=%d\n", dh_key(little, el->s),
				el->value.i));
    
    exit(0);
}

#endif

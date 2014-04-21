#include <stdio.h>
#include <string.h>
#include "error.h"
#include "hash.h"

/* 
Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

 * $Log: hash.c,v $
 * Revision 1.7  1995/06/26  13:55:33  ted
 * fixed silly bug which occcurred when using the
 * empty string as a key.
 *
 * Revision 1.6  1993/10/25  18:52:56  ted
 * added copyright and fixed bug in hash() which might break with
 * 8 bit characters.
 *
 * Revision 1.5  1991/07/19  17:32:49  ted
 * added rcs log comment
 * */

/* a few handy primes... for sizing hash tables nicely */
static int hash_sizes[] = {
    17, 31, 61, 127, 257, 509, 1021, 2053, 4093, 8191, 16381, 20011,
    32771, 40009, 50021, 65521, 70001, 80021, 90001,
    0
};

static int last_hash;		/* a handy cache for the last hash */

/* hash()
   compute a hash of a string.  this is just one hash function of many
   that has been found to work well for english words */
static unsigned int hash(s)
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

/* make_hash_table()
   create an empty hash table of at least the specified size. */
hash_table make_hash_table(size)
int size;
{
    int i;
    hash_table r;		/* our result */

    i=0;			/* round to a nice prime number */
    while (hash_sizes[i] < size && hash_sizes[i]) i++;
    if (hash_sizes[i] == 0) i--;
    size = hash_sizes[i];
	
				/* allocate and zero the table */
    r = emalloc(sizeof(*r)+size*sizeof(r->table[0]));
    r->size = size;
    for (i=0;i<size;i++) {
	r->table[i] = NULL;
    }
	
    return r;
}

/* make_bucket()
   allocate, partially fill and link a single bucket entry */
static hash_bucket make_bucket(t, s)
hash_table t;
char *s;
{
    hash_bucket p;
    p = (hash_bucket) emalloc(sizeof(*p));
    p->s = s;
    p->next = t->table[last_hash];
    t->table[last_hash] = p;
    return p;
}

/* find()
   in table T, find a hash_bucket which has a particular KEY.  return
   NULL if none found */ 
hash_bucket find(tbl,s)
hash_table tbl;
char *s;
{
    hash_bucket p;

    last_hash = hash(s)%tbl->size; /* reduce the hash to be a table index */

    p = tbl->table[last_hash];

    while (p && (*s != p->s[0] || strcmp(s,p->s) != 0)) {
	p = p->next;
    }

    return p;
}

/* find_or_create()
   in table T, find a hash_bucket which has a particular KEY.  if none
   is found, create one with a specified default value. */
hash_bucket find_or_create(t,s,def)
hash_table t;
char *s;
hash_value def;
{
    hash_bucket p;
    p = find(t,s);

    if (p == NULL) {
	p = make_bucket(t, s);
	p->value = def;
    }
    return p;
}

/* must_find()
   same as find(), but blow out with a fatal error if no entry is
   found */
hash_value must_find(t,s)
hash_table t;
char *s;
{
    hash_bucket p;
    p = find(t,s);

    if (p == NULL) {
	char *message;
	message = emalloc(strlen(s)+100);
	sprintf(message, "cannot find key [%s] in table", s);
	fatal("must_find", message);
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
void insert(t, s, v)
hash_table t;
char *s;
hash_value v;
{
    hash_bucket p;

    p = find_or_create(t, s, v);
    p->value = v;
}
    
/* insert_new()
   same as insert, but it is a fatal error if the entry already exists */
void insert_new(t, s, v)
hash_table t;
char *s;
hash_value v;
{
    hash_bucket p;

    p = find(t, s);
    if (p) {
	fatal("insert_new", "element already exists");
    }
 
    p = make_bucket(t, s);
    p->value = v;
}

/* the_string()

   convert a char* to a hash_value.  this is done directly much as a
   cast would do it.  some case could be made that the_string should
   allocate new space and copy s to there */
hash_value the_string(s)
char *s;
{
    hash_value r;

    r.s = s;
    return r;
}

/* the_int()
   convert an int to a hash_value */
hash_value the_int(i)
int i;
{
    hash_value r;

    r.i = i;
    return r;
}

/* the_float()
   convert a float to a hash_value */
hash_value the_float(f)
double f;
{
    hash_value r;

    r.f = f;
    return r;
}

/* the_voidp()
   convert a void* to a hash_value */
hash_value the_voidp(vp)
void *vp;
{
    hash_value r;

    r.p = vp;
    return r;
}


#ifdef TEST

/* test driver.  read a file and fill some tables, then print the
   results */ 
main(argc, argv)
int argc;
char *argv[];
{
    char key[100], *k;
    int count;
    hash_table big, little;
    hash_bucket el;

    make_progname(argc, argv);

    little = make_hash_table(10);
    big = make_hash_table(1000);

    count = 0;

    while (scanf("%s", key) == 1) {
	k = strdup(key);

	insert(little, k, the_int(count));
	insert(big, k, the_int(count));
	count++;
    }

    insert_new(little, "xyzzy", the_int(3));
    printf("%d\n",  must_find(little, "one").i);

    printf("%s\n",  the_string("a string").s);
    printf("%d\n",  the_int(34).i);
    printf("%lf\n", the_float(45.0).f);
    printf("%d\n", the_voidp(&el).p == (void *) &el);

    printf("little\n");
    for_hash(little, el, printf("%s=%d\n", el->s, el->value.i));
    printf("big\n");
    for_hash(big, el, printf("%s=%d\n", el->s, el->value.i));
    exit(0);
}
#endif

    

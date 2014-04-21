/* read words without lots of malloc overhead, and without worrying
   about how big they are

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

 */

/* $Log: stringpool.c,v $
 * Revision 1.2  1996/05/16  21:53:02  ted
 * eliminated compiler warnings
 *
 * Revision 1.1  1996/05/02  15:45:58  ted
 * Initial revision
 *
 * Revision 1.3  1993/10/25  19:02:23  ted
 * added copyright message
 *
 * Revision 1.2  1991/09/06  17:58:19  ted
 * added next_word and latest_word for general purpose
 * string creation
 * */

#include <stdio.h>
#include <ctype.h>
#include "error.h"
#include "stringpool.h"

#define INITIAL_POOL_SIZE 1000
static char *old_pool, *current_pool;
static int pool_cnt=0;
static int pool_size=0;
static int current_generation=0;
static char *pool=NULL;

void allocate_pool()
{
    if (pool_size > 0) {	  /* a growing pool handles enormous words */
	pool_size = 2*pool_size;  /* but grows less and less often */
    }
    else {
	pool_size = INITIAL_POOL_SIZE;
    }

    old_pool = current_pool = pool = emalloc(pool_size);
    pool_cnt = 0;
}

void new_word()
{
    old_pool = current_pool;
}

char *latest_word()
{
    return old_pool;
}

void addch(ch)
int ch;
{
    if (pool_cnt >= pool_size) {
	char *p, *q;
	int i;
	int old_size;

	/* reallocate pool space */
	p = old_pool;
	q = current_pool;
	old_size = pool_size;
	current_generation++;
	allocate_pool();

	/* copy the word currently being formed to the new pool */
	for (i=0;p < q && i<old_size;i++) {
	    addch(*p++);
	}
	if (pool_cnt >= pool_size) {
	    fatal("addch", "string pool overflow");
	}
    }
    pool_cnt++;
    *current_pool++ = ch;
}
		
/* read a word from a file */
char *get_word(f)
FILE *f;
{
    int ch;
    new_word();
    
    ch = getc(f);
    if (ch == EOF) return NULL;

    while (ch != EOF && isspace(ch) && ch != '\n') {
	ch = getc(f);
    }

    if (ch == '\n' || ch == EOF) {
	return NULL;
    }

    while (ch != EOF && !isspace(ch)) {
	addch(ch);
	ch = getc(f);
    }
    addch(0);

    if (ch == '\n') {
	ungetc(ch, f);
    }

    return latest_word();
}

/* read a line from a file */
char *get_line(f)
FILE *f;
{
    int ch;

    new_word();
    
    ch = getc(f);
    if (ch == EOF) return NULL;

    while (ch != EOF && ch != '\n') {
	addch(ch);
	ch = getc(f);
    }
    addch(0);
    return latest_word();
}

void free_last_word()
{
    pool_cnt -= (current_pool-old_pool);
    current_pool = old_pool;
}

pool_marker mark_pool()
{
    pool_marker r;
    
    r.generation = current_generation;
    r.mark = current_pool;
    return r;
}

void release_to_mark(m)
pool_marker m;
{
    if (m.generation < current_generation) {
	old_pool = pool;
    }
    else {
	old_pool = m.mark;
    }
    free_last_word();
}

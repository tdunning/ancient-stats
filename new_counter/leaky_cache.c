/*****************************************************************************
 *
 * Copyright (c) 1996, Aptex Inc. 9605 Scranton Road Suite 240, San Diego.
 * All Rights Reserved.
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF APTEX INC
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.                       
 * 
 * RESTRICTED RIGHTS LEGEND:  Use, duplication, or disclosure by the government
 * is subject to restrictions as set forth in subparagraph (c)(1)(ii) of the
 * Rights in Technical Data and Computer Software clause at DFARS 252.227-7013
 * and FAR 52.227-19.
 * 
 *      Product:       
 *          Stat - statistical text analysis software
 *
 *      Module:        
 *          leaky_cache.c
 *
 *      Created:   
 *          5/1/96
 *
 *      Authors         Initials        Involvement
 *      -------         --------        -----------
 *      Jonathan Hughes JPH             Creator   
 *                                                
 ****************************************************************************


  support for a leaky cache object which supports counting of string
  bigrams.  the idea is that we have a rectangular cache of count cells.
  as bigrams are found in the cache, they migrate toward the front of the
  cache row.  items which are pushed all the way to the back of the
  cache can fall off the end of the cache row and be forgotten.  the
  reason for doing this is to live within finite memory and get an
  approximate count.

  also associated with the main bigram cache is a table of strings which
  is used to avoid storing more than one copy of any string in memory.

$Log: leaky_cache.c,v $
 * Revision 1.1  1996/07/16  22:29:26  ted
 * Initial revision
 *

 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "error.h"
#include "hash.h"
#include "stringpool.h"

#include "bit_count.h"
#include "leaky_cache.h"

leaky_cache make_leaky_cache(int w, int d, int insertion_backup, int random)
{
    leaky_cache r;

    r = emalloc(sizeof(*r));
    r->table = ecalloc(w*d, sizeof(r->table[0]));
    r->search_lengths = ecalloc(d);
    r->leaks = 0;

    r->max_leaked_count = 10;
    r->leak_profile = ecalloc(r->max_leaked_count, sizeof(r->leak_profile[0]));

    r->total = 0;

    r->bitmap_size = (w*d*3+37)/32;
    r->bitmap = ecalloc(r->bitmap_size, sizeof(r->bitmap[0]));

    r->width = w;
    r->depth = d;

    r->random = random;
    if (insertion_backup < 1) insertion_backup = 1;
    if (insertion_backup > d) insertion_backup = d;
    if (d < 10) {
	r->insertion_point = d-1;
    }
    else {
	r->insertion_point = d-insertion_backup;
    }

    return r;
}


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

	if (len == (unsigned) -1) len = strlen(keyarg);

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

/* increment the count for the pair of words w1 and w2.  w1 and w2 should
   point to safe storage which will not be overwritten or deallocated.
   */
void count_in_cache(leaky_cache tbl, char *w1, char *w2)
{
    unsigned int hv;
    extern double drand48();
    int n;
    int i;
    cache_cell *row;

    hv = hash3(w1, -1) ^ (hash3(w2, -1) << 1);
    tbl->total++;
    row = tbl->table + (hv%tbl->width) * tbl->depth;

    /* look in the current row for a match */
    for (i=0;row[i].w1 && i<tbl->depth;i++) {
	/* if we find it, move toward the front and increment count */
	if (hv == row[i].hash && row[i].w1 == w1 && row[i].w2 == w2) {
	    tbl->search_lengths[i]++;
	    if (i) {
		cache_cell t;
		t = row[i-1];
		row[i-1] = row[i];
		row[i] = t;
		i--;
	    }
	    row[i].count++;
	    return;
	}
    }

    /* did we hit an empty spot? */
    if (i<tbl->depth) {
	/* add to this cache line */
	tbl->search_lengths[i]++;
	row[i].hash = hv;
	row[i].w1 = w1;
	row[i].w2 = w2;
	row[i].count = 1;
    }
    else {
	/* we hit the end of the line. */
	tbl->leaks += row[tbl->depth-1].count;
	if (row[tbl->depth-1].count >= tbl->max_leaked_count) {
	    tbl->leak_profile[tbl->max_leaked_count-1]++;
	}
	else {
	    tbl->leak_profile[row[tbl->depth-1].count]++;
	}
	    
	tbl->search_lengths[tbl->depth-1]++;


	/* "remember" what got leaked */
	bit_set(tbl->bitmap, tbl->bitmap_size, row[tbl->depth-1].hash);

	/* add near the end of the line.  this leaks the very last
	   item (which gets lost) */
	if (tbl->random) {
	    n = (drand48() * (tbl->depth - tbl->insertion_point)) +
		tbl->insertion_point;
	}
	else {
	    n = tbl->insertion_point;
	}

	for (i=tbl->depth-1;i > n;i--) {
	    row[i] = row[i-1];
	}

	row[i].hash = hv;
	row[i].w1 = w1;
	row[i].w2 = w2;
	row[i].count = 1;
    }

}



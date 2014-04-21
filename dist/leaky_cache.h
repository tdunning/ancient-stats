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
 ****************************************************************************/


typedef struct {
    char *w1, *w2;
    int hash;
    int count;
} cache_cell;

typedef struct {
    int random;			/* randomize insertion point */
    int insertion_point;	/* where new items get stuck */

    int total;			/* total items counted */

    int width, depth;		/* shape of the actual cache */
    cache_cell *table;		/* the cache which is width*depth long */

    int *bitmap;		/* a record of what we have seen for stats */
    int bitmap_size;

    int *search_lengths;	/* a record of searches */
    int leaks;			/* and how many things fell out the back end */

    int max_leaked_count;	/* size of leak count profile */
    int *leak_profile;		/* the leak profile itself */
} *leaky_cache;

leaky_cache make_leaky_cache(int w, int d, int insertion_backup, int random);

/* increment the count for the pair of words w1 and w2.  w1 and w2 should
   point to safe storage which will not be overwritten or deallocated.
   */
void count_in_cache(leaky_cache tbl, char *w1, char *w2);




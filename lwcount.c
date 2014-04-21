/* read a list of words pairs, one to a line and output a list of unique 
 * pairs with counts.  this program uses a leaky cache which will
 * selectively forget about items which have not been used recently in
 * order to avoid using more memory than is available.

Copyright 1996, Ted Dunning, ted@aptex.com

Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: lwcount.c,v $
 * Revision 1.2  1996/06/04  21:04:50  ted
 * changed email address
 *
 * Revision 1.1  1996/05/16  21:45:18  ted
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

leaky_cache counts;
hash_table words=NULL;

char *read_and_intern_word(FILE *f)
{
    char *w;
    hash_bucket p;
    pool_marker m;

    if (!words) words = make_hash_table(100000);

    m = mark_pool();
    w = get_word(f);
    if (!w) return w;
    else {
	p = find_or_create(words, w, the_int(0));
	if (p->value.i != 0) {	/* release storage if possible */
	    release_to_mark(m);
	}
	p->value.i++;
	return p->s;
    }
}

void count_lines(char *file_name)
{
    int ch;
    FILE *f;
    char *w1, *w2;

    if (strcmp(file_name, "-") == 0) {
	f = stdin;
    }
    else {
	f = efopen(file_name, "r");
    }

    w1 = read_and_intern_word(f);
    w2 = read_and_intern_word(f);
    ch = getc(f);
    while (ch != EOF && ch != '\n') ch = getc(f);
    while (w1 && w2) {
	count_in_cache(counts, w1, w2);
	w1 = read_and_intern_word(f);
	w2 = read_and_intern_word(f);
	ch = getc(f);
	while (ch != EOF && ch != '\n') ch = getc(f);
    }

    if (strcmp(file_name, "-") != 0) {
	fclose(f);
    }
}

int main(int argc, char *argv[])
{
    int i;
    int stats=0;
    int width, depth, backup, random;

    extern char *optarg;

    int error;
    int squashed_output=0;

    int ch;
    extern optind;
    extern int getopt();

    error = 0;
    width = 100000;
    depth = 64;
    backup = 5;
    random = 0;
    while ((ch = getopt(argc, argv, "b:c:vsr")) != EOF) {
	switch (ch) {
	  case 'r':
	      random = 1;
	      break;
	  case 'c':
	      if (sscanf(optarg, "%dx%d", &width, &depth) != 2) {
		  fprintf(stderr,
			  "%s got bad cache shape specification.  "
			  "Wanted wxd (for example -c 1000x16)\n", argv[0]);
		  error++;
	      }
	      break;
	  case 'b':
	      sscanf(optarg, "%d", &backup);
	      break;
	  case 's':
	      squashed_output = 1;
	      break;
	  case 'v':
	      stats = 1;
	      break;
	  case '?':
	  default:
	      error = 1;
	      break;
	}
    }
    if (error) {
	fprintf(stderr, "usage: lwcount [-vsh] [files ...]\n");
	fprintf(stderr, "-v means print hash table stats\n");
	fprintf(stderr, "-s means don't output lots of spaces\n");
	fprintf(stderr, "-c wxd means use a cache which of width w and depth d\n");
	fprintf(stderr, "-b n means insert items n from the end of full cache lines\n");
	exit(1);
    }

    counts = make_leaky_cache(width, depth, backup, random);

    if (argc == optind) {
	count_lines("-");
    }
    else {
	for (i=optind;i<argc;i++) {
	    count_lines(argv[i]);
	}
    }

    if (stats) {
	int total, unique;
	double s;
	s = 0;
	total = 0;
	for (i=0;i<counts->depth;i++) {
	    total += counts->search_lengths[i];
	    s += i * counts->search_lengths[i];
	}
	unique = 0;
	for (i=0;i<counts->depth * counts->width;i++) {
	    if (counts->table[i].w1) unique++;
	}

	printf("\"%d/%d total, %.2f avg, %d/%d leaks\"\n",
	       unique, counts->total,
	       s/total+1,
	       bitv_count(counts->bitmap, counts->bitmap_size),
	       counts->leaks);

	printf("\"leak counts\"\n");
	for (i=0;i<counts->max_leaked_count;i++) {
	    printf("%2d %3d\n", i, counts->leak_profile[i]);
	}
	printf("\n");

	printf("\"search lengths\"\n");
	for (i=0;i<depth;i++) {
	    printf("%3d %3d\n", i, counts->search_lengths[i]);
	}
	printf("\n");
    }
    else {
	for (i=0;i<width*depth;i++) {
	    if (counts->table[i].w1) {
		printf("%d %s %s\n",
		       counts->table[i].count,
		       counts->table[i].w1,
		       counts->table[i].w2);
	    }
	}
    }
    return 0;
}

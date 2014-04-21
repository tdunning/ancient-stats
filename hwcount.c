/* read a list of words, one to a line and output a list of unique 
 * words with counts.

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: hwcount.c,v $
 * Revision 1.6  1994/04/07  00:32:44  ted
 * fixed return status from main
 *
 * Revision 1.5  1993/10/25  18:59:50  ted
 * cleaned some stuff up and made the stats more interesting.
 *
 */

#include <stdio.h>
#include <string.h>
#include "error.h"
#include "hash.h"
#include "stringpool.h"

hash_table counts;

void count_words(file_name)
char *file_name;
{
    FILE *f;
    char *w;
    pool_marker m;

    if (strcmp(file_name, "-") == 0) {
	f = stdin;
    }
    else {
	f = efopen(file_name, "r");
    }

    m = mark_pool();
    w = get_line(f);
    while (w != NULL) {
	hash_bucket p;
	p = find_or_create(counts, w, the_int(0));
	if (p->value.i != 0) {	/* release storage if possible */
	    release_to_mark(m);
	}
	p->value.i++;
	m = mark_pool();
	w = get_line(f);
    }
    if (strcmp(file_name, "-") != 0) {
	fclose(f);
    }
}

int main(argc,argv)
int argc;
char *argv[];
{
    int i;
    int stats=0;
    hash_bucket p;

    int error;
    int squashed_output=0;

    int ch;
    extern optind;
    extern int getopt();

    error = 0;
    while ((ch = getopt(argc, argv, "vs")) != EOF) {
	switch (ch) {
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
	fprintf(stderr, "usage: hwcount [-vsh] [files ...]\n");
	fprintf(stderr, "-v means print hash table stats\n");
	fprintf(stderr, "-s means don't output lots of spaces\n");
	exit(1);
    }

    counts = make_hash_table(50000);

    if (argc == optind) {
	count_words("-");
    }
    else {
	for (i=optind;i<argc;i++) {
	    count_words(argv[i]);
	}
    }

    if (stats) {
	int chains, total;
	int len, min, max;
	chains = total = 0;
	max = 0;
	min = -1;
	for (i=0;i<counts->size;i++) {
	    p = counts->table[i];
	    chains += (p!=NULL);
	    len = 0;
	    while (p) {
		len++;
		p = p->next;
	    }
	    total += len;
	    if (min == -1 || min > len) min = len;
	    if (len > max) max = len;
	}

	printf("%d/%d chains used (average length %.1f, min=%d, max=%d), to store %d items\n",
	       chains, counts->size, (double) total/chains, min, max, total);
    }
    else {
	for_hash(counts, p, {
	    if (squashed_output) printf("%d %s\n", p->value.i, p->s);
	    else printf("%15d %s\n", p->value.i, p->s);
	});
    }
    return 0;
}

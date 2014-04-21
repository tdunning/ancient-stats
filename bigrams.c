/* read a list of word counts, one to a line and then process a list
   of bigram counts testing each bigram count to determine whether or
   not the bigram count is significantly higher than expected from the
   isolated word frequencies.

Copyright 1996, Ted Dunning, ted@hnc.com

Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: bigrams.c,v $
 * Revision 1.1  1996/05/16  21:54:21  ted
 * Initial revision
 *

 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "error.h"
#include "hash.h"
#include "stringpool.h"

hash_table words=NULL;

int read_word_counts(char *file_name)
{
    int n;
    char *w;
    int count;
    int total;
    FILE *f;

    if (strcmp(file_name, "-") == 0) {
	f = stdin;
    }
    else {
	f = efopen(file_name, "r");
    }

    if (!words) words = make_hash_table(100000);

    total = 0;
    n = fscanf(f, "%d", &count);
    w = get_word(f);
    while (n == 1 && w) {
	total += count;
	insert_new(words, w, the_int(count));
	n = fscanf(f, "%d", &count);
	w = get_word(f);
    }
    return total;
}

char *find_word(FILE *f, int *count)
{
    char *r, *w;
    hash_bucket v;
    pool_marker m;

    m = mark_pool();
    w = get_word(f);
    if (w) {
	v = find(words, w);
	if (v) {
	    r = v->s;
	    *count = v->value.i;
	}
	else {
	    r = NULL;
	}
    }
    else {
	r = NULL;
    }
    release_to_mark(m);
    return r;
}

int bayes_estimation = 0;

#define gamma 0.57721566490153286061

/* trigramma function of z.
   for z<10, this is accurate to machine precision, for z=10, error is
   about 10^-8.  at z=40, error decreases to about 10^-12 */
double psi(double z)
{
    double u, u2;

    if (z < 10) {
	u = -gamma;
	for (u2=1;u2<z;u2++) {
	    u += 1/u2;
	}
	return u;
    }
    else {
	u = 1.0/z;
	u2 = u*u;
	return log(z)+(-1.0/2+(-1.0/12+(1.0/120-1.0/252*u2)*u2)*u)*u;
    }
}
      
double logl(int *p, int total, int *k, int cols, int m)
{
    int j;
    double sum, logtotal;

    sum = 0;
    logtotal = log((double) total);

    for (j=0;j<cols;j++) {
	switch (bayes_estimation) {
	default:
	case 0:
	    if (p[j] != 0) {
		sum += k[j] * log((double) p[j] / total);
	    }
	    else if (k[j] != 0) {
		fprintf(stderr,
			"data error, column %d is completely zero\n", j);
		exit(1);
	    }
	    break;
	case 1:
	    sum += (k[j]+1) * log((double) (p[j]+1) / (total+m));
	    break;
	case 2:
	    sum += (k[j]+1) *
		(psi((double) p[j]+2)-psi((double) total+m+1));
	    break;
	}
    }
    return sum;
}

double lr(int **table, int rows, int cols)
{
    double sum;
    int *rowsums, *colsums;
    int total;
    int i,j;

    total = 0;
    rowsums = (int *) ecalloc(rows, sizeof(rowsums[0]));
    colsums = (int *) ecalloc(cols, sizeof(colsums[0]));
    for (j=0;j<cols;j++) {
	colsums[j] = 0;
    }

    for (i=0;i<rows;i++) {
	rowsums[i] = 0;
	for (j=0;j<cols;j++) {
	    rowsums[i] += table[i][j];
	    colsums[j] += table[i][j];
	}
	total += rowsums[i];
    }

    sum = 0;
    for (i=0;i<rows;i++) {
	sum +=
	    logl(table[i], rowsums[i], table[i], cols, rows*cols) -
	    logl(colsums, total, table[i], cols, rows*cols);
    }
    free(rowsums);
    free(colsums);
    return 2*sum;
}

void process_lines(char *file_name, int total)
{
    int n, k12, k1, k2;
    FILE *f;
    char *w1, *w2;

    if (strcmp(file_name, "-") == 0) {
	f = stdin;
    }
    else {
	f = efopen(file_name, "r");
    }

    n = fscanf(f, "%d", &k12);
    w1 = find_word(f, &k1);
    w2 = find_word(f, &k2);
    while (n == 1) {
	if (w1 && w2) {
	    int *k[2], kk[4];

	    k[0] = kk;
	    k[1] = kk+2;

	    k[0][0] = k12;
	    k[1][0] = k1-k12;
	    k[0][1] = k2-k12;
	    k[1][1] = total - k1 - k2 - k12;
	    if (k12 > k2 * ((double) k1/total)) {
		if (squashed_output) {
		    printf("%.2f %d %d %d %s %s\n",
			   lr(k, 2, 2), k12, k1, k2, w1, w2);
		}
		else {
		    printf("%20s %-20s %15.2f %7d %6d %6d\n",
			   w1, w2, lr(k, 2, 2), k12, k1, k2);
		}
	    }

	}
	n = fscanf(f, "%d", &k12);
	w1 = find_word(f, &k1);
	w2 = find_word(f, &k2);
    }

    if (strcmp(file_name, "-") != 0) {
	fclose(f);
    }
}

int main(int argc, char *argv[])
{
    int i;
    int stats=0;
    int total=0;

    extern char *optarg;

    int error;
    int squashed_output=0;

    int ch;
    extern optind;
    extern int getopt();

    error = 0;

    while ((ch = getopt(argc, argv, "bBw:vs")) != EOF) {
	switch (ch) {
	case 'b':
	    bayes_estimation = 1;
	    break;
	case 'B':
	    bayes_estimation = 2;
	    break;
	  case 'w':
	      total = read_word_counts(optarg);
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
    if (error || !words) {
	fprintf(stderr, "usage: lwcount [-vsh] [files ...]\n");
	fprintf(stderr, "-v means print hash table stats\n");
	fprintf(stderr, "-s means don't output lots of spaces\n");
	fprintf(stderr, "-c wxd means use a cache which of width w and depth d\n");
	exit(1);
    }

    if (argc == optind) {
	process_lines("-", total);
    }
    else {
	for (i=optind;i<argc;i++) {
	    process_lines(argv[i], total);
	}
    }

    return 0;
}

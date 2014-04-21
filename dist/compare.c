/* compares two word lists to find significant differences
   in frequency.  both inputs should be lines with count first followed
   by word.

   on the output there will be one line for every word in the union of
   the vocabularies of the two inputs.  each output line will contain
   a likelihood ratio score of how significant the difference between
   the two files is, the optional p-value for this score, followed by
   a > (or <) if the first file has a higher (or lower) ratio for a
   particular word, followed by the word itself.

   the command line arguments look like

compare [-p] file-1 file-2

   where file-1 and file-2 are filenames for the two counted and
   sorted word lists.

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: compare.c,v $
 * Revision 1.2  1996/05/16  21:50:14  ted
 * cleanup to avoid compiler warnings
 *
 * Revision 1.1  1996/05/02  15:46:18  ted
 * Initial revision
 *
 * Revision 1.3  1993/10/25  18:28:56  ted
 * added copyright message and rcs log message
 *
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include "chi2_lib.h"
#include "error.h"
#include "hash.h"

/* stolen from chi2 */
double logl(p, total, k, cols)
int *p, total, *k, cols;
{
    int j;
    double sum, logtotal;
    double log();

    sum = 0;
    logtotal = log((double) total);

    for (j=0;j<cols;j++) {
	if (p[j] != 0) {
	    sum += k[j] * log((double) p[j] / total);
	}
	else if (k[j] != 0) {
	    fprintf(stderr, "data error, column %d is completely zero\n", j);
	    exit(1);
	}
    }
    return sum;
}
    

double lr(table, rows, cols)
int **table;
int rows, cols;
{
    char *calloc();
    double sum;
    int *rowsums, *colsums;
    int total;
    int i,j;

    total = 0;
    rowsums = (int *) calloc(rows, sizeof(rowsums[0]));
    colsums = (int *) calloc(cols, sizeof(colsums[0]));
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
	    logl(table[i], rowsums[i], table[i], cols) -
	    logl(colsums, total, table[i], cols);
    }
    free(rowsums);
    free(colsums);
    return 2*sum;
}


int main(argc, argv)
int argc;
char *argv[];
{
    int i, j;
    char *calloc();
    extern int optind, getopt();
    hash_bucket el1, el2;	/* for looping through tables */
    hash_table t1, t2;		/* to store words and counts */
    int **counts;		/* used as an argument to lr */
    char w1[502], w2[502];	/* the words themselves */
    FILE *f1, *f2;		/* the input files */
    int  n1,  n2;		/* the total word counts */
    int  k1,  k2;		/* the particular word counts */
    int error, ch;		/* for recording error in command line */
    int print_significance=0;	/* should we print p-value? */

    make_progname(argc, argv);
    error = 0;
    while ((ch = getopt(argc, argv, "p")) != EOF) {
	switch (ch) {
	  case 'p':
	      print_significance = 1;
	      break;
	  case '?':
	      error = 1;
	      break;
	}
    }

    if (error || argc < optind+2) {
	fprintf(stderr, "usage: compare [-fp] file1 file2 ...\n");
	fprintf(stderr,
		"  -f means to not compute stats for words not in filen\n");
	fprintf(stderr, "  files are names of counted word lists\n");
	fprintf(stderr, "  file1 is compared to file2, then to file3 ...\n");
	fprintf(stderr, "  output is in the form\n");
	fprintf(stderr, "\n");

	exit(1);
    }

    counts = (int **) calloc(2, sizeof(counts[0]));
    counts[0] = (int *) calloc(2, sizeof(counts[0][0]));
    counts[1] = (int *) calloc(2, sizeof(counts[1][0]));

    if (strcmp(argv[optind], "-") == 0) {
	f1 = stdin;
    }
    else {
	f1 = efopen(argv[optind], "r");
    }

    n1 = 0;
    t1 = make_hash_table(10000);
    while (fscanf(f1, "%d", &k1) == 1) {
	getc(f1);
	fgets(w1, sizeof(w1)-1, f1);
	w1[sizeof(w1)-1] = 0;
	insert(t1, strdup(w1), the_int(k1));
	n1 += k1;
    }
    fclose(f1);

    for (i=optind+1;i<argc;i++) {
	if (optind+2 != argc) printf("%s\n", argv[i]);
	if (strcmp(argv[i], "-") == 0) {
	    f2 = stdin;
	}
	else {
	    f2 = efopen(argv[i], "r");
	}

	n2 = 0;
	t2 = make_hash_table(10000);
	while (fscanf(f2, "%d", &k2) == 1) {
	    char *s;
	    getc(f2);
	    fgets(w2, sizeof(w2)-1, f2);
	    w2[sizeof(w2)-1] = 0;
	    s = strdup(w2);
	    insert(t2, s, the_int(k2));
	    n2 += k2;
	    if (!find(t1, w2)) {
		insert(t1, s, the_int(0));
	    }
	}

	for(j=0;j<t2->size;j++) {
	    el2 = t2->table[j];
	    while (el2) {
		hash_bucket p;

		k2 = el2->value.i;
		el1 = find(t1, el2->s);
		if (el1) {
		    k1 = el1->value.i;
		}
		else {
		    k1 = 0;
		}
		counts[0][0] = k1;
		counts[1][0] = n1-k1;
		counts[0][1] = k2;
		counts[1][1] = n2-k2;
		
		if (print_significance) {
		    double r;		/* for saving a result */
		    
		    r = lr(counts, 2, 2);
		    printf("%.3f %.3f %c %s",
			   r, significance(r, 1),
			   "<>"[((double) k1/n1) > ((double) k2/n2)],
			   el1->s);
		}
		else {
		    printf("%.3f %c %s",
			   lr(counts, 2, 2),
			   "<>"[((double) k1/n1) > ((double) k2/n2)],
			   el1->s);
		}
		p = el2->next;
		free(el2->s);
		free(el2);
		el2 = p;
	    }
	}
	free(t2);
	fclose(f2);
    }
    return 0;
}

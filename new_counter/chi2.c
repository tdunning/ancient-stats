/*
analyze a sequence of contingency tables.

if the -l command line option is given, then a likelihood ratio test
is used.  this test is also known as the G-statistic.  for tables
where chi^2 is appropriate, the G-statistic and pearson's chi^2 should
give nearly identical values.

if the -k command line option is given, or if neither l, m nor k are
given, then pearson's chi^2 statistic is used.  if the expected value
in any cell is too small, a warning is printed to standard error and the
value -1 is used instead of the computed value.  the -x option turns off
these warnings and causes the (bogus) value which is actually computed
to be used.

with the -m command line option, then mutual information is estimated.

the -k, -m and -l options can be used together to compute more than
one statistic.  the -k and -l options can be combined with the -p
option to cause significance levels to be printed.  -p has no affect
on the mutual information output.

if the -x option is used, then the chi^2 value is computed and printed
even in cases where it is inappropriate.  all warning messages on
standard error are suppressed.

the -b and -B arguments influence what sort of estimators are used.
-b causes the laplace sample size correction to be used to estimate
probabilities (p = (k+1)/(N+m) where k is the count, N is the total of
all counts, m is the number of categories).  -B causes Bayesian
uniform prior estimators to be used.  generally this means that
expressions like

	sum k[i]/N log k[i]/N

to be replaced with

	sum (k[i]+1)/(N+m) Psi (k[i]+2)/(N+m+1)

although there is much more to the derivation than just this
substitution.  Psi in the formula above is the trigamma function.

finally, the only required command line arguments are the number of
rows and the number of columns in each contingency table.

a contingency table of the specified size is filled by reading numbers
from standard input and analyzed with the specified tests.  the 
values computed go to standard output.  the remainder of the last line of the
table is then echoed to the standard output.  this allows tables to be
tagged for later reference.  it also means that no more than one table
can appear on a line.

this analysis is repeated as long as there is input.  an error message
is produced if insufficient input is available to fill a table before
end of file is reached.

for example, the following would be legal input for chi2 2 2,

100 120 200 250 first
30  40  40  50  second

and would produce the following output:

      0.06  first
      0.04   second

the first line is the chi^2 value for the table

  100 120
  200 250

and the second line is the chi^2 value for the table

   30  40
   40  50

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: chi2.c,v $
 * Revision 1.2  1996/05/16  21:49:21  ted
 * cleanups to avoid compile time warnings
 *
 * Revision 1.1  1996/05/03  18:39:56  ted
 * Initial revision
 *
 * Revision 1.3  1993/12/21  23:11:53  ted
 * added bayesian estimators for mutual information
 * calculation.
 *
 * Revision 1.2  1993/10/25  18:22:54  ted
 * updated copyright notice.  added rcs log message.  changed
 * main to use prototypes (more changes like that will be needed).
 *

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>

#include "chi2_lib.h"

#define sq(x) ((x) * (x))

int use_lr_test = 0;
int use_chi2_test = 0;
int use_mutual = 0;
int bayes_estimation = 0;	/* 1 means use laplace population correction */
				/* 2 means use bayesian estimators */
int mark_errors = 1;
int print_significance = 0;

double chi2(table, rows, cols)
int **table;
int rows, cols;
{
    int *rowsums, *colsums;
    int total;
    int i,j;
    double result, expected;

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

    result = 0;
    for (i=0;i<rows;i++) {
	for (j=0;j<cols;j++) {
	    expected = (double) colsums[j] * rowsums[i]/total;
	    if ((expected < 5 && mark_errors) || expected == 0) {
		if (expected != 0 || table[i][j] != 0) {
		    fprintf(stderr,
			    "chi^2 inappropriate (table[%d][%d] = %d,"
			    " %.2f expected < 5)\n",
			    i, j, table[i][j], expected);
		    return -1;
		}
	    }
	    else {
		result += sq(table[i][j] - expected)/expected;
	    }
	}
    }
    free(rowsums);
    free(colsums);
    return result;
}

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
	u=1.0/z;
	u2=u*u;
	return log(z)+(-1.0/2+(-1.0/12+(1.0/120-1.0/252*u2)*u2)*u)*u;
    }
}
      
double logl(p, total, k, cols, m)
int *p, total, *k, cols, m;
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

double lr(table, rows, cols)
int **table;
int rows, cols;
{
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
	    logl(table[i], rowsums[i], table[i], cols, rows*cols) -
	    logl(colsums, total, table[i], cols, rows*cols);
    }
    free(rowsums);
    free(colsums);
    return 2*sum;
}

double entropy_term(p, total, cols, m)
int *p, total, cols, m;
{
    double sum;

    sum = logl(p, total, p, cols, m);
    switch (bayes_estimation) {
    default:
    case 0:
	sum /= total;
	break;
    case 1:
    case 2:
	sum /= (total+m);
	break;
    }

    return sum;
}
    
double mutual(table, rows, cols)
int **table;
int rows, cols;
{
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

    sum = -entropy_term(rowsums, total, cols, cols) -
	entropy_term(colsums, total, rows, rows);

    for (i=0;i<rows;i++) {
	sum += entropy_term(table[i], total, cols, rows*cols);
    }

    free(rowsums);
    free(colsums);
    return sum;
}
    
int main(int argc, char **argv)
{
    extern int optind, getopt();
    int **table;
    int i, j, rows, cols;
    int ch, error;
    int explain = 0;
    double r;

    error = 0;
    while ((ch = getopt(argc, argv, "klxmbBp")) != EOF) {
	switch (ch) {
	case 'l':
	    use_lr_test = 1;
	    break;
	case 'k':
	    use_chi2_test = 1;
	    break;
	case 'x':
	    mark_errors = 0;
	    use_chi2_test = 0;
	    break;
	case 'v':
	    explain = 1;
	    break;
	case 'm':
	    use_mutual = 1;
	    break;
	case 'p':
	    print_significance = 1;
	    break;
	case 'b':
	    bayes_estimation = 1;
	    break;
	case 'B':
	    bayes_estimation = 2;
	    break;
	case '?':
	    error = 1;
	    break;
	}
    }
    if ( !(use_chi2_test || use_mutual || use_lr_test)) {
	use_chi2_test = 1;
    }
    if (bayes_estimation && use_lr_test) {
	fprintf(stderr, "WARNING: using bayesian estimators with likelihood\n");
	fprintf(stderr, "ratio test may lead to strange results\n");
    }

    if (error) {
	fprintf(stderr, "usage: chi2 [-lmk] [-x] [-p] nrows ncols\n");
	fprintf(stderr,
		"-l means use likelihood ratio test\n");
	fprintf(stderr,
		"-l means use chi^2 test\n");
	fprintf(stderr,
		"-x means don't bitch if using chi^2 and counts are small\n");
	fprintf(stderr,
		"-m means print mutual information\n");
	fprintf(stderr,
		"-p causes a p-value to be printed for chi^2 or l.r. tests\n");
	fprintf(stderr,
		"-b causes Laplace's sample size correction to be used\n");
	fprintf(stderr,
		"-B causes Bayesian estimators to be used\n");
	fprintf(stderr,
		"-v means explain what options are in effect\n");
	exit(1);
    }

    if (use_lr_test) {
	mark_errors = 0;
    }

    if (explain) {
	if (use_chi2_test) {
	    fprintf(stderr, "using chi^2 test\n");
	}
	if (use_lr_test) {
	    fprintf(stderr, "using likelihood ratio test\n");
	}
	if (use_mutual) {
	    fprintf(stderr, "computing mutual information\n");
	}
	    

	if (mark_errors) {
	    fprintf(stderr, "noting invalid input for chi^2 test\n");
	}
	else {
	    fprintf(stderr, "not marking invalid input for chi^2 test\n");
	}
    }

    if (argc != optind+2) {
	fprintf(stderr, "usage: chi2 [-l] [-x] [-m] nrows ncols\n");
	fprintf(stderr,
		"-l means use likelihood ratio test instead of chi^2\n");
	fprintf(stderr,
		"-x means use normal chi^2 test no matter what\n");
	fprintf(stderr,
		"-m means use mutual information\n");
	fprintf(stderr,
		"-v means explain what options are in effect\n");
	exit(1);
    }

    rows = atoi(argv[optind]);
    cols = atoi(argv[optind+1]);

    table = (int **) calloc(rows, sizeof(table[0]));
    for (i=0;i<rows;i++) {
	table[i] = (int *) calloc(cols, sizeof(table[0][0]));
    }

    while (!feof(stdin)) {
	for (i=0;i<rows;i++) {
	    for (j=0;j<cols;j++) {
		if (scanf("%d", &table[i][j]) != 1) {
		    if (i || j) {
			fprintf(stderr, "unexpected EOF\n");
			exit(1);
		    }
		    else {
			exit(0);
		    }
		}
	    }
	}

	if (use_chi2_test) {
	    r = chi2(table, rows, cols);
	    printf("%10.2f ", r);
	    if (print_significance) {
		printf("%.5f ", significance(r, (rows-1)*(cols-1)));
	    }
	}

	if (use_lr_test) {
	    r = lr(table, rows, cols);
	    printf("%10.2f ", r);
	    if (print_significance) {
		printf("%.5f ", significance(r, (rows-1)*(cols-1)));
	    }
	}

	if (use_mutual) {
	    r = mutual(table, rows, cols);
	    printf("%.2f ", r);
	}

	ch = getchar();
	while (ch != '\n' && ch != EOF) {
	    putchar(ch);
	    ch = getchar();
	}
	putchar('\n');
    }
    return 0;
}

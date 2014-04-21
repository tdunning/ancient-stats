/*

  input is groups of lines each of which contains a count, followed by
  the token counted.  the token extends from just after the space
  which terminates the count until the end of the line.  groups of
  lines are separated by a blank line.

*/

#include <stdio.h>
#include <math.h>

#define gamma 0.57721566490153286061
#define MAX_COUNTS 100000

double p[MAX_COUNTS];

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
      
void do_entropy(char *file_name, FILE *f, int use_bayes, int label_output)
{
    int i,j;
    int total;
    int ch;
    double sum;
    double log2;
    int line;

    log2 = log(2.0); 

    ch = getc(f);
    line = 1;
    while (ch != EOF) {
	i = 0;
	while (ch != '\n' &&
	       i < MAX_COUNTS &&
	       ch != EOF) {
	    ungetc(ch, f);
	    if (fscanf(f, "%lf", &p[i]) != 1) {
		fprintf(stderr,
			"format error in input for entropy at line %d.\n",
			line);
		fprintf(stderr, "I want lines starting with counts\n");
	    }
	    else {
		i++;
	    }
	    ch = getc(f);
	    while (ch != EOF && ch != '\n') ch = getc(f);
	    line++;
	    ch = getc(f);
	}
	line++;
	ch = getc(f);

	total = 0;
	for (j=0;j<i;j++) {
	    total += p[j];
	}
	if (use_bayes & 0x4) {
	    sum = 0;
	    for (j=0;j<i;j++) {
		sum += (p[j]+1) * psi(p[j]+2);
	    }
	    sum /= (total+i);
	    sum -= psi(total+i+1.0);
	    printf("%10.3f", -sum/log2);
	}
	if (use_bayes & 0x2) {
	    sum = 0;
	    for (j=0;j<i;j++) {
		sum += (p[j]+1)/(total+i) * log((p[j]+1)/(total+i));
	    }
	    printf("%10.3f", -sum/log2);
	}
	if (use_bayes & 1) {
	    for (j=0;j<i;j++) {
		p[j] /= total;
	    }
	    sum = 0;
	    for (j=0;j<i;j++) {
		if (p[j]) {
		    sum += p[j] * log(p[j]);
		}
	    }
	    printf("%10.3f", -sum/log2);
	}
	if (label_output) printf(" %s\n", file_name);
	else printf("\n");
    }
}


int main(int argc, char *argv[])
{
    extern int optind, getopt();
    int ch, use_bayes, error;

    error = 0;
    use_bayes = 1;
    while ((ch = getopt(argc, argv, "bB")) != EOF) {
	switch (ch) {
	case 'b':
	    use_bayes = 2;
	    break;
	case 'B':
	    use_bayes = 4;
	    break;
	default:
	    error = 1;
	    break;
	}
    }
    if (error) {
	fprintf(stderr, "usage: entropy [-b] [-B] {files}\n");
	fprintf(stderr, " no option means use k/N as probability estimate\n");
	fprintf(stderr, " -b means use (k+1)/(N+m) as probability estimate\n");
	fprintf(stderr, " -B means use Bayesian unbiased estimator for H\n");
	exit(1);
    }
    
    if (optind >= argc) {
	do_entropy("-", stdin, use_bayes, 0);
    }
    else {
	int i;
	FILE *f;
	for (i=optind;i<argc;i++) {
	    f = fopen(argv[i], "r");
	    if (!f) {
		fprintf(stderr, "entropy: cannot open file %s\n", argv[i]);
		exit(1);
	    }
	    do_entropy(argv[i], f, use_bayes, 1);
	}
    }

    return 0;
}

    

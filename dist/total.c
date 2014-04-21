/* total all counts in a file (or files) of counts.  the input is in
   the form

   count tag

   where the tags are ignored and the counts are added

   */


#include <stdio.h>
#include <ctype.h>

#include "error.h"

int total_file(f)
FILE *f;
{
    int total=0;
    int count, ch;

    fscanf(f, "%d", &count);
    ch = getc(f);
    while (ch != '\n' && ch != EOF) {
	ch = getc(f);
    }

    while (ch != EOF) {
	total += count;
	fscanf(f, "%d", &count);
	ch = getc(f);
	while (ch != '\n' && ch != EOF) {
	    ch = getc(f);
	}
    }
    return total;
}

int main(int argc, char *argv[])
{
    int total;

    int error;
    int verbose=0;

    int ch;
    extern int optind;
    extern int getopt();

    error = 0;
    while ((ch = getopt(argc, argv, "vh")) != EOF) {
	switch (ch) {
	case 'v':
	    verbose = 1;
	    break;
	case 'h':
	    fprintf(stderr, "usage: total [-vh] [files ...]\n");
	    fprintf(stderr, "-v means talk a lot\n");
	    fprintf(stderr, "-h means print this help\n");
	    exit(0);
	case '?':
	default:
	    error = 1;
	    break;
	}
    }
    if (error) {
	    fprintf(stderr, "usage: total [-vh] [files ...]\n");
	    fprintf(stderr, "-v means talk a lot\n");
	    fprintf(stderr, "-h means print this help\n");
	exit(1);
    }

    if (argc > optind) {
	int i;
	for (i=optind;i<argc;i++) {
	    FILE *f;
	    int count;

	    f = efopen(argv[i], "r");
	    count = total_file(f);
	    if (verbose) {
		printf("%d %s\n", count, argv[i]);
	    }
	    total += count;
	    fclose(f);
	}
	printf("%d\n",  total);
    }
    else {
	printf("%d\n",  total_file(stdin));
    }

    return 0;
}


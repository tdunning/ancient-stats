/* print out sliding or non-overlapping window character n-grams.

   command line options include:

    -o     don't slide the window
    -n N   the size of the n-gram
    files  the files to be processed as if concatenated

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef NTGCC
#include <malloc.h>
#endif

int  chars_read=0;
int  next;
char *buffer;

inline int getch(FILE *f)
{
    int ch;

    ch = getc(f);
    if (ch != EOF && !isprint(ch) && ch < 128) {
	ch = '*';
    }
    if (isspace(ch)) {
	ch = '_';
    }
    return ch;
}


void cgram_file(FILE *f, int N, int slide)
{
    int ch;
    ch = getch(f);
    while (ch != EOF && chars_read < N) {
	buffer[chars_read++] = ch;
	ch = getch(f);
    }
    next = 0;

    while (ch != EOF) {
	fwrite(buffer+next, N-next, 1, stdout);
	fwrite(buffer, next, 1, stdout);
	putchar('\n');

	if (slide) {
	    buffer[next] = ch;
	    next = (next+1)%N;
	    ch = getch(f);
	}
	else {
	    chars_read = 0;
	    while (ch != EOF && chars_read < N) {
		buffer[chars_read++] = ch;
		ch = getch(f);
	    }
	}
    }
}

int main(int argc, char *argv[])
{
    int N=6;
    int slide=1;
    int error_flag;

    int ch, i;

    extern char *optarg;
    extern int optind;
    extern int getopt();

    error_flag = 0;
    while (!error_flag
	   && (ch = getopt(argc, argv, "on:")) != EOF) {
	switch (ch) {
	case 'o':
	    slide = 0;
	    break;
	case 'n':
	    sscanf(optarg, "%d", &N);
	    break;
	default:
	    error_flag = 1;
	}
    }

    if (error_flag) {
	fprintf(stderr, "usage: cgram [-o] [-n window-size] {files}\n");
	exit(1);
    }

    buffer = malloc(N);
    if (!buffer) {
	fprintf(stderr, "cannot allocate %d byte character buffer\n", N);
	exit(1);
    }

    if (optind >= argc) {
	cgram_file(stdin, N, slide);
    }
    else {
	FILE *f;
        for (i = optind; i < argc; i++) {
	    if (strcmp(argv[i], "-") == 0) {
		f = stdin;
	    }
	    else {
		f = fopen(argv[i], "r");
		if (f == NULL) {
		    fprintf(stderr, "Error: can't open file.");
		    exit(1);
		}
	    }

	    cgram_file(f, N, slide);

            if (f != stdin) {
		fclose(f);
	    }
	}
    }
    exit(0);
}


    

/*
Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: error.c,v $
 * Revision 1.2  1993/10/25  18:31:31  ted
 * added copyright and rcs log message
 *
 * Revision 1.2  1993/10/25  18:31:31  ted
 * added copyright and rcs log message
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

void *calloc(), *malloc();

static char *progname;

void make_progname(argc, argv)
int argc;
char *argv[];
{
    char *strdup();
    if (argc > 0) {
	progname = strdup(argv[0]);
	if (progname == NULL) fatal("make_progname", "cannot allocate space");
    }
    else {
	progname = "???";
    }
}

FILE *efopen(name, mode)
char *name, *mode;
{
    FILE *f;

    f = fopen(name, mode);
    if (f == NULL) {
	char message[1000];	/* file name should be shorter than this */
	strcpy(message, "cannot open ");
	strcat(message, name);
	fatal("efopen", message);
    }
    return f;
}

void *ecalloc(n,size)
int n,size;
{
    void *r;

    r = calloc(n,size);

    if (r == NULL) {
	fatal("ecalloc", "cannot allocate space");
    }
    return r;
}

void *emalloc(n)
int n;
{
    void *r;

    r = malloc(n);

    if (r == NULL) {
	fatal("emalloc", "cannot allocate space");
    }
    return r;
}

void fatal(where, what)
char *where, *what;
{
    fprintf(stderr, "ERROR\n%s:%s: %s\n", where, what, progname);
    exit(1);
}

void warning(where, what)
char *where, *what;
{
    fprintf(stderr, "ERROR\n%s:%s: %s\n", where, what, progname);
}

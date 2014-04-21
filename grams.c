/* put out a list of n-grams 
   each n-grams is formed by printing the previous n lines from stdin on 
   stdout separated by spaces.

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: grams.c,v $
 * Revision 1.2  1996/05/16  21:52:04  ted
 * eliminated compiler warnings
 *
 * Revision 1.1  1996/05/03  18:38:19  ted
 * Initial revision
 *
 * Revision 1.1  1993/10/25  18:51:27  ted
 * Initial revision
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(c,v)
int c;
char *v[];
{
    int  i;
    int  history;
    char **old, *t;

    if (c == 1) history = 3;
    else {
	history = atol(v[1]);
	if (history < 0) history = -history;
	if (history == 0) {
	    fprintf(stderr, "usage: grams [n] < words\n");
	    exit(1);
	}
    }

    old = (char **) calloc(history, sizeof(old[0]));
    for (i=0;i<history;i++) {
	old[i] = malloc(100);
    }

    for (i=0;i<history-1;i++) {
	if (!gets(old[i])) {
	    exit(0);
	}
    }

	gets(old[history-1]);
    while (!feof(stdin)) {
	for (i=0;i<history;i++) {	
	    printf("%s ", old[i]);
	}
	printf("\n");
	t = old[0];
	for (i=0;i<history-1;i++) {
	    old[i] = old[i+1];
	}
	old[history-1] = t;
	gets(old[history-1]);
    }
    return 0;
}

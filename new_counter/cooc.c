/*              
   Copyright (c) 1996, Aptex Inc. 9605 Scranton Road Suite 240, San Diego.
   All Rights Reserved.

   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF APTEX INC
   The copyright notice above does not evidence any actual or intended
   publication of such source code.

   RESTRICTED RIGHTS LEGEND:  Use, duplication, or disclosure by the government
   is subject to restrictions as set forth in subparagraph (c)(1)(ii) of the
   Rights in Technical Data and Computer Software clause at DFARS 252.227-7013
   and FAR 52.227-19.

	Product:
	    Stats - text statistical analysis software

	Module:
	    cooc.c

	Created:
	    5/1/96

	Authors         Initials        Involvement
	-------         --------        -----------
	Ted Dunning     ted             Creator
*/

/* Pump out all coocurring pairs of lines.  If requested, the pairs
   are counted to assist with statistical analysis.

   Cooc uses a window as a limit for the cooccurrence and can look for
   sentence boundaries or not as desired.  It is assumed that each
   word will be alone on a line and that lines with the string #S
   will be used to denote sentence boundaries (if necessary).

   This program analyses only unidirectional coocurrence.  That is, in
   the sequence "a b d c a d", and  window size of the, the coocurring
   pairs would be 

	a	b
	a	d
	b	d
	b	c
	d	c
	c	a
	d	a
	c	d
	a	d

   Items to note include the fact that words do not cooccur with
   themselves, and we do not see a "c b" pair in this list.  When cooc
   is asked to count the pairs in this sequence, "b c" is counted, but
   "c b" is not.

   When the count output is selected, each distinct cooccurring pair
   has four numbers preprended to it.  These numbers are suitable for
   input to the chi2 contingency table analysis program (which would
   get command line options of -l 2 2).  For the sequence given, the
   counts output by cooc would be

       1 1 1 5 c a
       1 1 2 4 d c
       1 1 1 5 b c
       1 0 2 4 a b
       1 1 2 4 d a
       2 2 2 4 a d
       1 3 1 5 b d
       1 3 1 5 c d

   The numbers in the columns are the entries in a contingency table
   which would be used to analyse whether the cooccurring pair was
   likely to have happened by chance.  This table has elements of the
   form (for the cooccurrence of a and b):


   		T(a b)     T( not(a) b)
		T(a *)     T( not(a) *)

   In this notation T(something) is the number of times "something"
   occurred.  As might be guessed, T(a *) is the number of times a
   occurred to the left of anything and T(not(a) b) is the number of
   times anything other than a occurred to the left of b.

   The command line options which control the operation of the counter
   can be divided into several conceptual categories

     a) control of the size, shape and behavior of the leaky cache

     b) control of what sorts of coocurrence events are counted.

   Options in category (a) include:

     -r       random backoff for placement in cache line (default is off).
     -b n     backup level which controls deterministic placement in
              cache line (default is 5)
     -C RxC   this option determines the cache shape.  bigger is
              generally better, subject to memory limits.  each cache
              element takes up 16 bytes. (default is 100000x64)

   For the most part, options other than -C which are in category (a)
   should not need to be adjusted.
	     
   Options in category (b) include:

     -c       count pairs instead of just printing them (default is to print)
     -k file  kill list.  pairs which include a word in the kill list
              will not be counted.  the format is one word per line.
     -n N     window size.
     -s       don't count across sentence boundaries.  if this is set,
              then only pairs which are in the same sentence will be
              counted.  sentence boundaries should be indicated by the
	      occurrence of the pseudo word "#S".


   Questions regarding this program should be sent to ted@aptex.com or
   ted@nmsu.edu

 ****************************************************************************


 * $Log: cooc.c,v $
 * Revision 1.4  1996/07/16  22:49:08  ted
 * fixed another typo
 *
 * Revision 1.3  1996/07/16  22:46:18  ted
 * fixed silly comment nesting typo
 *
 * Revision 1.2  1996/07/16  21:43:06  ted
 * added capability to change parameters of leaky cache
 * and to output full table of counts for chi2
 *
 * Revision 1.1  1996/05/16  21:54:30  ted
 * Initial revision
 *
   */

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "hash.h"
#include "xvector.h"
#include "intern_word.h"

#include "leaky_cache.h"

/* define a word as an extensible vector of characters */
x_vector(word, char);

typedef struct {
    word w;
    int killed;
    hash_bucket right;
} word_info;

int print_pairs;		/* print the pairs (or count them) */

int total_count = 0, killed_count = 0;
hash_table right, words;
leaky_cache pairs;

int get_line(FILE *f, word w)
{
    int ch;
    x_length(w) = 0;

    ch = getc(f);
    while (ch != EOF && ch != '\n') {
	x_push(w, ch);
	ch = getc(f);
    }
    if (ch == EOF && x_length(w) == 0) return EOF;
    else {
	x_push(w, 0);
	return 1;
    }
}

void process(FILE *f, word_info *old, int history,
	     hash_table kill_file, int sentence_bound)
{
    int i;
    int current, wrapped;

    current = 0;
    wrapped = 0;
    while (get_line(f, old[current].w) != EOF) {
	if (sentence_bound && strcmp("#S", old[current].w->contents) == 0) {
	    current = wrapped = 0;
	}
	else {
	    total_count++;
	    old[current].killed = (kill_file &&
				   find(kill_file, old[current].w->contents));

	    if (old[current].killed) {
		killed_count++;
	    }
	    else {
		hash_bucket p;

		p = intern_word(words, old[current].w->contents);
		p->value.i++;

		old[current].right = intern_word(right,
						 old[current].w->contents);
		for (i=0;i<current;i++) {
		    old[current].right->value.i++;
		    if (!old[i].killed) {
			if (print_pairs) {
			    printf("%s\t%s\n",
				   old[i].w->contents,
				   old[current].w->contents);
			}
			else {
			    count_in_cache(pairs,
					   old[i].right->s,
					   old[current].right->s);
			}
		    }
		}
		if (wrapped) {
		    for (i=current+1;i<history;i++) {
			old[current].right->value.i++;
			if (!old[i].killed) {
			    if (print_pairs) {
				printf("%s\t%s\n",
				       old[i].w->contents,
				       old[current].w->contents);
			    }
			    else {
				count_in_cache(pairs,
					   old[i].right->s,
					   old[current].right->s);
			    }
			}
		    }
		}
	    }
	    current++;
	    if (current == history) {
		wrapped = 1;
		current = 0;
	    }
	}
    }
}

hash_table read_hash_table(char *filename)
{
    word line;
    FILE *kill_file;
    hash_table r;

    kill_file = fopen(filename, "r");
    if (!kill_file) {
	fprintf(stderr, "cannot open kill file (%s)\n", filename);
	exit(1);
    }

    r = make_hash_table(1000);
    x_create(line, 10);
    while (get_line(kill_file, line) != EOF) {
	insert(r, strdup(line->contents), the_int(0));
    }
    x_free(line);
    return r;
}

int main(int argc, char *argv[])
{
    int  i, j, k;
    int  history, sentence_bound = 0;
    char *malloc(), *calloc();
    word_info *old;

    int width, depth, backup, random;

    hash_table kill_file=NULL;

    int error_flag;
    int ch;

    extern int getopt();
    extern char *optarg;
    extern int optind;

    history = 3;
    error_flag = 0;
    print_pairs = 1;

    width = 100000;
    depth = 64;
    backup = 5;
    random = 0;

    while (!error_flag
	   && (ch = getopt(argc, argv, "rb:C:ck:n:s")) != EOF) {
	switch (ch) {
	  case 'r':
	      random = 1;
	      break;
	  case 'b':
	      sscanf(optarg, "%d", &backup);
	      break;
	  case 'C':
	      if (sscanf(optarg, "%dx%d", &width, &depth) != 2) {
		  fprintf(stderr,
			  "%s got bad cache shape specification.  "
			  "Wanted wxd (for example -c 1000x16)\n", argv[0]);
		  error_flag++;
	      }
	      break;
	  case 'c':
	      print_pairs = 0;
	      break;
	  case 'k':
	      kill_file = read_hash_table(optarg);
	      break;
	  case 'n':
	      sscanf(optarg, "%d", &history);
	      break;
	  case 's':
	      sentence_bound = 1;
	      break;
	  default:
	      error_flag = 1;
	}
    }

    if (history <= 0 || history > 30) {
	error_flag++;
    }
    if (error_flag) {
	fprintf(stderr, "usage: cooc [-n window] [-s] files...\n");
	exit(1);
    }

    old = calloc(history, sizeof(old[0]));
    for (i=0;i<history;i++) {
	x_create(old[i].w, 10);
    }

    words = make_hash_table(100000);
    right = make_hash_table(100000);
    pairs = make_leaky_cache(width, depth, backup, random);

    if (optind >= argc) {
	process(stdin, old, history, kill_file, sentence_bound);
    }
    else {
	FILE *f;
	for (i=optind;i<argc;i++) {
	    f = fopen(argv[i], "r");
	    if (!f) {
		fprintf(stderr, "Can't open %s\n", argv[i]);
	    }
	    else {
		process(f, old, history, kill_file, sentence_bound);
		fclose(f);
	    }
	}
    }
    if (!print_pairs) {
	k = 0;
	for (i=0;i<pairs->width;i++) {
	    for (j=0;j<pairs->depth;j++) {
		hash_value v1, v2;
		if (pairs->table[k].w1) {
		    v1 = must_find(words, pairs->table[k].w1);
		    v2 = must_find(right, pairs->table[k].w2);

		    printf("%d %d %d %d %s %s\n",
			   pairs->table[k].count,
			   v2.i - pairs->table[k].count,
			   v1.i,
			   total_count - v1.i,
			   pairs->table[k].w1,
			   pairs->table[k].w2);
		}
		k++;
	    }
	}
    }
    for (i=0;i<history;i++) {
	x_free(old[i].w);
    }
    free(old);
    return 0;
}

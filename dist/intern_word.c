/* intern a word so that a unique pointer is returned for each string
 * that is intern'ed.  this allows comparisons to be made using == and also
 * makes some hash packages happier.

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
	    intern_word.c

	Created:
	    5/1/96

	Authors         Initials        Involvement
	-------         --------        -----------
	Ted Dunning     ted             Creator



$Log: intern_word.c,v $
 * Revision 1.3  1996/07/16  22:39:25  ted
 * added aptex copyright notice
 *
 * Revision 1.2  1996/06/04  21:50:39  ted
 * streamlined the api and semantics for intern_word
 *
 * Revision 1.1  1996/05/16  21:13:13  ted
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "error.h"
#include "hash.h"
#include "stringpool.h"

hash_bucket intern_word(hash_table word_table, char *w)
{
    hash_bucket p;

    w = strdup(w);
    p = find_or_create(word_table, w, the_int(0));
    if (p->value.i != 0) {	/* release storage if possible */
	free(w);
    }
    return p;
}

char *get_interned_string(hash_bucket p)
{
    return p->s;
}

#ifdef TEST
int main(int argc, char *argv[])
{

    printf("%x %s\n", intern_word("tiger"), "tiger");
    printf("%x %s\n", intern_word("this"), "this");
    printf("%x %s\n", intern_word("that"), "that");
    printf("%x %s\n", intern_word("other"), "other");
    printf("%x %s\n", intern_word("most"), "most");
    printf("%x %s\n", intern_word("fun"), "fun");
    printf("%x %s\n", intern_word("tiger"), "tiger");
    printf("%x %s\n", intern_word("fun"), "fun");

    printf("%s %s\n", get_interned_string(intern_word("tiger")), "tiger");

    return 0;
}
#endif

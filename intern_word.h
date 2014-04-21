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
	    intern_word.c

	Created:
	    5/1/96

	Authors         Initials        Involvement
	-------         --------        -----------
	Ted Dunning     ted             Creator
*/
#include "hash.h"

hash_bucket intern_word(hash_table word_table, char *w);

char *get_interned_string(hash_bucket p);

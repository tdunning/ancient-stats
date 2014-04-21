#include <stdio.h>
#include <ctype.h>

/* reasonably good hex dumper */

/* 
usage is:
hd [option-list] [files]
option-list is a string with a leading '-' followed by 
zero or more of the following

	a	output addresses
	h	output hex codes
	t	output sanitized ascii representation (text)

a '-' alone is the same as -h
if no options are given -aht is assumed.

files is a list of file names to be dumped.  if no files are
specified, then the standard input is used.  a '-' in place of a file
name specifies the standard input.

the hex codes are surrounded by ( and ) so that unhd can find them.

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: hd.c,v $
Revision 1.2  1996/08/23 17:25:22  ted
changed to ANSI C and improved variable names

Revision 1.1  1996/05/16 21:52:20  ted
Initial revision

 * Revision 1.1  1993/10/25  18:54:49  ted
 * Initial revision
 *
*/

/* arguments to copy are the 3 flags, the number of bytes to 
   print per line, and the input file */
void copy(int print_address, int print_hex, int print_text, int byte_count,
	  FILE *in)
{
    long add;				/* address of this byte */
    int n;				/* counter within a line */
    int i;				/* loop counter */
    int current_byte;			/* current byte */
    char buf[64];			/* where to stash in ascii dump */

    /* initialize some counters and get the first character */
    add = 0;
    n = 0;
    current_byte = getc(in);

    while (current_byte != EOF) {
	if (!(add%byte_count)) {	/* print the address field if needed */
	    if (print_address) printf("%6lx ",add);
	    if (print_hex) printf("( ");
	    else printf("(...) ");
	    n = 0;
	}

	if (print_hex) {		/* print a hex byte */
	    printf("%02x ", (unsigned) 0xff & current_byte);
	}

	if (print_text) {		/* perhaps save that byte */
	    buf[n] = current_byte;
	}

	/* check for end of line time */
	if (n == byte_count-1) {
	    if (print_hex) {		/* finish the hex bytes */
		printf(")");
	    }

	    if (print_text) {		/* should we display the text?? */

		/* do we need a gutter after the hex dump part? */
		if (print_hex) printf("  ");

		for (i=0;i<=n;i++) {	/* print the characters */
		    /* if we can */
		    if (isascii(buf[i]) && isprint(buf[i]))
			putchar(buf[i]);

		    else putchar('.');	/* otherwise print something else */
		}
	    }

	    putchar('\n');		/* end of line always gets this */
	    n = 0;
	}
	else {				/* not end of line */
	    n++;
	}

	current_byte = getc(in);
	add++;
    }

    /* pretty much done, may have started a partial line */
    if (n) {
	if (print_hex) {
	    for (i=n;i<byte_count;i++) printf("   ");
	    printf(")");

	    /* put in the gutter if we had a hex dump */
	    if (print_hex) printf("  ");
	}

	/* print any text dump that is needed */
	if (print_text) {
	    /* and print the printable chars */
	    for (i=0;i<n;i++) {
		if (isascii(buf[i]) && isprint(buf[i]))
		    putchar(buf[i]);
		else putchar('.');
	    }
	}

	putchar('\n');			/* even partial lines get this */
    }
}

int main(int argc, char *argv[])
{
    int addresses, hex, text;		/* flags for options */
    int bytes;				/* number of bytes to show on a line */
    int i;				/* handy for loops */
    FILE *file;

    addresses = text = hex = 1;		/* no flags defaults for everything */

    if (argc == 1) {
    }
    else if (argv[1][0] == '-') {	/* check for flags */
	addresses = text = hex = 0;

	/* scan a resonable number of characters */
	for (i=1;i<5 && argv[1][i];i++) {
	    switch (argv[1][i]) {

	    case 'a':			/* turn on address field */
		addresses = 1;
		break;

	    case 'h':			/* turn on hex dump */
		hex = 1;
		break;

	    case 't':			/* turn on ascii dump */
		text = 1;
		break;
	    }
	}
    }

    /* set up the number of bytes to display on each line */
    if (hex) {
	bytes = 16;
    }
    else if (text) {
	bytes = 64;
    }
    else {
	fprintf(stderr,"must use either -h or -t");
	exit(1);
    }

    /* either no files specififed on command line */
    if (argc == 1 || (argc == 2 && argv[1][0] == '-')) {
	copy(addresses,hex,text,bytes,stdin);
    }
    else {
	/* or for all files that are specified */
	for (i=(argv[1][0] == '-')+1;i<argc;i++) {
	    if (argv[i][0] != '-') {
		file = fopen(argv[i],"r");
		if (file) copy(addresses,hex,text,bytes,file);
		fclose(file);
	    }
	    else {
		copy(addresses,hex,text,bytes,stdin);
	    }
	}
    }
    return 0;
}


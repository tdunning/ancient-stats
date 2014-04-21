#include <stdio.h>
#include <ctype.h>

/* the hex UN-dumper....takes a stream of hexadecimal numbers and
	converts them to bytes....usage is:
unhd files

where files is a list of files to be undumped.  if files is null, then
the standard input is undumped.  only things in parentheses are
converted.  this program is an inverse of hd.

  Copyright 1993, Ted Dunning, ted@nmsu.edu
  Redistribution permitted if I am notified, and if this copyright notice
  is preserved.  Inclusion in other software is also allowed on the same
  terms.  Other terms by request.

$Log: unhd.c,v $
Revision 1.2  1996/08/23 17:17:39  ted
changed to ANSI C

Revision 1.1  1996/05/16 21:53:22  ted
Initial revision

 * Revision 1.1  1993/10/25  19:03:01  ted
 * Initial revision
 *
*/

int unhex(c)
int c;
{
	if (isdigit(c)) return c-'0';
	else return c-'a'+10;
}

void undump(FILE *in)
{
     int c;
     int byte;

     c = getc(in);
     while (c != EOF) {		/* do each line */
	  /* skip to the real stuff */
	  while (c != EOF && c != '(' && c != '\n') c = getc(in);

	  if (c == '\n') {	/* empty line? */
	       c = getc(in);	/* go on to the next one */
	  }
	  else {
	       /* eat the ( */
	       if (c != EOF) c = getc(in);

	       /* skip any fluffy whitespace */
	       while (c != EOF && isspace(c)) c = getc(in);

	       /* convert this line */
	       while (c != ')') {
		    /* can't have any garbage in the hex part */
		    if (c == EOF || !isxdigit(c)) {
			 fprintf(stderr,"not in unhd format!!\n");
			 exit(1);
		    }
		    
		    /* we know we see a hex digit */
		    byte = unhex(c);
		    c = getc(in);
		    if (c != EOF && isxdigit(c)) {
			 byte = byte*16+unhex(c);
			 c = getc(in);
		    }
		    putchar(byte);
		    
		    while (c != EOF && isspace(c)) c = getc(in);
	       }
	       /* eat to the end of the line */
	       while (c != EOF && c != '\n') c = getc(in);
	       if (c != EOF) c = getc(in);
	  }
     }
}

int main(int argc, char *argv[])
{
    int i;
    FILE *file;

    if (argc == 1) undump(stdin);
    else {
	for (i=1;i<argc;i++) {
	    file = fopen(argv[i], "r");
	    if (file) undump(file);
	    fclose(file);
	}
    }
    return 0;
}


#ifndef _map_file_h
#define _map_file_h
#include <stdio.h>

/* given a file name return a pointer to memory which contains a
   read-only copy of the contents of the file.  this is done with
   virtual memory tricks, not by reading the file */
char *map_file(FILE *f, int len, int off);

/* return the size of a file in bytes */
int file_size(FILE *f);
#endif /* _map_file_h */

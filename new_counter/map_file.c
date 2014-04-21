#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include "map_file.h"

int file_size(FILE *f)
{
     struct stat buf;

     if (fstat(fileno(f), &buf) == -1) {
	 return -1;
     }
     else {
	 if (!S_ISREG(buf.st_mode)) return -1;
	 else return buf.st_size;
     }
}

/* code which when given a file name will return a pointer to memory
   which contains a read-only copy of the contents of the file.  this
   is done with virtual memory tricks, not by reading the file */

char *map_file(FILE *f, int len, int off)
{
    char *r;

    if (!f) {
	fprintf(stderr, "map_file: cannot map null file");
	return NULL;
    }

    if (len == 0) {
	len = file_size(f);
    }

    r = mmap(NULL, len, PROT_READ, MAP_SHARED, fileno(f), off);
    if (!r) {
	fprintf(stderr, "map_file: cannot map file\n");
	perror("map_file");
    }

    fclose(f);

    return r;
}


    

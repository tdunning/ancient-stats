/* error reporting and various error checked system calls */

void make_progname( /* char *name */ );
void *ecalloc( /* int n,size */ );
void *emalloc( /* int n */ );

FILE *efopen( /* char *name, *mode */ );

void fatal( /* char *where, *what */ );
void warning( /* char *where, *what */ );

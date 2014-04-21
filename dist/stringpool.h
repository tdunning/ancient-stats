#ifndef STRINGPOOL_H
#define STRINGPOOL_H
typedef struct s_pool_marker {
    int generation;
    char *mark;
} pool_marker;

void allocate_pool();

void addch( /* int ch */ );
		
void new_word();
char *latest_word();

char *get_word( /* FILE *f */ );
char *get_line( /* FILE *f */ );

void free_last_word();

pool_marker mark_pool();

void release_to_mark( /* pool_marker m */ );
#endif

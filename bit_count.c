/*

  copyright 1994 ted dunning

  unlimited noncommercial use, copying and redistribution allowed.

  commercial use, copying and redistribution allowed by written
  permission.  contact ted@nmsu.edu for more information.

  */
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "bit_count.h"

void bit_set(int *tbl, int l, unsigned int bit)
{
    int b;
    b = bit%32;
    bit = bit/32;
    tbl[bit%l] |= 1<<b;
}

void bit_clear(int *tbl, int l, unsigned int bit)
{
    int b;
    b = bit%32;
    bit = bit/32;
    tbl[bit%l] &= ~(1<<b);
}

int bit_test(int *tbl, int l, unsigned int bit)
{
    int b;
    b = bit%32;
    bit = bit/32;
    return !!(tbl[bit%l] & (1<<b));
}

/* quickly count the number of one bits in a 32 bit word */
inline int bit_count(int n)
{
    /* this procedure does lots of additions in parallel to speed up the
       process of counting the bits in a word */
    n = (n&0x55555555) + ((n&0xaaaaaaaa)>>1); /* lots of single bit adds */
    n = (n&0x33333333) + ((n&0xcccccccc)>>2); /* lots of two bit adds */
    n = (n&0x0f0f0f0f) + ((n&0xf0f0f0f0)>>4); /* and four bit adds */
    n = (n&0x00ff00ff) + ((n&0xff00ff00)>>8); /* then 8 bit adds */
    return (n&0x0000ffff) + ((n&0xffff0000)>>16); /* finally two 16 bit adds */
}

/* count the number of one bits in a vector made up of 32 bit words */
int bitv_count(int *vec, int n)
{
    int r;

    r = 0;
    while (n--) {
	if (*vec) r += bit_count(*vec++);
    }
    return r;
}

#ifdef TEST

/* reliably count the bits in a word for comparison purposes */
int xbit_count(int n)
{
    int i, sum;

    sum = 0;
    for (i=1;i;i<<=1) {
	sum += (n&i)!=0;
    }
    return sum;
}

int flagged=1;

void stopper(int sig)
{
    flagged = 0;
}
    

int main()
{
    int i, n, errors;

    signal(SIGQUIT, stopper);
    signal(SIGALRM, stopper);
    alarm(20);
    errors = 0;
    srandom(time(0));
    for (i=0;flagged && errors < 10 && i<1000000;i++) {
	n = random();
	if (bit_count(n) != xbit_count(n)) {
	    errors++;
	}
    }
    printf("%d test cases run, %d errors found\n", i, errors);
    return errors!=0;
}
	    
	
#endif

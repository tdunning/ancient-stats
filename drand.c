#include <math.h>
#include "drand.h"

/* clone of the drand48 procedures */

/*
     X sub (n+1) = (aX sub n + c) sub (mod m)       n>=0.

The parameter m=2**48; hence 48-bit  integer  arithmetic  is
performed.   Unless  lcong48()  has been invoked, the multi-
plier value a and the addend value c are given by

      a = 5DEECE66D base 16 = 273673163155 base 8
      c = B base 16 = 13 base 8.

      */

static unsigned short global_seed[7] = {
    0x330e, 0xabcd, 0x1234, 0xe66d, 0xdeec, 0x5, 0xb
};

static void mutate_seed(unsigned short x[], unsigned short *y)
{
    unsigned short s[3];
    unsigned long  t;

    t = x[0] * y[0] + y[3];
    s[0] = t & 0xffff;
    t = x[1] * y[0] + x[0] * y[1] + (t>>16);
    s[1] = t & 0xffff;
    t = x[1] * y[1] + x[0] * y[2] + x[2] * y[0] + (t>>16);
    s[2] = t & 0xffff;

    x[0] = s[0];
    x[1] = s[1];
    x[2] = s[2];
}


double x_drand48(void)
{
    return x_erand48(global_seed);
}

double x_erand48(unsigned short xsubi[3])
{
    mutate_seed(xsubi, global_seed+3);
    return scalbn((double) xsubi[0], -16) +
	scalbn((double) xsubi[1], -32) +
	scalbn((double) xsubi[2], -48);
}

long x_nrand48(unsigned short xsubi[])
{
    unsigned long r;
    mutate_seed(xsubi, global_seed+3);
    r = xsubi[2];
    r = (r<<16) + xsubi[1];
    r &= 0x7fffffff;
    return r;
}

long x_lrand48(void)
{
    return x_nrand48(global_seed);
}

long x_jrand48(unsigned short xsubi[3])
{
    unsigned long r;
    mutate_seed(xsubi, global_seed+3);
    r = xsubi[2];
    r = r<<16;
    r += xsubi[1];
    if (r == 0x80000000) r = 0;
    return r;
}

long x_mrand48(void)
{
    return x_jrand48(global_seed);
}
    
void x_srand48(long seedval)
{
    global_seed[0] = seedval;
    global_seed[1] = seedval>>16;
    global_seed[2] = 0x330e;
}

unsigned short *x_seed48(unsigned short seed16v[3])
{
    static unsigned short old_seed[3];
    old_seed[0] = global_seed[0];
    old_seed[1] = global_seed[1];
    old_seed[2] = global_seed[2];
    global_seed[0] = seed16v[0];
    global_seed[1] = seed16v[1];
    global_seed[2] = seed16v[2];
    return old_seed;
}


void x_lcong48(unsigned short param[7])
{
    global_seed[0] = param[0];
    global_seed[1] = param[1];
    global_seed[2] = param[2];
    global_seed[3] = param[3];
    global_seed[4] = param[4];
    global_seed[5] = param[5];
    global_seed[6] = param[6];
}

#ifdef TEST

int main(int argc, char *argv[])
{
    unsigned short *x, *seed48();
    long lrand48();
    x = seed48(global_seed);
    printf("%x %x %x\n", x[0], x[1], x[2]);

    x = x_seed48(global_seed);
    printf("%x %x %x\n", x[0], x[1], x[2]);

    printf("%lx %lx\n", lrand48(), x_lrand48());
    return 0;
}
#endif

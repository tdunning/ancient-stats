#include <math.h>

#include "compand.h"

static int initialized = 0;

static float compand_table[256];

static unsigned char char_mul[256*256];
static unsigned char char_add[256*256];

static float fl_mul[256*256];

void init_compand(void)
{
    int i, j;
    double x, y;
    if (!initialized) {
	initialized = 1;
	for (x=1;x>=0;x-=0.0005) {
	    y = erf(x * sqrt(280));
	    i = (y+1)/2 * 255.0 + 0.5;
	    compand_table[i] = x;
	    compand_table[255-i] = -x;
	}
	for (i=0;i<256;i++) {
	    for (j=0;j<256;j++) {
		x = compand_table[i] * compand_table[j];
		fl_mul[i*256 + j] = x;
		char_mul[i*256 + j] = compress_element(x);

		x = compand_table[i] + compand_table[j];
		char_add[i*256 + j] = compress_element(x);
	    }
	}
    }
}

int compress_element(float x)
{
    int i, diff;

    i = 128;
    diff = 64;
    while (diff > 0) {
	if (compand_table[i] < x) {
	    i += diff;
	}
	else if (compand_table[i] > x) {
	    i -= diff;
	}
	else if (compand_table[i] == x) {
	    return i;
	}
	diff /= 2;
    }
    if (i && compand_table[i] > x) {
	i--;
    }
    return i;
}

void compress_vector(unsigned char *r, float *x, int n)
{
    int i;
    init_compand();
    for (i=0;i<n;i++) {
	r[i] = compress_element(x[i]);
    }
}
    
float expand_element(int x)
{
    if (x < 0) return compand_table[0];
    else if (x > 255) return compand_table[255];
    else return compand_table[x];
}

void expand_vector(float *r, unsigned char *x, int n)
{
    int i;
    init_compand();
    for (i=0;i<n;i++) {
	r[i] = expand_element(x[i]);
    }
}

int mul8(int x, int y)
{
    if (x < 0) x = 0;
    if (x > 255) x = 255;
    if (y < 0) y = 0;
    if (y > 255) y = 255;

    return char_mul[x*256 + y];
}

int add8(int x, int y)
{
    if (x < 0) x = 0;
    if (x > 255) x = 255;
    if (y < 0) y = 0;
    if (y > 255) y = 255;

    return char_add[x*256 + y];
}

float dot8(unsigned char *x, unsigned char *y, int n)
{
    int i, sum;

    init_compand();
    sum = compress_element(0.0);
    for (i=0;i<n;i++) {
	sum = add8(sum, mul8(x[i], y[i]));
    }
    return expand_element(sum);
}


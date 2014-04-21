/*
 * send this puppy a buncha numbers, it sorts 
 * and normalizes the distribution so that plotting is simpler
 *
 * originally written by owen white.  simplified by ted dunning.
 * 
 * This code is in the public domain.
 */

#include <stdio.h>
#include <math.h>
#include <malloc.h>

int dbl_compare(double *a, double *b)
{
    if (*a > *b) return 1;
    else if (*a < *b) return -1;
    else return 0;
}

void sort(double ra[], int n)
{
    void qsort(void *, int, int, int(*)());
    qsort(ra, n, sizeof(ra[0]), dbl_compare);
}

double *read_file(long *array_length)
{
    double *x;
    long i, n, size;

    size = 10;
    x = (double *) calloc(size,sizeof(x[0]));
    i = 0;

    n = fscanf(stdin,"%lf", &x[0]);
    while(n == 1) {
	i++;
	if(i >= size) {
            size = 2*size+10;
	    x = (double *) realloc(x, sizeof(double) * size);
	}
	n = fscanf(stdin,"%lf", &x[i]);
    }
    *array_length = i;
    return x;
}

int main(int argc, char **argv)
{
    double *array;
    long i, array_length;

    array = read_file(&array_length);

    sort(array, array_length);

    for(i=0;i<array_length;i++) {
	printf("%f %f\n", array[i], (double) i/(array_length-1));
    }
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <aptex/dvector.h>

#include "sc_vector.h"

#include "compand.h"

/*

  8 bit companded 
	by vector
	by matrix

 */

#include <sys/time.h>
#include <sys/resource.h>

double millitime()
{
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
    return rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec/1000000.0;
}

double gauss_dev(double m, double s)
{
    int i;
    double sum;

    sum = 0;
    for (i=0;i<12;i++) {
	sum += drand48();
    }
    return (sum-6);
}

int main(int argc, char *argv[])
{
    int i, j;
    unsigned char *z;
    unsigned char r[100*1000];

    double drand48();

    unsigned char **threaded_topics, **threaded_docs;

    float t0, t1;
    float tmp[280];

    init_compand();

    threaded_docs = calloc(100, sizeof(threaded_docs[0]));
    for (i=0;i<100;i++) {
	threaded_docs[i] = calloc(280, sizeof(threaded_docs[0][0]));
	for (j=0;j<280;j++) {
	    tmp[j] = gauss_dev(0, 1);
	}
	scv_normalize(tmp);
	compress_vector(threaded_docs[i], tmp, 280);
	j = drand48() * (i+1);
	z = threaded_docs[i];
	threaded_docs[i] = threaded_docs[j];
	threaded_docs[j] = z;
    }

    threaded_topics = calloc(1000, sizeof(threaded_topics[0]));
    for (i=0;i<1000;i++) {
	threaded_topics[i] = calloc(280, sizeof(threaded_topics[0][0]));
	for (j=0;j<280;j++) {
	    tmp[j] = gauss_dev(0, 1);
	}
	scv_normalize(tmp);
	compress_vector(threaded_topics[i], tmp, 280);
	j = drand48() * (i+1);
	z = threaded_topics[i];
	threaded_topics[i] = threaded_topics[j];
	threaded_topics[j] = z;
    }

    /* convectis style threaded matrix 
	by vector
	by matrix
	*/
    t0 = millitime();
    for (i=0;i<100;i++) {
	for (j=0;j<1000;j++) {
	    r[i*1000+j] = dot8(threaded_topics[j], threaded_docs[i], 280);
	}
    }
    t1 = millitime();
    printf("%.3f seconds for %d convectis style categorizations\n",
	   t1-t0, 100);

    t0 = millitime();
    for (j=0;j<1000;j++) {
	for (i=0;i<100;i++) {
	    r[i*1000+j] = dot8(threaded_topics[j], threaded_docs[i], 280);
	}
    }
    t1 = millitime();
    printf("%.3f seconds for %d convectis matrix categorizations\n",
	   t1-t0, 100);

    return 0;
}

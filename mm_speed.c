#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mmult.h"
#include "sc_vector.h"

#include "drand.h"

/*

  convectis style threaded matrix 
	by vector
	by matrix

  convectis style block multiply
	by vector
	by matrix
		
  contiguous arrays
	by vector
	by matrix

  contiguous arrays block multiply
	by vector
	by matrix

  record:

      data load time
      first multiply time
      second through nth multiply time

 */

#include <sys/time.h>
#include <sys/resource.h>

double millitime()
{
#ifndef NTGCC
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
    return rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec/1000000.0;
#else
    return time(0);
#endif
}

int main(int argc, char *argv[])
{
    int i, j;
    float *z;
    float *cont_topics;
    float *cont_docs;
    float r[100*1000];

    double drand48();

    float **threaded_topics, **threaded_docs;

    float t0, t1;

    threaded_docs = calloc(100, sizeof(threaded_docs[0]));
    for (i=0;i<100;i++) {
	threaded_docs[i] = calloc(280, sizeof(threaded_docs[0][0]));
	for (j=0;j<280;j++) {
	    threaded_docs[i][j] = 2*drand48()-1;
	}
	j = drand48() * (i+1);
	z = threaded_docs[i];
	threaded_docs[i] = threaded_docs[j];
	threaded_docs[j] = z;
    }

    threaded_topics = calloc(1000, sizeof(threaded_topics[0]));
    for (i=0;i<1000;i++) {
	threaded_topics[i] = calloc(280, sizeof(threaded_topics[0][0]));
	for (j=0;j<280;j++) {
	    threaded_topics[i][j] = 2*drand48()-1;
	}
	j = drand48() * (i+1);
	z = threaded_topics[i];
	threaded_topics[i] = threaded_topics[j];
	threaded_topics[j] = z;
    }

    cont_docs = calloc(100*280, sizeof(cont_docs[0]));
    cont_topics = calloc(1000*280, sizeof(cont_topics[0]));

    for (i=0;i<100;i++) {
	for (j=0;j<280;j++) {
	    cont_docs[i*280 + j] = threaded_docs[i][j];
	}
    }

    for (i=0;i<1000;i++) {
	for (j=0;j<280;j++) {
	    cont_topics[i*280 + j] = threaded_topics[i][j];
	}
    }

    /* convectis style threaded matrix 
	by vector
	by matrix
	*/
    t0 = millitime();
    printf("a\n");
    for (i=0;i<100;i++) {
	for (j=0;j<1000;j++) {
	    r[i*1000+j] = scv_dot(threaded_topics[j], threaded_docs[i]);
	}
    }
    t1 = millitime();
    printf("b\n");
    printf("%.3f seconds for %d convectis style categorizations\n",
	   t1-t0, 100);

    t0 = millitime();
    printf("c\n");
    scv_matrix_by_matrix(r, threaded_topics, threaded_docs, 1000, 100);
    t1 = millitime();
    printf("%.3f seconds for %d matrix categorizations\n",
	   t1-t0, 100);

    t0 = millitime();
    printf("d\n");
    scv_matrix_by_matrix(r, threaded_docs, threaded_topics, 100, 1000);
    t1 = millitime();
    printf("e\n");
    printf("%.3f seconds for %d matrix categorizations\n",
	   t1-t0, 100);

    t0 = millitime();
    printf("f\n");
    for (j=0;j<1000;j++) {
	for (i=0;i<100;i++) {
	    r[i*1000+j] = scv_dot(threaded_topics[j], threaded_docs[i]);
	}
    }
    t1 = millitime();
    printf("g\n");
    printf("%.3f seconds for %d convectis matrix categorizations\n",
	   t1-t0, 100);

    /* convectis style block multiply
	by vector
	by matrix
	*/
    /* not done */
		
    /* contiguous arrays
	by vector
	by matrix
	*/
    t0 = millitime();
    printf("g\n");
    for (i=0;i<100;i++) {
	for (j=0;j<1000;j++) {
	    r[i*1000+j] = scv_dot(cont_topics+j*280, cont_docs+i*280);
	}
    }
    t1 = millitime();
    printf("h\n");
    printf("%.3f seconds for %d contiguous array by vector categorizations\n",
	   t1-t0, 100);

    t0 = millitime();
    for (j=0;j<1000;j++) {
	for (i=0;i<100;i++) {
	    r[i*1000+j] = scv_dot(cont_topics+j*280, cont_docs+i*280);
	}
    }
    t1 = millitime();
    printf("%.3f seconds for %d contiguous array by matrix categorizations\n",
	   t1-t0, 100);

    /* contiguous arrays block multiply
	by vector
	by matrix
	*/
    t0 = millitime();
    MMT2x2PB(r, cont_topics, cont_docs, 1000, 100, 280, 280, 280, 20);
    t1 = millitime();
    printf("%.3f seconds for %d contiguous array blocked categorizations\n",
	   t1-t0, 100);

    
    return 0;
}

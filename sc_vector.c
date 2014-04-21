/*********************************************************************
 * Copyright (c) 1996 by Aptex Software Inc. (an HNC Software Company)
 * 9605 Scranton Road Suite 240, San Diego, CA
 * All Rights Reserved.
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF APTEX INC
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the
 * government is subject to restrictions as set forth in subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and FAR 52.227-19.
 *
 *      Product:
 *          SelectCast - Vector utilities
 *
 *      Module:
 *	    sc_vector.c
 *
 *      Created:
 *	    9/11/96
 *
 *      Authors		Initials	Involvement
 *      -------		--------	-----------
 *      Brad Kindig	bdk		Creator
 *	Kevin L. Sitze	kls		Port to MPVDB
 *
 *	Synopsis:
 *
 *********************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <math.h>

#include "sc_vector.h"

/*
 *  Macros used for unrolling loops
 */

#ifndef UNROLL
#define UNROLL 16
#endif

#define UNROLL_BODY_01(body) body
#define UNROLL_BODY_02(body) UNROLL_BODY_01(body); UNROLL_BODY_01(body)
#define UNROLL_BODY_04(body) UNROLL_BODY_02(body); UNROLL_BODY_02(body)
#define UNROLL_BODY_08(body) UNROLL_BODY_04(body); UNROLL_BODY_04(body)
#define UNROLL_BODY_16(body) UNROLL_BODY_08(body); UNROLL_BODY_08(body)
#define UNROLL_BODY_32(body) UNROLL_BODY_16(body); UNROLL_BODY_16(body)

/*
 *  The FINISH_LOOP macros below assume that NUM_FEATURES == 280
 */

#if NUM_FEATURES != 280
#   error Need to redefine UNROLL routines.
#endif

#if UNROLL == 2
#   define UNROLL_BODY(body) UNROLL_BODY_02(body)
#   define FINISH_LOOP(body)
#endif

#if UNROLL == 4
#   define UNROLL_BODY(body) UNROLL_BODY_04(body)
#   define FINISH_LOOP(body)
#endif

#if UNROLL == 8
#   define UNROLL_BODY(body) UNROLL_BODY_08(body)
#   define FINISH_LOOP(body)
#endif

#if UNROLL == 16
#   define UNROLL_BODY(body) UNROLL_BODY_16(body)
#   define FINISH_LOOP(body) UNROLL_BODY_08(body)
#endif

#if UNROLL == 32
#   define UNROLL_BODY(body) UNROLL_BODY_32(body)
#   define FINISH_LOOP(body) UNROLL_BODY_16(body); UNROLL_BODY_08(body)
#endif

/*
 * rvs claims that factor 2 unrolling would be essentially as good
 */

sc_vector *
sc_vector_alloc(void)
{
    return (sc_vector*) calloc(1, sizeof(sc_vector));
} /* sc_vector_alloc() */

Scvp
scv_alloc(void)
{
    return (Scvp) calloc(NUM_FEATURES, sizeof(float));
} /* scv_alloc() */

sc_vector *
sc_vector_dup(sc_vector *src)
{
    sc_vector *r = sc_vector_alloc();
    memcpy(r, src, sizeof(sc_vector));
    return r;
} /* sc_vector_dup() */

Scvp
scv_dup(Scvp src)
{
    Scvp dest = scv_alloc();
    scv_copy(dest, src);
    return dest;
} /* scv_dup() */

void
sc_vector_copy(sc_vector* dest, sc_vector* src)
{
    memcpy(dest, src, sizeof(sc_vector));
} /* sc_vector_copy() */

void
scv_copy(Scvp dest, Scvp src)
{
    memcpy(dest, src, SCV_SIZE);
} /* scv_copy() */

float
scv_dot(Scvp a, Scvp b)
{
    float sum;
    int i;

    sum = 0.0;
    for (i = 0; i < NUM_FEATURES / UNROLL; ++i) {
	UNROLL_BODY(sum += (*a++) * (*b++));
    }
    FINISH_LOOP(sum += (*a++) * (*b++));
    return sum;
} /* scv_dot() */

float
scv_dot1(Scvp a, Scvp b)
{
    float sum;
    int i;

    sum = 0.0;
    for (i = 0; i < NUM_FEATURES / UNROLL; ++i) {
	UNROLL_BODY(sum += (*a++) * (*b++));
    }
    FINISH_LOOP(sum += (*a++) * (*b++));
    return sum;
} /* scv_dot() */

float
scv_dot2(Scvp a, Scvp b)
{
    register float *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7;
    register float *b0, *b1, *b2, *b3, *b4, *b5, *b6, *b7;
    float sum0, sum1, sum2, sum3, sum4, sum5, sum6, sum7;
    int i;

    sum0 = sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = 0;
    a0 = a+0; a1 = a+1; a2 = a+2; a3 = a+3;
    a4 = a+4; a5 = a+5; a6 = a+6; a7 = a+7;
    b0 = b+0; b1 = b+1; b2 = b+2; b3 = b+3;
    b4 = b+4; b5 = b+5; b6 = b+6; b7 = b+7;

    for (i=0;i<NUM_FEATURES/8;i++) {
	sum0 += (*a0) * (*b0);
	a0 += 8; b0 += 8;
	sum1 += (*a1) * (*b1);
	a1 += 8; b1 += 8;
	sum2 += (*a2) * (*b2);
	a2 += 8; b2 += 8;
	sum3 += (*a3) * (*b3);
	a3 += 8; b3 += 8;
	sum4 += (*a4) * (*b4);
	a4 += 8; b4 += 8;
	sum5 += (*a5) * (*b5);
	a5 += 8; b5 += 8;
	sum6 += (*a6) * (*b6);
	a6 += 8; b6 += 8;
	sum7 += (*a7) * (*b7);
	a7 += 8; b7 += 8;
    }
    return sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7;
} /* scv_dot() */

void scv_matrix_by_vector1(Scvp r, Scvp *A, int nvec, Scvp x)
{
    int i;

    for (i=0;i<nvec;i++) {
	r[i] = scv_dot1(A[i], x);
    }
}

void scv_matrix_by_vector2(Scvp r, Scvp *A, int nvec, Scvp x)
{
    int i;

    for (i=0;i<nvec;i++) {
	r[i] = scv_dot2(A[i], x);
    }
}

/* this formulation wins somewhat by improving the memory access
   patterns, but isn't as good as a fully blocked and unrolled
   matrix code. */
void scv_matrix_by_matrix(Scvp r, Scvp *A, Scvp *x, int n1, int n2)
{
    int i0, i1, j0, j1, k0;

    for (i0=0;i0<n1;i0+=10) {
	for (j0=0;j0<n2;j0+=10) {
	    for (i1=0;i1<10;i1++) {
		for (j1=0;j1<10;j1++) {
		    float s, *pa, *pb;
		    pa = A[i0+i1];
		    pb = x[j0+j1];
		    s = 0;
		    for (k0=0;k0<NUM_FEATURES/UNROLL;k0++) {
			UNROLL_BODY(s += (*pa++) * (*pb++));
		    }
		    r[(i0+i1)*n2+(j0+j1)] = s;
		}
	    }
	}
    }
}


/* performs the operation `a * X + Y' where X and Y are vectors and a
 * is a scalar.
 *
 * essentially the same as the linpak routine of the same name.
 * some special cases are handled differently for speed.
 *
 * NULL can be used to indicate a zero vector.
 */

void
scv_axpy(Scvp r, double da, Scvp x, Scvp y)
{
    float a;			/* this is a float for speed. */
    int i;

    a = da;			/* but da is a double for convenience */
    if (a == 1.0) {
	if (x && y) {
	    for (i = 0; i < NUM_FEATURES / UNROLL; ++i) {
		UNROLL_BODY(*r++ = (*x++) + (*y++));
	    }
	    FINISH_LOOP(*r++ = (*x++) + (*y++));
	} else {
	    /* if x is NULL then y will either be defined or NULL */
	    scv_axpy(r, 0.0, NULL, x ? x : y);
	}
    } else if (a == 0.0) {
	if (y) {
	    memcpy(r, y, NUM_FEATURES * sizeof(float));
	} else {
#ifdef IEEE_FLOAT
	    memset(r, 0, NUM_FEATURES * sizeof(float));
#else
	    for (i = 0; i < NUM_FEATURES; ++i) {
		r[i] = 0.0;
	    }
#endif
	}
    } else {
	if (x && y) {
	    for (i = 0; i < NUM_FEATURES / UNROLL; ++i) {
		UNROLL_BODY(*r++ = a * (*x++) + (*y++));
	    }
	    FINISH_LOOP(*r++ = a * (*x++) + (*y++));
	} else if (x && !y) {
	    for (i = 0; i < NUM_FEATURES / UNROLL; ++i) {
		UNROLL_BODY(*r++ = a * (*x++));
	    }
	    FINISH_LOOP(*r++ = a * (*x++));
	} else {
	    scv_axpy(r, 0.0, NULL, y);
	}
    }
} /* scv_axpy() */

void
sc_vector_zero(sc_vector *v)
{
    memset((void*) v, 0, sizeof(sc_vector));
} /* sc_vector_zero() */

void
scv_zero(Scvp v)
{
    memset((void*) v, 0, SCV_SIZE);
} /* scv_zero() */

void
scv_normalize(Scvp v)
{
    float x;

    x = scv_dot(v, v);
    if (x > 1e-9) {
	scv_axpy(v, 1.0 / sqrt(x), v, NULL);
    }
} /* scv_normalize() */

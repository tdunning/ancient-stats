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
 *
 *	Synopsis:
 *
 *********************************************************************/

#ifndef sc_vector_h
#define sc_vector_h 1

#ifndef NUM_FEATURES
#define NUM_FEATURES	280
#endif

/*
 * This allows a sc_vector to be declared statically or in
 * a structure.  Someday we may wish to add members.  (i.e.
 * a flag to indicate normalized or not
 */
typedef struct {
    float f[NUM_FEATURES];
} sc_vector;

/*
 * In practice, we typically operate on the vectors themselves
 * which are passed as pointers to the floating point elements.
 * A Scvp is really a pointer to float[NUM_FEATURES].
 */
typedef float *Scvp;	/* select cast vector pointer */

#define SCV_SIZE (NUM_FEATURES * sizeof(float))
#define SCVP(scv) ((scv)->f)

extern sc_vector *sc_vector_alloc(void);
extern Scvp scv_alloc(void);

extern sc_vector *sc_vector_dup(sc_vector *src);
extern Scvp scv_dup(Scvp src);

extern void sc_vector_copy(sc_vector *dest, sc_vector *src);
extern void scv_copy(Scvp dest, Scvp src);

#define sv_vector_zero(scv) scv_zero(SCVP(scv))
extern void scv_zero(Scvp v);

extern void scv_axpy(Scvp result, double da, Scvp x, Scvp y);

extern void scv_normalize(Scvp v);
#define sc_vector_normalize(scv) scv_normalize(SCVP(scv))

extern float scv_dot(Scvp v1, Scvp v2);
extern float scv_dot2(Scvp v1, Scvp v2);
#define sc_vector_dot(v1, v2) scv_dot(SCVP(v1), SCVP(v2))
#define sc_vector_dot2(v1, v2) scv_dot(SCVP(v1), SCVP(v2))

void scv_matrix_by_vector1(Scvp r, Scvp *A, int nvec, Scvp x);
void scv_matrix_by_vector2(Scvp r, Scvp *A, int nvec, Scvp x);

#endif

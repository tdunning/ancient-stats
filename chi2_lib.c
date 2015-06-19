/*

Original version by Rebecca Bruce.

Copyright 1993, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: chi2_lib.c,v $
 * Revision 1.2  1996/05/16  21:49:48  ted
 * cleanup to avoid compiler warnings
 *
 * Revision 1.1  1996/05/02  15:46:59  ted
 * Initial revision
 *
 */
#include <stdio.h>
#include <math.h>

#ifdef WIN32
#define M_PI 3.1415926535897931
#endif

/**
 * Normal distribution PDF
 */
static double Z(double chi)
{
    return exp(-1.0*chi*chi/2.0)/sqrt(2.0*M_PI);;
}

/**
 * Cumulative normal distribution.  Uses approximation from
 * Zelen & Severo (1964).  Only valid for chi > 0.
 *
 * See
 * https://en.wikipedia.org/wiki/Normal_distribution#Numerical_approximations_for_the_normal_CDF
 */
static double Q(double chi)
{
    double t, b[6];
    double total;
    double x;
    int i;

    b[0] = 0.2316419;
    b[1] =  0.319381530;
    b[2] = -0.356563782;
    b[3] =  1.781477937;
    b[4] = -1.821255978;
    b[5] =  1.330274429;

    t = 1.0/(1.0+(b[0]*fabs(chi)));
    x = 1;
    total = 0;
    for (i=1; i<=5; i++) {
        x *= t;
        total += b[i] * x;
    }
    if (chi < 0) {
      return total*Z(chi);
    }
    else {
      return 1 - total*Z(chi);
    }
}

static double sum(int dof, double chi)
{
    int limit, r;
    double sumvalue, element;
    
    if (dof % 2 ==0) {
        limit = (dof-2)/2;
        sumvalue = 0;
        element = 1;
        for (r=1;r<=limit;r++) {
            element = element * chi * chi / (2*r);
            sumvalue += element;
        }
    }
    else {
        limit=(dof-1)/2;
        sumvalue = 0;
        element = 1/chi;
        for (r=1;r<=limit;r++) {
            element *= chi*chi/(2*r-1);
            sumvalue += element;
        }
    }

    return sumvalue;
}
            
double significance(double chi2, int dof)
{               
    double signif;

    if (dof>100) {
        // Fisher's approximation
        signif = 1 - Q(sqrt(2.0*chi2) - sqrt(2.0*dof-1.0));
    }
    else if (dof>30) {
        // Wilson–Hilferty transformation
        signif = 1 - Q((pow(chi2/dof,1.0/3.0) - (1.0-2.0/(9*dof))) / sqrt(2.0/(9*dof)));
    }
    else if (dof%2==0) {
        signif = sqrt(2*M_PI)*Z(sqrt(chi2))*(1.0+sum(dof,sqrt(chi2)));
    }
    else {
        signif = 2*Z(sqrt(chi2))*sum(dof,sqrt(chi2)) + 2*Q(sqrt(chi2));
    }
    return signif;
}

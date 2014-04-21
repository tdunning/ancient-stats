/****************************************************************************/
/*                             MM.C                                         */
/****************************************************************************/

#include "mmult.h"

/****************************************************************************/
/*                           MM1x1P                                         */
/****************************************************************************/

void MM1x1P(REAL* c, REAL* a, REAL* b,
	    int Ni,int Nj,int Nk, int NaOffs, int NbOffs)
{
    int i,j,k;
    REAL s00;
    REAL *pa,*pb,*pc;

    for (i=0; i<Ni; i++) {
	for (j=0; j<Nj; j++) {
	    pc = c+j+Nj*i;
	    s00 = 0.0;
	    pb = b + j;
	    pa = a + NaOffs*i;
	    for (k=0; k<Nk; k++) {
		    s00 += (*pa)*(*pb);
		    pa++;
		    pb += NbOffs;
	    }
	    *pc = s00;
	}
    }
}

/****************************************************************************/
/*                           MM2x2P                                         */
/****************************************************************************/

void MM2x2P(REAL* c, REAL* a, REAL* b,
	    int Ni, int Nj, int Nk, int NaOffs, int NbOffs)
{
    int i,j,k;
    REAL s00,s01,s10,s11;
    REAL *pa0,*pa1,*pb0,*pb1,*pc0,*pc1;

    for (i=0; i<=Ni-2; i+=2) {
	for (j=0; j<=Nj-2; j+=2) {
	    pc0 = c+j+Nj*i;
	    pc1 = c+j+Nj*(i+1);
	    s00 = 0.0; s01 = 0.0; s10 = 0.0; s11 = 0.0;

	    pb0=b+j;
	    pb1=b+j*NbOffs+1;
	    pa0=a+NaOffs*i;
	    pa1=a+NaOffs*(i+1);
	    for (k=0; k<Nk; k++) {
		s00 += (*pa0)*(*pb0);
		s01 += (*pa0)*(*pb1);
		s10 += (*pa1)*(*pb0);
		s11 += (*pa1)*(*pb1);
		pa0++;
		pa1++;
		pb0+=NbOffs;
		pb1+=NbOffs;
	    }
	    *pc0 = s00; *(pc0+1) = s01; *pc1 = s10; *(pc1+1) = s11;
	}
	for (j=j; j<Nj; j++) {
	    pc0 = c+j+Nj*i;
	    pc1 = c+j+Nj*(i+1);
	    s00 = 0.0; s10 = 0.0;

	    pb0=b+j;
	    pa0=a+NaOffs*i;
	    pa1=a+NaOffs*(i+1);
	    for (k=0; k<Nk; k++) {
		s00 += (*pa0)*(*pb0);
		s10 += (*pa1)*(*pb0);
		pa0++;
		pa1++;
		pb0+=NbOffs;
	    }
	    *pc0 = s00; *pc1 = s10;
	}
    }
    for (i=i; i<Ni; i++) {
	for (j=0; j<=Nj-2; j+=2) {
	    pc0 = c+j+Nj*i;
	    s00 = 0.0; s01 = 0.0;

	    pb0=b+j;
	    pb1=b+j+1;
	    pa0=a+NaOffs*i;

	    for (k=0; k<Nk; k++) {
		s00 += (*pa0)*(*pb0);
		s01 += (*pa0)*(*pb1);
		pa0++;
		pb0+=NbOffs;
		pb1+=NbOffs;
	    }
	    *pc0 = s00; *(pc0+1) = s01;
	}
	for (j=j; j<Nj; j++) {
	    pc0 = c+j+Nj*i;
	    s00 = 0.0;

	    pb0=b+j;
	    pa0=a+NaOffs*i;
	    for (k=0; k<Nk; k++) {
		s00 += (*pa0)*(*pb0);
		pa0++;
		pb0+=NbOffs;
	    }
	    *pc0 = s00;
	}
    }
}



/****************************************************************************/
/*                           MM2x2PB                                        */
/****************************************************************************/

void
MM2x2PB (REAL * c, REAL * a, REAL * b,
	 int Ni, int Nj, int Nk, int NaOffs, int NbOffs, int NB)
{
    int i, j, k, ii, jj, kk;
    REAL s00, s01, s10, s11;
    REAL *pa0, *pa1, *pb0, *pb1, *pc0, *pc1;

    for (ii = 0; ii <= Ni - NB; ii += NB) {
	for (jj = 0; jj <= Nj - NB; jj += NB) {

	    for (i = ii; i < ii + NB; i++) {
		for (j = jj; j < jj + NB; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk not in block */
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
	    }
	}

	for (jj = jj; jj < Nj; jj += NB) {	/* Part of jj not in block */

	    for (i = ii; i < ii + NB; i++) {
		for (j = jj; j < Nj; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }

		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii) not in block */
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
	    }
	}
    }

    for (ii = ii; ii < Ni; ii += NB) {	/* Part of ii not in block */
	for (jj = 0; jj <= Nj - NB; jj += NB) {

	    for (i = ii; i < Ni; i++) {
		for (j = jj; j < jj + NB; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < Ni - 2; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i;
			     k < kk + NB;
			     k++, pa0++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii) not in block */
		for (i = ii; i < Ni - 2; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i;
			     k < Nk;
			     k++, pa0++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		}
	    }
	}

	for (jj = jj; jj < Nj; jj += NB) {	/* Part of jj(ii) not in block */

	    for (i = ii; i < Ni; i++) {
		for (j = jj; j < Nj; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i <= Ni - 2; i += 2) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i;
			     k < kk + NB;
			     k++, pa0++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pa0 = a + k + NaOffs * i;
			     k < kk + NB;
			     k++, pa0++,
			     pb0 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			}
			*pc0 = s00;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii,jj) not in block */
		for (i = ii; i <= Ni - 2; i += 2) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pb1 = b + j + k * NbOffs + 1,
			     pa0 = a + k + NaOffs * i;
			     k < Nk;
			     k++, pa0++,
			     pb0 += NbOffs,
			     pb1 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;

			for (k = kk, pb0 = b + j + k * NbOffs,
			     pa0 = a + k + NaOffs * i;
			     k < Nk;
			     k++, pa0++,
			     pb0 += NbOffs) {
			    s00 += (*pa0) * (*pb0);
			}
			*pc0 = s00;
		    }
		}
	    }
	}
    }
}

/****************************************************************************/
/*                           MMT1x1P                                        */
/****************************************************************************/

void
MMT1x1P (REAL * c, REAL * a, REAL * b,
	 int Ni, int Nj, int Nk, int NaOffs, int NbOffs)
{
    int i, j, k;
    REAL s00;
    REAL *pa, *pb, *pc;

    for (i = 0; i < Ni; i++) {
	for (j = 0; j < Nj; j++) {
	    pc = c + j + Nj * i;
	    s00 = 0.0;
	    pb = b + NbOffs * j;
	    pa = a + NaOffs * i;
	    for (k = 0; k < Nk; k++) {
		s00 += (*pa) * (*pb);
		pa++;
		pb++;
	    }
	    *pc = s00;
	}
    }
}

/****************************************************************************/
/*                           MMT2x2P                                        */
/****************************************************************************/

void 
MMT2x2P (REAL * c, REAL * a, REAL * b,
	 int Ni, int Nj, int Nk, int NaOffs, int NbOffs)
{
    int i, j, k;
    REAL s00, s01, s10, s11;
    REAL *pa0, *pa1, *pb0, *pb1, *pc0, *pc1;

    for (i = 0; i <= Ni - 2; i += 2) {
	for (j = 0; j <= Nj - 2; j += 2) {
	    pc0 = c + j + Nj * i;
	    pc1 = c + j + Nj * (i + 1);
	    s00 = 0.0;
	    s01 = 0.0;
	    s10 = 0.0;
	    s11 = 0.0;

	    for (k = 0, pb0 = b + k + NbOffs * j,
		 pb1 = b + k + NbOffs * (j + 1),
		 pa0 = a + k + NaOffs * i,
		 pa1 = a + k + NaOffs * (i + 1);
		 k < Nk;
		 k++, pa0++,
		 pa1++,
		 pb0++,
		 pb1++) {
		s00 += (*pa0) * (*pb0);
		s01 += (*pa0) * (*pb1);
		s10 += (*pa1) * (*pb0);
		s11 += (*pa1) * (*pb1);
	    }
	    *pc0 = s00;
	    *(pc0 + 1) = s01;
	    *pc1 = s10;
	    *(pc1 + 1) = s11;
	}
	for (j = j; j < Nj; j++) {
	    pc0 = c + j + Nj * i;
	    pc1 = c + j + Nj * (i + 1);
	    s00 = 0.0;
	    s10 = 0.0;

	    for (k = 0, pb0 = b + k + NbOffs * j,
		 pa0 = a + k + NaOffs * i,
		 pa1 = a + k + NaOffs * (i + 1);
		 k < Nk;
		 k++, pa0++,
		 pa1++,
		 pb0++) {
		s00 += (*pa0) * (*pb0);
		s10 += (*pa1) * (*pb0);
	    }
	    *pc0 = s00;
	    *pc1 = s10;
	}
    }
    for (i = i; i < Ni; i++) {
	for (j = 0; j <= Nj - 2; j += 2) {
	    pc0 = c + j + Nj * i;
	    s00 = 0.0;
	    s01 = 0.0;

	    for (k = 0, pb0 = b + k + NbOffs * j,
		 pb1 = b + k + NbOffs * (j + 1),
		 pa0 = a + k + NaOffs * i;
		 k < Nk;
		 k++, pa0++,
		 pb0++,
		 pb1++) {
		s00 += (*pa0) * (*pb0);
		s01 += (*pa0) * (*pb1);
	    }
	    *pc0 = s00;
	    *(pc0 + 1) = s01;
	}
	for (j = j; j < Nj; j++) {
	    pc0 = c + j + Nj * i;
	    s00 = 0.0;

	    for (k = 0, pb0 = b + k + NbOffs * j,
		 pa0 = a + k + NaOffs * i;
		 k < Nk;
		 k++, pa0++,
		 pb0++) {
		s00 += (*pa0) * (*pb0);
	    }
	    *pc0 = s00;
	}
    }
}



/****************************************************************************/
/*                           MMT2x2PB                                       */
/****************************************************************************/

void 
MMT2x2PB (REAL * c, REAL * a, REAL * b,
	  int Ni, int Nj, int Nk, int NaOffs, int NbOffs, int NB)
{
    int i, j, k, ii, jj, kk;
    REAL s00, s01, s10, s11;
    REAL *pa0, *pa1, *pb0, *pb1, *pc0, *pc1;

    for (ii = 0; ii <= Ni - NB; ii += NB) {
	for (jj = 0; jj <= Nj - NB; jj += NB) {

	    for (i = ii; i < ii + NB; i++) {
		for (j = jj; j < jj + NB; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk not in block */
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
	    }
	}

	for (jj = jj; jj < Nj; jj += NB) {	/* Part of jj not in block */

	    for (i = ii; i < ii + NB; i++) {
		for (j = jj; j < Nj; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii) not in block */
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
	    }
	}
    }

    for (ii = ii; ii < Ni; ii += NB) {	/* Part of ii not in block */
	for (jj = 0; jj <= Nj - NB; jj += NB) {

	    for (i = ii; i < Ni; i++) {
		for (j = jj; j < jj + NB; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < Ni - 2; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i;
			     k < kk + NB;
			     k++, pa0++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii) not in block */
		for (i = ii; i < Ni - 2; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i;
			     k < Nk;
			     k++, pa0++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		}
	    }
	}

	for (jj = jj; jj < Nj; jj += NB) {	/* Part of jj(ii) not in block */

	    for (i = ii; i < Ni; i++) {
		for (j = jj; j < Nj; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i <= Ni - 2; i += 2) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < kk + NB;
			     k++, pa0++,
			     pa1++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i;
			     k < kk + NB;
			     k++, pa0++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i;
			     k < kk + NB;
			     k++, pa0++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			}
			*pc0 = s00;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii,jj) not in block */
		for (i = ii; i <= Ni - 2; i += 2) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i,
			     pa1 = a + k + NaOffs * (i + 1);
			     k < Nk;
			     k++, pa0++,
			     pa1++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pb1 = b + k + NbOffs * (j + 1),
			     pa0 = a + k + NaOffs * i;
			     k < Nk;
			     k++, pa0++,
			     pb0++,
			     pb1++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;

			for (k = kk, pb0 = b + k + NbOffs * j,
			     pa0 = a + k + NaOffs * i;
			     k < Nk;
			     k++, pa0++,
			     pb0++) {
			    s00 += (*pa0) * (*pb0);
			}
			*pc0 = s00;
		    }
		}
	    }
	}
    }
}

/****************************************************************************/
/*                           MTM1x1P                                        */
/****************************************************************************/

void 
MTM1x1P (REAL * c, REAL * a, REAL * b,
	 int Ni, int Nj, int Nk, int NaOffs, int NbOffs)
{
    int i, j, k;
    REAL *pa, *pb, *pc;
    REAL s00;

    for (i = 0; i < Ni; i++) {
	for (j = 0; j < Nj; j++) {
	    pc = c + j + Nj * i;
	    s00 = 0.0;
	    pb = b + j;
	    pa = a + i;
	    for (k = 0; k < Nk; k++) {
		s00 += (*pa) * (*pb);
		pa += NaOffs;
		pb += NbOffs;
	    }
	    *pc = s00;
	}
    }
}

/****************************************************************************/
/*                           MTM2x2P                                        */
/****************************************************************************/

void 
MTM2x2P (REAL * c, REAL * a, REAL * b,
	 int Ni, int Nj, int Nk, int NaOffs, int NbOffs)
{
    int i, j, k;
    REAL s00, s01, s10, s11;
    REAL *pa0, *pa1, *pb0, *pb1, *pc0, *pc1;

    for (i = 0; i <= Ni - 2; i += 2) {
	for (j = 0; j <= Nj - 2; j += 2) {
	    pc0 = c + j + Nj * i;
	    pc1 = c + j + Nj * (i + 1);
	    s00 = 0.0;
	    s01 = 0.0;
	    s10 = 0.0;
	    s11 = 0.0;

	    for (k = 0, pb0 = b + j + k * NbOffs,
		 pb1 = b + j + k * NbOffs + 1,
		 pa0 = a + k * NaOffs + i,
		 pa1 = a + k * NaOffs + (i + 1);
		 k < Nk;
		 k++, pa0 += NaOffs,
		 pa1 += NaOffs,
		 pb0 += NbOffs,
		 pb1 += NbOffs) {
		s00 += (*pa0) * (*pb0);
		s01 += (*pa0) * (*pb1);
		s10 += (*pa1) * (*pb0);
		s11 += (*pa1) * (*pb1);
	    }
	    *pc0 = s00;
	    *(pc0 + 1) = s01;
	    *pc1 = s10;
	    *(pc1 + 1) = s11;
	}
	for (j = j; j < Nj; j++) {
	    pc0 = c + j + Nj * i;
	    pc1 = c + j + Nj * (i + 1);
	    s00 = 0.0;
	    s10 = 0.0;

	    for (k = 0, pb0 = b + j + k * NbOffs,
		 pa0 = a + k * NaOffs + i,
		 pa1 = a + k * NaOffs + (i + 1);
		 k < Nk;
		 k++, pa0 += NaOffs,
		 pa1 += NaOffs,
		 pb0 += NbOffs) {
		s00 += (*pa0) * (*pb0);
		s10 += (*pa1) * (*pb0);
	    }
	    *pc0 = s00;
	    *pc1 = s10;
	}
    }
    for (i = i; i < Ni; i++) {
	for (j = 0; j <= Nj - 2; j += 2) {
	    pc0 = c + j + Nj * i;
	    s00 = 0.0;
	    s01 = 0.0;

	    for (k = 0, pb0 = b + j + k * NbOffs,
		 pb1 = b + j + k * NbOffs + 1,
		 pa0 = a + k * NaOffs + i;
		 k < Nk;
		 k++, pa0 += NaOffs,
		 pb0 += NbOffs,
		 pb1 += NbOffs) {
		s00 += (*pa0) * (*pb0);
		s01 += (*pa0) * (*pb1);
	    }
	    *pc0 = s00;
	    *(pc0 + 1) = s01;
	}
	for (j = j; j < Nj; j++) {
	    pc0 = c + j + Nj * i;
	    s00 = 0.0;

	    for (k = 0, pb0 = b + j + k * NbOffs,
		 pa0 = a + k * NaOffs + i;
		 k < Nk;
		 k++, pa0 += NaOffs,
		 pb0 += NbOffs) {
		s00 += (*pa0) * (*pb0);
	    }
	    *pc0 = s00;
	}
    }
}

/****************************************************************************/
/*                           MTM2x2PB                                       */
/****************************************************************************/

void
MTM2x2PB (REAL * c, REAL * a, REAL * b,
	  int Ni, int Nj, int Nk, int NaOffs, int NbOffs, int NB)
{
    int i, j, k, ii, jj, kk;
    REAL s00, s01, s10, s11;
    REAL *pa0, *pa1, *pb0, *pb1, *pc0, *pc1;

    for (ii = 0; ii <= Ni - NB; ii += NB) {
	for (jj = 0; jj <= Nj - NB; jj += NB) {

	    for (i = ii; i < ii + NB; i++) {
		for (j = jj; j < jj + NB; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk; k < kk + NB; k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk not in block */
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk; k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
	    }
	}

	for (jj = jj; jj < Nj; jj += NB) {	/* Part of jj not in block */
	    for (i = ii; i < ii + NB; i++) {
		for (j = jj; j < Nj; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }

		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			pb0 = b + j;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii) not in block */
		for (i = ii; i < ii + NB; i += 2) {
		    for (j = jj; j < Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			pb0 = b + j;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
	    }
	}
    }

    for (ii = ii; ii < Ni; ii += NB) {	/* Part of ii not in block */
	for (jj = 0; jj <= Nj - NB; jj += NB) {

	    for (i = ii; i < Ni; i++) {
		for (j = jj; j < jj + NB; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i < Ni - 2; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    pa0 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii) not in block */
		for (i = ii; i < Ni - 2; i += 2) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j < jj + NB; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			for (k = kk;
			     k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    pa0 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		}
	    }
	}

	for (jj = jj; jj < Nj; jj += NB) {	/* Part of jj(ii) not in block */

	    for (i = ii; i < Ni; i++) {
		for (j = jj; j < Nj; j++) {
		    *(c + j + Nj * i) = 0.0;
		}
	    }

	    for (kk = 0; kk <= Nk - NB; kk += NB) {
		for (i = ii; i <= Ni - 2; i += 2) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			pb0 = b + j;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    pa0 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;

			pb0 = b + j;
			pa0 = a + i;
			for (k = kk;
			     k < kk + NB;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    pa0 += NaOffs;
			    pb0 += NbOffs;
			}
			*pc0 = s00;
		    }
		}
	    }

	    for (kk = kk; kk < Nk; kk += NB) {	/* Part of kk(ii,jj) not in block */
		for (i = ii; i <= Ni - 2; i += 2) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s01 = *(pc0 + 1);
			s10 = *pc1;
			s11 = *(pc1 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			pa1 = a + (i + 1);
			for (k = kk;
			     k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    s10 += (*pa1) * (*pb0);
			    s11 += (*pa1) * (*pb1);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
			*pc1 = s10;
			*(pc1 + 1) = s11;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			pc1 = c + j + Nj * (i + 1);
			s00 = *pc0;
			s10 = *pc1;

			pb0 = b + j;
			pa0 = a + i,
			    pa1 = a + (i + 1);
			for (k = kk;
			     k < Nk;
			     k++) {
			    s00 += (*pa0) * (*pb0);
			    s10 += (*pa1) * (*pb0);
			    pa0 += NaOffs;
			    pa1 += NaOffs;
			    pb0 += NbOffs;
			}
			*pc0 = s00;
			*pc1 = s10;
		    }
		}
		for (i = i; i < Ni; i++) {
		    for (j = jj; j <= Nj - 2; j += 2) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;
			s01 = *(pc0 + 1);

			pb0 = b + j;
			pb1 = b + j + 1;
			pa0 = a + i;
			for (k = kk; k < Nk; k++) {
			    s00 += (*pa0) * (*pb0);
			    s01 += (*pa0) * (*pb1);
			    pa0 += NaOffs;
			    pb0 += NbOffs;
			    pb1 += NbOffs;
			}
			*pc0 = s00;
			*(pc0 + 1) = s01;
		    }
		    for (j = j; j < Nj; j++) {
			pc0 = c + j + Nj * i;
			s00 = *pc0;

			pb0 = b + j;
			pa0 = a + i;
			for (k = kk; k < Nk; k++) {
			    s00 += (*pa0) * (*pb0);
			    pa0 += NaOffs;
			    pb0 += NbOffs;
			}
			*pc0 = s00;
		    }
		}
	    }
	}
    }
}

/****************************************************************************/
/*                           END OF FILE                                    */
/****************************************************************************/


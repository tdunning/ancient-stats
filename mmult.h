/****************************************************************************/
/*                             MM.H                                         */
/****************************************************************************/

#ifndef _H_MM
#define _H_MM

typedef float REAL;

#define BLOCK_DIMENSION 50

/****************************************************************************/
/*                           MM1x1P                                         */
/****************************************************************************/
/* Matrix multiply with no unrolling, no blocking but use of explicit       */
/* pointers.                                                                */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and a                              */
/*             Nj         # of cols of c and b                              */
/*             Nk         # of cols of a and # of rows of b                 */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*                                                                          */
/****************************************************************************/

void MM1x1P(REAL* c, REAL* a, REAL* b,
	    int Ni,int Nj,int Nk, int NaOffs, int NbOffs);

/****************************************************************************/
/*                           MM2x2P                                         */
/****************************************************************************/
/* Matrix multiply with 2x2 unrolling, no blocking and use of explicit      */
/* pointers.                                                                */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and a                              */
/*             Nj         # of cols of c and b                              */
/*             Nk         # of cols of a and # of rows of b                 */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*                                                                          */
/****************************************************************************/

void MM2x2P(REAL* c, REAL* a, REAL* b,
	    int Ni, int Nj, int Nk, int NaOffs, int NbOffs);

/****************************************************************************/
/*                           MM2x2PB                                        */
/****************************************************************************/
/* Matrix multiply with 2x2 unrolling, blocking and use of explicit         */
/* pointers.                                                                */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and a                              */
/*             Nj         # of cols of c and b                              */
/*             Nk         # of cols of a and # of rows of b                 */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*             NB         Block dimension (must be even)                    */
/*                                                                          */
/****************************************************************************/

void MM2x2PB(REAL* c, REAL* a, REAL* b,
	     int Ni, int Nj, int Nk, int NaOffs, int NbOffs, int NB);

/****************************************************************************/
/*                           MMT1x1P                                        */
/****************************************************************************/
/* Matrix multiply with no unrolling, no blocking and use of explicit       */
/* pointers.                              t                                 */
/* The second matrix is transposed. (c=a*b )                                */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and a                              */
/*             Nj         # of cols of c and # of rows of b                 */
/*             Nk         # of cols of a and b                              */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*                                                                          */
/****************************************************************************/

void MMT1x1P(REAL* c, REAL* a, REAL* b,
	     int Ni, int Nj, int Nk, int NaOffs, int NbOffs);

/****************************************************************************/
/*                           MMT2x2P                                        */
/****************************************************************************/
/* Matrix multiply with 2x2 unrolling, no blocking and use of explicit      */
/* pointers.                                t                               */
/* The second matrix is transposed. (c = a*b )                              */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and a                              */
/*             Nj         # of cols of c and # of rows of b                 */
/*             Nk         # of cols of a and b                              */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*                                                                          */
/****************************************************************************/

void MMT2x2P(REAL* c, REAL* a, REAL* b,
	     int Ni, int Nj, int Nk, int NaOffs, int NbOffs);


/****************************************************************************/
/*                           MMT2x2PB                                       */
/****************************************************************************/
/* Matrix multiply with 2x2 unrolling, blocking and use of explicit         */
/* pointers.                                t                               */
/* The second matrix is transposed. (c = a*b )                              */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and a                              */
/*             Nj         # of cols of c and # of rows of b                 */
/*             Nk         # of cols of a and b                              */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*             NB         Block dimension (must be even)                    */
/*                                                                          */
/****************************************************************************/

void MMT2x2PB(REAL* c, REAL* a, REAL* b,
	      int Ni, int Nj, int Nk, int NaOffs, int NbOffs, int NB);

/****************************************************************************/
/*                           MTM1x1P                                        */
/****************************************************************************/
/* Matrix multiply with no unrolling, no blocking and use of explicit       */
/* pointers.                             t                                  */
/* The first matrix is transposed. (c = a *b )                              */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and # of cols of a                 */
/*             Nj         # of cols of c and b                              */
/*             Nk         # of rows of a and # of rows of b                 */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*                                                                          */
/****************************************************************************/

void MTM1x1P(REAL* c, REAL* a, REAL* b,
	     int Ni, int Nj, int Nk, int NaOffs, int NbOffs);

/****************************************************************************/
/*                           MTM2x2P                                        */
/****************************************************************************/
/* Matrix multiply with 2x2 unrolling, no blocking and use of explicit      */
/* pointers.                             t                                  */
/* The first matrix is transposed. (c = a *b )                              */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and # of cols of a                 */
/*             Nj         # of cols of c and b                              */
/*             Nk         # of rows of a and # of rows of b                 */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*                                                                          */
/****************************************************************************/

void MTM2x2P(REAL* c, REAL* a, REAL* b,
	     int Ni, int Nj, int Nk, int NaOffs, int NbOffs);

/****************************************************************************/
/*                           MTM2x2PB                                       */
/****************************************************************************/
/* Matrix multiply with 2x2 unrolling, blocking and use of explicit         */
/* pointers.                             t                                  */
/* The first matrix is transposed. (c = a *b)                               */
/*                                                                          */
/* Parameters                                                               */
/*             c          result matrix                                     */
/*             a          first matrix                                      */
/*             b          second matrix                                     */
/*             Ni         # of rows of c and # of cols of a                 */
/*             Nj         # of cols of c and b                              */
/*             Nk         # of rows of a and # of rows of b                 */
/*             NaOffs     rows overlap of a                                 */
/*             NbOffs     rows overlap of b                                 */
/*             NB         Block dimension (must be even)                    */
/*                                                                          */
/****************************************************************************/

void MTM2x2PB(REAL* c, REAL* a, REAL* b,
	      int Ni, int Nj, int Nk, int NaOffs, int NbOffs, int NB);

#endif

/****************************************************************************/
/*                                 END OF FILE                              */
/****************************************************************************/

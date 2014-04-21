/*
  copyright 1994 ted dunning

  unlimited noncommercial use, copying and redistribution allowed.

  commercial use, copying and redistribution allowed by written
  permission.  contact ted@nmsu.edu for more information.
  */

/* set a bit in a bit-vector */
void bit_set(int *tbl, int l, unsigned int bit);

/* clear a bit in a bit-vector */
void bit_clear(int *tbl, int l, unsigned int bit);

/* test a bit in a bit-vector */
int bit_test(int *tbl, int l, unsigned int bit);

/* quickly count the number of one bits in a 32 bit word */
int bit_count(int n);

/* count the number of one bits in a vector made up of 32 bit words */
int bitv_count(int *vec, int n);

#ifndef COMPAND_H

void init_compand(void);

int compress_element(float x);
void compress_vector(unsigned char *r, float *x, int n);
float expand_element(int x);
void expand_vector(float *r, unsigned char *x, int n);

int mul8(int x, int y);
float dot8(unsigned char *x, unsigned char *y, int n);

#endif

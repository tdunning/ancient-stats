/* clone of the drand48 procedures */

double x_drand48(void);
double x_erand48(unsigned short xsubi[3]);
long x_nrand48(unsigned short xsubi[]);
long x_lrand48(void);
long x_jrand48(unsigned short xsubi[3]);
long x_mrand48(void);
void x_srand48(long seedval);
unsigned short *x_seed48(unsigned short seed16v[3]);
void x_lcong48(unsigned short param[7]);

#define drand48() x_drand48()
#define erand48 x_erand48
#define nrand48 x_nrand48
#define lrand48 x_lrand48
#define jrand48 x_jrand48
#define mrand48 x_mrand48
#define srand48 x_srand48
#define seed48 x_seed48
#define lcong48 x_lcong48

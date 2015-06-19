include Makefile.$(ARCH)

CFLAGS = -g $(ARCH_CFLAGS)
LDFLAGS = $(ARCH_LDFLAGS)
LDLIBS = $(ARCH_LDLIBS)

all:  hwcount cgram compare entropy fwords grams total cooc chi2

test: chi2_lib_test
	./chi2_lib_test

chi2_lib_test: chi2_lib_test.o  chi2_lib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o chi2_lib_test chi2_lib_test.o  chi2_lib.o -lm $(LDLIBS)

stat_tcl: stat_tcl.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o stat_tcl stat_tcl.o -ltcl -lm $(LDLIBS)

drand: drand.c
	$(CC) $(CFLAGS) -DTEST $(LDFLAGS) -o drand drand.c -lm $(LDLIBS)

entropy: entropy.c
	$(CC) $(CFLAGS) $(LDFLAGS) entropy.c -o entropy -lm $(LDLIBS)

total: total.o error.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ total.o error.o $(LDLIBS) $(LDLIBS)

cgram:

cooc: cooc.o hash.o error.o intern_word.o leaky_cache.o bit_count.o $(ARCH_DRAND)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ cooc.o hash.o intern_word.o bit_count.o leaky_cache.o error.o $(ARCH_DRAND) $(LDLIBS)
cooc.o:   xvector.h intern_word.h leaky_cache.h

fwords: fwords.o map_file.o
	$(CC) $(CFLAGS) fwords.o map_file.o -o fwords $(LDLIBS)

lwcount.o: hash.h stringpool.h bit_count.h leaky_cache.h

lwcount: hash.o stringpool.o bit_count.o error.o leaky_cache.o lwcount.o
	$(CC) $(CFLAGS) lwcount.o hash.o stringpool.o bit_count.o error.o leaky_cache.o -o lwcount $(LDLIBS)


compare: compare.o hash.o error.o chi2_lib.o
	$(CC) $(CFLAGS) compare.o hash.o error.o chi2_lib.o -o compare -lm $(LDLIBS)

chi2.o : chi2_lib.h
chi2 : chi2.o chi2_lib.o
	$(CC) -o chi2 chi2.o chi2_lib.o -lm $(LDLIBS)

hwcount: error.o hash.o hwcount.o stringpool.o
	$(CC) $(CFLAGS) $(LDFLAGS) hwcount.o hash.o error.o stringpool.o -o hwcount

intern_word: error.o hash.o stringpool.o intern_word.c
	$(CC) $(CFLAGS) -DTEST intern_word.c error.o hash.o stringpool.o -o intern_word $(LDLIBS)

intern_word.o: intern_word.c error.h hash.h stringpool.h

hash.o : hash.h

error.o: error.h

clean: /tmp
	rm -f *.o *~ hwcount cgram compare entropy fwords grams total cooc chi2 lwcount

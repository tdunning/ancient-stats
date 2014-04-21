CC = gcc
CFLAGS += -g -Wall -D__USE_FIXED_PROTOTYPES__

all:   fwords cooc chi2

cooc: cooc.o hash.o error.o intern_word.o leaky_cache.o bit_count.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ cooc.o hash.o intern_word.o bit_count.o leaky_cache.o error.o $(LDLIBS)
cooc.o:   xvector.h intern_word.h leaky_cache.h

fwords: fwords.o map_file.o
	$(CC) $(CFLAGS) fwords.o map_file.o -o fwords

chi2.o : chi2_lib.h
chi2 : chi2.o chi2_lib.o
	$(CC) -o chi2 chi2.o chi2_lib.o -lm

hash.o : hash.h

error.o: error.h

clean: /tmp
	rm -f *.o *~  fwords cooc chi2
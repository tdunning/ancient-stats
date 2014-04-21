#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

char *optarg;
int optind = 0;

static int current = 0;
static int offset = 0;

static
int start_next_package(int argc, char *argv[], char *options)
{
    if (current < optind) current = optind+1;
    else current++;
    if (optind <= current) optind = current;
    
    if (current >= argc || argv[current] == NULL) return EOF;
    if (argv[current][0] == '-') {
	offset = 1;
	return getopt(argc, argv, options);
    }
    else {
	return EOF;
    }
}

static
int scan_for_option(int ch, char *options)
{
    int i;

    i = 0;
    while (options[i]) {
	if (ch == options[i]) {
	    if (options[i+1] == ':') return 1;
	    else return 0;
	}
	if (options[i+1] == ':') i += 2;
	else i += 1;
    }
    return -1;
}

int getopt(int argc, char *argv[], char *options)
{
    int ch;
    int found;

    if (current >= argc || argv[current] == NULL) return EOF;
    if (offset > 0) {
	if (argv[current][offset]) {
	    found = scan_for_option(argv[current][offset], options);
	    if (found == -1) {
		return '?';
	    }
	    if (found == 0) {
		ch = argv[current][offset];
		offset++;
	    }
	    if (found == 1) {
		if (optind <= current) optind = current+1;
		else optind = optind+1;
		optarg = argv[optind];
		ch = argv[current][offset];
		offset++;
	    }
	    return ch;
	}
	else {
	    return start_next_package(argc, argv, options);
	}
    }
    else {
	return start_next_package(argc, argv, options);
    }
}

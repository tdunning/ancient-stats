/* fast text segmenter for English.

   read a sequence of documents and separate the documents and words,
   adding positional information which can be used to form an index.

   document structure is fixed, but the delimiters for that structure
   can be kept in a configuration file.  fwords is flexible enough to
   handle TREC documents, babyl and unix mail formats.  what else could
   you want?  :-)


Copyright 1994, Ted Dunning, ted@nmsu.edu
Redistribution permitted if I am notified, and if this copyright notice
is preserved.  Inclusion in other software is also allowed on the same
terms.  Other terms by request.

$Log: fwords.c,v $
 * Revision 1.5  1996/07/16  22:44:33  ted
 * added sentence bound testing and revamped command line options
 * also tweaked to track xvector changes
 *
 * Revision 1.4  1996/05/16  21:51:30  ted
 * improved speed and handling of unstructured documents.  also eliminated
 * warnings under -Wall.
 *
 * Revision 1.3  1996/04/05  22:15:27  ted
 * cleaned up the initial log comment (tossing all ancestral notes from
 * the CRL log)
 *
 * Revision 1.2  1996/04/05  22:12:34  ted
 * got rid of the boneheaded buffer management stuff and changed to
 * mmap'ing of input files.  also tuned the token character sets
 * and changed the default behavior to just dump out words suitable
 * for counting
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "map_file.h"
#include <sys/mman.h>
#include "xvector.h"

/* these must not conflict with EOF */
enum {
    END_OF_SENTENCE = 1, WORD = 2, IGNORABLE = 3, SOLO = 4
};

int file_position;
int word_number;
int sentence_number;

int print_word=0, print_position=0, print_sentence=0, verbose=0;
char *number_format = "%d";

#define DOC_ID_SIZE	100

char *actual_file_name;
char *file_name=NULL;
char doc_id[DOC_ID_SIZE];

/* the implied structure is 

Section	Optional start? Optional end?	Contents		Default?

doc	no		no		doc_id text		none
doc_id	no		no		pcdata			from filename
text	no		no		words			none

*/

/* default segment markers make the entire file be the document */
char *doc_start = "", *doc_end = NULL;
char *doc_id_start = "", *doc_id_end = "";
char *text_start = "", *text_end = NULL;

/* flags which control output format */
int prolog_print=0, lisp_print=0;

int mark_documents=0;
int mark_sentences=0;

/* this next stuff should only be used by get_character and friends */

static char *limit_string;	/* the next delimiter pattern */
static int string_point;	/* where that pattern might be or -1 */

static char *buffer;		/* the buffer containing characters to read */

static int buffer_point;	/* where to find the next character */
static int buffer_fill;		/* how much data is in the buffer */

/* simple minded search routine.  doesn't seem to matter that it is
   about as slow as can be imagined */
int srch(char *buffer, char *pattern, int buffer_size)
{
    int i, patlen;

    if (!pattern) return -1;
    else if (!pattern[0]) return 0;
    else {
	patlen = strlen(pattern);
	for (i=0;i<=buffer_size-patlen;i++) {
	    if (buffer[i] == pattern[0] &&
		strncmp(buffer+i, pattern, patlen) == 0) {
		return i;
	    }
	}
	return -1;
    }
}

/* toss old buffer contents, and set up for reading a new file */
void setup_buffer(FILE *f)
{
    buffer_point = 0;
    buffer_fill = 0;

    limit_string = NULL;
    string_point = -1;
}


static int mapped_file=0;

/* try to fill the buffer if it hasn't been done yet */
void fill_buffer(FILE *f)
{
    if (!buffer) {
	buffer_fill = file_size(f);
	if (buffer_fill == -1) {
	    int n;
	    struct {
		int size, count;
		char *contents;
	    } *buf;

	    x_create(buf, 1000);
	    
	    do {
		x_resize(buf, x_length(buf) + 81920);
		n = fread(buf->contents+x_length(buf), 1, 81920, f);
		x_length(buf) += n;
	    } while (n > 0);
	    buffer_fill = x_length(buf);
	    buffer = buf->contents;
	    mapped_file = 0;
	}
	else {
	    buffer = map_file(f, 0, 0);
	    mapped_file = 1;
	}
	    
	buffer_point = 0;
    }
}

void close_buffer(FILE *f)
{
    if (mapped_file) {
	munmap(buffer, buffer_fill);
    }
    else {
	free(buffer);
    }
    buffer = NULL;
}

/* get ready to read up to a newly specified limit string */
int reset_buffer(FILE *f, char *bound)
{
    int n;

    /* insure we have the data */
    fill_buffer(f);

    /* if we hit a soft EOF already */
    if (limit_string && string_point == buffer_point) {
	/* skip safely over the current limit string */
	if (limit_string) {
	    n = strlen(limit_string);
	    if (n > buffer_fill-buffer_point) n = buffer_fill-buffer_point;
	    if (strncmp(buffer+buffer_point, limit_string, n) == 0) {
		buffer_point += n;
		string_point = -1;
	    }
	}
    }

    limit_string = bound;
    string_point = srch(buffer+buffer_point, limit_string,
			buffer_fill-buffer_point);
    if (string_point != -1) string_point += buffer_point;

    return buffer_fill>buffer_point;
}
	
/* read the next character.  check for virtual EOF's and buffer underflows */
inline int get_character(FILE *f)
{
    if (buffer_point >= buffer_fill) {
	return EOF;		       /* can't do it, hard EOF */
    }
    else if (buffer_point >= string_point && string_point != -1) {
	return EOF;		       /* at virtual EOF */
    }
    else {
	return buffer[buffer_point++];
    }
}

inline void unget_character(int ch)
{
    while (buffer_point && buffer[buffer_point] != ch) {
	buffer_point--;
    }
}

/* print in a way acceptable to lisp quoting conventions */
void lisp_print_string(s)
char *s;
{
    putchar('"');
    for (;*s;s++) {
	if (*s == '"') {
	    fputs("\\\"", stderr);
	}
	else {
	    putchar(*s);
	}
    }
    putchar('"');
}

/* print like c would like to read */
void c_print_string(s)
char *s;
{
    for (;*s;s++) {
	if (*s == ' ') {
	    fputs("\\ ", stdout);
	}
	else {
	    putchar(*s);
	}
    }
}

/* or print like prolog likes to see */
void prolog_print_string(s)
char *s;
{
    putchar('\'');
    for (;*s;s++) {
	if (*s == '\'') {
	    fputs("''", stdout);
	}
	else {
	    putchar(*s);
	}
    }
    putchar('\'');
}

/* print a string however is appropriate */
void print_string(s)
char *s;
{
    if (lisp_print) {
	lisp_print_string(s);
    }
    else if (prolog_print) {
	prolog_print_string(s);
    }
    else {
	c_print_string(s);
    }
}

/* print a language dependent separator */
inline void print_separator()
{
    if (prolog_print) {
	fputs(", ", stdout);
    }
    else {
	fputs(" ", stdout);
    }
}

/* print a number */
inline void print_number(d)
int d;
{
    printf(number_format, d);
}

/* finish a line of output */
void other_stuff()
{
    if (print_position || print_word || print_sentence) {
	if (print_word) {
	    print_separator();
	    print_number(word_number);
	}
	if (print_position) {
	    print_separator();
	    print_number(file_position);
	}
	if (print_sentence) {
	    print_separator();
	    print_number(sentence_number);
	}
    }
    if (prolog_print) {
	fputs(").\n", stdout);
    }
    else if (lisp_print) {
	fputs(")\n", stdout);
    }
    else putchar('\n');
}

void lisp_quote(char *s)
{
    int i;

    putchar('"');
    for (i=0;s[i];i++) {
	switch (s[i]) {
	  case '\\':
	  case '"':
	      printf("\\%c", s[i]);
	      break;
	  default:
	      putchar(s[i]);
	}
    }
    putchar('"');
}

void prolog_quote(char *s)
{
    int i;

    putchar('\'');
    for (i=0;s[i];i++) {
	switch (s[i]) {
	  case '\'':
	      putchar(s[i]);
	      /* fall through */
	  default:
	      putchar(s[i]);
	}
    }
    putchar('\'');
}

/* print a word summary */
void put_word(char w[])
{
    if (w[0]) {
	if (prolog_print) {
	    fputs("post(", stdout);
	    prolog_quote(w);
	}
	else if (lisp_print) {
	    fputs("(", stdout);
	    lisp_quote(w);
	}
	else {
	    fputs(w, stdout);
	}
	other_stuff();
    }
}

/* print a sentence boundary */
void put_bound()
{
    if (lisp_print) {
	printf("SENTENCE\n");
    }
    else if (prolog_print) {
	printf("sentence.\n");
    }
    else {
	printf("#S\n");
    }
}

char *abbrev[] = {
    "Gov.", "Rev.", "Amb.", "Asst.", "Atty.", "Sec.", "Supt.", "Sen.",
    "Rep.", "Hon.", "Mr.", "Dr.", "Mrs.", "Ms.", "Sr.", "Sra.",
    "Prof.", "Mlle.", "Mme.", "Msgr.", "Msgrs.", "H.M.S.", "S.S.",
    "U.S.S.", "Brig.", "Gen.", "Lt.", "Capt.", "Maj.", "Col.", "Ens.",
    "Adm.", "Sgt.", "Tsgt.", "Cpl.", "U.S.", "U.N.", "Jan.",
    "Feb.", "Mar.", "Apr.", "Jun.",
    "Jul.", "Aug.", "Sep.", "Sept.",
    "Oct.", "Nov.", "Dec.", "a.k.a.", 

    /* these take care of Ted E. Dunning */
    "A.", "B.", "C.", "D.", "E.", "F.", "G.", "H.", "I.", "J.", "K.",
    "L.", "M.", "N.", "O.", "P.", "Q.", "R.", "S.", "T.", "U.", "V.",
    "W.", "X.", "Y.", "Z.", NULL
};

int preceded_by (char *pattern, char *buf, int offset)
{
    int len;
    len = strlen(pattern);
    if (offset < len) return 0;
    return strncmp(pattern, (buf+offset+1)-len, len) == 0;

}

/* return 1 if buffer+offset points to a sentence boundary and 0 if not */
int sentence_bound(char *buf, int offset, int size)
{
    int i;

    /* if we have 2 newlines make it a sentence */
    if (buf[offset] == '\n') {
	i = offset+1;
	while (isspace(buf[i])) {
	    if (buf[i] == '\n')
		return 1;
	    i++;
	}
    }

    /* not looking at a period, question mark, exclamation point */
    if (buf[offset] != '.' &&
        buf[offset] != '?' &&
        buf[offset] != '!' ) return 0;

    /* at the beginning of the buffer (which is a beginning of a line */
    if (offset == 0) return 0;


    /* preceded by a disqualifying abbreviation */
    for (i=0;abbrev[i];i++) {
	if (preceded_by(abbrev[i], buf, offset)) {
	    return 0;
	}
    }

    /* if at the end of the buffer, it qualifies */
    if (offset == size-1) return 1;

    /* if followed by whitespace and then a lower case letter, it fails */
    if (offset < size-1 && isspace(buf[offset+1])) {
	i=offset+1;
	while (buf[i] && isspace(buf[i])) i++;
	if (i < size && islower(buf[i])) return 0;
    }
	       
    /* if preceded by a period, it fails */
    if (offset > 0 && buf[offset-1] == '.') return 0;

    /* if at the beginning of a line, it fails */
    if (offset > 0 && buf[offset-1] == '\n') return 0;

    /* if succeeded by a period, it fails */
    if (offset < size-1 && buf[offset+1] == '.') return 0;

    /* if followed immediately by a digit, it fails */
    if (offset < size-1 && isdigit(buf[offset+1])) return 0;

    /* if followed by a space, it succeeds */
    if (offset < size-1 && isspace(buf[offset+1])) return 1;

    /* if followed by a double quote and then a space, it succeeds */
    if (offset < size-1 && buf[offset+1] == '"' && isspace(buf[offset+2]))
	return 1;

    /* in general, it isn't a period */
    return 0;
}


/*
  word buffer... sorry about the limited size.  at least we won't
  croak on very long 'words'.
 */
char word[100];

int char_class_array[128+257];	/* room for EOF and 8 bit chars */
int *char_class = NULL;

#define C_START		1
#define C_MORE		2
#define C_TRAILER	4
#define C_QUOTABLE	8
#define C_SENTENCE	16

int get_word(FILE *f)
{
    int i, ch;

    /* build a character class table only once */
    if (!char_class) {
	char_class = char_class_array+1+128;
	
	for (i=0;i<256;i++) {
	    char_class[i] = 0;
	    if (isalpha(i) || (i&0x80) || isdigit(i) || i == '-' || i == '<') {
		char_class[i] |= C_START;
	    }
	    
	    if ((isalpha(i) || isdigit(i) || (i&0x80) || 
		 i == '/' || i == ',' || i == '.' || i == '\'' || i == ':'
		 || i == '_' || i == '~' || i == '-')) {
		char_class[i] |= C_MORE;
	    }

	    if (i == '.' || i == ',' || i == '\'' || i == '"' ||
		i == '\'') {
		char_class[i] |= C_TRAILER;
	    }

	    if (i == '$' || i == '#' || i == '_') {
		char_class[i] |= C_QUOTABLE;
	    }

	    if (i == '.' || i == '?' || i == '!' || i == '\n') {
		char_class[i] |= C_SENTENCE;
	    }

	    /* hack to make signed characters safe for all mankind */
	    if (i >= 128) {
		char_class[i-256] = char_class[i];
	    }
	}

	char_class[EOF] = 0;
    }

    ch = get_character(f);
    if (ch == EOF) return ch;

    if (mark_sentences && (char_class[ch] & C_SENTENCE)) {
	if (sentence_bound(buffer, buffer_point-1, buffer_fill)) {
	    return END_OF_SENTENCE;
	}
    }

    /* words and numbers */
    if (char_class[ch] & C_START) {
	i = 0;
	while (i<100 && char_class[ch] & C_MORE) {
	    word[i] = ch;
	    i++;
	    ch = get_character(f);
	}
	word[i] = 0;
	i--;
	while (i && (char_class[word[i]] & C_TRAILER)) {
	    if (mark_sentences) unget_character(word[i]);
	    word[i] = 0;	       /* trim trailing punctuation */
	    i--;
	}
	return WORD;
    }
    else if (char_class[ch] & C_QUOTABLE) {
	word[0] = '_';
	word[1] = ch;
	word[2] = 0;
	return SOLO;
    }
    else {
	word[0] = 0;
	return IGNORABLE;
    }
}

/* skip to virtual end of file */
void skip_to_eof(FILE *f)
{
    int ch;
    ch = get_character(f);
    while (ch != EOF) {
	ch = get_character(f);
    }
}

/* make a document id be one line with no embedded spaces.
   it still may have noxious things like quotes, brackets and
   other things, but it won't have spaces.
 */
void sanitize_doc_id(char *s, int size)
{
    int i, j;

    i = 0;
    j = 0;
    while (s[i] && isspace(s[i])) i++;

    for (;s[i] && i < size;i++) {
	if (isspace(s[i])) {
	    s[j++] = '_';
	}
	else if (isprint(s[i])) {
	    s[j++] = s[i];
	}
    }
    s[j] = 0;
    for (j--;j>=0 && s[j] == '_';j--) {
	s[j] = 0;
    }
}

/* read a document */
int read_document(FILE *f, int doc_number)
{
    int i;
    int document_start;
    int ch;

    (void) reset_buffer(f, doc_start);
    skip_to_eof(f);
    document_start = buffer_point;

    if (!reset_buffer(f, doc_id_start)) return EOF;
    skip_to_eof(f);

    if (!reset_buffer(f, doc_id_end)) return EOF;

    i = 0;
    ch = get_character(f);
    while (ch != EOF && i<DOC_ID_SIZE-1) {
	doc_id[i++] = ch;
	ch = get_character(f);
    }
    while (ch != EOF) {		       /* could we call skip_to_eof? */
	ch = get_character(f);
    }
    doc_id[i] = 0;

    if (!reset_buffer(f, text_start)) return EOF;    
    skip_to_eof(f);

    if (!reset_buffer(f, text_end)) return EOF;

    /* generate a default doc_id if necessary */
    if (doc_id[0] == 0) {
	sprintf(doc_id, "%s:%d", file_name, doc_number);
    }
    sanitize_doc_id(doc_id, sizeof(doc_id));

    /* only now do we commit to recognizing a document */
    if (mark_documents) {
	if (lisp_print) {
	    printf("(doc-start ");
	    lisp_quote(doc_id);
	    printf(" ");
	    lisp_quote(file_name);
	    printf(" %d)\n", document_start);
	}
	else if (prolog_print) {
	    printf("doc_start(");
	    prolog_quote(doc_id);
	    printf(", ");
	    prolog_quote(file_name);
	    printf(", %d).\n", document_start);
	}
	else {
	    printf("$ %s %s %d\n", doc_id, file_name, document_start);
	}
    }
    if (verbose) {
	fprintf(stderr, "%s %s\n", file_name, doc_id);
    }

    word_number = 0;
    sentence_number = 0;
    file_position = buffer_point;
    ch = get_word(f);
    while (ch != EOF) {
	switch (ch) {
	  case SOLO:
	  case WORD:
	    put_word(word);
	    word_number++;
	    file_position = buffer_point;
	    ch = get_word(f);
	    break;
	  case END_OF_SENTENCE:
	      if (mark_sentences) {
		  sentence_number++;
		  put_bound();
	      }
	      while (ch == END_OF_SENTENCE || ch == IGNORABLE) {
		  file_position = buffer_point;
		  ch = get_word(f);
	      }
	      break;
	  case IGNORABLE:
	      file_position = buffer_point;
	      ch = get_word(f);
	      break;
	}
    }

    (void) reset_buffer(f, doc_end);
    skip_to_eof(f);
    if (mark_documents) {
	if (doc_end) {
	    if (lisp_print) {
		printf("(doc-end ");
		lisp_quote(doc_id);
		printf(" %d)\n", buffer_point+strlen(doc_end));
	    }
	    else if (prolog_print) {
		printf("doc_end(");
		prolog_quote(doc_id);
		printf(", %d).\n", buffer_point+strlen(doc_end));
	    }
	    else {
		printf("$ %s %d\n", doc_id, buffer_point+strlen(doc_end));
	    }
	}
	else {
	    if (lisp_print) {
		printf("(doc-end ");
		lisp_quote(doc_id);
		printf(" %d)\n", buffer_point);
	    }
	    else if (prolog_print) {
		printf("doc_end(");
		prolog_quote(doc_id);
		printf(", %d).\n", buffer_point);
	    }
	    else {
		printf("$ %s %d\n", doc_id, buffer_point);
	    }
	}
    }

    return 0;
}

/* read many documents from a file */
void process_file(FILE *f)
{
    int n, document_count;

    setup_buffer(f);

    document_count = 0;
    n = read_document(f, document_count);
    document_count++;
    while (n != EOF) {
	n = read_document(f, document_count);
	document_count++;
    }
    close_buffer(f);
}

/* read a configuration line and expand the character escapes */
char *read_and_fix_line(char *file_name, FILE *f, char *old)
{
    char buf[1000];		       /* limited size.  so what. */
    int i, j;

    if (fgets(buf, sizeof(buf), f)) {
	buf[strlen(buf)-1] = 0;
	i = 0;
	for (j=0;j < sizeof(buf) && buf[j] != '\n' && buf[j];j++) {
	    switch (buf[j]) {
	    case ' ':
	    case '\t':		/* ignore white space */
		break;		/* let them use \s and \t */
	    case '\\':
		j++;
		switch (buf[j]) {
		case 's':	/* space, non-standard, nice for visibility */
		    buf[i++] = ' ';
		    break;
		case 'n':	/* new line */
		    buf[i++] = '\n';
		    break;
		case 'r':	/* return */
		    buf[i++] = '\r';
		    break;
		case 't':	/* tab */
		    buf[i++] = '\t';
		    break;
		case 'f':	/* form-feed */
		    buf[i++] = '\f';
		    break;
		case '\\':	/* back-slash itself */
		    buf[i++] = '\\';
		    break;
		case 'c':	/* control character such as \c-x */
		    j++;
		    if (buf[j] == '-') {
			j++;
			buf[i++] = buf[j]&0x1f;
		    }
		    else {
			fprintf(stderr,
				"bad control character (\\%c%c%c)\n",
				buf[j-1], buf[j], buf[j+1]);
			exit(1);
		    }
		    break;
		default:	/* huh? */
		    fprintf(stderr,
			    "bad escape char. (\\%c) in configuration file\n",
			    buf[j]);
		    exit(1);
		}
		break;
	    default:
		buf[i++] = buf[j];
		break;
	    }
	}
	if (j == sizeof(buf)) {
	    fprintf(stderr,
		    "fwords: *very* long configuration line in file %s\n",
		    file_name);
	    exit(1);
	}

	buf[i] = 0;
	return strdup(buf);
    }
    else {
	return old;
    }
}    
			
/* read the configuration file and set the various delimiters */
void configure_delimiters(char *conf_file)
{
    FILE *f;

    f = fopen(conf_file, "r");
    if (!f) {
	fprintf(stderr,
		"fwords: can't open configuration file %s\n", conf_file);
	exit(1);
    }

    doc_start = read_and_fix_line(conf_file, f, doc_start);
    doc_end  = read_and_fix_line(conf_file, f, doc_end);
    doc_id_start = read_and_fix_line(conf_file, f, doc_id_start);
    doc_id_end = read_and_fix_line(conf_file, f, doc_id_end);
    text_start = read_and_fix_line(conf_file, f, text_start);
    text_end = read_and_fix_line(conf_file, f, text_end);

    fclose(f);
}

/* used in command line argument parsing */
extern int optind;
extern char *optarg;

int main(int argc, char *argv[])
{
    int error_flag;
    int ch, i;

    extern int getopt();

    error_flag = 0;
    while (!error_flag
	   && (ch = getopt(argc, argv, "DSLPF:Wwbsvf:x")) != EOF) {
	switch (ch) {
	  case 'x':
	      number_format = "%x";
	      break;
	  case 'F':
	      file_name = optarg;
	      break;
	  case 'L':
	      number_format = "%d";
	      lisp_print = 1;
	      prolog_print = 0;
	      break;
	  case 'P':
	      number_format = "%d";
	      prolog_print = 1;
	      lisp_print = 0;
	      break;
	  case 'D':
	      mark_documents = 1;
	      break;
	  case 'S':
	      mark_sentences = 1;
	      break;
	  case 'f':
	      configure_delimiters(optarg);
	      break;
	  case 'w':
	      print_word = 1;
	      break;
	  case 'b':
	      print_position = 1;
	      break;
	  case 's':
	      print_sentence = 1;
	      break;
	  case 'v':
	      verbose = 1;
	      break;
	  default:
	      error_flag = 1;
	}
    }

    if (error_flag) {
	fprintf(stderr,
		"usage: fwords [-DSLPwbsvx] [-F name] [-f conf-file] {files}\n"
		"    -D means mark document boundaries\n"
		"    -S means mark sentence ends\n"
		"    -L means output in lispy format\n"
		"    -P means output in prolog-style\n"
		"    -w means print word sequence number\n"
		"    -b means print byte offset from beginning of document\n"
		"    -s means print sentence number\n"
		"    -v means print diagnostics on stderr\n"
		"    -x means print numbers in hexadecimal (it's faster!)\n");
	exit(1);
    }

    if (optind >= argc) {
	if (file_name == NULL) file_name = "-";
	file_position = word_number = 0;
	process_file(stdin);
    }

    else {
	FILE *f;
        for (i = optind; i < argc; i++) {
	    if (strcmp(argv[i], "-") == 0) {
		file_name = "-";
		f = stdin;
	    }
	    else {
		file_name = argv[i];
		f = fopen(argv[i], "r");
		if (f == NULL) {
		    fprintf(stderr, "Error: can't open file.\n");
		    exit(1);
		}
	    }

	    file_position = word_number = 0;
	    process_file(f);

            if (f != stdin) {
		fclose(f);
	    }
	}
    }
    return 0;
}


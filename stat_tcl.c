#include <tcl.h>

/* tcl extension which supports various word counting tasks 

 *****************************************************************************
 *
 * Copyright (c) 1996, Aptex Inc. 9605 Scranton Road Suite 240, San Diego.
 * All Rights Reserved.
 * based on software which is copyright 1994 and 1995 by ted dunning
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF APTEX INC
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.                       
 * 
 * RESTRICTED RIGHTS LEGEND:  Use, duplication, or disclosure by the government
 * is subject to restrictions as set forth in subparagraph (c)(1)(ii) of the
 * Rights in Technical Data and Computer Software clause at DFARS 252.227-7013
 * and FAR 52.227-19.
 * 
 *      Product:       
 *          stat_tcl: word level statistics for tcl
 *
 *      Module:        
 *          stat_tcl.h
 *
 *      Created:   
 *          8/1/96
 *
 *      Authors         Initials        Involvement
 *      -------         --------        -----------
 *      Ted Dunning	 ted             Creator   
 *                                                
 ****************************************************************************

including:

   fwords - a fast text segmenter for English.

   reads a sequence of documents and separate the documents and words,
   adding positional information which can be used to form an index.

   document structure is fixed, but the delimiters for that structure
   can be kept in a configuration file.  fwords is flexible enough to
   handle TREC documents, babyl and unix mail formats.  what else could
   you want?  :-)

*/

#include <tcl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "xvector.h"

/* these must not conflict with EOF */
enum {
    END_OF_SENTENCE = 1, WORD = 2, IGNORABLE = 3, SOLO = 4
};

static int file_position;
static int word_number;
static int sentence_number;

static int print_word=0, print_position=0, print_sentence=0;
static char *number_format = "%d";

#define DOC_ID_SIZE	100

static char doc_id[DOC_ID_SIZE];

/* the implied structure is 

Section	Optional start? Optional end?	Contents		Default?

doc	yes		no		doc_id text		none
doc_id	yes		yes		pcdata			from filename
text	yes		no		words			none

*/

/* default segment markers make the entire file be the document */
static char *doc_start = "", *doc_end = NULL;
static char *doc_id_start = "", *doc_id_end = "";
static char *text_start = "", *text_end = NULL;

static int mark_documents=0;
static int mark_sentences=0;

/* this next stuff should only be used by get_character and friends */

static char *limit_string;	/* the next delimiter pattern */
static int string_point;	/* where that pattern might be or -1 */

static char *buffer;		/* the buffer containing characters to read */

static int buffer_point;	/* where to find the next character */
static int buffer_fill;		/* how much data is in the buffer */

/* simple minded search routine.  doesn't seem to matter that it is
   about as slow as can be imagined */
static int srch(char *buffer, char *pattern, int buffer_size)
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
static void setup_buffer(void)
{
    buffer_point = 0;
    buffer_fill = 0;

    limit_string = NULL;
    string_point = -1;
}


/* try to fill the buffer if it hasn't been done yet */
static void fill_buffer(char *data)
{
    if (!buffer) {
	buffer_fill = strlen(data);
	buffer = data;
	buffer_point = 0;
    }
}

static void close_buffer(char *data)
{
    buffer = NULL;
}

/* get ready to read up to a newly specified limit string */
static int reset_buffer(char *data, char *bound)
{
    int n;

    /* insure we have the data */
    fill_buffer(data);

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
static inline int get_character(void)
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

static inline void unget_character(int ch)
{
    while (buffer_point && buffer[buffer_point] != ch) {
	buffer_point--;
    }
}

static int count_words = 0;
static int output_counts = 0;
static Tcl_HashTable counts;

/* print a word summary */
static void put_word(Tcl_Interp *interp, char w[])
{
    Tcl_HashEntry *entry;
    char tmp[100];		/* big enough for %d */
    int new;

    if (w[0]) {
	if (count_words) {
	    entry = Tcl_CreateHashEntry(&counts, w, &new);
	    if (new) {
		Tcl_SetHashValue(entry, 1);
	    }
	    else {
		Tcl_SetHashValue(entry, Tcl_GetHashValue(entry)+1);
	    }
	}
	else {
	    if (print_position || print_word || print_sentence) {
		Tcl_AppendResult(interp, "{", NULL);
	    }
	    Tcl_AppendElement(interp, w);

	    if (print_word) {
		sprintf(tmp, number_format, word_number);
		Tcl_AppendElement(interp, tmp);
	    }
	    if (print_position) {
		sprintf(tmp, number_format, file_position);
		Tcl_AppendElement(interp, tmp);
	    }
	    if (print_sentence) {
		sprintf(tmp, number_format, sentence_number);
		Tcl_AppendElement(interp, tmp);
	    }
	    if (print_position || print_word || print_sentence) {
		Tcl_AppendResult(interp, "} ", NULL);
	    }
	}
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

static int preceded_by (char *pattern, char *buf, int offset)
{
    int len;
    len = strlen(pattern);
    if (offset < len) return 0;
    return strncmp(pattern, (buf+offset+1)-len, len) == 0;

}

/* return 1 if buffer+offset points to a sentence boundary and 0 if not */
static int sentence_bound(char *buf, int offset, int size)
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
static char word[100];

static int char_class_array[128+257];	/* room for EOF and 8 bit chars */
static int *char_class = NULL;

#define C_START		1
#define C_MORE		2
#define C_TRAILER	4
#define C_QUOTABLE	8
#define C_SENTENCE	16

static int get_word(void)
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

    ch = get_character();
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
	    ch = get_character();
	}
	word[i] = 0;
	i--;
	while (i && (char_class[(int) word[i]] & C_TRAILER)) {
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
static void skip_to_eof(void)
{
    int ch;
    ch = get_character();
    while (ch != EOF) {
	ch = get_character();
    }
}

/* make a document id be one line with no embedded spaces.
   it still may have noxious things like quotes, brackets and
   other things, but it won't have spaces.
 */
static void sanitize_doc_id(char *s, int size)
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
int read_document(Tcl_Interp *interp, char *data, int doc_number)
{
    int i;
    int ch;
    char tmp[100];
    int document_start;
    int inside_sentence;
    Tcl_HashEntry *entry;
    Tcl_HashSearch search;

    (void) reset_buffer(data, doc_start);
    skip_to_eof();
    document_start = buffer_point;

    if (!reset_buffer(data, doc_id_start)) return EOF;
    skip_to_eof();

    if (!reset_buffer(data, doc_id_end)) return EOF;

    i = 0;
    ch = get_character();
    while (ch != EOF && i<DOC_ID_SIZE-1) {
	doc_id[i++] = ch;
	ch = get_character();
    }
    while (ch != EOF) {		       /* could we call skip_to_eof? */
	ch = get_character();
    }
    doc_id[i] = 0;

    if (!reset_buffer(data, text_start)) return TCL_BREAK;    
    skip_to_eof();

    if (!reset_buffer(data, text_end)) return TCL_BREAK;

    /* generate a default doc_id if necessary */
    if (doc_id[0] == 0) {
	sprintf(doc_id, "doc_%d", doc_number);
    }
    sanitize_doc_id(doc_id, sizeof(doc_id));

    if (count_words) Tcl_InitHashTable(&counts, TCL_STRING_KEYS);

    /* only now do we commit to recognizing a document */
    if (mark_documents) {
	Tcl_AppendResult(interp, "{{", NULL);
	Tcl_AppendElement(interp, doc_id);
	sprintf(tmp, "%d", document_start);
	Tcl_AppendElement(interp, tmp);
	Tcl_AppendResult(interp, "} {", NULL);
    }

    word_number = 0;
    inside_sentence = 0;
    sentence_number = 0;
    file_position = buffer_point;
    if (mark_sentences) {
	Tcl_AppendResult(interp, "{", NULL);
	inside_sentence = 1;
    }
    ch = get_word();
    while (ch != EOF) {
	switch (ch) {
	  case SOLO:
	  case WORD:
	    put_word(interp, word);
	    word_number++;
	    file_position = buffer_point;
	    ch = get_word();
	    break;
	  case END_OF_SENTENCE:
	      if (mark_sentences) {
		  sentence_number++;
		  Tcl_AppendResult(interp, "}", NULL);
		  inside_sentence = 0;
	      }
	      while (ch == END_OF_SENTENCE || ch == IGNORABLE) {
		  file_position = buffer_point;
		  ch = get_word();
	      }
	      if (ch != EOF) {
		  Tcl_AppendResult(interp, " {", NULL);
		  inside_sentence = 1;
	      }
	      break;
	  case IGNORABLE:
	      file_position = buffer_point;
	      ch = get_word();
	      break;
	}
    }

    (void) reset_buffer(data, doc_end);
    skip_to_eof();

    if (mark_sentences && inside_sentence) {
	Tcl_AppendResult(interp, "}", NULL);
    }
    if (count_words) {
	for (entry = Tcl_FirstHashEntry(&counts, &search);
	     entry != NULL;
	     entry = Tcl_NextHashEntry(&search)) {
	    sprintf(tmp, number_format, (int) Tcl_GetHashValue(entry));
	    if (output_counts) Tcl_AppendResult(interp, "{", tmp, NULL);
	    Tcl_AppendElement(interp, Tcl_GetHashKey(&counts, entry));
	    if (output_counts) Tcl_AppendResult(interp, "} ", NULL);
	}
    }

    if (mark_documents) {
	Tcl_AppendResult(interp, "} ", NULL);
	if (doc_end) {
	    sprintf(tmp, "%d", buffer_point+strlen(doc_end));
	}
	else {
	    sprintf(tmp, "%d", buffer_point);
	}
	Tcl_AppendElement(interp, tmp);
	Tcl_AppendResult(interp, "} ", NULL);
    }
    if (count_words) Tcl_DeleteHashTable(&counts);

    return TCL_OK;
}

/* read many documents from a string */
static int process_file(Tcl_Interp *interp, char *data)
{
    int n, document_count;

    setup_buffer();

    document_count = 0;
    n = read_document(interp, data, document_count);
    document_count++;
    while (n == TCL_OK) {
	n = read_document(interp, data, document_count);
	document_count++;
    }
    close_buffer(data);
    if (n == TCL_ERROR) return TCL_ERROR;
    else return TCL_OK;
}

/* read a configuration line and expand the character escapes */
void fixup_line(char *buf)
{
    int i, j, n;

    i = 0;
    n = strlen(buf);
    for (j=0;j < n && buf[j] != '\n' && buf[j];j++) {
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
    buf[i] = 0;
}
			
/* read the configuration data and set the various delimiters */
static char **configure_delimiters(Tcl_Interp *interp, char *conf_data)
{
    int conf_count;
    char **conf_lines;

    if (Tcl_SplitList(interp, conf_data, &conf_count, &conf_lines) != TCL_OK) {
	return NULL;
    }
    if (conf_count != 6) {
	Tcl_AppendResult(interp, "configuration information must have",
			 " exactly six components", NULL);
	free(conf_lines);
	return NULL;
    }
    doc_start = conf_lines[0];
    fixup_line(doc_start);

    doc_end  = conf_lines[1];
    fixup_line(doc_end);

    doc_id_start = conf_lines[2];
    fixup_line(doc_id_start);

    doc_id_end = conf_lines[3];
    fixup_line(doc_id_end);

    text_start = conf_lines[4];
    fixup_line(text_start);

    text_end = conf_lines[5];
    fixup_line(text_end);

    return conf_lines;
}

/* tokenize some strings.  results are a individual words possibly
   with positional information and various kinds of delimiters.

   fwords
	-hex				put out offsets in hexadecimal
	-mark_documents			insert document boundary info
	-mark_sentences			insert sentence boundary markers
	-delimiters <delimiter-spec>	specify how to parse documents
	-words				output word sequence numbers
	-offsets			output word offsets
	-sentences			output sentence numbers for each word
	-count				put out counts for each word
	-unique				list each word once per document

*/

int fwords(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int error_flag = 0;

    int optind = 1;
    int more_args;

    char **conf_lines = NULL;

    count_words = 0;
    output_counts = 0;

    print_word = 0;
    print_position = 0;
    print_sentence = 0;
    number_format  =  "%d";
    
    doc_start = "";
    doc_end = NULL;
    doc_id_start = "";
    doc_id_end = "";
    text_start = "";
    text_end = NULL;
    
    mark_documents = 0;
    mark_sentences = 0;

    if (argv[optind]) more_args = (argv[optind][0] == '-');
    else more_args = 1;
    while (more_args) {
	if (strcmp("-hex", argv[optind]) == 0) {
	    number_format = "%x";
	}
	else if (strcmp("-count", argv[optind]) == 0) {
	    count_words = 1;
	    output_counts = 1;
	    print_sentence = print_position = print_word = 0;
	    mark_sentences = 0;
	}
	else if (strcmp("-unique", argv[optind]) == 0) {
	    count_words = 1;
	    output_counts = 0;
	    print_sentence = print_position = print_word = 0;
	    mark_sentences = 0;
	}
	else if (strcmp("-mark_documents", argv[optind]) == 0) {
	    mark_documents = 1;
	}
	else if (strcmp("-mark_sentences", argv[optind]) == 0) {
	    mark_sentences = 1;
	    count_words = 0;
	}
	else if (strcmp("-delimiters", argv[optind]) == 0) {
	    optind++;
	    conf_lines = configure_delimiters(interp, argv[optind]);
	}
	else if (strcmp("-words", argv[optind]) == 0) {
	    print_word = 1;
	    count_words = 0;
	}
	else if (strcmp("-offsets", argv[optind]) == 0) {
	    print_position = 1;
	    count_words = 0;
	}
	else if (strcmp("-sentences", argv[optind]) == 0) {
	    print_sentence = 1;
	    count_words = 0;
	}
	else if (strcmp("--", argv[optind]) == 0) {
	    optind++;
	    break;
	}
	else {
	    if (argv[optind][0] == '-') {
		error_flag++;
		more_args = 0;
	    }
	}
	optind++;
	if (argv[optind]) {
	    more_args = (argv[optind][0] == '-');
	}
	else more_args = 0;
    }
    if (error_flag) {
	Tcl_AppendResult(interp, 
			 "-hex				put out offsets in hexadecimal (faster!)\n",
			 "-mark_documents			insert document boundary info\n",
			 "-mark_sentences			insert sentence boundary markers\n",
			 "-delimiters <delimiter-spec>	specify how to parse documents\n",
			 "-words				output word sequence numbers\n",
			 "-offsets			output word offsets\n",
			 "-sentences			output sentence numbers for each word\n",
			 "-count			put out counts for each word\n",
			 NULL);
	return TCL_ERROR;
    }

    while (optind < argc) {
	file_position = word_number = 0;
	if (process_file(interp, argv[optind]) != TCL_OK) {
	    return TCL_ERROR;
	}
	optind++;
    }

    if (conf_lines) free(conf_lines);

    return TCL_OK;
}

int Stat_Init(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "fwords", fwords, NULL, NULL);

    return TCL_OK;
}
 
#ifdef STANDALONE
int Stat_AppInit(Tcl_Interp *interp)
    if (Tcl_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }


int main(int argc, char **argv)
{
    Tcl_Main(argc, argv, Stat_AppInit);
 
    return 0;                   /* Needed only to prevent compiler warning. */
}
#endif


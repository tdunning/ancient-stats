#include <tcl.h>

/*
 * ngram_table
 *
 * Synopsis:
 *
 *	 TCL function which Creates an object which can be used to
 *	 keep track of ngrams.  This object can be passed lists of
 *	 ngrams and it will record each one and also return the
 *	 approximate number of ngrams which were already in the table.
 *
 * TCL Arguments:
 *
 *	 name			The name of the object to be created.
 *
 * Assumptions:
 */

static
int ngram_table(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    Tcl_HashTable *ngrams;

    ngrams = malloc(sizeof(*ngrams));
    Tcl_InitHashTable(ngrams);

    return Tcl_CreateCommand(interp, argv[1], ngram_methods,
			     ngrams, ngram_delete);
}

/*
 * ngram_delete
 *
 * Synopsis:
 *
 *	 Function which deallocates an ngram table.
 *
 * Arguments:
 *
 *	 obj		A pointer to the clientdata associated with the
 *			object being deleted.  This clientdata is a
 *	 		pointer to a tcl hash table.
 *
 * Assumptions:
 */
 *	 
static
void ngram_delete(ClientData x)
{
    Tcl_HashTable ngrams;

    ngrams = (Tcl_HashTable) x;
    Tcl_DeleteHashTable(ngrams);
    free(ngrams);
}

/*
 * ngram_methods
 *
 * Synopsis:
 *
 *	 TCL function which handles the actual operation of an ngram
 *	 table.  Each argument to this function is split and each
 *	 element is added to the table.  The number of novel elements
 *	 is returned.
 *
 * TCL Arguments:
 *
 *	 list1, list2...	The lists of strings to put into the
 *	 			table.
 *
 * Assumptions:
 * 
 * 1.  This function should only be called as a result of using an
 *     object created by ngram_table.
 */
static
int ngram_methods(ClientData cldata, Tcl_Interp *interp,
		  int argc, char *argv[])
{
    int i, j, n;
    char **grams;
    Tcl_HashTable ngrams;

    int score;
    
    score = 0;
    ngrams = (Tcl_HashTable) cldata;
    for (i=1;i<argc;i++) {
	if (Tcl_SplitList(interp, argv[i], &n, &grams) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (j=0;j<n;j++) {
	    entry = Tcl_CreateHashEntry(ngrams, grams[j], &new);
	    if (new) {
		Tcl_SetHashValue(entry, 1);
	    }
	    else {
		Tcl_SetHashValue(entry, (int) Tcl_GetHashValue(entry)+1);
		score++;
	    }
	}
	free(grams);
    }

    sprintf(interp->result, "%d", score);
    return TCL_OK;
}

/*
 * count
 *
 * Synopsis:
 *
 *	TCL function which accepts lists of strings and which counts
 *	these strings.  The return value is a list with an even number
 *	of items in it which are the original strings and the number
 *	of times each string appeared.  If the -nocount option is
 *	used, then only the strings are returned which is handy if you
 *	only want to eliminate duplicates.
 *
 * TCL Arguments:
 *
 *	 list1, ... listn	The lists of items to be counted.
 *
 * Assumptions:
 */
static
int count(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int i, base;
    int n;
    char **item;

    int keep_count = 1;

    Tcl_DString r;
    Tcl_HashTable counts;
    Tcl_HashEntry *entry;
    Tcl_InitHashTable(&counts, TCL_STRING_KEYS);

    base = 1;
    while (base < argc && argv[base][0] == '-') {
	if (strcmp(argv[base], "--") == 0) {
	    break;
	}
	else if (strcmp(argv[base], "-nocount") == 0) {
	    keep_count = 0;
	}
	base++;
    }

    for (i=base;i<argc;i++) {
	if (Tcl_SplitList(interp, argv[i], &n, &item) != TCL_OK) {
	    return TCL_ERROR;
	}

	for (j=0;j<n;j++) {
	    entry = Tcl_CreateHashEntry(&counts, item[j], &new);
	    if (new) {
		Tcl_SetHashValue(entry, 1);
	    }
	    else {
		Tcl_SetHashValue(entry, (int) Tcl_GetHashValue(entry)+1);
	    }
	}
	free(item);
    }

    for (entry = Tcl_FirstHashEntry(&counts, &search);
	 entry != NULL;
	 entry = Tcl_NextHashEntry(&search)) {

	Tcl_DStringAppendElement(&r, Tcl_GetHashKey(entry));
	if (keep_count) {
	    sprintf(tmp, "%d", Tcl_GetHashValue(entry));
	    Tcl_DStringAppendElement(&r, tmp);
	}
    }
    Tcl_DeleteHashTable(&counts);
    Tcl_DStringResult(interp &r);
    return TCL_OK;
}

/*
 * cgram
 *
 * Synopsis:
 *
 *	TCL function which accepts strings and which breaks these
 *	strings down into n-long substrings.  The size of these
 *	substrings can be set using the -window option.  If the
 *	-squash_spaces option is used then consecutive whitespace
 *	characters are collapsed into a single character.  If the
 *	-visible option is used, whitespace characters are made
 *	visible by converting them to + signs.
 *
 * TCL Arguments:
 *
 *	 list1, ... listn	The lists of items to be counted.
 *
 * Assumptions:
 */
static
int cgram(ClientData cldata, Tcl_Interp *interp, int argc, char *argv[])
{
    int i, j, arg, base;
    
    int window = 3;
    int squash_spaces = 0;
    int visible = 0;

    Tcl_DString r;

    Tcl_DStringInit(&r);

    base = 1;
    while (base < argc && argv[base][0] == '-') {
	if (strcmp(argv[base], "--") == 0) {
	    break;
	}
	else if (strcmp(argv[base], "-visible") == 0) {
	    visible = 1;
	}
	else if (strcmp(argv[base], "-window") == 0) {
	    if (base < argc-1) {
		if (Tcl_GetInt(interp, argv[base+1], &window) != TCL_OK) {
		    return TCL_ERROR;
		}
		base++;
	    }
	    else {
		Tcl_AppendResult(interp, "Must have window size with -window ",
				 " option", NULL);
		return TCL_ERROR;
	    }
	}
	else if (strcmp(argv[base], "-squash") == 0) {
	    squash_spaces = 1;
	}
	base++;
    }

    for (arg=base;arg<argc;i++) {
	i = j = 0;
	while (argv[arg][i]) {
	    ch = argv[arg][i++];
	    if (isspace(ch)) {
		if (visible) {
		    argv[j++] = '+';
		}
		else {
		    argv[j++] = ch;
		}
		while (squash_spaces && isspace(ch)) {
		    ch = argv[arg][i++];
		}
	    }
	    else {
		argv[arg][j++] = ch;
		ch = argv[arg][i++];
	    }
	}
	for (i=0;i < j-window;i++) {
	    Tcl_DStringAppend(&r, argv[arg]+i, window);
	}
    }

    Tcl_DStringResult(interp, &r);
    return TCL_OK;
}

/*
 * Counter_Init
 *
 * Synopsis:
 *
 *	TCL package initialization function which installs the
 *	commands which are part of the counter package.  These
 *	commands include the cgram, ngram and count commands.
 *
 * Arguments:
 *
 *	 interp		A TCL interpreter into which the commands are
 *			installed. 
 *
 * Assumptions:
 */

int Counter_Init(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "cnt_cgram", cgram, NULL, NULL);
    Tcl_CreateCommand(interp, "cnt_ngram", ngram, NULL, NULL);
    Tcl_CreateCommand(interp, "cnt_count", count, NULL, NULL);

    Tcl_PkgProvide(interp, "counter", "1.0");
    return TCL_OK;
}

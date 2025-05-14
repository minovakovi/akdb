
/*-------------------------------------------------------------------------*/
/**
   @file    iniparser.c
   @author  N. Devillard
   @brief   Parser for ini files.
*/
/*--------------------------------------------------------------------------*/
/*---------------------------- Includes ------------------------------------*/
#include <ctype.h>
#include "iniparser.h"
#include "constants.h"

/*---------------------------- Defines -------------------------------------*/
#define ASCIILINESZ (1024)
#define INI_INVALID_KEY ((char *)-1)

pthread_mutex_t iniParserMutex = PTHREAD_MUTEX_INITIALIZER;

/*---------------------------------------------------------------------------
                        Private to this module
 ---------------------------------------------------------------------------*/
/**
 * This enum stores the status for each parsed line (internal use only).
 */
typedef enum _line_status_
{
    LINE_UNPROCESSED,
    LINE_ERROR,
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_SECTION,
    LINE_VALUE
} line_status;

/*-------------------------------------------------------------------------*/
/**
  @brief    Convert a string to lowercase.
  @param    s   String to convert.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string
  containing a lowercased version of the input string. Do not AK_free
  or modify the returned string! Since the returned string is statically
  allocated, it will be modified at each function call (not re-entrant).
 */
/*--------------------------------------------------------------------------*/
static char *strlwc(const char *s)
{
    static char l[ASCIILINESZ + 1];
    int i;
    AK_PRO;
    if (s == NULL)
    {
        AK_EPI;
        return NULL;
    }
    memset(l, 0, ASCIILINESZ + 1);
    i = 0;
    while (s[i] && i < ASCIILINESZ)
    {
        l[i] = (char)tolower((int)s[i]);
        i++;
    }
    l[ASCIILINESZ] = (char)0;
    AK_EPI;
    return l;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Remove blanks at the beginning and the end of a string.
  @param    s   String to parse.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end and the beg. of the string have been removed.
  Do not AK_free or modify the returned string! Since the returned string
  is statically allocated, it will be modified at each function call
  (not re-entrant).
 */
/*--------------------------------------------------------------------------*/
static char *strstrip(const char *s)
{
    static char l[ASCIILINESZ + 1];
    char *last;
    AK_PRO;
    if (s == NULL)
    {
        AK_EPI;
        return NULL;
    }

    while (isspace((int)*s) && *s)
        s++;
    memset(l, 0, ASCIILINESZ + 1);
    strcpy(l, s);
    last = l + strlen(l);
    while (last > l)
    {
        if (!isspace((int)*(last - 1)))
            break;
        last--;
    }
    *last = (char)0;
    AK_EPI;
    return (char *)l;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get number of sections in a dictionary
  @param    d   Dictionary to examine
  @return   int Number of sections found in dictionary

  This function returns the number of sections found in a dictionary.
  The test to recognize sections is done on the string stored in the
  dictionary: a section name is given as "section" whereas a key is
  stored as "section:key", thus the test looks for entries that do not
  contain a colon.

  This clearly fails in the case a section name contains a colon, but
  this should simply be avoided.

  This function returns -1 in case of error.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getnsec(dictionary *d)
{
    int i;
    int nsec;
    AK_PRO;
    if (d == NULL)
    {
        AK_EPI;
        return -1;
    }
    nsec = 0;
    for (i = 0; i < d->size; i++)
    {
        if (d->key[i] == NULL)
            continue;
        if (strchr(d->key[i], ':') == NULL)
        {
            nsec++;
        }
    }
    AK_EPI;
    return nsec;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get name for section n in a dictionary.
  @param    d   Dictionary to examine
  @param    n   Section number (from 0 to nsec-1).
  @return   Pointer to char string

  This function locates the n-th section in a dictionary and returns
  its name as a pointer to a string statically allocated inside the
  dictionary. Do not AK_free or modify the returned string!

  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/
char *iniparser_getsecname(dictionary *d, int n)
{
    int i;
    int foundsec;
    AK_PRO;
    if (d == NULL || n < 0)
    {
        AK_EPI;
        return NULL;
    }
    foundsec = 0;
    for (i = 0; i < d->size; i++)
    {
        if (d->key[i] == NULL)
            continue;
        if (strchr(d->key[i], ':') == NULL)
        {
            foundsec++;
            if (foundsec > n)
                break;
        }
    }
    if (foundsec <= n)
    {
        AK_EPI;
        return NULL;
    }
    AK_EPI;
    return d->key[i];
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump.
  @param    f   Opened file pointer to dump to.
  @return   void

  This function prints out the contents of a dictionary, one element by
  line, onto the provided file pointer. It is OK to specify @c stderr
  or @c stdout as output files. This function is meant for debugging
  purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump(dictionary *d, FILE *f)
{
    int i;
    AK_PRO;
    if (d == NULL || f == NULL)
    {
        AK_EPI;
        return;
    }
    for (i = 0; i < d->size; i++)
    {
        if (d->key[i] == NULL)
            continue;
        if (d->val[i] != NULL)
        {
            fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
        }
        else
        {
            fprintf(f, "[%s]=UNDEF\n", d->key[i]);
        }
    }
    AK_EPI;
    return;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary to a loadable ini file
  @param    d   Dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given dictionary into a loadable ini file.
  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump_ini(dictionary *d, FILE *f)
{
    int i;
    int nsec;
    char *secname;
    AK_PRO;

    if (d == NULL || f == NULL)
    {
        AK_EPI;
        return;
    }

    nsec = iniparser_getnsec(d);
    if (nsec < 1)
    {
        /* No section in file: dump all keys as they are */
        for (i = 0; i < d->size; i++)
        {
            if (d->key[i] == NULL)
                continue;
            fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
        }
        return;
    }
    for (i = 0; i < nsec; i++)
    {
        secname = iniparser_getsecname(d, i);
        iniparser_dumpsection_ini(d, secname, f);
    }
    fprintf(f, "\n");
    AK_EPI;
    return;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary section to a loadable ini file
  @param    d   Dictionary to dump
  @param    s   Section name of dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given section of a given dictionary into a loadable ini
  file.  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dumpsection_ini(dictionary *d, char *s, FILE *f)
{
    int j;
    char keym[ASCIILINESZ + 1];
    int seclen;
    AK_PRO;
    if (d == NULL || f == NULL)
    {
        AK_EPI;
        return;
    }
    if (!iniparser_find_entry(d, s))
    {
        AK_EPI;
        return;
    }

    seclen = (int)strlen(s);
    fprintf(f, "\n[%s]\n", s);
    sprintf(keym, "%s:", s);
    for (j = 0; j < d->size; j++)
    {
        if (d->key[j] == NULL)
            continue;
        if (!strncmp(d->key[j], keym, seclen + 1))
        {
            fprintf(f,
                    "%s = %s\n",
                    d->key[j] + seclen + 1,
                    d->val[j] ? d->val[j] : "");
        }
    }
    fprintf(f, "\n");
    AK_EPI;
    return;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the number of keys in a section of a dictionary.
  @param    d   Dictionary to examine
  @param    s   Section name of dictionary to examine
  @return   Number of keys in section
 */
/*--------------------------------------------------------------------------*/
int iniparser_getsecnkeys(dictionary *d, char *s)
{
    int seclen, nkeys;
    char keym[ASCIILINESZ + 1];
    int j;
    AK_PRO;
    nkeys = 0;

    if (d == NULL)
    {
        AK_EPI;
        return nkeys;
    }
    if (!iniparser_find_entry(d, s))
    {
        AK_EPI;
        return nkeys;
    }

    seclen = (int)strlen(s);
    sprintf(keym, "%s:", s);

    for (j = 0; j < d->size; j++)
    {
        if (d->key[j] == NULL)
            continue;
        if (!strncmp(d->key[j], keym, seclen + 1))
            nkeys++;
    }
    AK_EPI;
    return nkeys;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the number of keys in a section of a dictionary.
  @param    d   Dictionary to examine
  @param    s   Section name of dictionary to examine
  @return   pointer to statically allocated character strings

  This function queries a dictionary and finds all keys in a given section.
  Each pointer in the returned char pointer-to-pointer is pointing to
  a string allocated in the dictionary; do not AK_free or modify them.

  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/
char **iniparser_getseckeys(dictionary *d, char *s)
{

    char **keys;

    int i, j;
    char keym[ASCIILINESZ + 1];
    int seclen, nkeys;
    AK_PRO;
    keys = NULL;

    if (d == NULL)
    {
        AK_EPI;
        return keys;
    }
    if (!iniparser_find_entry(d, s))
    {
        AK_EPI;
        return keys;
    }

    nkeys = iniparser_getsecnkeys(d, s);

    keys = (char **)AK_malloc(nkeys * sizeof(char *));

    seclen = (int)strlen(s);
    sprintf(keym, "%s:", s);

    i = 0;

    for (j = 0; j < d->size; j++)
    {
        if (d->key[j] == NULL)
            continue;
        if (!strncmp(d->key[j], keym, seclen + 1))
        {
            keys[i] = d->key[j];
            i++;
        }
    }
    AK_EPI;
    return keys;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key
  @param    d       Dictionary to search
  @param    key     Key string to look for
  @param    def     Default value to return if key not found.
  @return   pointer to statically allocated character string

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the pointer passed as 'def' is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not AK_free or modify it.
 */
/*--------------------------------------------------------------------------*/
char *iniparser_getstring(dictionary *d, const char *key, char *def)
{
    char *lc_key;
    char *sval;
    AK_PRO;
    pthread_mutex_lock(&iniParserMutex);

    if (d == NULL || key == NULL)
    {
        pthread_mutex_unlock(&iniParserMutex);
        AK_EPI;
        return def;
    }

    lc_key = strlwc(key);
    sval = dictionary_get(d, lc_key, def);
    pthread_mutex_unlock(&iniParserMutex);
    AK_EPI;
    return sval;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to an int
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  Supported values for integers include the usual C notation
  so decimal, octal (starting with 0) and hexadecimal (starting with 0x)
  are supported. Examples:

  "42"      ->  42
  "042"     ->  34 (octal -> decimal)
  "0x42"    ->  66 (hexa  -> decimal)

  Warning: the conversion may overflow in various ways. Conversion is
  totally outsourced to strtol(), see the associated man page for overflow
  handling.

  Credits: Thanks to A. Becker for suggesting strtol()
 */
/*--------------------------------------------------------------------------*/
int iniparser_getint(dictionary *d, const char *key, int notfound)
{
    char *str;
    AK_PRO;
    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str == INI_INVALID_KEY)
    {
        AK_EPI;
        return notfound;
    }
    AK_EPI;
    return (int)strtol(str, NULL, 0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a double
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   double

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
double iniparser_getdouble(dictionary *d, const char *key, double notfound)
{
    char *str;
    AK_PRO;
    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str == INI_INVALID_KEY)
    {
        AK_EPI;
        return notfound;
    }
    AK_EPI;
    return atof(str);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a boolean
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  A true boolean is found if one of the following is matched:

  - A string starting with 'y'
  - A string starting with 'Y'
  - A string starting with 't'
  - A string starting with 'T'
  - A string starting with '1'

  A false boolean is found if one of the following is matched:

  - A string starting with 'n'
  - A string starting with 'N'
  - A string starting with 'f'
  - A string starting with 'F'
  - A string starting with '0'

  The notfound value returned if no boolean is identified, does not
  necessarily have to be 0 or 1.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getboolean(dictionary *d, const char *key, int notfound)
{
    char *c;
    int ret;
    AK_PRO;
    c = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (c == INI_INVALID_KEY)
    {
        AK_EPI;
        return notfound;
    }
    if (c[0] == 'y' || c[0] == 'Y' || c[0] == '1' || c[0] == 't' || c[0] == 'T')
    {
        ret = 1;
    }
    else if (c[0] == 'n' || c[0] == 'N' || c[0] == '0' || c[0] == 'f' || c[0] == 'F')
    {
        ret = 0;
    }
    else
    {
        ret = notfound;
    }
    AK_EPI;
    return ret;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Finds out if a given entry exists in a dictionary
  @param    ini     Dictionary to search
  @param    entry   Name of the entry to look for
  @return   integer 1 if entry exists, 0 otherwise

  Finds out if a given entry exists in the dictionary. Since sections
  are stored as keys with NULL associated values, this is the only way
  of querying for the presence of sections in a dictionary.
 */
/*--------------------------------------------------------------------------*/
int iniparser_find_entry(
    dictionary *ini,
    const char *entry)
{
    int found = 0;
    AK_PRO;
    if (iniparser_getstring(ini, entry, INI_INVALID_KEY) != INI_INVALID_KEY)
    {
        found = 1;
    }
    AK_EPI;
    return found;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Set an entry in a dictionary.
  @param    ini     Dictionary to modify.
  @param    entry   Entry to modify (entry name)
  @param    val     New value to associate to the entry.
  @return   int 0 if Ok, -1 otherwise.

  If the given entry can be found in the dictionary, it is modified to
  contain the provided value. If it cannot be found, -1 is returned.
  It is Ok to set val to NULL.
 */
/*--------------------------------------------------------------------------*/
int iniparser_set(dictionary *ini, const char *entry, const char *val)
{
    int ret;
    AK_PRO;
    ret = dictionary_set(ini, strlwc(entry), val);
    AK_EPI;
    return ret;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete an entry in a dictionary
  @param    ini     Dictionary to modify
  @param    entry   Entry to delete (entry name)
  @return   void

  If the given entry can be found, it is deleted from the dictionary.
 */
/*--------------------------------------------------------------------------*/
void iniparser_unset(dictionary *ini, const char *entry)
{
    AK_PRO;
    dictionary_unset(ini, strlwc(entry));
    AK_EPI;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Load a single line from an INI file
  @param    input_line  Input line, may be concatenated multi-line input
  @param    section     Output space to store section
  @param    key         Output space to store key
  @param    value       Output space to store value
  @return   line_status value
 */
/*--------------------------------------------------------------------------*/
static line_status iniparser_line(
    const char *input_line,
    char *section,
    char *key,
    char *value)
{
    line_status sta;
    char line[ASCIILINESZ + 1];
    int len;

    AK_PRO;

    strcpy(line, strstrip(input_line));
    len = (int)strlen(line);

    sta = LINE_UNPROCESSED;
    if (len < 1)
    {
        /* Empty line */
        sta = LINE_EMPTY;
    }
    else if (line[0] == '#' || line[0] == ';')
    {
        /* Comment line */
        sta = LINE_COMMENT;
    }
    else if (line[0] == '[' && line[len - 1] == ']')
    {
        /* Section name */
        sscanf(line, "[%[^]]", section);
        strcpy(section, strstrip(section));
        strcpy(section, strlwc(section));
        sta = LINE_SECTION;
    }
    else if (sscanf(line, "%[^=] = \"%[^\"]\"", key, value) == 2 || sscanf(line, "%[^=] = '%[^\']'", key, value) == 2 || sscanf(line, "%[^=] = %[^;#]", key, value) == 2)
    {
        /* Usual key=value, with or without comments */
        strcpy(key, strstrip(key));
        strcpy(key, strlwc(key));
        strcpy(value, strstrip(value));
        /*
         * sscanf cannot handle '' or "" as empty values
         * this is done here
         */
        if (!strcmp(value, "\"\"") || (!strcmp(value, "''")))
        {
            value[0] = 0;
        }
        sta = LINE_VALUE;
    }
    else if (sscanf(line, "%[^=] = %[;#]", key, value) == 2 || sscanf(line, "%[^=] %[=]", key, value) == 2)
    {
        /*
         * Special cases:
         * key=
         * key=;
         * key=#
         */
        strcpy(key, strstrip(key));
        strcpy(key, strlwc(key));
        value[0] = 0;
        sta = LINE_VALUE;
    }
    else
    {
        /* Generate syntax error */
        sta = LINE_ERROR;
    }
    AK_EPI;
    return sta;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini files. This function is called, providing
  the name of the file to be read. It returns a dictionary object that
  should not be accessed directly, but through accessor functions
  instead.

  The returned dictionary must be AK_freed using iniparser_AK_freedict().
 */
/*--------------------------------------------------------------------------*/
dictionary *iniparser_load(const char *ininame)
{
    FILE *in;

    char line[ASCIILINESZ + 1];
    char section[ASCIILINESZ + 1];
    char key[ASCIILINESZ + 1];
    char tmp[ASCIILINESZ + 1];
    char val[ASCIILINESZ + 1];

    int last = 0;
    int len;
    int lineno = 0;
    int errs = 0;

    dictionary *dict;
    AK_PRO;
    if ((in = fopen(ininame, "r")) == NULL)
    {
        fprintf(stderr, "iniparser: cannot open %s\n", ininame);
        AK_EPI;
        return NULL;
    }

    dict = dictionary_new(0);
    if (!dict)
    {
        fclose(in);
        AK_EPI;
        return NULL;
    }

    memset(line, 0, ASCIILINESZ);
    memset(section, 0, ASCIILINESZ);
    memset(key, 0, ASCIILINESZ);
    memset(val, 0, ASCIILINESZ);
    last = 0;

    while (fgets(line + last, ASCIILINESZ - last, in) != NULL)
    {
        lineno++;
        len = (int)strlen(line) - 1;
        if (len == 0)
            continue;
        /* Safety check against buffer overflows */

        if (line[len] != '\n' && !feof(in))
        {
            fprintf(stderr,
                    "iniparser: input line too long in %s (%d)\n",
                    ininame,
                    lineno);
            dictionary_del(dict);
            fclose(in);
            AK_EPI;
            return NULL;
        }

        /* Get rid of \n and spaces at end of line */

        while ((len >= 0) &&
               ((line[len] == '\n') || (isspace(line[len]))))
        {
            line[len] = 0;
            len--;
        }
        /* Detect multi-line */
        if (line[len] == '\\')
        {
            /* Multi-line value */
            last = len;
            continue;
        }
        else
        {
            last = 0;
        }
        switch (iniparser_line(line, section, key, val))
        {
        case LINE_EMPTY:
        case LINE_COMMENT:
            break;

        case LINE_SECTION:
            errs = dictionary_set(dict, section, NULL);
            break;

        case LINE_VALUE:
            sprintf(tmp, "%s:%s", section, key);
            errs = dictionary_set(dict, tmp, val);
            break;

        case LINE_ERROR:
            fprintf(stderr, "iniparser: syntax error in %s (%d):\n",
                    ininame,
                    lineno);
            fprintf(stderr, "-> %s\n", line);
            errs++;
            break;

        default:
            break;
        }
        memset(line, 0, ASCIILINESZ);
        last = 0;
        if (errs < 0)
        {
            fprintf(stderr, "iniparser: memory allocation failure\n");
            break;
        }
    }
    if (errs)
    {
        dictionary_del(dict);
        dict = NULL;
    }
    fclose(in);
    AK_EPI;
    return dict;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Free all memory associated to an ini dictionary
  @param    d Dictionary to AK_free
  @return   void

  Free all memory associated to an ini dictionary.
  It is mandatory to call this function before the dictionary object
  gets out of the current context.
 */
/*--------------------------------------------------------------------------*/
void iniparser_AK_freedict(dictionary *d)
{
    AK_PRO;
    dictionary_del(d);
    AK_EPI;
}

dictionary *AK_config;
/*
char * AK_config_get(dictionary * d, char * key, char * def)
{
    char * lc_key ;
    char * sval;

    if (AK_config==NULL || key==NULL)
        return def ;

    lc_key = strlwc(key);
    sval = dictionary_get(d, lc_key, def);
    return sval ;
}
*/

// char * DB_FILE;

void AK_inflate_config()
{
    AK_PRO;
    AK_config = iniparser_load("config.ini");
    AK_EPI;
    // DB_FILE = AK_config_get(AK_config,"general:db_file", NULL);
    // printf("DB_FILE: %s \n",DB_FILE);
}

/*
int main(int argc, char *argv[]){
    //AK_inflate_config();
    AK_config = iniparser_load("config.ini");
 FILE * test;
 test = fopen("test.ini", "w");
    iniparser_dump_ini(AK_config, test);
    fclose(test);
    dictionary * AK_test;
    AK_test = iniparser_load("test.ini");
    char * val;
    val = AK_config_get(AK_test,"general:db_file", NULL);
    printf("DB_FILE: %s \n",val);
    }
*/

// vim: set ts=4 et sw=4 tw=75

/**
 * @author Marko Belusic
 * @brief Function for testing the implementation
 */
TestResult AK_iniparser_test()
{

    int succesfulTests = 0;
    int failedTests = 0;
    AK_PRO;

    // test if creation of dictionary is working
    printf("Testing if creation of dictionary is working\n");
    dictionary *dict_to_test = NULL;
    dict_to_test = dictionary_new(15);
    if (dict_to_test != NULL)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // test if adding a value is working
    printf("Testing if adding a value is working\n");
    iniparser_set(dict_to_test, "people", NULL);
    iniparser_set(dict_to_test, "people:paul", "34");
    iniparser_set(dict_to_test, "people:ariana", "38");
    iniparser_set(dict_to_test, "people:joe", "52");
    iniparser_set(dict_to_test, "cities", NULL);
    iniparser_set(dict_to_test, "cities:London", "25");
    if (iniparser_find_entry(dict_to_test, "people:paul") == 1)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if it is the correct value
    printf("Testing if getting a value is working\n");
    if (strcmp(iniparser_getstring(dict_to_test, "people:paul", NULL), "34") == 0)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if overwriting a value is working
    printf("Testing if value can be overwritten\n");
    iniparser_set(dict_to_test, "people:paul", "23");
    if (strcmp(iniparser_getstring(dict_to_test, "people:paul", NULL), "23") == 0)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if unset a key is working
    printf("Testing if key can be unset\n");
    iniparser_unset(dict_to_test, "people:paul");
    if (iniparser_find_entry(dict_to_test, "people:paul") == 0)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if number of sections is correct
    printf("Testing if number of sections is correct when using get sections\n");
    if (iniparser_getnsec(dict_to_test) == 2)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if get name of a section is working properly
    printf("Testing if getting a section name by n is working\n");
    if (strcmp(iniparser_getsecname(dict_to_test, 1), "cities") == 0)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if getting number of keys from section is correct
    printf("Testing if number of keys in given section name is correct\n");
    if (iniparser_getsecnkeys(dict_to_test, "people") == 2 && iniparser_getsecnkeys(dict_to_test, "cities") == 1)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // test iniparser_getseckeys
    printf("Test if getseckeys returns correct key in section\n");

    int getseckeys_test_success = 0;
    int number_of_keys = iniparser_getsecnkeys(dict_to_test, "people");
    char **keys = iniparser_getseckeys(dict_to_test, "people");

    for (int i = 0; i < number_of_keys; i++)
    {
        if (strcmp(keys[i], "people:ariana") == 0)
        {
            getseckeys_test_success = 1;
            break;
        }
    }

    if (getseckeys_test_success == 1)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf("Failed\n\n");
    }

    // printing all contents of dictionary
    printf("Printing all contents of created dictionary\n");
    iniparser_dump(dict_to_test, stdout);
    printf("\n");
    iniparser_dump_ini(dict_to_test, stdout);

    // check if loading a file into dict is working properly
    printf("Testing if loading a file into dict is working properly\n");
    FILE *fptr;
    fptr = fopen("testFileForIniParser.txt", "w");
    if (fptr == NULL)
    {
        failedTests++;
        printf(FAIL_MESSAGE);
    }
    else
    {
        fprintf(fptr, "%s", "[section1]\n");
        fprintf(fptr, "%s", "key1 = \"value1\"\n");
        fprintf(fptr, "%s", "key2 = \"value2\"\n");
        fprintf(fptr, "%s", "[section2]\n");
        fprintf(fptr, "%s", "key1 = \"value1\"\n");
        fprintf(fptr, "%s", "key2 = \"value2\"\n");
        fclose(fptr);
        dictionary *dict_to_test2 = NULL;
        dict_to_test2 = iniparser_load("testFileForIniParser.txt");
        remove("testFileForIniParser.txt");
        if (dict_to_test2 != NULL)
        {
            succesfulTests++;
            printf(SUCCESS_MESSAGE);
            // printing all contents of loaded dictionary
            printf("Printing all contents of loaded dictionary \n");
            iniparser_dump(dict_to_test2, stdout);
            printf("\n");
            iniparser_dump_ini(dict_to_test2, stdout);
            // cleaning dictionary
            iniparser_AK_freedict(dict_to_test2);
        }
        else
        {
            failedTests++;
            printf(FAIL_MESSAGE);
        }
    }

    // test strlwc
    printf("\nTesting if string is correctly lowercased\n");

    int strlwc_test_success = 1;
    char akdb[20] = "AKDB";
    char *akdb_lwc = strlwc(akdb);

    if (strcmp(akdb_lwc, "akdb") == 0)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf("Failed\n\n");
    }

    // test strstrip
    printf("\nTesting if string is correctly striped\n");

    strcpy(akdb, "     AKDB     ");
    char *akdb_strip = strstrip(akdb);

    if (strcmp(akdb_strip, "AKDB") == 0)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf("Failed\n\n");
    }

    // test iniparser_getint
    printf("\nTesting if iniparset_getint returns correctly value\n");

    iniparser_set(dict_to_test, "number", "10");
    int number = iniparser_getint(dict_to_test, "number", 0);

    if (number == 10)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf("Failed\n\n");
    }

    // test iniparser_getdouble
    printf("\nTesting if iniparser_getdouble returns correctly value\n");

    iniparser_set(dict_to_test, "double_number", "11.239");
    double double_number = iniparser_getdouble(dict_to_test, "double_number", 0);

    if (double_number == 11.239)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf("Failed\n\n");
    }

    // test iniparser_getboolean
    printf("\nTesting if iniparser_getboolean returns correctly value\n");

    iniparser_set(dict_to_test, "boolean", "T");
    int boolean = iniparser_getboolean(dict_to_test, "boolean", 0);

    if (boolean == 1)
    {
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else
    {
        failedTests++;
        printf("Failed\n\n");
    }

    // cleaning dictionary
    iniparser_AK_freedict(dict_to_test);
    AK_EPI;

    return TEST_result(succesfulTests, failedTests);
}

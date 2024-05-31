/*-------------------------------------------------------------------------*/
/**
   @file    dictionary.c
   @author  N. Devillard
   @brief   Implements a dictionary for string variables.

   This module implements a simple dictionary object, i.e. a list
   of string/string associations. This object is useful to store e.g.
   informations retrieved from a configuration file (ini files).
*/
/*--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/
#include "dictionary.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/** Maximum value size for integers and doubles. */
#define MAXVALSZ    1024

/** Minimal allocated number of entries in a dictionary */
#define DICTMINSZ   128

/** Invalid key token */
#define DICT_INVALID_KEY    ((char*)-1)

//* Debug */
//#define TESTDIC

/*---------------------------------------------------------------------------
                            Private functions
 ---------------------------------------------------------------------------*/

/* Doubles the allocated size associated to a pointer */
/* 'size' is the current allocated size. */
static void * mem_double(void * ptr, int size)
{
    void * newptr ;
    AK_PRO;
    newptr = AK_calloc(2*size, 1);
    if (newptr==NULL) {
        AK_EPI;
        return NULL ;
    }
    memcpy(newptr, ptr, size);
    AK_free(ptr);
    AK_EPI;
    return newptr ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Duplicate a string
  @param    s String to duplicate
  @return   Pointer to a newly allocated string, to be AK_freed with AK_free()

  This is a replacement for strdup(). This implementation is provided
  for systems that do not have it.
 */
/*--------------------------------------------------------------------------*/
static char * xstrdup(const char * s)
{
    char * t ;
    AK_PRO;
    if (!s){
        AK_EPI;
        return NULL ;
    }
    t = (char*)AK_malloc(strlen(s)+1) ;
    if (t) {
        strcpy(t,s);
    }
    AK_EPI;
    return t ;
}

/*---------------------------------------------------------------------------
                            Function codes
 ---------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the hash key for a string.
  @param    key     Character string to use for key.
  @return   1 unsigned int on at least 32 bits.

  This hash function has been taken from an Article in Dr Dobbs Journal.
  This is normally a collision-AK_free function, distributing keys evenly.
  The key is stored anyway in the struct so that collision can be avoided
  by comparing the key itself in last resort.
 */
/*--------------------------------------------------------------------------*/
unsigned dictionary_hash(const char * key)
{
    int         len ;
    unsigned    hash ;
    int         i ;
    AK_PRO;

    len = strlen(key);
    for (hash=0, i=0 ; i<len ; i++) {
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
    AK_EPI;
    return hash ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Create a new dictionary object.
  @param    size    Optional initial size of the dictionary.
  @return   1 newly allocated dictionary objet.

  This function allocates a new dictionary object of given size and returns
  it. If you do not know in advance (roughly) the number of entries in the
  dictionary, give size=0.
 */
/*--------------------------------------------------------------------------*/
dictionary *dictionary_new(int size)
{
    dictionary  *d;
    AK_PRO;
    d = (dictionary *)AK_calloc(1, sizeof(dictionary));
    /* If no size was specified, allocate space for DICTMINSZ */
    if (size < DICTMINSZ) size = DICTMINSZ ;

    if (!d) {
        AK_EPI;
        return NULL;
    }

    d->size = size;
    d->val  = (char **)AK_calloc(size, sizeof(char*));
    d->key  = (char **)AK_calloc(size, sizeof(char*));
    d->hash = (unsigned int *) AK_calloc(size, sizeof(unsigned));
    AK_EPI;
    return d ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete a dictionary object
  @param    d   dictionary object to deallocate.
  @return   void

  Deallocate a dictionary object and all memory associated to it.
 */
/*--------------------------------------------------------------------------*/
void dictionary_del(dictionary * d)
{
    int     i ;
    AK_PRO;
    if (d==NULL){
        AK_EPI;
        return ;
    }
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]!=NULL)
            AK_free(d->key[i]);
        if (d->val[i]!=NULL)
            AK_free(d->val[i]);
    }
    AK_free(d->val);
    AK_free(d->key);
    AK_free(d->hash);
    AK_free(d);
    AK_EPI;
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get a value from a dictionary.
  @param    d       dictionary object to search.
  @param    key     Key to look for in the dictionary.
  @param    def     Default value to return if key not found.
  @return   1 pointer to internally allocated character string.

  This function locates a key in a dictionary and returns a pointer to its
  value, or the passed 'def' pointer if no such key can be found in
  dictionary. The returned character pointer points to data internal to the
  dictionary object, you should not try to AK_free it or modify it.
 */
/*--------------------------------------------------------------------------*/
char * dictionary_get(dictionary * d, const char * key, char * def)
{
    unsigned    hash ;
    int         i ;
    AK_PRO;
    hash = dictionary_hash(key);
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        /* Compare hash */
        if (hash==d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, d->key[i])) {
                AK_EPI;
                return d->val[i] ;
            }
        }
    }
    AK_EPI;
    return def ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Set a value in a dictionary.
  @param    d       dictionary object to modify.
  @param    key     Key to modify or add.
  @param    val     Value to add.
  @return   int     0 if Ok, anything else otherwise

  If the given key is found in the dictionary, the associated value is
  replaced by the provided one. If the key cannot be found in the
  dictionary, it is added to it.

  It is Ok to provide a NULL value for val, but NULL values for the dictionary
  or the key are considered as errors: the function will return immediately
  in such a case.

  Notice that if you dictionary_set a variable to NULL, a call to
  dictionary_get will return a NULL value: the variable will be found, and
  its value (NULL) is returned. In other words, setting the variable
  content to NULL is equivalent to deleting the variable from the
  dictionary. It is not possible (in this implementation) to have a key in
  the dictionary without value.

  This function returns non-zero in case of failure.
 */
/*--------------------------------------------------------------------------*/
int dictionary_set(dictionary * d, const char * key, const char * val)
{
    int         i ;
    unsigned    hash ;
    AK_PRO;
    if (d==NULL || key==NULL) return -1 ;

    /* Compute hash for this key */
    hash = dictionary_hash(key) ;
    /* Find if value is already in dictionary */
    if (d->n>0) {
        for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
            if (hash==d->hash[i]) { /* Same hash value */
                if (!strcmp(key, d->key[i])) {   /* Same key */
                    /* Found a value: modify and return */
                    if (d->val[i]!=NULL)
                        AK_free(d->val[i]);
                    d->val[i] = val ? xstrdup(val) : NULL ;
                    /* Value has been modified: return */
                    AK_EPI;
                    return 0 ;
                }
            }
        }
    }
    /* Add a new value */
    /* See if dictionary needs to grow */
    if (d->n==d->size) {

        /* Reached maximum size: AK_reallocate dictionary */
        d->val  = (char **)mem_double(d->val,  d->size * sizeof(char*)) ;
        d->key  = (char **)mem_double(d->key,  d->size * sizeof(char*)) ;
        d->hash = (unsigned int *)mem_double(d->hash, d->size * sizeof(unsigned)) ;
        if ((d->val==NULL) || (d->key==NULL) || (d->hash==NULL)) {
            /* Cannot grow dictionary */
            AK_EPI;
            return -1 ;
        }
        /* Double size */
        d->size *= 2 ;
    }

    /* Insert key in the first empty slot. Start at d->n and wrap at
       d->size. Because d->n < d->size this will necessarily
       terminate. */
    for (i=d->n ; d->key[i] ; ) {
        if(++i == d->size) i = 0;
    }
    /* Copy key */
    d->key[i]  = xstrdup(key);
    d->val[i]  = val ? xstrdup(val) : NULL ;
    d->hash[i] = hash;
    d->n ++ ;
    AK_EPI;
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete a key in a dictionary
  @param    d       dictionary object to modify.
  @param    key     Key to remove.
  @return   void

  This function deletes a key in a dictionary. Nothing is done if the
  key cannot be found.
 */
/*--------------------------------------------------------------------------*/
void dictionary_unset(dictionary * d, const char * key)
{
    unsigned    hash ;
    int         i ;
    AK_PRO;
    if (key == NULL) {
        AK_EPI;
        return;
    }

    hash = dictionary_hash(key);
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        /* Compare hash */
        if (hash==d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, d->key[i])) {
                /* Found key */
                break ;
            }
        }
    }
    if (i>=d->size){
        /* Key not found */
        AK_EPI;
        return ;
    }

    AK_free(d->key[i]);
    d->key[i] = NULL ;
    if (d->val[i]!=NULL) {
        AK_free(d->val[i]);
        d->val[i] = NULL ;
    }
    d->hash[i] = 0 ;
    d->n -- ;
    AK_EPI;
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump
  @param    f   Opened file pointer.
  @return   void

  Dumps a dictionary onto an opened file pointer. Key pairs are printed out
  as @c [Key]=[Value], one per line. It is Ok to provide stdout or stderr as
  output file pointers.
 */
/*--------------------------------------------------------------------------*/
void dictionary_dump(dictionary * d, FILE * out)
{
    int     i ;
    AK_PRO;
    if (d==NULL || out==NULL){
        AK_EPI;
        return ;
    }
    if (d->n<1) {
        fprintf(out, "empty dictionary\n");
        AK_EPI;
        return ;
    }
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]) {
            fprintf(out, "%20s\t[%s]\n",
                    d->key[i],
                    d->val[i] ? d->val[i] : "UNDEF");
        }
    }
    AK_EPI;
    return ;
}

/**
 * @author Marko Belusic
 * @brief Function for testing the implementation
 */
TestResult AK_dictionary_test(){
	
    int succesfulTests = 0;
    int failedTests = 0;
    AK_PRO;

    // test if creation of dictionary is working
    printf("Testing if creation of dictionary is working\n");
    dictionary * dict_to_test = NULL;
    dict_to_test = dictionary_new(15);
    if(dict_to_test != NULL){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }else{
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // test if adding a value is working
    printf("Testing if adding a value in dict is working\n");
    dictionary_set(dict_to_test,"john","22");
    dictionary_set(dict_to_test,"paul","34");
    dictionary_set(dict_to_test,"ariana","38");
    dictionary_set(dict_to_test,"joe","52");
    if(dictionary_get(dict_to_test, "john",NULL) != NULL){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }else{
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if it is the correct value
    printf("Testing if we can get the correct value from key\n");
    if(strcmp(dictionary_get(dict_to_test, "john",NULL),"22") == 0){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }else{
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if overwriting a value is working
    printf("Testing if we can overwrite value\n");
    dictionary_set(dict_to_test,"john","23");
    if(strcmp(dictionary_get(dict_to_test, "john",NULL),"23") == 0){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }else{
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    // check if unset a key is working
    printf("Testing if key can be unset\n");
    dictionary_unset(dict_to_test, "john");
    if(dictionary_get(dict_to_test, "john",NULL) == NULL){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }else{
        failedTests++;
        printf(FAIL_MESSAGE);
    }

    //printing all contents of dictionary
    printf("Printing contents of created dictionary\n");
    dictionary_dump(dict_to_test, stdout);

    //cleaning dictionary
    dictionary_del(dict_to_test);

    // test mem_double
    printf("\nTesting if doubled memory have initialized bits on zero\n");

    int mem_double_test_success = 1;
    int number_of_elements = 10;
    int size_of_element = sizeof(int);

    int* ptr = (int*)AK_calloc(number_of_elements, size_of_element);
    int* new_ptr = (int *)mem_double(ptr,  number_of_elements * size_of_element);
 
    for(int i=0; i<number_of_elements*2; i++){
        if(new_ptr[i] != 0){
            mem_double_test_success = 0;
            break;
        }
    }

    if(mem_double_test_success == 1){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else{
        failedTests++;
        printf("Failed\n\n");
    }

    // test xstrdup
    printf("\nTesting if string is correctly duplicated\n");

    int xstrdup_test_success = 1;
    char akdb[20]="AKDB";
    char *akdb_copy = xstrdup(akdb);

    for(int i=0; i<strlen(akdb); i++){
        if(akdb[i] != akdb_copy[i]){
            xstrdup_test_success = 0;
            break;
        }
    }

    if(xstrdup_test_success == 1){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else{
        failedTests++;
        printf("Failed\n\n");
    }

    // test dictionary_hash
    printf("\nTesting if dictionary hash is correctly calculated\n");
    unsigned akdb_expected_hash = 4194467538;
    unsigned akdb_actual_hash = dictionary_hash(akdb);

    if(akdb_actual_hash == akdb_expected_hash){
        succesfulTests++;
        printf(SUCCESS_MESSAGE);
    }
    else{
        failedTests++;
        printf("Failed\n\n");
    }

    AK_EPI;

	return TEST_result(succesfulTests, failedTests);
}

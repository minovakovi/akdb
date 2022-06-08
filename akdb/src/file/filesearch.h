/**
@file filesearch.h Header file provides data structures, functions and defines for file searching
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef FILESEARCH
#define FILESEARCH

#include "../auxi/test.h"
#include "../mm/memoman.h"
#include "files.h"
#include "../auxi/mempro.h"

#define SEARCH_NULL       0
#define SEARCH_ALL        1
#define SEARCH_PARTICULAR 2
#define SEARCH_RANGE      3

/**
 * @author Unknown
 * @struct search_params
 * @brief Structure that contains attribute name, lower and upper data value, special(NULL or *) which is input for AK_equisearch_unsorted and  		AK_rangesearch_unsorted
 */
typedef struct {
    /// name of attribute
    char *szAttribute;
    /// pointer to lower value of search range
    void *pData_lower;
    /// pointer to upper value of search range
    void *pData_upper;
    /// if searching for NULL values, set to SEARCH_NULL, all values -> SEARCH_ALL, particular value -> SEARCH_PARTICULAR, range of values -> SEARCH_RANGE
    int iSearchType;
} search_params;

/**
  * @author Unknown
  * @struct search_result
  * @brief Structure which represents search result of AK_equisearch_unsorted and AK_rangesearch_unsorted
 */
typedef struct {
    /// array of tuple addresses
    int *aiTuple_addresses;
    /// array of blocks to which the tuple addresses are relative
    int *aiBlocks;
    /// number of tuple addresses/blocks in corresponding arrays
    int iNum_tuple_addresses;
    /// array of indexes of searched-for attributes
    int *aiSearch_attributes;
    /// number of searched-for attributes in array
    int iNum_search_attributes;
    /// number of attributes in tuple
    int iNum_tuple_attributes;
} search_result;


/**
  * @author Miroslav Policki
  
  * @brief Function that searches through unsorted values of multiple attributes in a segment.
 	   Only tuples that are equal on all given attribute values are returned (A == 1 AND B == 7 AND ...).
	   SEARCH_RANGE is inclusive. Only one value (or range) per attribute allowed - use search_params.pData_lower for SEARCH_PARTICULAR.
 	   Supported types for SEARCH_RANGE: TYPE_INT, TYPE_FLOAT, TYPE_NUMBER, TYPE_DATE, TYPE_DATETIME, TYPE_TIME.
           Do not provide the wrong data types in the array of search parameters. There is no way to test for that and it could cause a memory access  	violation.
  * @param szRelation relation name
  * @param aspParams array of search parameters
  * @param iNum_search_params number of search parameters
  * @return search_result structure defined in filesearch.h. Use AK_deallocate_search_result to deallocate.
 */

search_result AK_search_unsorted(char *szRelation, search_params *aspParams, int iNum_search_params);

/**
  * @author Miroslav Policki
  * @brief Function that deallocates memory used by the search result returned by AK_search_unsorted.
  * @param srResult search result
  * @return No return value
 */

void AK_deallocate_search_result(search_result srResult);
TestResult AK_filesearch_test();

#endif

/**
@file hash.h Header file that provides data structures, functions and defines for Hash indices
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

#ifndef HASH
#define HASH

#include "../../auxi/test.h"
#include "index.h"
#include "../../file/table.h"
#include "../../auxi/constants.h"
#include "../../auxi/configuration.h"
#include "../files.h"
#include "../../auxi/mempro.h"

/**
 * @author Unknown
 * @struct hash_info
 * @brief Structure for defining a hash info element
*/
typedef struct {
	/// modulo value for hash function
    int modulo;
    /// bucket number
    int main_bucket_num;
    /// hash bucket number
    int hash_bucket_num;
} hash_info;

/**
 * @author Unknown
 * @struct bucket_elem
 * @brief Structure for defining a single bucket element
 */
typedef struct {
	/// bucket element hash value
    unsigned int value;
    /// bucket element address values
    struct_add add;
} bucket_elem;

/**
 * @author Unknown
 * @struct main_bucket
 * @brief Structure for defining main bucket for table hashing
 */
typedef struct {
	/// main bucket array of bucket_elem elements
    bucket_elem element[MAIN_BUCKET_SIZE];
} main_bucket;

/**
 * @author Unknown
 * @struct hash_bucket
 * @brief Structure for hash bucket for table hashing
 */
typedef struct {
	/// hash bucket level
    int bucket_level;
    /// hash bucket array of bucket_elem elements
    bucket_elem element[HASH_BUCKET_SIZE];
} hash_bucket;

/**
  * @author Mislav Čakarić
  * @brief Function that computes a hash value from varchar or integer
  * @param elem element of row for wich value is to be computed
  * @return hash value
 
 */
int AK_elem_hash_value(struct list_node *elem);

/**
  * @author Mislav Čakarić
  * @brief Function that inserts a bucket to block
  * @param indexName name of index
  * @param data content of bucket stored in char array
  * @param type type of bucket (MAIN_BUCKET or HASH_BUCKET)
  * @return address structure with data where the bucket is stored 
 */
struct_add* AK_insert_bucket_to_block(char *indexName, char *data, int type);

/**
  *  @author Mislav Čakarić
  *  @brief Function that updates a bucket in block
  *  @param add address of where the bucket is stored
  *  @param data content of bucket stored in char array
  *  @return No return value
 */
void AK_update_bucket_in_block(struct_add *add, char *data);

/**
  * @author Mislav Čakarić
  * @brief Function that changes a info of hash index
  * @param indexName name of index
  * @param modulo value for modulo hash function
  * @param main_bucket_num number of main buckets
  * @param hash_bucket_num number of hash buckets
  * @return No return value
 */
void AK_change_hash_info(char *indexName, int modulo, int main_bucket_num, int hash_bucket_num);

/**
  * @author Mislav Čakarić
  * @brief Function that fetches the info for hash index
  * @param indexName name of index
  * @return info bucket with info data for hash index
 */
hash_info* AK_get_hash_info(char *indexName);

/**
  * @author Mislav Čakarić
  * @brief Function that fetches nth main bucket
  * @param indexName name of index
  * @param n number of main bucket
  * @return address structure with data where the bucket is stored
 */
struct_add* AK_get_nth_main_bucket_add(char *indexName, int n);

/**
  *  @author Mislav Čakarić
  *  @brief Function that inserts a record in hash bucket
  *  @param indexName name of index
  *  @param hashValue hash value of record that is being inserted
  *  @param add address structure with data where the hash bucket is stored
  *  @return No return value
 */
void AK_insert_in_hash_index(char *indexName, int hashValue, struct_add *add);

/**
  * @author Mislav Čakarić
  * @brief Function that fetches or deletes a record from hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @param delete if delete is 0 then record is only read otherwise it's deleted from hash index
  * @return address structure with data where the record is in table
 
 */
struct_add *AK_find_delete_in_hash_index(char *indexName, struct list_node *values, int delete);

/**
  * @author Mislav Čakarić
  * @brief Function that fetches a record from the hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @return address structure with data where the record is in table
 
 */
struct_add * AK_find_in_hash_index(char *indexName, struct list_node *values);

/**
  * @author Mislav Čakarić
  * @brief Function that deletes a record from the hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @return No return value
 
 */
void AK_delete_in_hash_index(char *indexName, struct list_node *values);

/**
  * @author Mislav Čakarić
  * @brief Function that creates a hash index
  * @param tblName name of table for which the index is being created
  * @param indexName name of index
  * @param attributes list of attributes over which the index is being created
  * @return success or error
 
 */
int AK_create_hash_index(char *tblName, struct list_node *attributes, char *indexName);
void AK_delete_hash_index(char *indexName) ;

/**
  * @author Mislav Čakarić
  * @brief Function that tests hash index
  * @return No return value 
 */
TestResult AK_hash_test();

#endif

/**
   @file constants.h Header file that provides global macros, constants and variables
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

#ifndef CONSTANTS
#define CONSTANTS
/**
  * @def MAX_VARCHAR_LENGTH
  * @brief Constant declaring the maximum length of varchar data value
 */
#define MAX_VARCHAR_LENGTH 200
/**
  * @def MAX_ATTRIBUTES
  * @brief Constant declaring the maximum number of attributes per block
 */
#define MAX_ATTRIBUTES 10
/**
  * @def MAX_ATT_NAME
  * @brief Constant declaring the maximum length of attribute name string (used in AK_header->att_name)
 */
#define MAX_ATT_NAME 255
/**
  * @def MAX_CONSTRAINTS
  * @brief Constant declaring the maximum number of constraints per attribute
 */
#define MAX_CONSTRAINTS 5
/**
  * @def MAX_CONSTR_NAME
  * @brief Constant declaring the maximum length of constraint name string (used in AK_header->constr_name)
 */
#define MAX_CONSTR_NAME 255
/**
  * @def MAX_CONSTR_CODE
  * @brief Constant declaring the maximum lenght of constraint code string
 */
#define MAX_CONSTR_CODE 255
/**
  * @def MAX_OBSERVABLE_OBSERVERS
  * @brief Constant for declaring the maximum number of observers objects for some observable type
 */
#define MAX_OBSERVABLE_OBSERVERS 4096

/**
  * @def MAX_ACTIVE_TRANSACTIONS_COUNT
  * @brief Constant for declaring the maximum number of active trasactions in DBMS
 */
#define MAX_ACTIVE_TRANSACTIONS_COUNT 100

/**
  * @def DATA_BLOCK_SIZE
  * @brief Constant declaring length of data block size (used in AK_block->data)
 */
#define DATA_BLOCK_SIZE 500
/**
  * @def DATA_ENTRY_SIZE
  * @brief Constant declaring lenght of data entry in sizeof( int )
 */
#define DATA_ENTRY_SIZE 10
/**
  * @def MAX_QUERY_LIB_MEMORY
  * @brief Constant declaring the maximum size of query lib memory
 */
#define MAX_QUERY_LIB_MEMORY 255
/**
  * @def MAX_CACHE_MEMORY
  * @brief Constant declaring the maximum size of DB cache memory
 */
#define MAX_CACHE_MEMORY 255
/**
 * @def MAX_QUERY_DICT_MEMORY
 * @brief Constant declaring the maximum size of query dictionary memory
 */
#define MAX_QUERY_DICT_MEMORY 255

/**
 * @def MAX_QUERY_RESULT_MEMORY
 * @brief Constant declaring the maximum size of query result cache memory
 */
#define MAX_QUERY_RESULT_MEMORY 255
/**
 * @def MAX_TOKENS
 * @brief Constant declaring the maximum number of attributes to handle in relation equivalence function
 */
#define MAX_TOKENS 255
/**
 * @def MAX_MAIN_BUCKETS
 * @brief Constant declaring the maximum number of main buckets
*/
#define MAX_MAIN_BUCKETS 512
/**
 * @def MAIN_BUCKET_SIZE
 * @brief Constant declaring the size of main buckets
*/
#define MAIN_BUCKET_SIZE 4
/**
 * @def HASH_BUCKET_SIZE
 * @brief Constant declaring the size of hash buckets
*/
#define HASH_BUCKET_SIZE 4
/**
 * @def NUMBER_OF_KEYS
 * @brief Constant declaring the number of buckets in hash table
*/
#define NUMBER_OF_KEYS 4096
/**
  * @def EXIT_SUCCESS
  * @brief Constant declaring a successful exit
 */
#define EXIT_SUCCESS 0
/**
  * @def EXIT_ERROR
  * @brief Constant declaring unsuccessful exit
 */
#define EXIT_ERROR -1


/**
 * @def EXIT_WARNING -2
 * @brief Constant declaring warning exit
 */
#define EXIT_WARNING -2

/**
  * @def BLOCK_TYPE_FREE
  * @brief Constant declaring AK_free block type (used in AK_block->type)
 */
#define BLOCK_TYPE_FREE -1
/**
  * @def BLOCK_TYPE_NORMAL
  * @brief Constant declaring normal block type e.g. used by some extent (used in AK_block->type)
 */
#define BLOCK_TYPE_NORMAL 0
/**
  * @def BLOCK_TYPE_CHAINED
  * @brief Constant declaring chained block type e.g. used if the block is chained with another (used in AK_block->type)
 */
#define BLOCK_TYPE_CHAINED 1
/**
  * @def NOT_CHAINED
  * @brief Constant used in AK_block->chained_with if the block isn't chained
 */
#define NOT_CHAINED -1
/**
  * @def FREE_INT
  * @brief Constant declaring dummy data for empty integers
 */
#define FREE_INT -10
/**
  * @def FREE_CHAR
  * @brief Constant declaring dummy data for empty chars
 */
#define FREE_CHAR '\0'
/**
  * @def SEGMENT_TYPE_SYSTEM_TABLE
  * @brief Constant declaring system table segment type (used in system catalog)
 */
#define SEGMENT_TYPE_SYSTEM_TABLE 0
/**
  * @def SEGMENT_TYPE_TABLE
  * @brief Constant declaring table segment type (used in system catalog)
 */
#define SEGMENT_TYPE_TABLE 1
/**
  * @def SEGMENT_TYPE_INDEX
  * @brief Constant declaring index segment type (used in system catalog)
 */
#define SEGMENT_TYPE_INDEX 2
/**
   * @def SEGMENT_TYPE_TRANSACTION
   * @brief Constant declaring transaction segment type (used in system catalog)
 */
#define SEGMENT_TYPE_TRANSACTION 3
/**
  * @def SEGMENT_TYPE_TEMP
  * @brief Constant declaring temporary segment type (used in system catalog)
 */
#define SEGMENT_TYPE_TEMP 4
/**
  * @def TYPE_INTERNAL
  * @brief Constant declaring internal data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_INTERNAL 0
/**
 * @def TYPE_INT
 * @brief integer data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_INT 1
/**
  * @def TYPE_FLOAT
  * @brief Constant declaring float data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_FLOAT 2
/**
  * @def TYPE_NUMBER
  * @brief Constant declaring number data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_NUMBER 3
/**
  * @def TYPE_VARCHAR
  * @brief Constant declaring varchar data type (used in AK_header->type and AK_tuple_dict->type)
 */

#define TYPE_VARCHAR 4
/**
  * @def TYPE_DATE
  * @brief Constant declaring date data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_DATE 5
/**
  * @def TYPE_DATETIME
  * @brief Datetime data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_DATETIME 6
/**
  * @def TYPE_TIME
  * @brief Constant declaring time data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_TIME 7
/**
  * @def TYPE_BLOB
  * @brief Blob data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_BLOB 8
/**
  * @def TYPE_BOOL
  * @brief Constant declaring boolean data type (used in AK_header->type and AK_tuple_dict->type)
 */
#define TYPE_BOOL 9
/**
 * @def TYPE_OPERAND
 * @brief Constant indicating operand in AK_list
 */
#define TYPE_OPERAND 10
/**
  * @def TYPE_OPERATOR
  * @brief indicates operator in AK_list
 */
#define TYPE_OPERATOR 11
/**
  * @def TYPE_ATTRIBS
  * @brief Constant indicating attribute/s in AK_list
 */
#define TYPE_ATTRIBS 12
/**
  * @def TYPE_CONDITION
  * @brief Constant indicating condition in AK_list
 */
#define TYPE_CONDITION 13
/**
 * @def BLOCK_CLEAN
 * @brief Constant indicating block cleaning (not changed since read from disk)
 */
#define BLOCK_CLEAN 0
/**
 * @def BLOCK_DIRTY
 * @brief Constant indicating dirty block (changed since read from disk, has to be written)
 */
#define BLOCK_DIRTY 1
/**
 * @def ATTR_DELIMITER
 * @brief Constant declaring attributes delimiter
 */
#define ATTR_DELIMITER ";"
/**
  * @def ATTR_ESCAPE
  * @brief Constant indicating attributes escape section
 */
#define ATTR_ESCAPE '`'
/**
  * @def NULLL
  * @brief Constant declaring null value for tables
 */
#define NULLL "asdfgXYZ"
/**
  * @def relational operators
 */
#define RO_SELECTION 's'
#define RO_PROJECTION 'p'
#define RO_NAT_JOIN 'n'
#define RO_RENAME 'r'
#define RO_UNION 'u'
#define RO_INTERSECT 'i'
#define RO_EXCEPT 'e'
#define RO_THETA_JOIN 't'
/**
  * @def NEW_VALUE
  * @brief Constant indicating that the data is a new value
 */
#define NEW_VALUE 0
/**
  * @def SEARCH_CONSTRAINT
  * @brief Constant indicating that the data is constraint to search for
 */
#define SEARCH_CONSTRAINT 1
/**
  * @def UPDATE
  * @brief Constant indicating that the operation to be performed is 'update'
 */
#define UPDATE 0
/**
 * @def DELETE
 * @brief Constant indicating that the operation to be performed is 'delete'
 */
#define DELETE 1
/**
 * @def INSERT
 * @brief Constant indicating that the operation to be performed is 'insert'
 */
#define INSERT 2
/**
 * @def SELECT
 * @brief Constant indicating 'select' operation
 */
#define SELECT 3
/**
 * @def FIND
 * @brief Constant indicating that the operation to be performed is 'search'
 */
#define FIND 2
/**
 * @def INFO_BUCKET
 * @brief Constant declaring the type of bucket as "info bucket" when inserting bucket to block
*/
#define INFO_BUCKET 0
/**
 * @def MAIN_BUCKET
 * @brief Constant declaring the type of bucket as "main bucket" when inserting bucket to block
*/
#define MAIN_BUCKET 1
/**
 * @def HASH_BUCKET
 * @brief Constant declaring the type of bucket as "hash bucket" when inserting bucket to block
*/
#define HASH_BUCKET 2
/**
 * @def SHARED_LOCK
 * @brief Constant declaring the type of lock as SHARED LOCK
*/
#define SHARED_LOCK 0
/**
 * @def EXCLUSIVE_LOCK
 * @brief Constant declaring the type of lock as EXCLUSIVE LOCK
*/
#define EXCLUSIVE_LOCK 1
/**
 * @def WAIT_FOR_UNLOCK
 * @brief Constant declaring that a lock has to wait until other locks release the resource
*/
#define WAIT_FOR_UNLOCK 0
/**
 * @def PASS_LOCK_QUEUE
 * @brief Constant declaring that a lock can acquire the resource AK_freely
*/
#define PASS_LOCK_QUEUE 1
/**
 * @def OK
 * @brief Constant declaring that the method is completed successfuly
*/
#define OK 1
/**
 * @def NOT_OK
 * @brief Constant declaring that the method isn't completed successfuly
*/
#define NOT_OK 0
/**
 * @def COMMIT
 * @brief Constant declaring that the transaction is completed successfuly
*/
#define COMMIT 1
/**
 * @def ABORT
 * @brief Constant declaring if the transaction is being aborted
*/
#define ABORT 0

/**
 * @def NEW_ID
 * @brief Constant declaring if new obj_id should be created
*/
#define NEW_ID 0

/**
 * @def MAX_BLOCKS_CURRENTLY_ACCESSED
 * @brief Indicates the maximum number of threads that can access (read or write) database at the same time
 */
#define MAX_BLOCKS_CURRENTLY_ACCESSED 32

/**
 * @def TEST_MODE_ON
 * @brief This constant is used to turn testMode (auxi/auxillary.h) ON
 */
#define TEST_MODE_ON 1

/**
 * @def TEST_MODE_OFF
 * @brief This constant is used to turn testMode (auxi/auxillary.h) OFF
 */
#define TEST_MODE_OFF 0

/**
 * @def SEPARATOR
 * @brief Used in unique.c for separation of names of attributes and their values when UNIQUE constraint is being set or tested on combination of values of attributes
 */
#define SEPARATOR "[{(|&&|)}]"

/**
 * @def AK_CONSTRAINTS_BEWTEEN
 * @brief Defines system table name for storing between constraints
 */
#define AK_CONSTRAINTS_BEWTEEN "AK_constraints_between"

/**
 * @def AK_CONSTRAINTS_CHECK_CONSTRAINT
 * @brief Defines system table name for storing check constraints
 */
#define AK_CONSTRAINTS_CHECK_CONSTRAINT "AK_constraints_check_constraint"

/**
 * @def AK_CONSTRAINTS_NOT_NULL
 * @brief Defines system table name for storing check constraints
 */
#define AK_CONSTRAINTS_NOT_NULL "AK_constraints_not_null"

/**
 * @def AK_CONSTRAINTS_UNIQUE
 * @brief Defines system table name for storing check constraints
 */
#define AK_CONSTRAINTS_UNIQUE "AK_constraints_unique"

/**
 *  * @def AK_CONSTRAINTS_INDEX
 *   * @brief Defines system table name for storing check constraints
 *    */
#define AK_CONSTRAINTS_INDEX "AK_constraints_index"

/**
 *  * @def AK_CONSTRAINTS_PRIMARY_KEY
 *   * @brief Defines system table name for storing check constraints
 *    */
#define AK_CONSTRAINTS_PRIMARY_KEY "AK_constraints_primary_key"

/**
 *  * @def AK_CONSTRAINTS_FOREIGN_KEY
 *   * @brief Defines system table name for storing check constraints
 *    */
#define AK_CONSTRAINTS_FOREIGN_KEY "AK_constraints_foreign_key"

/**
 *  * @def AK_CONSTRAINTS_DEFAULT
 *   * @brief Defines system table name for storing check constraints
 *    */
#define AK_CONSTRAINTS_DEFAULT "AK_constraints_default"

/**
 * @def AK_REFERENCE
 * @brief Defines system table name for storing check constraints
 */
#define AK_REFERENCE "AK_reference"

/**
 * @def DROP_TABLE
 * @brief Constant which defines the number of drop statement
 */
#define DROP_TABLE      0

/**
 * @def DROP_INDEX
 * @brief Constant which defines the number of drop statement
 */
#define DROP_INDEX      1

/**
 * @def DROP_VIEW
 * @brief Constant which defines the number of drop statement
 */
#define DROP_VIEW       2

/**
 * @def DROP_SEQUENCE
 * @brief Constant which defines the number of drop statement
 */
#define DROP_SEQUENCE   3

/**
 * @def DROP_TRIGGER
 * @brief Constant which defines the number of drop statement
 */
#define DROP_TRIGGER    4

/**
 * @def DROP_FUNCTION
 * @brief Constant which defines the number of drop statement
 */
#define DROP_FUNCTION   5

/**
 * @def DROP_USER
 * @brief Constant which defines the number of drop statement
 */
#define DROP_USER       6

/**
 * @def DROP_GROUP
 * @brief Constant which defines the number of drop statement
 */
#define DROP_GROUP      7

/**
 * @def DROP_CONSTRAINT
 * @brief Constant which defines thenumber of drop statement
 */
#define DROP_CONSTRAINT 8


/**
 * @def NUM_SYS_TABLES
 * @brief Constant which defines the length of system_catalog
 */
#define NUM_SYS_TABLES  20

#endif


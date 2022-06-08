/**
@file dbman.h Header file that contains  all defines, includes and data structures for the disk manager
of Kalashnikov DB
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

#ifndef DBMAN
#define DBMAN

#include "../auxi/test.h"
#include "../auxi/auxiliary.h"
#include <errno.h>
#include <pthread.h>


#include "sys/time.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../auxi/mempro.h"


#include <sys/stat.h>   /* for stat structure*/
#include <limits.h>     /* for CHAR_BIT */

//#define false 0
//#define true  !false

#define BITMASK(b)      (1 << ((b) % CHAR_BIT))
#define BITSLOT(b)      ((int)((b) / CHAR_BIT))
#define BITSET(a, b)    ((a)[BITSLOT(b)] |=  BITMASK(b))
#define BITCLEAR(a, b)  ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b)   ((a)[BITSLOT(b)] &   BITMASK(b))
#define BITNSLOTS(nb)   ((int)(nb + CHAR_BIT - 1) / CHAR_BIT)
#define SEGMENTLENGTH() (BITNSLOTS(DB_FILE_BLOCKS_NUM) + 2*sizeof(int))


/**
 * @author Markus Schatten
 * @struct AK_header
 * @brief Structure that represents header structure of blocks (describes an attribute inside an object). It contains
 type, attribute name, integrity, constraint name and constraint code.
 */
typedef struct {
    /// type of attribute
    int type;
    /// attribute name
    char att_name[MAX_ATT_NAME];
    /// standard integrity costraints
    int integrity[MAX_CONSTRAINTS];
    /// extra integrity constraint names
    char constr_name[MAX_CONSTRAINTS][MAX_CONSTR_NAME];
    /// extra integrity costraint codes
    char constr_code[MAX_CONSTRAINTS][MAX_CONSTR_CODE];
} AK_header;

/**
 * @author Markus Schatten
 * @struct AK_tuple_dict
 * @brief Structure that defines a mapping in a header of an object to the actual entries (data). It contains type, address
 and size.
 */
typedef struct {
    /// data entry type
    int type;
    /// data entry address (in AK_block->data)
    int address;
    /// data entry size (using sizeof( *** ) )
    int size;
} AK_tuple_dict;

/**
  * @author Markus Schatten
  * @struct AK_block
  * @brief Structure that defines a block of data inside a DB file. It contains address, type, chained_with, AK_free space,
  last_tuple_dict_id, header and tuple_dict and data.
  */
typedef struct {
    /// block number (address) in DB file
    int address;
    /// block type (can be BLOCK_TYPE_FREE, BLOCK_TYPE_NORMAL or BLOCK_TYPE_CHAINED)
    int type;
    /// address of chained block; NOT_CHAINED otherwise
    int chained_with;
    /// AK_free space in block
    int AK_free_space;
    int last_tuple_dict_id;
    /// attribute definitions
    AK_header header[MAX_ATTRIBUTES];
    /// dictionary of data entries
    AK_tuple_dict tuple_dict[DATA_BLOCK_SIZE];
    /// actual data entries
    unsigned char data[DATA_BLOCK_SIZE * DATA_ENTRY_SIZE];
} AK_block;

/**
 * @author Markus Schatten
 * @var db
 * @brief Variable that defines the DB file file handle
 */
    FILE * db;

/**
 * @author Markus Schatten
 * @var db_file_size
 * @brief Variable that defines the size of the DB file (in blocks)
 */
unsigned int db_file_size;

/**
  * @author Matija Novak
  * @struct table_addresses
  * @brief Structure that defines start and end address of extent
  */
typedef struct {
    ///sturcture for extents start end stop adresses
    int address_from[MAX_EXTENTS_IN_SEGMENT]; //start adress of the extent
    int address_to[MAX_EXTENTS_IN_SEGMENT]; //end adress of the extent
} table_addresses;

#define DB_FILE_SIZE_EX 200
#define DB_FILE_BLOCKS_NUM_EX (int)(1024 * 1024 * DB_FILE_SIZE_EX / sizeof(AK_block))


/**
  * @author dv
  * @struct blocktable
  * @brief Structure that defines bit status of blocks, last initialized and last allocated index
  */
typedef struct {
    unsigned int allocationtable[DB_FILE_BLOCKS_NUM_EX];
    unsigned char bittable[BITNSLOTS(DB_FILE_BLOCKS_NUM_EX)];
    int last_allocated;
    int last_initialized;
    int prepared;
    time_t ltime;
}AK_blocktable;

/**
 * @author dv
 * @var AK_allocationbit
 * @brief Global variable that holds allocation bit-vector
 */
AK_blocktable *AK_allocationbit;




/**
 * @author dv
 * @brief Holds size of allocation table
 */
#define AK_ALLOCATION_TABLE_SIZE sizeof(AK_blocktable)


/**
 * @author dv
 * @brief How many characters could line contain
 */
#define CHAR_IN_LINE 80


/**
 * @author dv
 * @brief How many blocks would be initially allocated
 */
#define MAX_BLOCK_INIT_NUM MAX_CACHE_MEMORY

/**
 * @author dv
 * @brief Different modes to obtain allocation indexes:
 * SEQUENCE - first found set of sequence indexes
 * UPPER    - set tries to place itself to upper part od allocation table
 * LOWER    - set tries to place itself to lower part od allocation table
 * AROUND   - set tries to place itself around targeted index
 */
typedef enum{
    allocationSEQUENCE = 10001,
    allocationUPPER,
    allocationLOWER,
    allocationAROUND,
    allocationNOMODE
}AK_allocation_set_mode;

/**
 * @author Domagoj Šitum
 * @brief Structure which holds information about each block,
 * whether it is locked for reading or writing.
 * It is important to note such information, to enable quick and thread-safe
 * reading from or writing to disk.
 * Structure contains of:
 * locked_for_reading - thread which locks particular block for reading will set this value
 * locked_for_writing - thread which locks particular block for writing will set this value
 * block_lock - each reading and writing operation will be done atomically and uninteruptable, using this mutex block lock
 * reading_done - represents signal, which sends thread that just finished reading block. This signal will indicate that
 *      writing thread can start writing to block
 * writing_done - represents signal, which sends thread that just finished writing to block. This signal will indicate
 *      that other threads can start reading from this block or even writing to it
 * thread_holding_lock - the only thread which can unlock locked "block_lock" is the one that locked it.
 *      This variable makes sure that ONLY the thread, which actually holds the lock, releases it.
 */
typedef struct {
    short locked_for_reading;
    short locked_for_writing;
    pthread_mutex_t block_lock;
    pthread_cond_t writing_done;
    pthread_cond_t reading_done;
    int *thread_holding_lock;
} AK_block_activity;

/**
 * @author Domagoj Šitum
 * @var AK_accessed_blocks
 * @brief Blocks which are currently being written to disk or read from it.
 */
AK_block_activity *AK_block_activity_info;

/**
 * @author Marko Sinko
 * @var fileLock
 * @brief Synchronization object for files used by dbman.c
 */
AK_synchronization_info* dbmanFileLock;

int AK_print_block(AK_block * block, int num, char* gg, FILE *fpp);
TestResult AK_allocationbit_test();
TestResult AK_allocationtable_test();
int* AK_increase_extent(int start_address, int add_size, AK_allocation_set_mode* mode, int border, int target, AK_header *header, int gl);
int* AK_get_extent(int start_address, int desired_size, AK_allocation_set_mode* mode, int border, int target, AK_header *header, int gl);
int AK_get_allocation_set(int* bitsetbs, int fromWhere, int gaplength, int num, AK_allocation_set_mode mode, int target);
int AK_copy_header(AK_header *header, int * blocknum, int num);
int  AK_allocate_blocks(FILE* db, AK_block * block, int FromWhere, int HowMany);
AK_block *  AK_init_block();
int AK_allocationtable_dump(int zz);
void AK_blocktable_dump(int zz);
int AK_blocktable_flush();
// void AK_allocate_array_currently_accessed_blocks(); // ne postoji nikakva implementacija
TestResult AK_thread_safe_block_access_test();
void* AK_read_block_for_testing(void *address);
void* AK_write_block_for_testing(void *block);
int AK_blocktable_get();
int fsize(FILE *fp);
int AK_init_allocation_table();
int AK_init_db_file(int size);
AK_block * AK_read_block(int address);
int AK_write_block(AK_block * block);
int AK_new_extent(int start_address, int old_size, int extent_type, AK_header *header);
int AK_new_segment(char * name, int type, AK_header *header);
AK_header * AK_create_header(char * name, int type, int integrity, char * constr_name, char * contr_code);
void AK_insert_entry(AK_block * block_address, int type, void * entry_data, int i);
int AK_init_system_tables_catalog(int relation, int attribute, int index, int view, int sequence, int function, int function_arguments,
    int trigger, int trigger_conditions, int db, int db_obj, int user, int group, int user_group, int user_right, int group_right, int constraint, int constraintNull, int constraintCheck, int constraintUnique, int reference);
void AK_memset_int(void *block, int value, size_t num);
int AK_register_system_tables(int relation, int attribute, int index, int view, int sequence, int function, int function_arguments,
    int trigger, int trigger_conditions, int db, int db_obj, int user, int group, int user_group, int user_right, int group_right, int constraint, int constraintNull, int constraintCheck, int constraintUnique, int reference);
int AK_init_system_catalog();
int AK_delete_block(int address);
int AK_delete_extent(int begin, int end);
int AK_delete_segment(char * name, int type);
int AK_init_disk_manager();

#endif

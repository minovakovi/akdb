/**
@file memoman.h Defines includes and datastructures for the memory manager
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
#ifndef MEMOMAN
#define MEMOMAN

#include "auxiliary.h"
#include "dbman.h"
#include "fileio.h"

/**
\struct AK_mem_block
\brief Defines a block of data in memory
*/
typedef struct {
	/// pointer to block from DB file
	AK_block * block;
	/// dirty bit (BLOCK_CLEAN if unchanged; BLOCK_DIRTY if changed but not yet written to file)
	int dirty;
	/// timestamp when the block has lastly been read
	unsigned long timestamp_read;
	/// timestamp when the block has lastly been changed
	unsigned long timestamp_last_change;	
} AK_mem_block;

/**
\struct AK_db_cache
\brief Global cache memory
*/
typedef struct {
	/// last recently read blocks
	AK_mem_block * cache[ MAX_CACHE_MEMORY ];
	/// next cached block to be replaced (0 - MAX_CACHE_MEMORY-1); depends on caching algorithm
	int next_replace;
} AK_db_cache;

/**
\struct AK_redo_log
\brief Global redo log
*/
typedef struct {
	/// last recently changed blocks
	AK_mem_block redo_log[ MAX_REDO_LOG_MEMORY ];
	/// next block to be replaced (0 - MAX_REDO_LOG_MEMORY-1); field pointer (LIFO)
	int next_replace;
} AK_redo_log;

/**
\struct AK_query_mem_lib
\brief Global query memory for libraries
*/
typedef struct {
	/// last parsed queries; to be changed to more adequate data structure
	char parsed[ MAX_QUERY_LIB_MEMORY ];
	/// next query to be replaced (0 - MAX_QUERY_LIB_MEMORY-1); field pointer (LIFO)
	int next_replace;
} AK_query_mem_lib;

/**
\struct AK_query_mem_dict
\brief Global query memory for data dictionaries
*/
typedef struct {
	/// last used data dictionaries
	AK_tuple_dict * dictionary[ MAX_QUERY_DICT_MEMORY ];
	/// next dictionary to be replaced (0 - MAX_QUERY_DICT_MEMORY-1); field pointer (LIFO)
	int next_replace;
} AK_query_mem_dict;

/**
\struct AK_query_mem_result
\brief Global query memory for results
*/
typedef struct {
	/// last recently obtained results; to be changed to more adequate data structure
	char results[ MAX_QUERY_RESULT_MEMORY ];
	/// next result to be replaced (0 - MAX_QUERY_RESULT_MEMORY-1); field pointer (LIFO)
	int next_replace;
} AK_query_mem_result;

/**
\struct AK_query_mem
\brief Global query memory
*/
typedef struct {
	/// parsed queries
	AK_query_mem_lib * parsed;
	/// obtained data dictionaries
	AK_query_mem_dict * dictionary;
	/// obtained query results
	AK_query_mem_result * result;
} AK_query_mem;

/**
\var db_cache
\brief Defines the db cache
*/
AK_db_cache * db_cache;
/**
\var redo_log
\brief Defines the global redo log
*/
AK_redo_log * redo_log;
/**
\var query_mem
\brief Defines the global query memory
*/
AK_query_mem * query_mem;


#endif

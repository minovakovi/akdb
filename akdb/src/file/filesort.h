/**
@file filesort.h Header file that provides functions and defines for file sorting
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

#ifndef FILESORT
#define FILESORT

#include "../auxi/test.h"
#include "../mm/memoman.h"
#include "table.h"
#include "files.h"
#include "fileio.h"
#include "../auxi/mempro.h"
/**
  * @def DATA_ROW_SIZE
  * @brief Constatnt declaring size of data to be compared
  */

#define DATA_ROW_SIZE 200
/**
  * @def DATA_TUPLE_SIZE
  * @brief Constant declaring size of data to be copied
  */

#define DATA_TUPLE_SIZE 500


/**
 * @author Unknown
 * @brief Function that returns the total number of headers in the block
 * @return number of attribute in header (0 - MAX_ATTRIBUTES). USE in tuple_dict[num]...
 */
int AK_get_total_headers(AK_block *iBlock);

/**
 * @author Unknown
 * @brief Function that returns the number of header in the block which to sort
 * @return number of attribute in header (0 - MAX_ATTRIBUTES). USE in tuple_dict[num]...
 */
int AK_get_header_number(AK_block *iBlock, char *attribute_name);

/**
 * @author Unknown
 * @brief Function that returns tuples number in block
 * @return tuples number in block
 */
int AK_get_num_of_tuples(AK_block *iBlock);

/**
 * @author Tomislav Bobinac, updated by Filip Žmuk
 * @todo Make it to suport multiple sort atributes and ASC|DESC ordering
 * @brief Function that sorts a segment
 * @return No return value.
 */
int AK_sort_segment(char *srcTable, char *destTable, struct list_node* attributes);

/**
 * @author Unknown
 * @brief Function that resets block
 * @param block block to be resetted
 * @return No return value
 */
void AK_reset_block(AK_block * block);

/**
  * @author Bakoš Nikola
  * @version v1.0
  * @brief Function that sorts the given block
  * @param iBlock block to be sorted
  * @return No return value
 */
void AK_block_sort(AK_block * iBlock, char * atr_name);
TestResult AK_filesort_test();

#endif


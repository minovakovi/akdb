/**
@file nat_join.h Header file that provides functions and defines for relational natural join operation
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
#ifndef NATJOIN
#define NATJOIN

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../rel/projection.h"
#include "../auxi/mempro.h"
#include "../sql/drop.h"
/*
void AK_create_join_block_header(int table_address1, int table_address2, char *new_table, AK_list *att);
void AK_merge_block_join(AK_list *row_root, AK_list *row_root_insert, AK_block *temp_block, char *new_table);
void AK_copy_blocks_join(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, AK_list *att, char *new_table);*/

/**
 * @author Matija Novak, optimized, and updated to work with AK_list by Dino Laktašić
 * @brief  Function that makes a header for the new table and call the function to create the segment
 * @param table_address1 address of the block of the first table
 * @param table_address2 address of the block of the second table
 * @param new_table name of the join table
 * @param att_root ttributes on which we make nat_join
 * @return No return value
 */
void AK_create_join_block_header(int table_address1, int table_address2, char *new_table, struct list_node *att);

/**
 * @author Matija Novak, updated by Dino Laktašić
 * @brief  Function that searches the second block and when found matches with the first one makes a join and writes a row to join the tables
 * @param row_root - list of values from the first table to be marged with table2
 * @param row_root_insert - list of values from the first table to be inserted into nat_join table
 * @param temp_block - block from the second table to be merged
 * @param new_table - name of the nat_join table
 * @return No return value
 */
void AK_merge_block_join(struct list_node *row_root, struct list_node *row_root_insert, AK_block *temp_block, char *new_table);

/**
 * @author Matija Novak, optimized, and updated to work with AK_list by Dino Laktašić
 * @brief Function that iterates through block of the first table and copies data that needs for join, then it calls a merge function to
        merge with the second table
 * @param tbl1_temp_block block of the first table
 * @param tbl2_temp_block block of the second join table
 * @param att attributes on which we make nat_join
 * @param new_table name of the nat_join table
 * @return No return value
 */
void AK_copy_blocks_join(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, struct list_node *att, char *new_table);
//int AK_join(char *srcTable1, char * srcTable2, char * dstTable, AK_list *att);

/**
 * @author Matija Novak, updated to work with AK_list and support cacheing by Dino Laktašić
 * @brief Function that makes a  nat_join betwen two tables on some attributes
 * @param srcTable1 name of the first table to join
 * @param srcTable2 name of the second table to join
 * @param att attributes on which we make nat_join
 * @param dstTable name of the nat_join table
 * @return if success returns EXIT_SUCCESS
 */
int AK_join(char *srcTable1, char * srcTable2, char * dstTable, struct list_node *att);
TestResult AK_op_join_test();

#endif

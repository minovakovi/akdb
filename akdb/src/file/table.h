/** @file table.h Header file that provides data structures, functions and defines for table abstraction
 *
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

#ifndef RENAME
#define RENAME

#endif

#ifndef TABLE
#define TABLE

#include "../mm/memoman.h"


struct AK_create_table_struct {
	char name[MAX_ATT_NAME];
	int type;
};

typedef struct AK_create_table_struct AK_create_table_parameter;




AK_create_table_parameter* AK_create_create_table_parameter(int type, char* name);

/**
 * @author Matija Novak, updated by Dino Laktašić
 * @brief  Temporary function that  creates table, and inserts an entry to the system_relation catalog
 * @param table table name
 * @param header AK_header of the new table
 * @param type_segment type of the new segment
 * @return No return value
 */
void AK_create_table(char* tblName, AK_create_table_parameter* parameters, int attribute_count) ;
void AK_temp_create_table(char *table, AK_header *header, int type_segment);

/**
 * @author Matija Šestak.
 * @brief  Functions that determines the number of attributes in the table
 * <ol>
 * <li>Read addresses of extents</li>
 * <li>If there is no extents in the table, return EXIT_WARNING</li>
 * <li>else read the first block</li>
 * <li>while  header tuple exists in the block, increment num_attr</li>
 * </ol>
 * @param  * tblName table name
 * @return number of attributes in the table
 */
int AK_num_attr(char * tblName);

/**
 * @author Matija Šestak.
 * @brief  Function that determines the number of rows in the table
 * <ol>
 * <li>Read addresses of extents</li>
 * <li>If there is no extents in the table, return EXIT_WARNING</li>
 * <li>For each extent from table</li>
 * <li>For each block in the extent</li>
 * <li>Get a block</li>
 * <li>Exit if there is no records in block</li>
 * <li>Count tuples in block</li>
 * <li>Return the number of tuples divided by number of attributes</li>
 * </ol>
 * @param *tableName table name
 * @return number of rows in the table
 */
int AK_get_num_records(char *tblName);

/**
 * @author Matija Šestak.
 * @brief  Function that fetches the table header
 * <ol>
 * <li>Read addresses of extents</li>
 * <li>If there is no extents in the table, return 0</li>
 * <li>else read the first block</li>
 * <li>allocate array</li>
 * <li>copy table header to the array</li>
 * </ol>
 * @param  *tblName table name
 * @result array of table header
 */
AK_header *AK_get_header(char *tblName);

/**
 * @author Matija Šestak.
 * @brief  Function that fetches attribute name for some zero-based index
 * @param *tblName table name
 * @param index zero-based index
 * @return attribute name
 */
char *AK_get_attr_name(char *tblName, int index);

/**
 * @author Matija Šestak.
 * @brief  Function that fetches zero-based index for atrribute
 * @param  *tblName table name
 * @param *attrName attribute name
 * @return zero-based index
 */
int AK_get_attr_index(char *tblName, char *attrName) ;

/**
 * @author Matija Šestak.
 * @brief  Function that fetches all values in some column and put on the list
 * @param num zero-based column index
 * @param  *tblName table name
 * @return column values list
 */
struct list_node *AK_get_column(int num, char *tblName);

/**
 * @author Markus Schatten, Matija Šestak.
 * @brief  Function that fetches all values in some row and put on the list
 * @param num zero-based row index
 * @param  * tblName table name
 * @return row values list
 */
struct list_node * AK_get_row(int num, char * tblName);

/**
 * @author Matija Šestak.
 * @brief Function that fetches a value in some row and column
 * @param row zero-based row index
 * @param column zero-based column index
 * @param *tblName table name
 * @return value in the list
 */
struct list_node *AK_get_tuple(int row, int column, char *tblName);

/**
 * @author Matija Šestak.
 * @brief  Function that converts tuple value to string
 * @param *tuple tuple in the list
 * @return tuple value as a string
 */
char * AK_tuple_to_string(struct list_node *tuple);

/**
 * @author Dino Laktašić.
 * @brief Function that prints row spacer
 * @param col_len[] max lengths for each attribute cell
 * @param length total table width
 * @return printed row spacer
 */
void AK_print_row_spacer(int col_len[], int length);

/**
 * @author Dino Laktašić
 * @brief  Function that prints table row
 * @param col_len[] array of max lengths for each attribute
 * @param *row  list with row elements
 * @return No return value
 */
void AK_print_row(int col_len[], struct list_node *row);

/**
 * @author Dino Laktašić and Mislav Čakarić (replaced old print table function by new one)
 * @brief  Function for printing table
 * @param *tblName table name
 * @return No return value
 */
void AK_print_table(char *tblName);

/**
 * @author Dino Laktašić.
 * @brief Function that prints row spacer
 * update by Luka Rajcevic
 * @param col_len[] max lengths for each attribute cell
 * @param length total table width
 * @return printed row spacer
 */
void AK_print_row_spacer_to_file(int col_len[], int length);

/**
 * @author Dino Laktašić
 * @brief  Function that prints the table row
 * update by Luka Rajcevic
 * @param col_len[] array of max lengths for each attribute
 * @param *row  list with row elements
 * @return No return value
 */
void AK_print_row_to_file(int col_len[], struct list_node *row);

/**
 * @author Dino Laktašić and Mislav Čakarić (replaced old print table function by new one)
 * update by Luka Rajcevic
 * @brief  Function that prints a table
 * @param *tblName table name
 * @return No return value
 * update by Anto Tomaš (corrected the AK_DeleteAll_L3 function)
 */
void AK_print_table_to_file(char *tblName);

/**
 * @author Matija Šestak.
 * @brief  Function that checks whether the table is empty
 * @param *tblName table name
 * @return true/false
 */
int AK_table_empty(char *tblName);

/**
 * @author Dejan Frankovic
 * @brief  Function that fetches an obj_id of named table from AK_relation system table
 * @param *table table name
 * @return obj_id of the table or EXIT_ERROR if there is no table with that name
 */
int AK_get_table_obj_id(char *table);

/**
 * @author Dino Laktašić, abstracted from difference.c for use in difference.c, intersect.c and union.c by Tomislav Mikulček
 * @brief  Function that checks if tables have the same relation schema
 * @param tbl1_temp_block first cache block of the first table
 * @param tbl2_temp_block first cache block of the second table
 * @param operator_name the name of operator, used for displaying error message
 * @return if success returns num of attributes in schema, else returns EXIT_ERROR
 */
int AK_check_tables_scheme(AK_mem_block *tbl1_temp_block, AK_mem_block *tbl2_temp_block, char *operator_name);

/**
 * @author Leon Palaić
 * @brief  Function that returns the value of an attribute from the row
 * @param column index of column atribute
 * @param *row  list with row elements
 * @return atribute data
 */
char *get_row_attr_data(int column, struct list_node *node);
//struct list_node *AK_get_table_row(int num, char * tblName);
TestResult AK_table_test() ;

/**
 * @author Mislav Čakarić edited by Ljubo Barać
 * @brief Function for renaming table and/or attribute in table (moved from rename.c)
 * @param old_table_name old name of the table
 * @param new_table_name new name of the table
 * @param old_attr name of the attribute to rename
 * @param new_attr new name for the attribute to rename
 * @return EXIT_ERROR or EXIT_SUCCESS
 */
int AK_rename(char *old_table_name, char *old_attr, char *new_table_name, char *new_attr);
TestResult AK_op_rename_test() ;

// NOVO: Deklaracije funkcija za problem #143
void AK_update_table_timestamp(const char* tableName);
time_t AK_get_table_timestamp(const char* tableName);

#endif

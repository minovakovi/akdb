/**
@file test.h Header file that provides functions and defines for testing purposes
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

#ifndef TEST
#define TEST

#include "files.h"
#include "../auxi/mempro.h"

/**
 * @author Goran Štrok
 * @brief returns a string containing attribute types for the supplied table name, seperated by ATTR_DELIMITER
 * @param tblName name of the table for which the attribute types will be returned
 */

char* AK_get_table_atribute_types(char* tblName);

/**
 * @author Luka Rajcevic
 * @brief Function for creating test table header
 * @param tbl_name - name of the table for which the header will be created
 * @param attr_name - array of attribute names
 * @param _num - number of attributes
 * @param _type - array of attribute types (eg. TYPE_INT, TYPE_VARCHAR, etc.)
 * @return 1 if ok, 0 otherwise
 */
int create_header_test(char* tbl_name, char** attr_name, int _num, int* _type);


/**
 * @author Luka Rajcevic
 * @brief Function for inserting test data into the table (needed for python testing)
 * @param tbl_name - name of the table for which the header will be created
 * @param attr_name - array of attribute names
 * @param attr_value - values of attributes to be inserted
 * @param _num - number of attributes
 * @param _type - array of attribute types (eg. TYPE_INT, TYPE_VARCHAR, etc.)
 * @return EXIT_SUCCESS if ok, EXIT_ERROR otherwise
 */
int insert_data_test(char* tbl_name, char** attr_name, char** attr_value, int _num, int* _type);

/**
 * @author Luka Rajcevic
 * @brief Function for selection operator on one table
 + @param src_table - name of the source table
 + @param dest_table - table in which selection will be stored
 * @param sel_query - array of operators, operands and attributes (postfix query)
 * @param _num - number of attributes
 * @param _type - array of attribute types (eg. TYPE_INT, TYPE_VARCHAR, etc.)
 * @return EXIT_SUCCESS if ok, EXIT_ERROR otherwise
 *
 */
int selection_test(char* src_table, char* dest_table, char** sel_query, int _num, int* _type);

/**
 * @author Luka Rajcevic
 * @brief Function that prints the requested column
 * @return 1 if column is found, 0 otherwise
 * @param num - 0 based index of column
 * @param tbl - name of the table
 */
int get_column_test(int num, char* tbl);

/**
 * @author Luka Rajcevic
 * @brief Function that prints the requested row
 * @return 1 if row is found, 0 otherwise
 * @param num - 0 based index of row
 * @param tbl - name of the table
 */

int get_row_test(int num, char* tbl);

/**
 * @author Dino Laktašić
 * @brief Function for creating test tables
 * @return No return value
 */
void AK_create_test_tables();

#endif


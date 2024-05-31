/**
@file theta_join.h Header file that provides functions and defines for theta-join
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


#ifndef THETA_JOIN
#define THETA_JOIN

#include "../auxi/test.h"
#include "expression_check.h"
#include "../file/fileio.h"
#include "../auxi/mempro.h"

//int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, AK_list *constraints);

/**
 * @author Tomislav Mikulček,updated by Nikola Miljancic
 * @brief Function that creates a theta join betwen two tables on specified conditions.
 * @param srcTable1 name of the first table to join
 * @param srcTable2 name of the second table to join
 * @param constraints list of attributes, (in)equality and logical operators which are the conditions for the join in postfix notation
 * @param dstTable name of the theta join table
 * @return if successful returns EXIT_SUCCESS and EXIT_ERROR otherwise
 *   
 */
int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, struct list_node *constraints);

/**
 * @author Tomislav Mikulček
 * @brief  Function that creates a header of the new table for theta join
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param new_table name of the destination table
 * @return EXIT_SUCCESS if the header was successfully created and EXIT_ERROR if the renamed headers are too long
 */
int AK_create_theta_join_header(char *srcTable1, char * srcTable2, char *new_table);/*
void AK_check_constraints(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, int tbl1_num_att, int tbl2_num_att, AK_list *constraints, char *new_table);
int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, AK_list *constraints);*/

/**
 * @author Tomislav Mikulček
 * @brief Function that iterates through blocks of the two tables and copies the rows which pass the constraint check into the new table
 * @param tbl1_temp_block block of the first table
 * @param tbl2_temp_block block of the second join table
 * @param tbl1_num_att number of attributes in the first table
 * @param tbl2_num_att number of attributes in the second table
 * @param constraints list of attributes, (in)equality and logical operators which are the conditions for the join in postfix notation
 * @param new_table name of the theta_join table
 * @return No return value
 */
void AK_check_constraints(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, int tbl1_num_att, int tbl2_num_att, struct list_node *constraints, char *new_table);
int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, struct list_node *constraints);
TestResult AK_op_theta_join_test();

#endif /* THETA_JOIN */

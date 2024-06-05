/**
@file projection.h Header file that provides data structures, functions and defines for relational projection operation
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
 
#ifndef PROJECTION
#define PROJECTION

#include "../auxi/test.h"
#include "expression_check.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../auxi/mempro.h"

 struct AK_operand {
	char value[MAX_VARCHAR_LENGTH];
	int type;
};


/**
    * @author Matija Novak, rewritten and optimized by Dino Laktašić to support AK_list  
    * @brief  Function that creates a new header for the projection table
    * @param old_block_add address of the block from which we copy headers we need
    * @param dstTable name of the new table - destination table
    * @param att list of the attributes which should the projection table contain
    * @result Newly created header
    * @return No return value
 
 */

void AK_create_block_header(int old_block, char *dstTable, struct list_node *att);

/**
    * @author Leon Palaić
    * @brief Function that fetches arithmetic operator from given expression string, determinates given operator so it can be used for aritmetic operations
    * @param exp input expression string
    * @result character - aritmetic operator
    * @return character 

*/
char *AK_get_operator(char *exp);

/**
    * @author Leon Palaić
    * @brief Function that iterates through given string and removes specified part of that string. 
    * @param s input string 
    * @param substring string that needs to be removed
    * @result Cleaned new string
    * @return No return value
*/
void AK_remove_substring(char *s,const char *substring);

/**
    * @author Leon Palaić
    * @brief Function that determines the new header type
    * @param firstOperand operand type
    * @param secondOperand operand type
    * @result Function returns determinated header type
    * @return Integer - type


*/
int AK_determine_header_type(int firstOperand,int secondOperand);

/**
    * @author Leon Palaić
    * @brief  Function that creates new header name from passed operand names and operator
    * @param first operand name
    * @param second operand name
    * @param operator given operator
    * @result Function returns set of characters that represent new header name
    * @return Character - new name

*/
char *AK_create_header_name(char * first,char *operator, char * second);

/** 
  * @author Matija Novak, rewritten and optimized by Dino Laktašić to support AK_list
  * @brief  Function that copies the data from old table block to the new projection table
  * @param old_block block from which we copy data
  * @param dstTable name of the new table
  * @param att list of the attributes which should the projection table contain
  * @param expr given expression to check 
  * @result New projection table that contains all blocks from old table
  * @return No return value
 */
void AK_copy_block_projection(AK_block *old_block, struct list_node *att, char *dstTable,struct list_node *expr); 

/**
    * @author Leon Palaić
    * @brief Function that performes arithmetics operation depended on given operator
    * @param firstOperand first operand
    * @param secondOperand second operand 
    * @param op aritmetic operator
    * @param type type of operand 
    * @result result of arithmetic operation
    * @return character

*/
char *AK_perform_operation(char *op,struct AK_operand *firstOperand, struct AK_operand *secondOperand,int type);


/**
 * @author Matija Novak, rewritten and optimized by Dino Laktašić, now support cacheing
 * @brief  Function that makes a projection of some table on given attributes
 * @param srcTable source table - table on which projection is made
 * @param expr given expression to check while doing projection
 * @param att list of atributes on which we make projection
 * @param dstTable table name for projection table - new table - destination table
 * @result Projection table on given attributes
 * @return EXIT_SUCCESS if continues succesfuly, when not EXIT_ERROR
 */
int AK_projection(char *srcTable, char *dstTable, struct list_node *att, struct list_node *expr);


/**
 * @author Dino Laktašić, rewritten and optimized by Irena Ilišević to support ILIKE operator and perform usual projection 
 * @brief  Function for projection operation testing, tests usual projection functionality, projection when it is given aritmetic operation or expresson
 * @result Projection tables and number od passed tests
 * @return Test result - number of successful and unsuccessful tests
 */
TestResult AK_op_projection_test();



#endif

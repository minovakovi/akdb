/**
@file expression_check.h Header file that functions and defines for expression ckecking
 */
/*
 * This program is free software; you can redistribute it and/or modifys
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

#ifndef EXPRESSION_CHECK
#define EXPRESSION_CHECK

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../auxi/mempro.h"
#include <regex.h>
/*
int AK_check_arithmetic_statement(AK_list_elem el, const char *op, const char *a, const char *b);
int AK_check_if_row_satisfies_expression(AK_list_elem row_root, AK_list *expr);
*/

/**
 * @author Dino Laktašić, abstracted by Tomislav Mikulček,updated by Nikola Miljancic
 * @brief  Function that compares values according to their data type, checks arithmetic statement which is part of expression given in 
 *   	  the function below.
 * @param el list element, last element put in list temp which holds elements of row ordered according to expression and results of their evaluation
 * @param *op comparison operator
 * @param *a left operand
 * @param *b right operand
 * @return 0 if arithmetic statement is false, 1 if arithmetic statement is true
 */
int AK_check_arithmetic_statement(struct list_node *el, const char *op, const char *a, const char *b);
/**
	* @Author Leon Palaić
	* @brief Function that replaces charachter wildcard (%,_) ch in string s with repl characters.
	* @param s input string
	* @param ch charachter to be replaced
	* @result new sequence of charachters

*/
int AK_check_if_row_satisfies_expression(struct list_node *row_root, struct list_node *expr);
/**
	* @Author Leon Palaić
	* @brief Function that evaluates regex expression on a given string input.
	* @param value string value that must match regex expression
	* @param expression POSIX regex expression
	* @param checkWildCard replaces SQL wildcard to correesponding POSIX regex charachter
	* @param sensitive case insensitive indicator 1-case sensitive,0- case insensitive
	* @result 0 if regex didnt match or sytnax of regex is incorecct 
			  1 if string matches coresponding regex expression
*/
int AK_check_regex_expression(const char *value,const char *expression,int sensitive,int checkWildCard);
/**
	* @Author Leon Palaić
	* @brief Function that evaluates regex expression on a given string input.
	* @param value string value that must match regex expression
	* @param expression POSIX regex expression
	* @result 0 if regex didnt match or sytnax of regex is incorecct 
			  1 if string matches coresponding regex expression
*/
int AK_check_regex_operator_expression(const char *value,const char *expression);
TestResult AK_expression_check_test();

#endif /* CONSTRAINT_CHECKER_H_ */

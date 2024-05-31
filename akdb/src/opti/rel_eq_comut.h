/**
@file rel_eq_comut.h Header file that provides data structures,functions and defines for relational equivalences regarding comutativity
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

#ifndef REL_EQ_COMUT
#define REL_EQ_COMUT

#include "../auxi/test.h"
#include "../file/table.h"
#include "./rel_eq_selection.h"
#include "../auxi/mempro.h"
#include "../auxi/auxiliary.h"

/**
 * @author Davor Tomala
 * @brief Function for printing optimized relation equivalence expression list regarding commutativity
 * @param *list_rel_eq RA expresion as the struct list_node
 */
void AK_print_rel_eq_comut(struct list_node *list_rel_eq);

/**
 * @author Davor Tomala
 * @brief Main function for generating RA expresion according to commutativity equivalence rules 
 * @param *list_rel_eq RA expresion as the struct list_node
 * @return optimised RA expresion as the struct list_node
 */
struct list_node *AK_rel_eq_comut(struct list_node *list_rel_eq);

/**
 * @author Dino Laktašić.
 * @brief Function that checks if the selection can commute with theta-join or product
 * <ol>
 * <li>For each token (delimited by " ") in selection condition first check if token represents attribute/s and is subset in the given table</li>
 * <li>If token is a subset set variable id to 1</li>
 * <li>else set id to 0, else make no changes to variable id</li>
 * <li>if token differs from "AND" and "OR" and id equals to 1 append current token to result condition</li>
 * <li>else if token equals to "AND" or "OR" and id equals to 1 and there are two added tokens add "AND" or "OR" to condition string</li>
 * <li>When exits from loop, return pointer to char array that contains new condition for a given table
 * </ol>
 * @param *cond condition array that contains condition data 
 * @param *tblName name of the table
 * @result pointer to char array that contains new condition for a given table
 */

char *AK_rel_eq_commute_with_theta_join(char *cond, char *tblName);

/**
 * @author Dino Laktašić (AK_rel_eq_commute_with_theta_join), Davor Tomala (AK_rel_eq_comut)
 * @brief Function that tests relational equivalences regarding commutativity
 * @return No return vlaue
 */
TestResult AK_rel_eq_comut_test();

#endif


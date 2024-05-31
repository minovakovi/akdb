/**
@file query_optimization.h Header file that provides data structure, functions and defines for general query optimization
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

#ifndef QUERY_OPTIMIZATION
#define QUERY_OPTIMIZATION

#include "../auxi/test.h"
#include "rel_eq_comut.h"
#include "rel_eq_assoc.h"
#include "rel_eq_projection.h"
#include "rel_eq_selection.h"

#include "../auxi/mempro.h"
#include "../sql/view.h"

/**
 * @def MAX_PERMUTATION
 * @brief Constant declaring maximum number of permutations 
*/
#define MAX_PERMUTATION 24

/**
 * @author Dino Laktašić.
 * @brief Function that prints optimization table for testing purposes
 * @param *list_query optimized RA expresion list 
 * @result list output
 */
void AK_print_optimized_query(struct list_node *list_query);

/**
 * @author Dino Laktašić.
 * @brief Function that calls and executes relation equivalence
 * RELATION EQUIVALENCE RULES FLAGS
 * c - commutation
 * a - associativity
 * p - projection
 * s - selection	
 * @param *list_query RA expresion list where we need to apply relational equivalences rules
 * @param rel_eq rel_eq to execute 
 * @param *FLAGS flags for relation equivalences (execute rel_eq for given flags) 
 * @return returns struct list_node (RA expresion list) optimized by given relational equivalence rule 
*/ 
struct list_node *AK_execute_rel_eq(struct list_node *list_query, const char rel_eq, const char *FLAGS);

/**
 * @author Dino Laktašić.
 * @brief Function that executes all relational equivalences provided by FLAGS (one or more), 
 * if DIFF_PLANS turned on execute permutations without repetition on given RA list from SQL parser output
 * @param *list_query RA expresion list where we need to apply relational equivalences rules
 * @param *FLAGS flags for relation equivalences (execute rel_eq for given flags) 
 * @return returns AK_list (RA expresion list) optimized by all relational equivalence rules provided by FLAGS 
 * (commented code can be edited so AK_list can return the list of lists (lists of different optimization plans),
 * with permutation switched on (DIFF_PLANS = 1) time for execution will be significantly increased
 * Current implementation without uncommenting code doesn't produce list of list, 
 * it rather apply all permutations on the same list
 * 
 * For futher development consider to implement cost estimation for given plan based on 
 * returned heuristicly optimized list
 */
struct list_node *AK_query_optimization(struct list_node *list_query, const char *FLAGS, const int DIFF_PLANS);
TestResult AK_query_optimization_test() ; // (struct list_node *list_query)

#endif

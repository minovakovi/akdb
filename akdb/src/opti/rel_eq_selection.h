/**
@file rel_eq_selection.h Header file that provides data structures, functions and defines for relational equivalences in selection
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

#ifndef REL_EQ_SELECTION
#define REL_EQ_SELECTION

#include "../auxi/test.h"
#include "../file/table.h"
#include "../auxi/mempro.h"


/**
 * @author Dino Laktašić.
 * @brief Function that checks if some set of attributes is subset of larger set
 * @param *set set array 
 * @param *subset subset array 
 * @return EXIT_SUCCESS if some set of attributes is subset of larger set, else returns EXIT_FAILURE
 */
int AK_rel_eq_is_attr_subset(char *set, char *subset) ;

/**
 * @author Dino Laktašić.
 * @brief Function that fetches attributes for a given table and store them to the char array 
 * @param *tblName name of the table
 * @result pointer to char array 
 */
char *AK_rel_eq_get_atrributes_char(char *tblName) ;

/**
 * @author Dino Laktašić.
 * @brief Function for filtering and returning attributes from condition 
 * @param *cond condition array that contains condition data
 * @result pointer to array that contains attributes for a given condition
 */
char *AK_rel_eq_cond_attributes(char *cond) ;

/**
 * @author Dino Laktašić.
 * @brief Function that checks if two sets share one or more of it's attributes
 * @param *set first set of attributes delimited by ";" (ATTR_DELIMITER)
 * @param *subset second set of attributes delimited by ";" (ATTR_DELIMITER)
 * @result EXIT_SUCCESS if set and subset share at least one attribute, else returns EXIT_FAILURE 
 */
int AK_rel_eq_share_attributes(char *set, char *subset) ;


/**
 * @author Dino Laktašić.
 * @brief Break conjunctive conditions to individual conditions 
 * @param *cond condition expression
 * @result conditions list
 */
struct list_node *AK_rel_eq_split_condition(char *cond) ;
/**
 * @author Dino Laktašić.
 * @brief Main function for generating RA expresion according to selection equivalence rules 
 * @param *list_rel_eq RA expresion as the struct list_node
 * @return optimised RA expresion as the struct list_node
 */
struct list_node *AK_rel_eq_selection(struct list_node *list_rel_eq);

/**
 * @author Dino Laktašić.
 * @brief Function for printing struct list_node to the screen 
 * @param *list_rel_eq RA expresion as the struct list_node
 * @return void
 */
void AK_print_rel_eq_selection(struct list_node *list_rel_eq) ;
/**
 * @author Dino Laktašić.
 * @brief Function for testing rel_eq_selection
 * @return No return value
 */
TestResult AK_rel_eq_selection_test();

#endif


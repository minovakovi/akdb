/**
@file rel_eq_projection.h Header file that provides data structures, functions and defines for relational equivalences in projection
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

#ifndef REL_EQ_PROJECTION
#define REL_EQ_PROJECTION

#include "../auxi/test.h"
#include "../file/table.h"
#include "../auxi/mempro.h"


/**
 * @author Dino Laktašić.
 * @brief Function that checks if some set of attributes is subset of larger set, used in cascading of the projections
 * <ol>
 * <li>Tokenize set and subset of projection attributes and store each of them to it's own array</li>
 * <li>Check if the size of subset array is larger than the size of set array</li>
 * <li>if the subset array is larger return 0</li>
 * <li>else sort both arrays ascending</li>
 * <li>Compare the subset and set items at the same positions, starting from 0</li>
 * <li>if there is an item in the subset array that doesn't match attribute at the same position in the set array return 0</li>
 * <li>else continue comparing until final item in the subset array is ritched</li>
 * <li>on loop exit return EXIT_SUCCESS</li>
 * </ol>
 * @param list_elem_set first list element containing projection attributes 
 * @param list_elem_subset second list element containing projection attributes 
 * @return EXIT_SUCCESS if some set of attributes is subset of larger set, else returns EXIT_FAILURE
 */
int AK_rel_eq_is_subset(struct list_node *list_elem_set, struct list_node *list_elem_subset);

/**
 * @author Dino Laktašić.
 * @brief Function that checks if selection uses only attributes retained by the projection before commuting
 * <ol>
 * <li>Tokenize set of projection attributes and store them to the array</li>
 * <li>For each attribute in selection condition check if exists in array of projection attributes</li>
 * <li>if exists increment match variable and break</li>
 * <li>else continue checking until the final attribute is checked</li>
 * <li>if match variable value equals 0 than return 0</li>
 * <li>else if match variable value greater than EXIT_SUCCESS, return EXIT_FAILURE</li>
 * </ol>
 * @param list_elem_attribs list element containing projection data 
 * @param list_elem_conds list element containing selection condition data
 * @return EXIT_SUCCESS if selection uses only attributes retained by projection, else returns EXIT_FAILURE
 */
int AK_rel_eq_can_commute(struct list_node *list_elem_attribs, struct list_node *list_elem_conds);

/**
 * @author Dino Laktašić.
 * @brief Function that gets attributes for a given table and store them to the struct list_node 
 * <ol>
 * <li>Get the number of attributes in a given table</li>
 * <li>Get the table header for a given table</li>
 * <li>Initialize struct list_node</li>
 * <li>For each attribute in table header, insert attribute in struct list_node as new struct list_node element</li>
 * <li>return struct list_node</li>
 * </ol>
 * @param *tblName name of the table
 * @result struct list_node
 */
struct list_node *AK_rel_eq_get_attributes(char *tblName);

/**
 * @author Dino Laktašić.
 * @brief Function used for filtering and returning only those attributes from list of projection attributes that exist in the given table  
 * <ol>
 * <li>Get the attributes for a given table and store them to the AK_list</li>
 * <li>Tokenize set of projection attributes and store them to the array</li>
 * <li>For each attribute in the array check if exists in the previously created AK_list</li>
 * <li>if exists append attribute to the dynamic atributes char array</li>
 * <li>return pointer to char array with stored attribute/s</li>
 * </ol>
 * @param *attribs projection attributes delimited by ";" (ATTR_DELIMITER)
 * @param *tblName name of the table
 * @return filtered list of projection attributes as the AK_list
 */
char *AK_rel_eq_projection_attributes(char *attribs, char *tblName);

/**
 * @author Dino Laktašić.
 * @brief Function used for filtering and returning only attributes from selection or theta_join condition 
 * @param list_elem list element that contains selection or theta_join condition data
 * @return only attributes from selection or theta_join condition as the AK_list
 */
char *AK_rel_eq_collect_cond_attributes(struct list_node *list_elem) ;

/**
 * @author Dino Laktašić.
 * @brief Function which removes duplicate attributes from attributes expresion 
 * @param *attribs attributes from which to remove duplicates
 * @return pointer to char array without duplicate attributes
 */
char *AK_rel_eq_remove_duplicates(char *attribs);

/**
 * @author Dino Laktašić.
 * @brief Main function for generating RA expresion according to projection equivalence rules 
 * @param *list_rel_eq RA expresion as the AK_list
 * @return optimised RA expresion as the AK_list
 */
struct list_node *AK_rel_eq_projection(struct list_node *list_rel_eq) ;

/**
 * @author Dino Laktašić.
 * @brief Function for printing AK_list to the screen 
 * @param *list_rel_eq RA expresion as the AK_list
 * @return No return value
 */
void AK_print_rel_eq_projection(struct list_node *list_rel_eq) ;

/**
 * @author Dino Laktašić.
 * @brief Function for testing rel_eq_selection
 * @return No return value 
 */
TestResult AK_rel_eq_projection_test();

#endif

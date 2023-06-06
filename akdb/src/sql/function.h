/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 @file function.h Header file that provides functions and function definitions.
*/

#ifndef FUNCTION
#define FUNCTION

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../auxi/mempro.h"
#include "../auxi/auxiliary.h"


/**
 * @author Unknown, updated by Jurica Hlevnjak - check function arguments included for drop purpose, updated by Tomislav Ilisevic
 * @brief Function that gets obj_id of a function by name and arguments list (transferred from trigger.c/drop.c).
 * @param [out] function name of the function
 * @param [out] arguments_list list of arguments
 * @return obj_id of the function or EXIT_ERROR
 */
int AK_get_function_obj_id(char* function, struct list_node *arguments_list);

/**
 * @author Boris Kišić
 * @brief Function that checks whether arguments belongs to a function.
 * @param [out] function_id id of the function
 * @param [out] arguments_list list of arguments
 * @return EXIT_SUCCESS of the function or EXIT_ERROR
 */
int AK_check_function_arguments(int function_id, struct list_node *arguments_list);

/**
 * @author Jurica Hlevnjak, updated by Aleksandra Polak
 * @brief Function that checks whether arguments belong to a function but only checks argument type (not name). Used for drop function.
 * @param [out] function_id id of the function
 * @param [out] args function arguments 
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_check_function_arguments_type(int function_id, struct list_node *args);

/**
 * @author Boris Kišić, updated by Tomislav Ilisevic
 * @brief Function that adds a function to system table.
 * @param [out] name name of the function
 * @param [out] return_type data type returned from a function - values from 0 to 13 - defined in constants.h
 * @param [out] arguments_list list of function arguments
 * @return function id or EXIT_ERROR
 */
int AK_function_add(char *name, int return_type, struct list_node *arguments_list);

/**
 * @author Boris Kišić
 * @brief Function that adds a function argument to system table.
 * @param [out] function_id id of the function to which the argument belongs
 * @param [out] arg_number number of the argument
 * @param [out] arg_type data type of the argument
 * @param [out] argname name of the argument
 * @return function argument id or EXIT_ERROR
 */
int AK_function_arguments_add(int function_id, int arg_number, int arg_type, char *argname);

/**
 * @author Boris Kišić, updated by Fran Turković
 * @brief Function that removes a function by its obj_id.
 * @param [out] obj_id obj_id of the function
 * @param [out] num_args number of agruments
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_remove_by_obj_id(int obj_id, int num_args);

/**
 * @author Boris Kišić
 * @brief Function that removes function arguments by function id.
 * @param [out] obj_id obj_id of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_arguments_remove_by_obj_id(int *obj_id);

/**
 * @author Boris Kišić
 * @brief Function that removes a function from system table by name and arguments.
 * @param [out] name name of the function
 * @param [out] arguments_list list of arguments
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_remove_by_name(char *name, struct list_node *arguments_list);

/**
 * @author Boris Kišić
 * @brief Function that changes the function name.
 * @param [out] name name of the function to be modified
 * @param [out] arguments_list list of arguments to be modified
 * @param [out] new_name new name of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_rename(char *name, struct list_node *arguments_list, char *new_name);

/**
 * @author Boris Kišić
 * @brief Function that changes the return type.
 * @param [out] name name of the function to be modified
 * @param [out] arguments_list list of function arguments
 * @param [out] new_return_type new return type
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_change_return_type(char *name, struct list_node *arguments_list, int new_return_type);
TestResult AK_function_test();

/**
 * @author Andrej Hrebak Pajk
 * @brief Function that retrieves the details of a function by its obj_id.
 * @param[out] obj_id obj_id of the function
 * @param[out] name pointer to store the name of the function
 * @param[out] return_type pointer to store the return type of the function
 * @param[out] arguments_list pointer to store the list of function arguments
 * @return EXIT_SUCCESS if the function details are successfully retrieved, or EXIT_ERROR otherwise
 */
int AK_get_function_details_by_obj_id(int obj_id, char **name, int *return_type, struct list_node **arguments_list);

#endif
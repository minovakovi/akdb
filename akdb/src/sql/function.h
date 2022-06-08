/**
@file function.h Header file that provides functions and defines for functions
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
 * @param *function name of the function
 * @param *arguments_list list of arguments
 * @return obj_id of the function or EXIT_ERROR
 */
int AK_get_function_obj_id(char* function, struct list_node *arguments_list);

/**
 * @author Boris Kišić
 * @brief Function that checks whether arguments belongs to a function
 * @param *function_id id of the function
 * @param *arguments_list list of arguments
 * @return EXIT_SUCCESS of the function or EXIT_ERROR
 */
int AK_check_function_arguments(int function_id, struct list_node *arguments_list);

/**
 * @author Jurica Hlevnjak updated by Aleksandra Polak
 * @brief Function that checks whether arguments belongs to a function but only checks argument type (not name). Used for drop function.
 * @param function_id id of the function
 * @param args function arguments 
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_check_function_arguments_type(int function_id, struct list_node *args);

/**
 * @author Boris Kišić, updated by Tomislav Ilisevic
 * @brief Function that adds a function to system table.
 * @param *name name of the function
 * @param *return_type data type returned from a function - values from 0 to 13 - defined in constants.h
 * @param *arguments_list list of function arguments
 * @return function id or EXIT_ERROR
 */
int AK_function_add(char *name, int return_type, struct list_node *arguments_list);

/**
 * @author Boris Kišić
 * @brief Function that adds a function argument to system table.
 * @param *function_id id of the function to which the argument belongs
 * @param *arg_number number of the argument
 * @param *arg_type data type of the argument
 * @param *argname name of the argument
 * @return function argument id or EXIT_ERROR
 */
int AK_function_arguments_add(int function_id, int arg_number, int arg_type, char *argname);

/**
 * @author Boris Kišić
 * @brief Function that removes a function by its obj_id.
 * @param obj_id obj_id of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_remove_by_obj_id(int obj_id);

/**
 * @author Boris Kišić
 * @brief Function that removes function arguments by function id.
 * @param obj_id obj_id of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_arguments_remove_by_obj_id(int *obj_id);

/**
 * @author Boris Kišić
 * @brief Function that removes a function from system table by name and arguments.
 * @param *name name of the function
 * @param *arguments_list list of arguments
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_remove_by_name(char *name, struct list_node *arguments_list);

/**
 * @author Boris Kišić
 * @brief Function that changes the function name.
 * @param *name name of the function to be modified
 * @param *arguments_list list of arguments to be modified
 * @param *new_name new name of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_rename(char *name, struct list_node *arguments_list, char *new_name);

/**
 * @author Boris Kišić
 * @brief Function that changes the return type.
 * @param *name name of the function to be modified
 * @param *arguments_list list of function arguments
 * @param *new_return_type new return type
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_change_return_type(char *name, struct list_node *arguments_list, int new_return_type);
TestResult AK_function_test();

#endif

/**
@file trigger.h Header file that provides functions and defines for  trigger.c
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

#ifndef TRIGGER
#define TRIGGER

#include "../auxi/test.h"
#include "../rec/archive_log.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../file/id.h"
#include "../sql/function.h"
#include "../rel/selection.h"
#include "../auxi/mempro.h"


/**
 * @author Unknown, updated by Mario Peroković, check if data is TYPE_INT
 * @brief Function that saves conditions for a trigger.
 * @param trigger obj_id of the trigger in question
 * @param *condition AK_list list of conditions
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_save_conditions(int trigger, struct list_node *condition);

/**
 * @author Unknown updated by Aleksandra Polak
 * @brief Function that adds a trigger to the system table.
 * @param *name name of the trigger
 * @param *event event that calls the trigger - this should perhaps be an integer with defined constants...
 * @param *condition AK_list list of conditions in postfix
 * @param *table name of the table trigger is hooked on
 * @param *function function that is being called by the trigger
 * @return trigger id or EXIT_ERROR
 */
int AK_trigger_add(char *name, char* event, struct list_node *condition, char* table, char* function, struct list_node *arguments_list);

/**
 * @author
 * @brief Function that gets obj_id of a trigger defined by name and table.
 * @param *name name of the trigger
 * @param *table name of the table on which the trigger is hooked
 * @return obj_id of the trigger or EXIT_ERROR
 */
int AK_trigger_get_id(char *name, char *table);

/**
 * @author Unknown
 * @brief Function that removes a trigger from the system table by name
 * @param *name name of the trigger
 * @param *table name of the table
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_remove_by_name(char *name, char *table) ;

/**
 * @author Unknown
 * @brief Function that removes a trigger by its obj_id
 * @param obj_id obj_id of the trigger
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_remove_by_obj_id(int obj_id);

/**
 * @author Unknown, fixed by Josip Susnjara
 * @brief Function that edits information about the trigger in system table.
 * @param *name name of the trigger (or NULL if using obj_id)
 * @param *event event of the trigger (or NULL if it isn't changing)
 * @param *condition list of conditions for trigger (or NULL if it isn't changing; empty list if all conditions are to be removed)
 * @param *table name of the connected table (or NULL id using obj_id)
 * @param *function name of the connected function (or NULL if it isn't changing)
 * @param *arguments_list arguments of the function (without arguments can't find passed function) 
 * @return EXIT_SUCCESS or EXIT_ERROR
*/
int AK_trigger_edit(char *name, char* event, struct list_node* condition, char* table, char* function, struct list_node *arguments_list);

/**
 * @author Unknown, updated by Mario Peroković
 * @brief Function that fetches postfix list of conditions for the trigger (compatible with selection)
 * @param trigger obj_id of the trigger
 * @return list of conditions for the trigger
 */
struct list_node *AK_trigger_get_conditions(int trigger);

/**
* @author Ljubo Barać
* @brief Function that renames the trigger
* @param old_name Name of the trigger to be renamed
* @param new_name New name of the trigger
* @return EXIT_SUCCESS or EXIT_ERROR
*/
int AK_trigger_rename(char *old_name, char *new_name, char *table);
TestResult AK_trigger_test();

#endif

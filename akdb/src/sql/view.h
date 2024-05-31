/**
@file function.h Header file that provides functions and defines for view.c
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

#ifndef VIEW
#define VIEW

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/id.h"
#include "../file/fileio.h"
#include "../auxi/mempro.h"

/**
 * @author Sara Kisic
 * @brief Function that checks if the name of the view already exists in AK_view table
 * @param name Name of the view
 * @return EXIT_ERROR if the name already exists or name
 */
char* AK_check_view_name(char *name);

/**
 * @author Kresimir Ivkovic
 * @brief Function that adds a new view to the view table with the corresponding name and value (view query); 
 * set_id is optional, if it's not set, the system will determine the new id automatically
 * @param name name og the view
 * @param query query of the view
 * @param rel_exp relation expression of the view
 * @param set_id id of view
 * @return Id of the newly inserted view
 */
int AK_view_add(char *name, char *query, char *rel_exp, int set_id);

/**
 * @author Kresimir Ivkovic
 * @brief Function that removes the view by its name by identifying the 
 * view's id and passing id to AK_view_remove_by_obj_id
 * @param name name of the view
 * @return Result of AK_view_remove_by_obj_id or EXIT_ERROR if no id is found
 */
int AK_view_remove_by_name(char *name);

/**
 * @author Kresimir Ivkovic
 * @brief Function that renames a view (based on it's name) from "name" to "new_name"
 * @param name name of the view
 * @param new_name new name of the view
 * @return error or success
 */
int AK_view_rename(char *name, char *new_name);

/**
 * @author Kresimir Ivkovic
 * @brief Function that changes the query from a view (determined by it's name) to "query"
 * @param name of the query
 * @param query new query of the view
 * @param rel_exp relation expression of the view
 * @return error or success
 */
int AK_view_change_query(char *name, char *query, char *rel_exp);
TestResult AK_view_test();

char* AK_get_view_query(char *name);

#endif

/**
@file select.h Header file that provides functions  for select.h
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


#include "../file/table.h"
#include "../auxi/test.h"
#include "../file/fileio.h"
#include "../rel/selection.h"
#include "../rel/projection.h"
#include "../auxi/auxiliary.h"
#include "../auxi/mempro.h"
#include "../file/filesort.h"


/**
 * @brief Structure to hold column name and its alias
 */
struct column_alias {
    char original_name[MAX_ATT_NAME];
    char alias_name[MAX_ATT_NAME];
};

/**
 * @brief Structure to hold collection of column aliases 
 */
struct column_alias_collection {
    struct column_alias *aliases;
    int count;
};


/**
 * @author Filip Žmuk, Edited by: Marko Belusic, Matija Karaula
 * @brief Function that implements SELECT relational operator
 * @param src_table original table that is used for selection
 * @param dest_table table that contains the result
 * @param attributes attributes to be selected
 * @param aliases column aliases array
 * @param alias_count number of aliases
 * @param condition condition for selection
 * @param ordering attributes for result sorting
 * @return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
 */
int AK_select(char *src_table, 
              char *dest_table, 
              struct list_node *attributes,
              struct column_alias *aliases,
              int alias_count,
              struct list_node *condition,
              struct list_node *ordering);


int AK_select_into(char *src_table, 
                   char *new_table,
                   struct list_node *attributes,
                   struct column_alias *aliases,
                   int alias_count,
                   struct list_node *condition, 
                   struct list_node *ordering);

TestResult AK_select_test();

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
 @file drop.h Header file that provides data structures, functions and defines for unique constraint
*/

#ifndef DROP
#define DROP

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../file/sequence.h"
#include "view.h"
#include "trigger.h"
#include "function.h"
#include "privileges.h"
#include "../auxi/mempro.h"
#include "../auxi/constants.h"
#include "./cs/unique.h"
#include "./cs/between.h"
#include "./cs/nnull.h"
#include "./cs/check_constraint.h"

struct drop_arguments {
    void *value;
    struct drop_arguments *next;
};

typedef struct drop_arguments AK_drop_arguments;

/**
 * @author Unknown, Jurica Hlevnjak, updated by Tomislav Ilisevic, Maja Vračan, Fran Turković
 * @brief Function for DROP table, index, view, sequence, trigger, function, user, group and constraint.
 * @param type drop type
 * @param drop_arguments arguments of DROP command
 */
int AK_drop(int type, AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific table
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_table(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific index
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_index(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific view
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_view(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific sequence
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_sequence(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific trigger
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_trigger(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific function
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_function(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific user
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_user(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific group
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_group(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @brief Drop function that deletes specific group
 * @param drop_arguments arguments of DROP command 
 */
int AK_drop_constraint(AK_drop_arguments *drop_arguments);

/**
 * @author unknown, Jurica Hlevnjak - fix bugs and reorganize code in this function
 * @brief Help function for the drop command. Delete memory blocks and addresses of table 
 * and removes table or index from system table.
 * @param tblName name of table or index
 * @param sys_table name of system catalog table
 */
void AK_drop_help_function(char *tblName, char *sys_table);

/**
 * @author Jurica Hlevnjak, updated by Tomislav Ilisevic
 * @brief Help function for checking if the element(view, function, sequence, user ...) exist in system catalog table
 * @param tblName name of table, index view, function, trigger, sequence, user, group or constraint
 * @param sys_table name of system catalog table
 * @return if element exist in system catalog returns 1, if not returns 0
 */
int AK_if_exist(char *tblName, char *sys_table);

/**
 * @author unknown, Jurica Hlevnjak - added all tests except drop table test, updated by Tomislav Ilisevic, Maja Vračan, Fran Turković
 * @brief Function for testing all DROP functions
 */
TestResult AK_drop_test(); 

#endif
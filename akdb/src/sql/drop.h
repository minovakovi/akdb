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
 @file drop.h Header file that provides data structures, functions and defines for DROP operations.
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
 * @updated by Antonio Brković
 * @brief Function for DROP table, index, view, sequence, trigger, function, user, group and constraint.
 * @param type drop type
 * @param drop_arguments arguments of DROP command
 */
int AK_drop(int type, AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_table(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_index(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_view(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_sequence(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_trigger(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_function(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_user(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_group(AK_drop_arguments *drop_arguments);

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_constraint(AK_drop_arguments *drop_arguments);

/**
 * @author unknown, Jurica Hlevnjak
 * @updated by Antonio Brković
 */
void AK_drop_help_function(char *tblName, char *sys_table);

/**
 * @author Jurica Hlevnjak
 * @updated by Antonio Brković
 */
int AK_if_exist(char *tblName, char *sys_table);

/**
 * @author unknown, Jurica Hlevnjak
 * @updated by Antonio Brković
 */
TestResult AK_drop_test();

/**
 * @author Antonio Brković
 * @brief Generic drop handler for entities with name and optional remove function
 * @param drop_arguments drop argument list
 * @param sys_table system catalog table name
 * @param remove_fn optional remove function (may be NULL)
 */
int AK_generic_drop(AK_drop_arguments *drop_arguments, const char *sys_table, void (*remove_fn)(char*));

#endif

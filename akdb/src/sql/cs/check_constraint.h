/**
 * @file check_constraint.h Header file that provides functions and defines for check constraint
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

#ifndef CHECK_CONSTRAINT
#define CHECK_CONSTRAINT

#include "../../auxi/test.h"
#include "../../file/table.h"
#include "../../file/fileio.h"
#include "../../rel/expression_check.h"
#include "../../auxi/mempro.h"


/**
 * @author Mislav Jurinić
 * @brief Function that for a given value, checks if it satisfies the "check" constraint.
 * @param condition logical operator ['<', '>', '!=', ...]
 * @param type data type [int, float, varchar, datetime, ...]
 * @param value condition to be set
 * @param row_data data in table
 * @return 1 - result, 0 - failure 
 */
int condition_passed(char *condition, int type, void *value, void *row_data);

/**
 * @author Mislav Jurinić
 * @brief Function that adds a new "check" constraint into the system table.
 * @param table_name target table for "check" constraint evaluation
 * @param constraint_name new "check" constraint name that will be visible in the system table
 * @param attribute_name target attribute for "check" constraint evaluation
 * @param condition logical operator ['<', '>', '!=', ...]
 * @param type data type [int, float, varchar, datetime, ...]
 * @param value condition to be set
 * @return 1 - result, 0 - failure 
 */
int AK_set_check_constraint(char *table_name, char *constraint_name, char *attribute_name, char *condition, int type, void *value);

/**
 * @author Mislav Jurinić
 * @brief Function that verifies if the value we want to insert satisfies the "check" constraint.
 * @param table target table name
 * @param attribute target attribute name
 * @param value data we want to insert
 * @return 1 - result, 0 - failure 
 */
int AK_delete_check_constraint(char* tableName, char* constraintName);
TestResult AK_check_constraint_test();

#endif

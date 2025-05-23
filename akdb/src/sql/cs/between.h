/**
@file between.h Header file that provides functions and defines for between constaint
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

#ifndef BETWEEN
#define BETWEEN

#include "../../auxi/test.h"
#include "../../mm/memoman.h"
#include "../../file/id.h"
#include "../../auxi/mempro.h"

/**
 * @author Mislav Jurinić
 * @brief Function that returns system tables addresses by name
 * @param _systemTableName table name
 * @return int
 **/
int AK_find_table_address(char *_systemTableName);

/**
 * @author Saša Vukšić, updated by Mislav Jurinić
 * @brief Function that sets between constraints on particular attribute, string constraint should be writen in lowercase.      
 * @param tableName table name
 * @param constraintName name of constraint
 * @param attName name of attribute
 * @param startValue initial constraint
 * @param endValue final constraint
 * @return No return value
 **/
void AK_set_constraint_between(char* tableName, char* constraintName, char* attName, char* startValue, char* endValue);

/**
 * @author Saša Vukšić, updated by Mislav Jurinić
 * @brief Function that checks if the given value is between lower and upper bounds of the "between" constraint.
 * @param tableName table name
 * @param newValue value we want to insert
 * @param attNamePar attribute name
 * @return EXIT_SUCCESS or EXIT_ERROR
 **/
int AK_read_constraint_between(char* tableName, char* newValue, char* attNamePar);

/**
 * @author Maja Vračan, updated by Blaž Rajič, Vilim Trakoštanec
 * @brief Function for deleting specific between constraint
 * @param tableName name of system table (AK_CONSTRAINTS_BEWTEEN)
 * @param constraintName name of constraint to delete
 * @return EXIT_SUCCESS when constraint is deleted, else EXIT_ERROR
 */
int AK_delete_constraint_between(char* tableName, char* constraintName);

/**
  * @author Saša Vukšić, updated by Mislav Jurinić, updated by Blaž Rajič, updated by Vilim Trakoštanec
  * @brief Function that tests the functionality of implemented between constraint.
  * @return TestResult structure containing number of passed and failed tests
  */
TestResult AK_constraint_between_test();

#endif

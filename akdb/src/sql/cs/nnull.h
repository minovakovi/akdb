/**
 * @file nnull.h Header file that provides functions and defines for NOT NULL constraint management
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

#ifndef NNULL
#define NNULL

#include "../../auxi/test.h"
#include "../../file/table.h"
#include "../../file/fileio.h"
#include "../../auxi/mempro.h"
#include "constraint_names.h"

/**
 * @author Saša Vukšić, updated by Nenad Makar, updated by Vilim Trakoštanec
 * @brief Creates a new NOT NULL constraint on a table column
 * @details Verifies that no NULL values exist in the column and that the constraint name is unique
 * before creating the constraint. Performs necessary validation to ensure data integrity.
 * @param tableName name of table to add constraint to
 * @param attName name of attribute (column) to constrain
 * @param constraintName unique name for the new constraint
 * @return EXIT_SUCCESS if constraint is created successfully, EXIT_ERROR if:
 *         - Any parameter is NULL
 *         - Constraint already exists on the attribute
 *         - Column contains NULL values
 *         - Constraint name is not unique
 */
int AK_set_constraint_not_null(char* tableName, char* attName, char* constraintName);

/**
 * @author Saša Vukšić, updated by Nenad Makar, updated by Vilim Trakoštanec
 * @brief Validates if a NOT NULL constraint can be created
 * @details Checks two conditions:
 *          1. No NULL values exist in the specified column
 *          2. The constraint name is unique in the system
 * @param tableName name of table to check
 * @param attName name of attribute to check
 * @param constraintName name of constraint to validate
 * @return EXIT_SUCCESS if constraint can be created, EXIT_ERROR if:
 *         - Any parameter is NULL
 *         - Column contains NULL values
 *         - Constraint name already exists
 */
int AK_check_constraint_not_null(char* tableName, char* attName, char* constraintName);

/**
 * @author Saša Vukšić, updated by Nenad Makar, updated by Vilim Trakoštanec
 * @brief Checks if a NOT NULL constraint already exists on a column
 * @details Searches the constraint system table for an existing NOT NULL constraint
 * on the specified table and attribute combination.
 * @param tableName name of table to check
 * @param attName name of attribute to check
 * @param newValue optional parameter (can be NULL) for future constraint modifications
 * @return EXIT_SUCCESS if no constraint exists, EXIT_ERROR if:
 *         - tableName or attName is NULL
 *         - Constraint already exists on the specified column
 */
int AK_read_constraint_not_null(char* tableName, char* attName, char* newValue);

/**
 * @author Bruno Pilošta, updated by Vilim Trakoštanec
 * @brief Removes a NOT NULL constraint from a table
 * @details Searches for and removes the specified constraint from the system table.
 * Verifies the constraint exists before attempting deletion.
 * @param tableName name of table containing the constraint
 * @param constraintName name of constraint to delete
 * @return EXIT_SUCCESS if constraint is deleted successfully, EXIT_ERROR if:
 *         - Any parameter is NULL
 *         - Constraint does not exist
 *         - Deletion operation fails
 */
int AK_delete_constraint_not_null(char* tableName, char* constraintName);

/**
 * @author Saša Vukšić, updated by Nenad Makar, updated by Tea Jelavić, updated by Vilim Trakoštanec
 * @brief Tests the NOT NULL constraint functionality
 * @details Runs a comprehensive suite of tests for creating, checking, and deleting
 * NOT NULL constraints. Includes cleanup of test data.
 * @return TestResult structure containing the number of passed and failed tests
 */
TestResult AK_nnull_constraint_test();

#endif

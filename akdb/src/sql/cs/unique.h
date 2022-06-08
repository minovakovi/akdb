/**
@file unique.h Header file that provides functions and defines for unique constraint
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

#ifndef UNIQUE
#define UNIQUE

#include "../../auxi/test.h"
#include "../../file/table.h"
#include "../../file/fileio.h"
#include "../../auxi/mempro.h"
#include "../../auxi/dictionary.h"
#include "constraint_names.h"

/**
 * @author Domagoj Tuličić, updated by Nenad Makar 
 * @brief Function that sets unique constraint on attribute(s)
 * @param char* tableName name of table
 * @param char attName[] name(s) of attribute(s), if you want to set UNIQUE constraint on combination of attributes seperate their names with constant SEPARATOR (see test)
 * @param char constraintName[] name of constraint
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/
int AK_set_constraint_unique(char* tableName, char attName[], char constraintName[]);

/**
 * @author Domagoj Tuličić, updated by Nenad Makar 
 * @brief Function that checks if the insertion of some value(s) would violate the UNIQUE constraint
 * @param char* tableName name of table
 * @param char attName[] name(s) of attribute(s), if you want to check combination of values of more attributes seperate names of attributes with constant SEPARATOR (see test)
 * @param char newValue[] new value(s)
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/
int AK_read_constraint_unique(char* tableName, char attName[], char newValue[]);

/**
 * @author Maja Vračan, updated by Blaž Rajič
 * @brief Function for deleting specific unique constraint
 * @param tableName name of table on which constraint refers
 * @param constraintName name of constraint 
 * @return EXIT_SUCCESS when constraint is deleted, else EXIT_ERROR
 */
//int AK_delete_constraint_unique(char* tableName, char attName[], char constraintName[]);
int AK_delete_constraint_unique(char* tableName, char constraintName[]);
TestResult AK_unique_test();

#endif

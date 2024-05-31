/**
@file sequence.h Header file that provides functions and defines for sequences
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

#ifndef SEQUENCE
#define SEQUENCE

#include "../auxi/test.h"
#include "table.h"
#include "id.h"
#include "fileio.h"
#include "../auxi/mempro.h"

/**
 * @author Boris Kišić
 * @brief Function for adding sequence.
 * @param name name of the sequence
 * @param start_value start value of the sequence
 * @param increment increment of the sequence
 * @param max_value maximium value of the sequence
 * @param min_value minimum value of the sequence
 * @param cycle 0:non-cyclic sequence, 1:cyclic sequence
 * @return sequence_id or EXIT_ERROR
 */
int AK_sequence_add(char *name, int start_value, int increment, int max_value, int min_value, int cycle);

/**
 * @author Boris Kišić
 * @brief Function for removing sequence.
 * @param name name of the sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_remove(char *name);

/**
 * @author Boris Kišić
 * @brief Function that returns the current value of the sequence.
 * @param name name of the sequence
 * @return current_value or EXIT_ERROR
 */
int AK_sequence_current_value(char *name);

/**
 * @author Boris Kišić
 * @brief Function that returns the next value of the sequence and writes it in a system table as current value.
 * @param name name of the sequence
 * @return next_value or EXIT_ERROR
 */
int AK_sequence_next_value(char *name);

/**
/**
 * @author Boris Kišić
 * @brief Function that renames the sequence.
 * @param old_name Name of the sequence to be renamed
 * @param new_name New name of the sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_rename(char *old_name, char *new_name);

/**
 * @author Boris Kišić fixed by Ljubo Barać
 * @brief Function for modifying a sequence
 * @param name Name of the sequence
 * @param start_value start value of the sequence
 * @param increment increment of the sequence
 * @param max_value maximium value of the sequence
 * @param min_value minimum value of the sequence
 * @param cycle 0:non-cyclic sequence, 1:cyclic sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_modify(char *name, int start_value, int increment, int max_value, int min_value, int cycle);

/**
 * @author Ljubo Barać
 * @brief Function that fetches sequence id
 * @param name Name of the sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_get_id(char *name);

/**
 * @author Boris Kišić fixed by Ljubo Barać
 * @brief Function used for sequences testing.
 * @return No return value
 */
TestResult AK_sequence_test();

#endif

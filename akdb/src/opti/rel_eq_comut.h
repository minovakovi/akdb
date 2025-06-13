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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

#ifndef REL_EQ_COMUT_H
#define REL_EQ_COMUT_H

#include "../auxi/auxiliary.h"
#include "../auxi/test.h"

/**
 * @brief Applies commutativity transformation on relational algebra expressions.
 *        Transforms (A⋈B) into (B⋈A) if pattern is matched.
 * @param list_rel_eq Pointer to pointer to list of relational algebra expressions.
 * @return Pointer to a new list of transformed expressions.
 */
struct list_node* AK_rel_eq_comut(struct list_node** list_rel_eq);

/**
 * @brief Basic unit test: (A⋈B) should become (B⋈A).
 */
TestResult AK_rel_eq_comut_test();

struct list_node* AK_Prev_L2(struct list_node* node);


/**
 * @brief No-change test: malformed expression should remain unchanged.
 */
TestResult AK_rel_eq_comut_nochange_test();

/**
 * @brief Test for multiple valid expressions in the list.
 */
TestResult AK_rel_eq_comut_multiple_test();

/**
 * @brief Test for handling NULL input.
 */
TestResult AK_rel_eq_comut_null_input_test();

/**
 * @brief Runs all tests for commutativity transformations.
 */
TestResult AK_rel_eq_comut_all_tests();

#endif // REL_EQ_COMUT_H

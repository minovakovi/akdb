/**
 * @file rel_eq_projection.h Header file that provides data structures, functions and defines for relational equivalences regarding projection
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

#ifndef REL_EQ_PROJECTION_H
#define REL_EQ_PROJECTION_H

#include "../auxi/auxiliary.h"
#include "../auxi/test.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Applies projection merge optimization to relational algebra expressions.
 */
struct list_node* AK_rel_eq_projection(struct list_node** list_rel_eq);

/**
 * @brief Extracts attribute names from a projection expression.
 */
char* AK_rel_eq_get_atrributes_char(struct list_node** list_rel_eq);

/**
 * @brief Test merging nested projections.
 */
TestResult AK_rel_eq_projection_test();

/**
 * @brief Test single projection (should remain unchanged).
 */
TestResult AK_rel_eq_projection_single_test();

TestResult AK_rel_eq_projection_all_tests();


/**
 * @brief Test malformed input with unrelated expressions.
 */
TestResult AK_rel_eq_projection_malformed_test();

#ifdef __cplusplus
}
#endif

#endif // REL_EQ_PROJECTION_H
